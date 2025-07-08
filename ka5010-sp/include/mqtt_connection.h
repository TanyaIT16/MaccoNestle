#ifndef MQTT_CONECTION_H
#define MQTT_CONNECTION_H

#include <WiFi.h>
#include <PubSubClient.h>
#include <Arduino.h>
#include <wifi_connection.h>

class MqttConnection{

    public:
        // Constructor
        MqttConnection(String mqtt_server, String client_name, String cmd_topic, String n_cups_topic, String update_topic);
        // Method
        void mqttConnect();

    private:

        // MQTT variables
        String mqtt_server;
        WiFiClient esp_client;          // Declara WiFiClient dentro de la clase
        PubSubClient client;
        String client_name;

        // MQTT topics
        String cmd_topic;
        String n_cups_topic;
        String update_topic;

};


#endif