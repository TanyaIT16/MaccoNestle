/******************************************************************/
/*                                                                */
/*     Controller for ESP32 service platform module               */
/*                                                                */
/*     Macco Robotics                                             */
/*     NYC Lele                                                   */
/*                                                                */
/*     Maintainer: Tanya Ilieva Timova                            */
/*                 tilieva@maccorobotics.com                      */
/*                                                                */
/******************************************************************/

/*********************/
/*                   */
/*     Variables     */
/*                   */
/*********************/

// MQTT
#include <PubSubClient.h>

// For reading JSON config file
#include <ArduinoJson.h>

//WiFi library
#include <wifi_connection.h>

// Platform library
#include "Platform.h"

// ESP32 filesystem handler
#include <SPIFFS.h>

// OTAdrive
#include <otadrive_esp.h>
#define APIKEY "d8301248-3044-403c-aa98-cbc25797751d"    // APIkey for this product
#define FW_VER "v@1.3.8"                                 // App version

// OTA send update progress through serial
void onUpdateProgress(int progress, int totalt);

// Class variables
typedef struct {
    int id;
    int n_cups;
    int turn_direction;
    int max_speed;
    int max_acc;
    int disable_after_moving; // Disable motor after moving
} PlatformConfig;

// Platform configuration
PlatformConfig config;

// Platform instance
extern Platform platform;


// WiFi Config
String ssid;
String password;
int wifi_timeout=8000;

// MQTT variables
String mqtt_server;
WiFiClient esp_client;
PubSubClient client(esp_client);

// MQTT topics
String state_topic;
String cmd_topic;
String update_topic;
String feedback_topic;
String refill_topic;
String medida_sensor;


// ESP Name
String client_name;
String kiosk_name;

// MQTT loop config
unsigned long int period_mqtt_loop;    // Periodic time to run the mqtt loop [ms]
unsigned long int time_last_mqtt_loop = 0;

// States msgs handling
unsigned long int period_state_pub;    // Periodic time to publish device state [ms]
unsigned long int time_last_msg_state = 0;

// Sensor msgs handling
unsigned long int period_sensor_pub; // Periodic time to publish sensor information [ms]
unsigned long int time_last_msg_sensor = 0;

// Current time of the loop
unsigned long int currentTime = 0;

// States
#define ERROR -1
#define INIT 0
#define READY 1
#define CHECK 2
#define WAITING_FOR_TAKE 3
#define WAITING_FOR_DROP 4
#define WAITING_FOR_ORDER 5
#define ROTATE 6
#define REFILL 7
#define ROTATING_TO_LIMIT 8

// Current state
int state = INIT;

// Current command in execution
String current_command = "none";

//To finnish an order
int finnish_command = 0;

// Bottles
//Bottle platform
const int NUM_BOTTLES = 8; // Number of bottles on the platform
typedef struct {
    String bottle_name; // Bottle name
    int pos;             // Bottle position on the platform (0-7)
    bool isBottle;       // Is there a bottle on this position?
    bool isEmpty;        // Is this bottle empty?
} Bottle;

Bottle bottles[NUM_BOTTLES];
int current_bottle_position = 0; // Current position of the platform


/*********************/
/*                   */
/*     Functions     */
/*                   */
/*********************/

// MQTT subscriptions callback
void callback(char * topic, byte * message, unsigned int length);

// Send device state trough MQTT topic
void sendState(String msg);
void sendState(int state);

// Get state string msg
String getStateString(int state);

// Send feedback through mqtt topic
void sendFeedback(String command, String feedback);

// Send sensor data through mqtt topic
void sendSensorData(float distanceCm);

// Load local config json file
void loadConfig();

// Set default config, used if config setup fails
void useDefaultConfig();

// Connect to mqtt broker
void mqttConnect();

void loadConfig();
