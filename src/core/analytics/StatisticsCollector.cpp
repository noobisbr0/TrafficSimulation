#include "StatisticsCollector.h"

#include <algorithm>

void StatisticsCollector::reset() {
  m_vehicleWaitTimes.clear();
  m_lastWaitUpdateTimes.clear();
  m_passedVehicleIds.clear();
  m_totalWaitTimeSec = 0.0;
  m_totalCarsPassed = 0;
  m_currentCarsInQueue = 0;
  m_timeSinceLastSample = 0.0;
  m_queueHistory.clear();
}

void StatisticsCollector::registerPassedVehicles(int vehicleID) {
  if (m_passedVehicleIds.find(vehicleID) != m_passedVehicleIds.end()) return;
  m_passedVehicleIds[vehicleID] = true;
  m_totalCarsPassed++;
  auto it = m_vehicleWaitTimes.find(vehicleID);
  if (it != m_vehicleWaitTimes.end()) {
    m_totalWaitTimeOfPassedVehicles += it->second;
  }
}

double StatisticsCollector::getAverageWaitTimeSec() const {
  if (m_totalCarsPassed == 0) return 0.0;
  return m_totalWaitTimeOfPassedVehicles / m_totalCarsPassed;
}

int StatisticsCollector::getCurrentCarsInQueue() const {
  return m_currentCarsInQueue;
}

int StatisticsCollector::getTotalCarsPassed() const {
  return m_totalCarsPassed;
}

const std::deque<QueueHistoryPoint>&
StatisticsCollector::getQueueHistory() const {
  return m_queueHistory;
}

void StatisticsCollector::update(double dt, double currentTimeSec, const std::vector <int>& waitingVehiclesId, int currentCarsInQueue) {
if (dt < 0.0) return;
m_currentCarsInQueue = currentCarsInQueue;
for (int vehicleID : waitingVehiclesId) {
  if (m_passedVehicleIds.find(vehicleID) != m_passedVehicleIds.end()) continue;
  m_vehicleWaitTimes[vehicleID] += dt;
  m_totalWaitTimeSec += dt;
}
m_timeSinceLastSample += dt;
if (m_timeSinceLastSample >= 1.0) {
  m_queueHistory.push_back({currentTimeSec, m_currentCarsInQueue});
  m_timeSinceLastSample = 0.0;
}
while (!m_queueHistory.empty() && currentTimeSec - m_queueHistory.front().timeSec > 60.0) {
  m_queueHistory.pop_front();
} 
}
