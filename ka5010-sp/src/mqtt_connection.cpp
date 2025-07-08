#include <mqtt_connection.h>

MqttConnection::MqttConnection(String mqtt_server, String client_name, String cmd_topic, String n_cups_topic, String update_topic):client(esp_client){

    this->mqtt_server = mqtt_server;
    this->client_name = client_name;
    this->cmd_topic = cmd_topic;
    this->update_topic = update_topic;
    this->n_cups_topic = n_cups_topic;


}

void MqttConnection::mqttConnect(){

        // Loop until connecting mqtt to broker
    while(!client.connected())
    {
        Serial.println("Attempting MQTT connection...");

        // Attempt to connect
        if(client.connect(client_name.c_str()))
        {
            Serial.println("Connected!");

            // Subscribe topics
            client.subscribe(cmd_topic.c_str(), 1);       // QoS 1
            client.subscribe(n_cups_topic.c_str(), 1); // QoS 1
            client.subscribe(update_topic.c_str(), 0);    // QoS 0 since it is not critical
        }
        else
        {
            Serial.print("Failed mqtt connection, result = ");
            Serial.println(client.state());

            Serial.println("Retrying...");
        }
    }

}