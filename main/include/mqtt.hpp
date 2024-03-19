#pragma once

#include "mqtt_client.h"

#include "mqtt_client.h"
#include "esp_netif.h"

class MqttClient
{
private:
    static MqttClient *p_instance;
    esp_mqtt_client_handle_t mqtt_client;

    static void eventHandler(void *handler_args, esp_event_base_t base,
                             int32_t event_id, void *event_data);
    void dataHandler(char *topic, int topic_len, char *data, int data_len);

    char *server_cert;
    char *client_cert;
    char *key;

    MqttClient();
    ~MqttClient();
    MqttClient(const MqttClient &) {}
    MqttClient &operator=(MqttClient &) = default;

public:
    static MqttClient &getInstance();
    static void init();

    esp_mqtt_client_handle_t getClient();
    void sendInitMessage();
    void subscribeTopics();
    int sendEnqueue(const char *topic, const char *data, int len, int qos, int retain, bool store);
    int send(const char *topic, const char *data, int len, int qos, int retain);

    //char *eventTypeToTopic(Notifications__EventType type);
};