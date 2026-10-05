#include "Vehicle.h"

Vehicle::Vehicle(int id, const Vector2D& position, double speed, TurnDirection turnDirection)
  : m_id(id),
    m_position(position),
    m_speed(speed),
    m_desiredSpeedKmh(speed),
    m_acceleration(0.0),
    m_angleDeg(0.0),
    m_turnDirection(turnDirection),
    m_approachDirection(DirectionId::North),
    m_targetDirection(DirectionId::North),
    m_laneId(0),
    m_isTurning(false),
    m_targetAngleDeg(0.0),
    m_turnProgress(0.0),
    m_turnCompleted(false),
    m_turnCenter(0.0, 0.0),
    m_turnStartAngle(0.0),
    m_isBraking(false),
    m_isWaitingInQueue(false) {
}

int Vehicle::getId() const {
  return m_id;
}

Vector2D Vehicle::getPosition() const {
  return m_position;
}

double Vehicle::getSpeed() const {
  return m_speed;
}

double Vehicle::getDesiredSpeed() const {
  return m_desiredSpeedKmh;
}

double Vehicle::getAcceleration() const {
  return m_acceleration;
}

double Vehicle::getAngleDeg() const {
  return m_angleDeg;
}

TurnDirection Vehicle::getTurnDirection() const {
  return m_turnDirection;
}

DirectionId Vehicle::getApproachDirection() const {
    return m_approachDirection;
}

DirectionId Vehicle::getTargetDirection() const {
  return m_targetDirection;
}

bool Vehicle::isTurning() const {
  return m_isTurning;
}

double Vehicle::getTargetAngleDeg() const {
  return m_targetAngleDeg;
}

bool Vehicle::isBraking() const {
  return m_isBraking;
}

bool Vehicle::isWaitingInQueue() const {
  return m_isWaitingInQueue;
}

void Vehicle::setPosition(const Vector2D& position) {
  m_position = position;
}

void Vehicle::setSpeed(double speed) {
  m_speed = speed;
}

void Vehicle::setDesiredSpeed(double speed) {
  m_desiredSpeedKmh = speed;
}

void Vehicle::setAcceleration(double acceleration) {
  m_acceleration = acceleration;
}

void Vehicle::setAngleDeg(double angleDeg) {
  m_angleDeg = angleDeg;
}

void Vehicle::setApproachDirection(DirectionId direction) {
    m_approachDirection = direction;
}

void Vehicle::setTargetDirection(DirectionId direction) {
  m_targetDirection = direction;
}

void Vehicle::setLaneId(int laneId) {
  m_laneId = laneId;
}

int Vehicle::getLaneId() const {
  return m_laneId;
}

void Vehicle::setTurning(bool turning) {
  m_isTurning = turning;
}

void Vehicle::setTargetAngleDeg(double angleDeg) {
  m_targetAngleDeg = angleDeg;
}

void Vehicle::setTurnProgress(double progress) {
  m_turnProgress = progress;
}

double Vehicle::getTurnProgress() const {
  return m_turnProgress;
}

void Vehicle::setTurnCompleted(bool completed) {
    m_turnCompleted = completed;
}

bool Vehicle::isTurnCompleted() const {
    return m_turnCompleted;
}

void Vehicle::setTurnCenter(const Vector2D& center) {
  m_turnCenter = center;
}

Vector2D Vehicle::getTurnCenter() const {
  return m_turnCenter;
}

void Vehicle::setTurnStartAngle(double angle) {
  m_turnStartAngle = angle;
}

double Vehicle::getTurnStartAngle() const {
  return m_turnStartAngle;
}

void Vehicle::setBraking(bool braking) {
  m_isBraking = braking;
}

void Vehicle::setWaitingInQueue(bool waiting) {
  m_isWaitingInQueue = waiting;
}