#include "mqtt.hpp"

#include "netif.hpp"


#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "driver/gpio.h"

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

    // If SSL enabled
    /*if (CONFIG_SSL_ENABLED)
    {
        // Copy server cert to class var
        std::string server_cert = ConfigDatabase::getInstance().getMqttServerSslCert();
        this->server_cert = (char *)malloc(server_cert.size() + 1);
        strlcpy(this->server_cert, server_cert.c_str(), server_cert.size());
        this->server_cert[server_cert.size()] = '\0';

        // Copy client cert to class var
        std::string client_cert = ConfigDatabase::getInstance().getMqttClientSslCert();
        this->client_cert = (char *)malloc(client_cert.size() + 1);
        strlcpy(this->client_cert, client_cert.c_str(), client_cert.size());
        this->client_cert[client_cert.size()] = '\0';

        // Copy client key to class var
        std::string key = ConfigDatabase::getInstance().getMqttClientSslPrivateKey();
        this->key = (char *)malloc(key.size() + 1);
        strlcpy(this->key, key.c_str(), key.size());
        this->key[key.size()] = '\0';

        mqtt_cfg.broker.verification.skip_cert_common_name_check = true;

        // Set certificate
        mqtt_cfg.broker.verification.certificate = static_cast<const char *>(this->server_cert);
        mqtt_cfg.broker.verification.certificate_len = server_cert.length() + 1;

        // Set client certificate
        mqtt_cfg.credentials.authentication.certificate = static_cast<const char *>(this->client_cert);
        mqtt_cfg.credentials.authentication.certificate_len = client_cert.length() + 1;

        // Set client key
        mqtt_cfg.credentials.authentication.key = static_cast<const char *>(this->key);
        mqtt_cfg.credentials.authentication.key_len = key.length() + 1;
    }*/

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
        //xTaskCreate(EventDatabase::sendNotSendedEvents, "sendnotsended", 5 * 1024,
        //            static_cast<void *>(&(EventDatabase::getInstance())), 7, NULL);
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

    // Change door mode
    /*if (mqtt_topic == FW_MQTT_TOPIC_CONTROL_DOOR_MODE)
    {
        ESP_LOGD(TAG, FW_MQTT_TOPIC_CONTROL_DOOR_MODE);

        if (!Door::isNotInited())
            Door::getInstance().actionChangeMode(data, data_len);
    }
    // Users sync
    else if (mqtt_topic == FW_MQTT_TOPIC_BACKEND_ACS_SYNC)
    {
        ESP_LOGD(TAG, FW_MQTT_TOPIC_BACKEND_ACS_SYNC);

        err = esp_mqtt_client_unsubscribe(MqttClient::getInstance().getClient(),
                                          FW_MQTT_TOPIC_BACKEND_ACS_SYNC);
        ESP_LOGD(TAG, "unsubscribe topic %s, %d",
                 FW_MQTT_TOPIC_BACKEND_ACS_SYNC, static_cast<int>(err));
        UserDatabase::getInstance().actionSyncUsers(data, data_len);
    }
    // Create new user
    else if (mqtt_topic == FW_MQTT_TOPIC_BACKEND_ACS_USER_CREATE)
    {
        ESP_LOGD(TAG, FW_MQTT_TOPIC_BACKEND_ACS_USER_CREATE);
        UserDatabase::getInstance().actionCreateUser(data, data_len);
    }
    // Update old user
    else if (mqtt_topic == FW_MQTT_TOPIC_BACKEND_ACS_USER_UPDATE)
    {
        ESP_LOGD(TAG, FW_MQTT_TOPIC_BACKEND_ACS_USER_UPDATE);
        UserDatabase::getInstance().actionUpdateUser(data, data_len);
    }
    // Delete old user
    else if (mqtt_topic == FW_MQTT_TOPIC_BACKEND_ACS_USER_DELETE)
    {
        ESP_LOGD(TAG, FW_MQTT_TOPIC_BACKEND_ACS_USER_DELETE);
        UserDatabase::getInstance().actionDeleteUser(data, data_len);
    }
    // Ping
    else if (mqtt_topic == FW_MQTT_TOPIC_MONITORING_CONNECTION_PING)
    {
        ESP_LOGD(TAG, FW_MQTT_TOPIC_MONITORING_CONNECTION_PING);
        MqttClient::getInstance().send(FW_MQTT_TOPIC_MONITORING_CONNECTION_PONG, NULL,
                                       0, ConfigDatabase::getInstance().getMqttQos(), 0);
    }
    // Monitoring
    else if (mqtt_topic == FW_MQTT_TOPIC_MONITORING_CONTROLLER_REQUEST)
    {
        ESP_LOGD(TAG, FW_MQTT_TOPIC_MONITORING_CONTROLLER_REQUEST);
        Monitoring::sendMonitoring(NULL);
    }
    // Set config
    else if (mqtt_topic == FW_MQTT_TOPIC_CONFIG_SET)
    {
        ESP_LOGD(TAG, FW_MQTT_TOPIC_CONFIG_SET);
        ConfigDatabase::getInstance().actionUpdateConfig(data, data_len);
    }
    // Reset config
    else if (mqtt_topic == FW_MQTT_TOPIC_CONFIG_RESET)
    {
        ESP_LOGD(TAG, FW_MQTT_TOPIC_CONFIG_RESET);
        ConfigDatabase::getInstance().actionResetConfig(data, data_len);
    }
    // Get config
    else if (mqtt_topic == FW_MQTT_TOPIC_CONFIG_GET_REQUEST)
    {
        ESP_LOGD(TAG, FW_MQTT_TOPIC_CONFIG_GET_REQUEST);
        ConfigDatabase::getInstance().actionSendConfig();
    }
    // Firmware update
    else if (mqtt_topic == FW_MQTT_TOPIC_OTA_REQUEST)
    {
        ESP_LOGD(TAG, FW_MQTT_TOPIC_OTA_REQUEST);
        actionOTAfromHTTP(data, data_len);
    }
    // Access request response
    else if (mqtt_topic == FW_MQTT_TOPIC_BACKEND_ACS_AUTH_ACCESS_RESPONSE)
    {
        ESP_LOGD(TAG, FW_MQTT_TOPIC_BACKEND_ACS_AUTH_ACCESS_RESPONSE);

        if (!Messenger::isNotInited())
            Messenger::getInstance().actionAddNewAccess(data, data_len);
    }
    // Camera face detected
    else if (mqtt_topic == FW_MQTT_TOPIC_NOTIFY_CAMERA_FACE_DETECTED)
    {
        ESP_LOGD(TAG, FW_MQTT_TOPIC_NOTIFY_CAMERA_FACE_DETECTED);

        if (!Camera::isNotInited())
            Camera::getInstance().addNewDetectionFromProto(data, data_len);
    }
    // Unloading events
    else if (mqtt_topic == FW_MQTT_TOPIC_NOTIFY_UNLOADING_REQUEST)
    {
        ESP_LOGD(TAG, FW_MQTT_TOPIC_NOTIFY_UNLOADING_REQUEST);
        xTaskCreate(EventDatabase::unloadingEvents, "unloading", 5 * 1024,
                    static_cast<void *>(&(EventDatabase::getInstance())), 7, NULL);
    }*/
    //todo #1
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
    /*EventUnit processing_event;
    fdb_err_t err;

    // Send start event
    EventUnit start_event = createEventUnit(NOTIFICATIONS__EVENT_TYPE__STATE_STARTED);
    err = EventDatabase::getInstance().insert(&start_event);
    if (err != FDB_NO_ERR)
    {
        ESP_LOGE(TAG, "Error insert start event to events database, in MqttClient::sendInitMessage: %d", (int)err);
    }
    this->send(this->eventTypeToTopic(start_event.type), static_cast<const char *>(start_event.data),
               start_event.data_len, ConfigDatabase::getInstance().getMqttQos(), 0);
    free(start_event.data);

    // Send monitoring
    Monitoring::sendMonitoring(NULL);

    // Send sync processing event
    processing_event = createEventUnit(NOTIFICATIONS__EVENT_TYPE__ACS_USER_SYNC_PROCESSING);
    err = EventDatabase::getInstance().insert(&processing_event);
    if (err != FDB_NO_ERR)
    {
        ESP_LOGE(TAG, "Error insert sync event to events database, in MqttClient::sendInitMessage: %d", (int)err);
    }
    this->send(this->eventTypeToTopic(processing_event.type), static_cast<const char *>(processing_event.data),
               processing_event.data_len, ConfigDatabase::getInstance().getMqttQos(), 0);
    free(processing_event.data);*/
    //todo #2
}

