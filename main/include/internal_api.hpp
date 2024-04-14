#pragma once

#include "small_mutex.hpp"
#include <queue>
#include "commands.pb.h"
#include "imu.pb.h"
#include "potentiometer.pb.h"
#include "servo.pb.h"
#include "shared.pb.h"
#include "straingauge.pb.h"
#include "variant"

/*
---------------------------------------------------
little guideline for using protobuf by your "killed" teamlead :)
---------------------------------------------------

for creating any protobuf's struct see it's definition in .proto file

for example struct Potentiometer defined by

message Potentiometer{
    Shared.Finger finger = 1; //Какой палец
    Position positoin = 2; //Позиция потенцометра
    uint32 angle = 3; //Угол, измеренный потенциометром
    string connectionPin = 4; // Пин (порт на плате) к которому будет крепиться датчик потенциометра
}

enum Position{
    p0 = 0; //первый сустав
    //(между кистью и первой фалангой), в случае с большим - привение
    p1 = 1; //второй сустав
    //(между первой и второй фалангами), в случае с большим - между кистью и первой фалангой
    p2 = 2; //третий сустав
    //перед последней
} 

in potentiometer.proto

can be created in c++ by
Potentiometer::Potentiometer potentiometer;
potentiometer.set_finger(Shared::Finger::f1);
potentiometer.set_positoin(Potentiometer::Position::p0);
potentiometer.set_angle(45);
potentiometer.set_connectionpin("aaaa otpayalsya, provod v vozduhe");

and can be unparsed by 

Shared::Finger finger = potentiometer.finger();
Potentiometer::Position position = potentiometer.positoin();
int angle = potentiometer.angle();
std::string connectionpin = potentiometer.connectionpin();
*/



class HandState{
    static SmallMutex mutex;
    static std::vector<Imu::IMU> imus;
    static std::vector<Imu::ResultIMU> processed_imus;
    static std::vector<Potentiometer::Potentiometer> potentiometers;
    static std::vector<Straingauge::StrainGuage> straingauges;
    static std::vector<Servo::Servo> servos;

public: 
    template<typename T>
    static T & getState(int idx){
        if constexpr(std::is_same_v<T, Imu::IMU>){
            return imus[idx];
        }
        else if constexpr(std::is_same_v<T, Imu::ResultIMU>){
            return processed_imus[idx];
        }
        else if constexpr(std::is_same_v<T, Potentiometer::Potentiometer>){
            return potentiometers[idx];
        }
        else if constexpr(std::is_same_v<T, Straingauge::StrainGuage>){
            return straingauges[idx];
        }
        else if constexpr(std::is_same_v<T, Servo::Servo>){
            return servos[idx];
        }
    }

    template<typename T>
    static int getStateExemplarsCount(){
        if constexpr(std::is_same_v<T, Imu::IMU>){
            return imus.size();
        }
        else if constexpr(std::is_same_v<T, Imu::ResultIMU>){
            return processed_imus.size();
        }
        else if constexpr(std::is_same_v<T, Potentiometer::Potentiometer>){
            return potentiometers.size();
        }
        else if constexpr(std::is_same_v<T, Straingauge::StrainGuage>){
            return straingauges.size();
        }
        else if constexpr(std::is_same_v<T, Servo::Servo>){
            return servos.size();
        }
    }

    static void lock(){
        mutex.lock();
    }
    static void unlock(){
        mutex.unlock();
    }
    static void init(int imus_count, int processed_imus_count, 
        int potentiometers_count, int straingauges_count, int servos_count){
        lock();
        imus.clear();
        processed_imus.clear();
        potentiometers.clear();
        straingauges.clear();
        servos.clear();
        for (int i = 0; i < imus_count; i++){
            Imu::IMU temp;
            imus.push_back(temp);
        }
        for (int i = 0; i < processed_imus_count; i++){
            Imu::ResultIMU temp;
            processed_imus.push_back(temp);
        }
        for (int i = 0; i < potentiometers_count; i++){
            Potentiometer::Potentiometer temp;
            potentiometers.push_back(temp);
        }
        for (int i = 0; i < straingauges_count; i++){
            Straingauge::StrainGuage temp;
            straingauges.push_back(temp);
        }
        for (int i = 0; i < servos_count; i++){
            Servo::Servo temp;
            servos.push_back(temp);
        }
        //todo
        unlock();
    }
};


//use functrions from it namespace like api
namespace CommandsQueue{
    //using namespace Commands;

    using CommandType = std::variant
        <Commands::ServoGoToAngle, Commands::ServoLock, Commands::ServoUnLock, 
        Commands::ServoSmoothlyMove, Commands::MoveToTargetPressure, Commands::HoldGesture>;

    //its a helpers
    template <typename T>
    bool commandIs(CommandType command){
        return std::holds_alternative<T>(command);
    }

    //use it only after if (commandIs<T>(command) == true)
    //as alternative you can use std::get<T>(command) like that
    /*
    try 
    {
        ServoGoToAngle f = std::get<ServoGoToAngle>(command); 
        //do some work
    }
    catch (std::bad_variant_access&) 
    {
        std::cout << "our variant doesn't hold ServoGoToAngle at this moment...\n";
    }*/
    template <typename T> 
    auto getIf(CommandType command){
        return *(std::get_if<T>(command));
    }
    

    //its a magic
    struct Queue{
    private:
        static std::queue<CommandType> queue;
    public: 
        static auto size(){
            return queue.size();
        }

        static void push(CommandType command){
            queue.push(command);
        }

        template<typename T>
        static void push(T command){
            CommandType command_ = command;
            queue.push(command_);
        }

        static CommandType pop(){
            auto val = queue.front();
            queue.pop();
            return val;
        }
    };

    //its an api
    static auto size(){
        return Queue::size();
    }

    static void push(CommandType command){
        return Queue::push(command);
    }

    template<typename T>
    static void push(T command){
        CommandType command_ = command;
        return Queue::push(command_);
    }

    static CommandType pop(){
        return Queue::pop();
    }
}