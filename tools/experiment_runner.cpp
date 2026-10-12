#include "core/simulation/SimulationEngine.h"
#include "common/SimulationConfig.h"
#include "common/Types.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>

struct Scenario {
    const char* name;
    ControllerMode mode;
    int flowNS;            // Север + Юг, авт/ч
    int flowEW;            // Восток + Запад, авт/ч
    bool permitLeft;       // true - ВСТР, false - выделенная стрелочная фаза
    bool parallelPeds;     // true - пешеходы параллельно с авто, false - общепешеходная фаза All-Red
    int pedFlow;           // суммарный поток пешеходов, пеш/ч
    double fixedCycleSec;  // > 0: подобрать Z так, чтобы цикл был ровно таким; 0 - оставить Z по умолчанию
};

// StaticController игнорирует totalCycleSec, поэтому подбираем Z вручную.
// Постоянные такты: жёлтый 3 + красно-жёлтый 1.5 (по два раза за цикл),
// стрелочные фазы по 5 с (если ВСТР выключен), общепешеходная фаза (если пешеходы не параллельно).
static void applyFixedCycle(SimulationConfig& c, double cycleSec) {
    double overhead = 2.0 * (3.0 + 1.5);
    if (!c.permitLeftTurnFilter) overhead += 2.0 * 5.0;
    if (!c.hasRightTurnArrow)    overhead += c.pedestrianGreenSec;

    const int totalGreen = static_cast<int>(std::lround(cycleSec - overhead));
    c.north.greenZ = totalGreen / 2;
    c.south.greenZ = c.north.greenZ;
    c.east.greenZ  = totalGreen - c.north.greenZ;
    c.west.greenZ  = c.east.greenZ;
}

static SimulationConfig makeConfig(const Scenario& s) {
    SimulationConfig c;
    c.mode = s.mode;
    c.north.flowP = s.flowNS;  c.south.flowP = s.flowNS;
    c.east.flowP  = s.flowEW;  c.west.flowP  = s.flowEW;
    c.permitLeftTurnFilter = s.permitLeft;
    c.hasRightTurnArrow = s.parallelPeds;   // в движке этот флаг = "пешеходы параллельно с авто"
    c.pedestrianFlow = s.pedFlow;
    if (s.fixedCycleSec > 0.0) applyFixedCycle(c, s.fixedCycleSec);
    return c;
}

struct RunMetrics {
    double avgQueue{0.0};
    int maxQueue{0};
    double waitEnd{0.0};
    int passedEnd{0};
    double throughput{0.0};   // авт/ч на измерительном интервале
};

static RunMetrics runOnce(const Scenario& s, FILE* csv, unsigned int seed) {
    const double dt = 0.05;
    const double warmupSec = 60.0;
    const double horizonSec = 600.0;

    SimulationEngine engine;
    // Если в SimulationEngine предусмотрена установка сида (например, через генератор), 
    // можно задействовать, либо инициализировать рандом внутри движка.
    // engine.setRandomSeed(seed); 
    
    SimulationConfig cfg = makeConfig(s);
    cfg.timeStepDt = dt;
    engine.updateConfig(cfg);
    engine.start();

    RunMetrics m;
    double queueSum = 0.0;
    long samples = 0;
    int passedFirst = -1;
    double cumWaitFirst = 0.0, cumWaitEnd = 0.0;

    const int steps = static_cast<int>(std::lround((warmupSec + horizonSec) / dt));
    for (int i = 0; i < steps; ++i) {
        engine.step(dt);
        const double currentTime = (i + 1) * dt;
        
        if (currentTime >= warmupSec && (i % 2 == 0)) {
            const auto st = engine.getSnapshot().stats;
            if (csv) {
                std::fprintf(csv, "%.2f,%d,%.2f,%d\n",
                             st.currentSimTimeSec, st.currentCarsInQueue,
                             st.averageWaitTimeSec, st.totalCarsPassed);
            }
            const double cumWait = st.averageWaitTimeSec * st.totalCarsPassed;
            if (passedFirst < 0) { 
                passedFirst = st.totalCarsPassed; 
                cumWaitFirst = cumWait; 
            }
            cumWaitEnd = cumWait;
            queueSum += st.currentCarsInQueue;
            ++samples;
            m.maxQueue = std::max(m.maxQueue, st.currentCarsInQueue);
            m.passedEnd = st.totalCarsPassed;
        }
    }
    const int passedInWindow = m.passedEnd - std::max(passedFirst, 0);
    m.avgQueue = samples > 0 ? queueSum / samples : 0.0;
    m.waitEnd = passedInWindow > 0 ? (cumWaitEnd - cumWaitFirst) / passedInWindow : 0.0;
    m.throughput = passedInWindow * 3600.0 / horizonSec;
    return m;
}

