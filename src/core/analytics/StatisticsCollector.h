#pragma once

#include <unordered_map>
#include <deque>
#include <vector>

struct QueueHistoryPoint {
    double timeSec = 0.0;
    int queueSize = 0;
};

class StatisticsCollector {
public:
  void reset();
  void update(double dt, double currentTimeSec, const std::vector <int>& waitingVehiclesId, int currentCarsInQueue);
  void registerPassedVehicles(int vehicleID);
  double getAverageWaitTimeSec() const;
  int getCurrentCarsInQueue() const;
  int getTotalCarsPassed() const;
  const std::deque<QueueHistoryPoint>& getQueueHistory() const;
private:
  std::unordered_map<int, double> m_vehicleWaitTimes;
  std::unordered_map<int, double> m_lastWaitUpdateTimes;
  std::unordered_map<int, bool> m_passedVehicleIds;
  double m_totalWaitTimeSec = 0.0;
  double m_totalWaitTimeOfPassedVehicles = 0.0;
  int m_totalCarsPassed = 0;
  int m_currentCarsInQueue = 0;
  double m_timeSinceLastSample = 0.0;
  std::deque<QueueHistoryPoint> m_queueHistory;
};