/**
 * @brief Subscribe MQTT topics
 */
void MqttClient::subscribeTopics()
{
    int msg_id;

    std::string topics[] = {
        /*
        // Ping topic
        std::string(FW_MQTT_TOPIC_MONITORING_CONNECTION_PING),
        // Chande door mode
        std::string(FW_MQTT_TOPIC_CONTROL_DOOR_MODE),
        // Users sync topic
        std::string(FW_MQTT_TOPIC_BACKEND_ACS_SYNC),
        // User create topic
        std::string(FW_MQTT_TOPIC_BACKEND_ACS_USER_CREATE),
        // User update topic
        std::string(FW_MQTT_TOPIC_BACKEND_ACS_USER_UPDATE),
        // User delete topic
        std::string(FW_MQTT_TOPIC_BACKEND_ACS_USER_DELETE),
        // Monitoring controller
        std::string(FW_MQTT_TOPIC_MONITORING_CONTROLLER_REQUEST),
        // Set config
        std::string(FW_MQTT_TOPIC_CONFIG_SET),
        // Reset config
        std::string(FW_MQTT_TOPIC_CONFIG_RESET),
        // Get config
        std::string(FW_MQTT_TOPIC_CONFIG_GET_REQUEST),
        // Update firmware
        std::string(FW_MQTT_TOPIC_OTA_REQUEST),
        // Access request response
        std::string(FW_MQTT_TOPIC_BACKEND_ACS_AUTH_ACCESS_RESPONSE),
        // Camera face detected
        std::string(FW_MQTT_TOPIC_NOTIFY_CAMERA_FACE_DETECTED),
        // Request for unloading events
        std::string(FW_MQTT_TOPIC_NOTIFY_UNLOADING_REQUEST),
        */
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

/*
char *MqttClient::eventTypeToTopic(Notifications__EventType type)
{
    switch (type)
    {
    case NOTIFICATIONS__EVENT_TYPE__STATE_CONNECTED:
        return FW_MQTT_TOPIC_NOTIFY_STATE_CONNECTED;
    case NOTIFICATIONS__EVENT_TYPE__STATE_STARTED:
        return FW_MQTT_TOPIC_NOTIFY_STATE_STARTED;
    case NOTIFICATIONS__EVENT_TYPE__DOOR_CLOSED:
        return FW_MQTT_TOPIC_NOTIFY_DOOR_CLOSED;
    case NOTIFICATIONS__EVENT_TYPE__DOOR_OPENED:
        return FW_MQTT_TOPIC_NOTIFY_DOOR_OPENED;
    case NOTIFICATIONS__EVENT_TYPE__DOOR_HOLD_AT_CLOSING:
        return FW_MQTT_TOPIC_NOTIFY_DOOR_HOLD_AT_CLOSING;
    case NOTIFICATIONS__EVENT_TYPE__DOOR_HOLD_AT_OPENING:
        return FW_MQTT_TOPIC_NOTIFY_DOOR_HOLD_AT_OPENING;
    case NOTIFICATIONS__EVENT_TYPE__ACS_USER_SYNC_PROCESSING:
        return FW_MQTT_TOPIC_ACS_NOTIFY_USER_SYNC_PROCESSING;
    case NOTIFICATIONS__EVENT_TYPE__ACS_USER_SYNC_ERROR:
        return FW_MQTT_TOPIC_ACS_NOTIFY_USER_SYNC_ERROR;
    case NOTIFICATIONS__EVENT_TYPE__ACS_USER_SYNC_SUCCESS:
        return FW_MQTT_TOPIC_ACS_NOTIFY_USER_SYNC_SUCCESS;
    case NOTIFICATIONS__EVENT_TYPE__ACS_USER_CREATE:
        return FW_MQTT_TOPIC_NOTIFY_ACS_USER_CREATE;
    case NOTIFICATIONS__EVENT_TYPE__ACS_USER_UPDATE:
        return FW_MQTT_TOPIC_NOTIFY_ACS_USER_UPDATE;
    case NOTIFICATIONS__EVENT_TYPE__ACS_USER_DELETE:
        return FW_MQTT_TOPIC_NOTIFY_ACS_USER_DELETE;
    case NOTIFICATIONS__EVENT_TYPE__ACS_AUTH_PROCESSING:
        return FW_MQTT_TOPIC_NOTIFY_ACS_AUTH_PROCESSING;
    case NOTIFICATIONS__EVENT_TYPE__ACS_AUTH_SUCCESS:
        return FW_MQTT_TOPIC_NOTIFY_ACS_AUTH_SUCCESS;
    case NOTIFICATIONS__EVENT_TYPE__ACS_AUTH_FAILED:
        return FW_MQTT_TOPIC_NOTIFY_ACS_AUTH_FAILED;
    case NOTIFICATIONS__EVENT_TYPE__ACS_AUTH_ERROR:
        return FW_MQTT_TOPIC_NOTIFY_ACS_AUTH_ERROR;
    case NOTIFICATIONS__EVENT_TYPE__ACS_AUTH_ACCESS_REQUEST:
        return FW_MQTT_TOPIC_NOTIFY_ACS_AUTH_ACCESS_REQUEST;
    case NOTIFICATIONS__EVENT_TYPE__MODULE_LOCK_LOCK:
        return FW_MQTT_TOPIC_MODULE_LOCK_LOCK;
    case NOTIFICATIONS__EVENT_TYPE__MODULE_LOCK_TRIGGER:
        return FW_MQTT_TOPIC_MODULE_LOCK_TRIGGER;
    case NOTIFICATIONS__EVENT_TYPE__MODULE_LOCK_HANDLE:
        return FW_MQTT_TOPIC_MODULE_LOCK_HANDLE;
    case NOTIFICATIONS__EVENT_TYPE__MODULE_LOCK_CYLINDER:
        return FW_MQTT_TOPIC_MODULE_LOCK_CYLINDER;
    case NOTIFICATIONS__EVENT_TYPE__MODULE_LOCK_ERROR:
        return FW_MQTT_TOPIC_MODULE_LOCK_ERROR;
    case NOTIFICATIONS__EVENT_TYPE__MODULE_LOCK_CLOSED:
        return FW_MQTT_TOPIC_MODULE_LOCK_CLOSED;
    case NOTIFICATIONS__EVENT_TYPE__CONFIG_ERROR:
        return FW_MQTT_TOPIC_NOTIFY_CONFIG_ERROR;
    case NOTIFICATIONS__EVENT_TYPE__CONFIG_SUCCESS:
        return FW_MQTT_TOPIC_NOTIFY_CONFIG_SUCCESS;
    case NOTIFICATIONS__EVENT_TYPE__CONFIG_CONTROLLER_SUCCESS:
        return FW_MQTT_TOPIC_NOTIFY_CONFIG_SUCCESS;
    case NOTIFICATIONS__EVENT_TYPE__FIRMWARE_UPDATE_SUCCESS:
        return FW_MQTT_TOPIC_NOTIFY_OTA_SUCCESS;
    case NOTIFICATIONS__EVENT_TYPE__FIRMWARE_UPDATE_ERROR:
        return FW_MQTT_TOPIC_NOTIFY_OTA_ERROR;
    case NOTIFICATIONS__EVENT_TYPE__INTEGRATION_CALL_SECURITY:
        return FW_MQTT_TOPIC_NOTIFY_INTEGRATION_CALL_SECURITY;
    case NOTIFICATIONS__EVENT_TYPE__INTEGRATION_CALL_EMPLOYEE:
        return FW_MQTT_TOPIC_NOTIFY_INTEGRATION_CALL_EMPLOYEE;
    case NOTIFICATIONS__EVENT_TYPE__INTEGRATION_OPEN_DOOR:
        return FW_MQTT_TOPIC_NOTIFY_INTEGRATION_OPEN;
    case NOTIFICATIONS__EVENT_TYPE__INTEGRATION_CLOSE_DOOR:
        return FW_MQTT_TOPIC_NOTIFY_INTEGRATION_CLOSE;
    case NOTIFICATIONS__EVENT_TYPE__INTEGRATION_IN_ENTRY_REQUEST:
        return FW_MQTT_TOPIC_NOTIFY_INTEGRATION_IN_ENTRY_REQUEST;
    case NOTIFICATIONS__EVENT_TYPE__INTEGRATION_OUT_ENTRY_REQUEST:
        return FW_MQTT_TOPIC_NOTIFY_INTEGRATION_OUT_ENTRY_REQUEST;
    case NOTIFICATIONS__EVENT_TYPE__ALARM_NORMALIZED:
        return FW_MQTT_TOPIC_NOTIFY_ALARM_NORMALIZED;
    case NOTIFICATIONS__EVENT_TYPE__ALARM_TRIGGERED:
        return FW_MQTT_TOPIC_NOTIFY_ALARM_TRIGGERED;
    case NOTIFICATIONS__EVENT_TYPE__TAMPER_NORMALIZED:
        return FW_MQTT_TOPIC_NOTIFY_TAMPER_NORMALIZED;
    case NOTIFICATIONS__EVENT_TYPE__TAMPER_TRIGGERED:
        return FW_MQTT_TOPIC_NOTIFY_TAMPER_TRIGGERED;
    case NOTIFICATIONS__EVENT_TYPE__MONITORING_SENDED:
        return FW_MQTT_TOPIC_MONITORING_CONTROLLER;
    case NOTIFICATIONS__EVENT_TYPE__MULTIPLE_PASS:
        return FW_MQTT_TOPIC_NOTIFY_MULTIPLE_PASS;
    case NOTIFICATIONS__EVENT_TYPE__CHANGE_MODE:
        return FW_MQTT_TOPIC_NOTIFY_MODE;
    default:
        return "";
    }
}
//todo
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
