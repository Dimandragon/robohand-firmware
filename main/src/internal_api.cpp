#include "internal_api.hpp"

SmallMutex HandState::mutex = SmallMutex();

std::vector<Imu::IMU> HandState::imus = {};
std::vector<Imu::ResultIMU> HandState::processed_imus = {};
std::vector<Potentiometer::Potentiometer> HandState::potentiometers = {};
std::vector<Straingauge::StrainGuage> HandState::straingauges = {};
std::vector<Servo::Servo> HandState::servos = {};

std::queue<CommandsQueue::CommandType> CommandsQueue::Queue::queue = {};