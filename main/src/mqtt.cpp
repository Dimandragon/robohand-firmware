#include "mqtt.hpp"
#include "config.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "driver/gpio.h"
#include "notifications.pb.h"
#include "commands.pb.h"
#include "internal_api.hpp"

#include <string>

static const char *TAG = "MQTT";

/**
 * @brief MqttClient instance
 */
MqttClient *MqttClient::p_instance = 0;

/**
 * @brief Construct a new MqttClient
 */
MqttClient::MqttClient()
{
    esp_mqtt_client_config_t mqtt_cfg = {};
    memset(&mqtt_cfg, 0x00, sizeof(esp_mqtt_client_config_t));

    esp_err_t err;

    // Setup ACS connected led pin
    //gpio_reset_pin(FW_NETWORK_ACS_CONNECT_LED_PIN);
    //gpio_set_direction(FW_NETWORK_ACS_CONNECT_LED_PIN, GPIO_MODE_OUTPUT);
    //gpio_set_level(FW_NETWORK_ACS_CONNECT_LED_PIN, 0);

    // Get params
    std::string uri = CONFIG_MQTT_BROKER_ADDRESS;
    std::string username = CONFIG_MQTT_BROKER_USER_NAME;
    std::string password = CONFIG_MQTT_BROKER_PASSWORD;

    // Set MQTT params
    mqtt_cfg.broker.address.uri = uri.c_str();
    mqtt_cfg.credentials.username = username.c_str();
    mqtt_cfg.credentials.client_id = CONFIG_UUID;
    mqtt_cfg.credentials.authentication.password = password.c_str();
    mqtt_cfg.session.keepalive = CONFIG_MQTT_KEEP_ALIVE_TIME;
    mqtt_cfg.buffer.size = 20 * 1024;

    this->mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    err = esp_mqtt_client_start(this->mqtt_client);
    ESP_LOGD(TAG, "esp_mqtt_client_start: %d", err);
}

/**
 * @brief Destroy the MqttClient
 */
MqttClient::~MqttClient()
{
    esp_mqtt_client_unregister_event(this->mqtt_client, MQTT_EVENT_ANY, MqttClient::eventHandler);
    esp_mqtt_client_disconnect(this->mqtt_client);
    esp_mqtt_client_destroy(this->mqtt_client);

    /*if (ConfigDatabase::getInstance().getMqttSslEnable())
    {
        free(this->server_cert);
        free(this->client_cert);
        free(this->key);
    }*/
}


/**
 * @brief MQTT events handler
 *
 * @param handler_args Handler arguments
 * @param base
 * @param event_id Event id
 * @param event_data Event data
 */
void MqttClient::eventHandler(void *handler_args, esp_event_base_t base,
                              int32_t event_id, void *event_data)
{
    esp_err_t err;
    esp_mqtt_event_handle_t mqtt_event;
    mqtt_event = static_cast<esp_mqtt_event_handle_t>(event_data);

    switch (mqtt_event->event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        // Turn on ACS led
        // Send init message
        MqttClient::getInstance().sendInitMessage();
        // Send not sended events
        // Subscribe MQTT topics
        MqttClient::getInstance().subscribeTopics();
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        vTaskDelay(pdMS_TO_TICKS(CONFIG_WIFI_RECONNECT_TIMEOUT));
        err = esp_mqtt_client_reconnect(MqttClient::getInstance().getClient());
        ESP_LOGE(TAG, "mqtt reconnect returned error %d", err);
        [[fallthrough]];
    case MQTT_EVENT_SUBSCRIBED:
        [[fallthrough]];
    case MQTT_EVENT_UNSUBSCRIBED:
        [[fallthrough]];
    case MQTT_EVENT_PUBLISHED:
        break;
    case MQTT_EVENT_DATA:
        MqttClient::getInstance().dataHandler(mqtt_event->topic, mqtt_event->topic_len,
                                              mqtt_event->data, mqtt_event->data_len);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "in mqtt error heap size: %ld", esp_get_free_heap_size());
        ESP_LOGE(TAG, "MQTT_EVENT_ERROR");
        break;
    default:
        break;
    }
}

/**
 * @brief MQTT_EVENT_DATA handler
 *
 * @param topic MQTT topic
 * @param topic_len MQTT topic length
 * @param data Data from MQTT
 * @param data_len Length data
 */