static double mean(const std::vector<double>& v) {
    double s = 0.0;
    for (double x : v) s += x;
    return v.empty() ? 0.0 : s / v.size();
}

static double stddev(const std::vector<double>& v) {
    if (v.size() < 2) return 0.0;
    const double m = mean(v);
    double s = 0.0;
    for (double x : v) s += (x - m) * (x - m);
    return std::sqrt(s / (v.size() - 1));
}

int main() {
    std::printf("experiment_runner v2 (5 runs, fixed cycle 90 s)\n");
    const int kRuns = 5;   // независимых прогонов на сценарий

    const std::vector<Scenario> scenarios = {
        //  имя                      режим                   NS    EW   ВСТР   пеш.парал  пеш/ч  цикл
        {"exp1_static_600",       ControllerMode::Static,   600,  600, true,  false,     300,   0.0},
        {"exp1_dynamic_600",      ControllerMode::Dynamic,  600,  600, true,  false,     300,   0.0},
        {"exp2_static_peak",      ControllerMode::Static,   1200, 300, true,  false,     300,   0.0},
        {"exp2_dynamic_peak",     ControllerMode::Dynamic,  1200, 300, true,  false,     300,   0.0},

        // Эксперимент 3: ВСТР против стрелочной фазы, цикл одинаковый (90 с)
        {"exp3_left_filter",      ControllerMode::Static,   600,  600, true,  false,     300,   90.0},
        {"exp3_left_arrow_phase", ControllerMode::Static,   600,  600, false, false,     300,   90.0},

        // Эксперимент 4: All-Red против параллельного пропуска пешеходов, цикл 90 с
        {"exp4_peds_allred",      ControllerMode::Static,   600,  600, false, false,     800,   90.0},
        {"exp4_peds_parallel",    ControllerMode::Static,   600,  600, false, true,      800,   90.0},
    };

    FILE* summary = std::fopen("summary.csv", "w");
    if (!summary) return 1;
    std::fprintf(summary,
        "Scenario,Runs,AvgQueue,AvgQueueSd,MaxQueue,WaitEndSec,WaitEndSd,Passed,ThroughputPerHour,ThroughputSd\n");

    for (const auto& s : scenarios) {
        std::vector<double> avgQ, maxQ, wait, passed, thr;

        for (int run = 0; run < kRuns; ++run) {
            FILE* csv = nullptr;
            if (run == 0) {   // временной ряд для графика — только с первого прогона
                char fname[128];
                std::snprintf(fname, sizeof(fname), "%s.csv", s.name);
                csv = std::fopen(fname, "w");
                if (csv) std::fprintf(csv, "SimTimeSec,CarsInQueue,AverageWaitTimeSec,TotalCarsPassed\n");
            }

            const RunMetrics m = runOnce(s, csv, 1000u + run);
            if (csv) std::fclose(csv);

            avgQ.push_back(m.avgQueue);
            maxQ.push_back(m.maxQueue);
            wait.push_back(m.waitEnd);
            passed.push_back(m.passedEnd);
            thr.push_back(m.throughput);
        }

        std::fprintf(summary, "%s,%d,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.0f,%.0f\n",
                     s.name, kRuns,
                     mean(avgQ), stddev(avgQ), mean(maxQ),
                     mean(wait), stddev(wait), mean(passed),
                     mean(thr), stddev(thr));
        std::printf("%-24s queue %.1f  wait %.1f s  thr %.0f veh/h\n",
                    s.name, mean(avgQ), mean(wait), mean(thr));
    }

    std::fclose(summary);
    return 0;
}