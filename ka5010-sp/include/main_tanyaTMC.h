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

// Stepper motor control
#include <AccelStepper.h>

// Vectors
#include <vector>

// MQTT
#include <PubSubClient.h>

// WiFi
#include <WiFi.h>

// For reading JSON config file
#include <ArduinoJson.h>

// ESP32 filesystem handler
#include <SPIFFS.h>

//WiFi library
#include <wifi_connection.h>

//MQTT library
//#include <mqtt_connection.h>

//TMC library
#include <TMCStepper.h>
#include <HardwareSerial.h>


// OTAdrive
#include <otadrive_esp.h>
#define APIKEY "d8301248-3044-403c-aa98-cbc25797751d"    // APIkey for this product
#define FW_VER "v@1.3.8"                                 // App version

// States
#define ERROR -1
#define READY 0
#define WAITING_FOR_TAKE 1
#define WAITING_FOR_DROP 2
#define ROTATE 3
#define REFILL 4
#define ROTATING_TO_LIMIT 6

// define sound speed in cm/uS for ultrasonic sensor
#define SOUND_SPEED 0.034

// ERRORES
typedef int error_macco;
#define ERROR_ARCHIVO_CONFIG 1

// Using TMC2209
#define R_SENSE 0.120f // Sense resistor
//#define SERIAL_PORT Serial2  // Serial para comunicación con el TMC2209
HardwareSerial SERIAL_PORT = Serial1;
#define DRIVER_ADDRESS 0b00



/*********************/
/*                   */
/*     Variables     */
/*                   */
/*********************/

/*
typedef struct
{
    String ssid;
    String password;
    int wifi_timeout;
} wifi_config_t;
*/

// Platform id(1-4)
int id;

// Desired cups to be served
int desired_cups = 8;

// Limit distance for cup detection
float limitDistance = 12.0; // cm, distance between the sensor and the cup when the cup is on the platform

// WiFi Config
String ssid;
String password;
int wifi_timeout=8000;

// MQTT variables
String mqtt_server;
WiFiClient esp_client;
PubSubClient client(esp_client);

// MQTT topics
//String sensor_topic;
String state_topic;
String cmd_topic;
String update_topic;
String feedback_topic;
//String n_cups_topic;
//String platform_cups;
String refill_topic;
String medida_sensor;
//String finnished_order;

// ESP Name
String client_name;
String kiosk_name;

// Delays
unsigned long int take_platform_delay = 5000; // Delay to take a cup from the platform [ms]
unsigned long int serve_platform_delay = 8000; // Delay to serve a cup on the platform [ms]

// Define Driver pins
const int driver_pul = 5;    // PUL- pulse pin
const int driver_dir = 4;    // DIR- direction pin
const int driver_en = 21;     // EN- enable pin

// Ultrasonic sensor pins 
const int trigPin = 22; 
const int echoPin = 34;

// Ultrasonic sensor config
long duration;
float distanceCm;
unsigned long int currentTime = 0;
unsigned long int lastMeasurementTime = 0;
int stableCupCount = 0;
const int stableCupThreshold = 3;
// Ultrasonic sensor
bool cup = false;
bool previousCup = false;
//bool initialCupStateProcessed = false; // Variable to avoid overwriting `cup` at the beginning
unsigned long cupConfirmationStartTime = 0; // Variable to store the start time of the cup confirmation process
const unsigned long cupConfirmationDelay = 3000; // Delay for cup confirmation
//Refill Button
int refill_button=0;
int previous_refill_button;

//To finnish an order
int finnish_command = 0;

//Limit switch
#define LIMIT_SWITCH 15
volatile bool limitReached = false;
int start_time=0;


// MQTT loop config
unsigned long int period_mqtt_loop;    // Periodic time to run the mqtt loop [ms]
unsigned long int time_last_mqtt_loop = 0;

// States msgs handling
unsigned long int period_state_pub;    // Periodic time to publish device state [ms]
unsigned long int time_last_msg_state = 0;

// Sensor msgs handling
unsigned long int period_sensor_pub; // Periodic time to publish sensor information [ms]
unsigned long int time_last_msg_sensor = 0;

///////////////////////////////////////

// Define a stepper and the pins it will use
AccelStepper stepper(AccelStepper::DRIVER, driver_pul, driver_dir);
TMC2209Stepper driver(&SERIAL_PORT,R_SENSE,DRIVER_ADDRESS);


// Stepper config
const double steps_per_revolution = (1600)*5.0*(58.0/14.0);         // Steps to complete a revolution considering gear ratio
int max_speed;             // Max speed [rev/min]
int max_acc;               // Max acceleration [rev/min2]
int turn_direction;          // Direction of turning (1 clockwise or -1 counterclockwise)
#define DIR_PIN 13         // Direction pin
int disable_after_moving;    // Stop the motor after moving, to reduce heating and consumption

// Current state
int state;

// Current command in execution
String current_command = "none";

// Last distance to go to recover in case of errors
float last_distance_to_go = 0;

// Current number of cups on the service platform (without considering if there is one on the
// service pos)
int cups_on_platform = 0;

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

// Connect to wifi
//void wifiConnect();

// Connect to mqtt broker
void mqttConnect();

// OTA send update progress through serial
void onUpdateProgress(int progress, int totalt);

void IRAM_ATTR stopMotor();

void configureDriver();