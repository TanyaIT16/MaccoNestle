#ifndef PLATFORM_H
#define PLATFORM_H

#include <Arduino.h>

//TMC library
#include <TMCStepper.h>
#include <HardwareSerial.h>

// Stepper motor control
#include <AccelStepper.h>

#define SOUND_SPEED 0.0343 // Speed of sound in cm/us



class Platform {

    public:

        // --- Constructor ---
        Platform();

        void attachPlatform(int _id, int _n_cups, int _turn_direction, int _max_speed, int _max_acc, int _disable_after_moving);
        void begin();

        //Methods
        void configureDriver();
        long stepsForNextCup(int n_cups, double steps_per_revolution, int turn_direction);
        float readUltrasonicSensor(int triggerPin, int echoPin);
        bool compareUltrasonicReadings(float distanceCm, double limitDistance);
        bool updateCupPresence();
        int getShortestRotation(int from_position, int to_position, int total);
        void moveToNextCup();
        void moveToPosition(AccelStepper &stepper, int current_pos,int target_pos);
        void goHome(int limitSwitchPin, bool &limitReached);
        void refillCups();
        void startRefillCycle();
        void moveRelativeSteps(long steps);
        void moveBackwards();
        //int rotateToLimitSwitch();


        // ISR and flag for limit switch
        static volatile bool limitReached; // Flag to indicate if the limit switch has been reached
        static void IRAM_ATTR stopMotor(); // Interrupt handler to stop the motor when the limit switch is triggered

    public:
        // --- Configurable variables ---
            int id;
            int n_cups;
            int turn_direction;
            double limitDistance; // Distance threshold for stable cup detection [cm]
            int max_speed; // Max speed [rev/min]
            int max_acc;   // Max acceleration [rev/min2]
            int disable_after_moving; // Disable motor after moving (true or false)
            // Delays
            unsigned long int take_platform_delay = 5000; // Delay to take a cup from the platform [ms]
            unsigned long int serve_platform_delay = 8000; // Delay to serve a cup on the platform [ms]

        // --- Hardware static varibales ---
        // Define Driver pins
        static constexpr int driver_pul = 5;    // PUL- pulse pin
        static constexpr int driver_dir = 4;    // DIR- direction pin
        static constexpr int driver_en = 21;     // EN- enable pin
        // Ultrasonic sensor pins 
        static constexpr int trigPin = 22; 
        static constexpr int echoPin = 34;
        // Limit switch pin
        static constexpr int limitSwitchPin = 15; // Pin for the limit switch //LIMIT_SWITCH
        // TMC2209 driver
        static constexpr float R_SENSE = 0.120f;
        static constexpr uint8_t DRIVER_ADDRESS = 0b00;

        // Variables for ultrasonic sensor
        long duration;
        float distanceCm;
        unsigned long int currentTime = 0;
        unsigned long int lastMeasurementTime = 0;
        int stableCupCount = 0;
        const int stableCupThreshold = 3;
        bool cup = false;
        bool previousCup = false;
        
        // Variables for stepper motor control
        static constexpr long steps_per_revolution = (1600)*5.0*(58.0/14.0);


        // Variablea for limit switch 
        int start_time=0; // Start time for the limit switch

        // Variables for refill button
        
        // Variables for cup
        int cups_on_platform; // Number of cups on the platform
        int current_position;

        // Variables for refill button
        int refill_button = 0; // State of the refill button
        int previous_refill_button = 0; // Previous state of the refill button

        // Bottle request
        String requested_bottle = ""; // Bottle requested by the user
        bool bottle_request_pending = false; // Is there a bottle request pending?

        // Last distance to go to recover in case of errors
        float last_distance_to_go = 0;


        // --- Motors and drivers ---
        AccelStepper stepper;
        TMC2209Stepper driver;

        // 
        enum RotationStatus {
            ROTATING,
            LIMIT_REACHED,
            ROTATION_ERROR,
            ROTATION_TIMEOUT
        };
        
        RotationStatus rotateToLimit();

};

// Serial port for TMC2209 (declaración, no definición)
extern HardwareSerial& SERIAL_PORT;

#endif
