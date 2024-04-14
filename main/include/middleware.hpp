#pragma once

#include "internal_api.hpp"
#include "mqtt.hpp"
#include "sdkconfig.h"
#include "config.hpp"


class MiddleWare{
    static void sendingStateTask (void *pvParameters){
        for(;;){
            for(int i = 0; i < 
                HandState::getStateExemplarsCount<Imu::IMU>(); 
                    i++)
            {
                std::string message;
                HandState::getState<Imu::IMU>(i).SerializeToString(&message);
                MqttClient::getInstance().sendEnqueue(
                    MQTT_TOPIC_MONITORING_IMU_RAW_DATA, 
                    message.data(), 
                    message.size(),
                    CONFIG_MQTT_QOS_LEVEL,
                    0,
                    1
                );
            }
            for(int i = 0; i < 
                HandState::getStateExemplarsCount<Imu::ResultIMU>(); 
                    i++)
            {
                std::string message;
                HandState::getState<Imu::ResultIMU>(i).SerializeToString(&message);
                MqttClient::getInstance().sendEnqueue(
                    MQTT_TOPIC_MONITORING_IMU_RAW_DATA, 
                    message.data(), 
                    message.size(),
                    CONFIG_MQTT_QOS_LEVEL,
                    0,
                    1
                );
            }
            for(int i = 0; i < 
                HandState::getStateExemplarsCount<Potentiometer::Potentiometer>(); 
                    i++)
            {
                std::string message;
                HandState::getState<Potentiometer::Potentiometer>(i).SerializeToString(&message);
                MqttClient::getInstance().sendEnqueue(
                    MQTT_TOPIC_MONITORING_IMU_RAW_DATA, 
                    message.data(), 
                    message.size(),
                    CONFIG_MQTT_QOS_LEVEL,
                    0,
                    1
                );
            }
            for(int i = 0; i < 
                HandState::getStateExemplarsCount<Straingauge::StrainGuage>(); 
                    i++)
            {
                std::string message;
                HandState::getState<Straingauge::StrainGuage>(i).SerializeToString(&message);
                MqttClient::getInstance().sendEnqueue(
                    MQTT_TOPIC_MONITORING_IMU_RAW_DATA, 
                    message.data(), 
                    message.size(),
                    CONFIG_MQTT_QOS_LEVEL,
                    0,
                    1
                );
            }
            for(int i = 0; i < 
                HandState::getStateExemplarsCount<Servo::Servo>(); 
                    i++)
            {
                std::string message;
                HandState::getState<Servo::Servo>(i).SerializeToString(&message);
                MqttClient::getInstance().sendEnqueue(
                    MQTT_TOPIC_MONITORING_IMU_RAW_DATA, 
                    message.data(), 
                    message.size(),
                    CONFIG_MQTT_QOS_LEVEL,
                    0,
                    1
                );
            }
            
            vTaskDelay(pdMS_TO_TICKS
                (CONFIG_MIDDLEWARE_SENDING_STATE_PERIOD));
        }
    }
public:
    static void init(){
        xTaskCreate(
            sendingStateTask,
            "MiddlewareTask",
            4096,
            nullptr,
            5,
            nullptr);
    }
};