void MqttClient::dataHandler(char *topic, int topic_len, char *data, int data_len)
{
    int err;

    if (topic_len <= 0)
    {
        ESP_LOGE(TAG, "Topic with zero length");
        return;
    }

    ESP_LOGD(TAG, "TOPIC=%.*s", topic_len, topic);

    std::string mqtt_topic(topic, topic_len);

    std::string message_str(data, data_len);

    // Change door mode
    if (mqtt_topic == MQTT_TOPIC_COMMANDS_SERVO_GO_TO_ANGLE){
        Commands::ServoGoToAngle message;
        message.ParseFromString(message_str);
        std::cout << message.DebugString() << std::endl;
        CommandsQueue::push(message);
    }
    else if (mqtt_topic == MQTT_TOPIC_COMMANDS_SERVO_LOCK){
        Commands::ServoLock message;
        message.ParseFromString(message_str);
        std::cout << message.DebugString() << std::endl;
        CommandsQueue::push(message);
    }
    else if (mqtt_topic == MQTT_TOPIC_COMMANDS_SERVO_UNLOCK){
        Commands::ServoUnLock message;
        message.ParseFromString(message_str);
        std::cout << message.DebugString() << std::endl;
        CommandsQueue::push(message);
    }
    else if (mqtt_topic == MQTT_TOPIC_COMMANDS_SERVO_SMOOTHLY_MOVE){
        Commands::ServoSmoothlyMove message;
        message.ParseFromString(message_str);
        std::cout << message.DebugString() << std::endl;
        CommandsQueue::push(message);
    }
    else if (mqtt_topic == MQTT_TOPIC_COMMANDS_MOVE_TARGET_PRESSURE){
        Commands::MoveToTargetPressure message;
        message.ParseFromString(message_str);
        std::cout << message.DebugString() << std::endl;
        CommandsQueue::push(message);
    }
    else if (mqtt_topic == MQTT_TOPIC_COMMANDS_HOLD_GESTURE){
        Commands::HoldGesture message;
        message.ParseFromString(message_str);
        std::cout << message.DebugString() << std::endl;
        CommandsQueue::push(message);
    }
}

/**
 * @brief Get MQTT client
 *
 * @return esp_mqtt_client_handle_t
 */
esp_mqtt_client_handle_t MqttClient::getClient()
{
    return mqtt_client;
}

/**
 * @brief Send data to MQTT non blocking
 *
 * @param topic MQTT topic
 * @param data Data for send
 * @param len Lenght of data
 * @param qos QOS
 * @param retain Retain flag
 * @param store Store flag
 * @return int Message ID if queued successfully, -1 otherwise
 */
int MqttClient::sendEnqueue(const char *topic, const char *data,
                            int len, int qos, int retain, bool store)
{
    return esp_mqtt_client_enqueue(this->mqtt_client, topic, data, len, qos, retain, store);
}

/**
 * @brief Send data to MQTT
 *
 * @return int Message ID of the publish message (for QoS 0 Message ID will always be zero) on success. -1 on failure.
 */
int MqttClient::send(const char *topic, const char *data, int len, int qos, int retain)
{
    return esp_mqtt_client_publish(this->mqtt_client, topic, data, len, qos, retain);
}

/**
 * @brief Send init message
 */
void MqttClient::sendInitMessage()
{
    Notifications::Notification notification;
    notification.set_notification(Notifications::NotificationType::connected);
    std::string message;
    notification.SerializeToString(&message);
    
    this->send(MQTT_TOPIC_NOTIFICATIONS, message.data(), message.size(), CONFIG_MQTT_QOS_LEVEL, 0);
}

/**
 * @brief Subscribe MQTT topics
 */
void MqttClient::subscribeTopics()
{
    int msg_id;

    std::string topics[] = {
        std::string(MQTT_TOPIC_COMMANDS),
        std::string(MQTT_TOPIC_COMMANDS_SERVO_GO_TO_ANGLE),
        std::string(MQTT_TOPIC_COMMANDS_SERVO_LOCK),
        std::string(MQTT_TOPIC_COMMANDS_SERVO_UNLOCK),
        std::string(MQTT_TOPIC_COMMANDS_SERVO_SMOOTHLY_MOVE),
        std::string(MQTT_TOPIC_COMMANDS_MOVE_TARGET_PRESSURE),
        std::string(MQTT_TOPIC_COMMANDS_HOLD_GESTURE),
    };
    //todo #0

    // Subscribe all topics
    for (std::string topic : topics)
    {
        msg_id = esp_mqtt_client_subscribe(this->mqtt_client, topic.c_str(), CONFIG_MQTT_QOS_LEVEL);
        ESP_LOGD(TAG, "subscribe successful %s, msg_id=%d", topic.c_str(), msg_id);
    }
}



/**
 * @brief Convert event type to MQTT topic
 * @see Notifications__EventType
 *
 * @param type Event Type
 * @return char* Topic
 */


/**
 * @brief Get MqttClient instance
 *
 * @return MqttClient& MqttClient instance
 */
MqttClient &MqttClient::getInstance()
{
    if (!p_instance)
    {
        ESP_LOGD(TAG, "MqttClient::getInstance p_instance is null");
    }

    return *p_instance;
}

/**
 * @brief Init MqttClient singleton
 */
void MqttClient::init()
{
    esp_err_t err;

    if (!p_instance)
    {
        p_instance = new MqttClient();
        err = esp_mqtt_client_register_event(MqttClient::getInstance().getClient(), MQTT_EVENT_ANY,
                                             MqttClient::eventHandler, static_cast<void *>(p_instance));
        ESP_LOGD(TAG, "esp_mqtt_client_register_event: %d", err);
    }
}
