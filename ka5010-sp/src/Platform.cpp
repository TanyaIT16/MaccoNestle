#include <Platform.h>

HardwareSerial& SERIAL_PORT = Serial1;
volatile bool Platform::limitReached = false;

Platform::Platform()
        :  driver(&SERIAL_PORT, R_SENSE, DRIVER_ADDRESS) // Initialize the TMC2209 driver
        , stepper(AccelStepper::DRIVER, driver_pul, driver_dir) // Initialize the stepper motor with the driver interface
{

}

// Attach the platform with the given parameters
void Platform::attachPlatform(int _id, int _n_cups, int _turn_direction, int _max_speed, int _max_acc, int _disable_after_moving) {
    id = _id;
    n_cups = _n_cups;
    turn_direction = _turn_direction;
    max_speed = _max_speed;
    max_acc = _max_acc;
    disable_after_moving = _disable_after_moving;
    cups_on_platform = n_cups;

    // Set the limit distance for stable cup detection
    limitDistance = 15.0; // Example value in cm
}

void Platform::begin() {
    // Configure GPIOs
    pinMode(driver_pul, OUTPUT);
    pinMode(driver_dir, OUTPUT);
    pinMode(driver_en, OUTPUT);

    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);

    pinMode(limitSwitchPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(Platform::limitSwitchPin), Platform::stopMotor, RISING);

    // Driver and stepper setup
    configureDriver();
    stepper.setMaxSpeed(max_speed);
    stepper.setAcceleration(max_acc);

    digitalWrite(driver_en, LOW); // Enable driver
}

// Configure the TMC2209 driver
void Platform::configureDriver() {

    // Configure TMC2209 driver
    driver.begin();
    driver.toff(3);
    driver.rms_current(1202);
    driver.microsteps(8);
    driver.pwm_autoscale(true);
    driver.pwm_autograd(true);
    driver.en_spreadCycle(false);
    driver.intpol(true);
    
}

// Calculate the number of steps needed to move to the next cup
long Platform::stepsForNextCup(int n_cups, double steps_per_revolution, int turn_direction)
{
    return steps_per_revolution / n_cups * turn_direction;
}

// Read the ultrasonic sensor and return the distance in cm
float Platform::readUltrasonicSensor(int triggerPin, int echoPin) {
    // Trigger the ultrasonic sensor
    digitalWrite(triggerPin, LOW);
    delayMicroseconds(2);
    digitalWrite(triggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(triggerPin, LOW);

    // Read the echo pin
    long duration = pulseIn(echoPin, HIGH);
    //Serial.print("Ultrasonic duration: ");
    //Serial.println(duration);

    // Calculate the distance in cm
    float distanceCm = (duration * SOUND_SPEED) / 2.0; // Divide by 2 for round trip
    Serial.print("Calculated distance: ");
    Serial.println(distanceCm);

    return distanceCm;
}

// Compare the ultrasonic reading with the limit distance
bool Platform::compareUltrasonicReadings(float distanceCm, double limitDistance) {
    // Compare the distance reading with the limit distance
    return (distanceCm < limitDistance);
}

// Get the shortest rotation direction from one position to another
int Platform::getShortestRotation(int from_position, int to_position, int total) {
    // Calculate the shortest rotation direction
    int steps_clockwise = (to_position - from_position + total) % total; // clockwise steps
    int steps_counterclockwise =(from_position - to_position + total) % total; // counterclockwise steps
    return (steps_clockwise <= steps_counterclockwise) ? steps_clockwise : -steps_counterclockwise;
}

void Platform::moveToNextCup()
{
    if(disable_after_moving) digitalWrite(driver_en,LOW);
    // Aqui hay que hacer configureDriver
    stepper.move(stepsForNextCup(n_cups, steps_per_revolution, turn_direction));
}

bool Platform::updateCupPresence()
{
    currentTime = millis();
    if (currentTime - lastMeasurementTime > 1000) {
        distanceCm = readUltrasonicSensor(trigPin, echoPin);
        Serial.print("Distance (cm): ");
        Serial.println(distanceCm);

        if (compareUltrasonicReadings(distanceCm, limitDistance)) {
            stableCupCount++;
        } else {
            stableCupCount = 0;
        }

        lastMeasurementTime = currentTime;
    }

    return (stableCupCount >= stableCupThreshold);
}

void Platform::refillCups(){
    // Reset cups on platform
    cups_on_platform = n_cups;
}

void Platform::moveRelativeSteps(long steps)
{
    Serial.print("moveRelativeSteps called with steps: ");
    Serial.println(steps);

    // Ensure driver is enabled before any movement
    digitalWrite(driver_en, LOW);
    Serial.print("driver_en state before move: ");
    Serial.println(digitalRead(driver_en));

    configureDriver();
    stepper.move(steps);
    Serial.print("driver_en state after configure: ");
    Serial.println(digitalRead(driver_en));
    Serial.print("Distance to go after command: ");
    Serial.println(stepper.distanceToGo());
}

void Platform::moveBackwards()
{
    stepper.moveTo(stepper.currentPosition() - (steps_per_revolution *0.1* turn_direction));
    while (stepper.distanceToGo() != 0) {
    stepper.run();
    }
}

void Platform::startRefillCycle()
{
    refillCups();

    if (disable_after_moving) digitalWrite(driver_en, LOW);
    configureDriver();

    // Liberar el final de carrera si está pulsado
    if (digitalRead(limitSwitchPin) == HIGH) {
        moveBackwards();
    }

    limitReached = false;

    // Mover una vuelta completa
    moveRelativeSteps(steps_per_revolution * turn_direction);

    start_time = millis(); // Reset para el timeout en el estado ROTATING_TO_LIMIT
}

/*
Platform::RotationStatus Platform::rotateToLimit(){
    stepper.run();
    // Check if the limit switch has been reached
    if (limitReached) {
        stepper.stop();
        limitReached = false; // Reset the limit switch state
        Serial.println("Limit switch reached. Stopping motor.");
        
        if (disable_after_moving) digitalWrite(driver_en, HIGH);
        return LIMIT_REACHED;
    }
    if(!stepper.isRunning() && !limitReached) {
        Serial.println("Motor completed the movement without reaching the limit switch.");
        return ROTATION_ERROR;
    }

    if (millis() - start_time > 20000) {
        stepper.stop();
        Serial.println("Rotation timeout reached. Stopping motor.");
        return ROTATION_TIMEOUT;
    }

    return ROTATING;
}
*/

void IRAM_ATTR Platform::stopMotor() {
    limitReached = true;
}

Platform::RotationStatus Platform::calibrate()
{
    Serial.println("Calibrating platform...");
    digitalWrite(driver_en, LOW); // Enable driver
    configureDriver();
    stepper.setMaxSpeed(max_speed);
    stepper.setAcceleration(max_acc);

    // Check if the limit switch is pressed at startup
    if (digitalRead(limitSwitchPin) == HIGH) {
        Serial.println("Limit switch is pressed at startup. Moving backwards to release.");
        moveBackwards(); // Move backwards to release the limit switch
    }

    limitReached = false;
    Serial.println("Performing full rotation to find limit switch.");
    moveRelativeSteps(steps_per_revolution * turn_direction);
    start_time = millis();

    while (!limitReached && stepper.isRunning()) {
        stepper.run();
        if (millis() - start_time > 20000) {
            Serial.println("Timeout: Could not find limit switch during initial rotation.");
            return ROTATION_TIMEOUT;
            stepper.stop();
            break;
        }
    }

    if (limitReached) {
        Serial.println("Initial positioning complete: limit switch reached.");
        limitReached = false; // Reset the limit switch state
        return LIMIT_REACHED;
    } else {
        Serial.println("Initial positioning failed: limit switch NOT reached.");
        if(disable_after_moving) {
            digitalWrite(driver_en, HIGH); // Disable driver if not moving
        }
        return ROTATION_ERROR;
    }

    stepper.stop();
    if (disable_after_moving) {
        digitalWrite(driver_en, HIGH); // Disable driver after calibration
    }
}

void Platform::scan(int *results) {

    Serial.println("Starting scan of platform cups...");

    if (results == nullptr) {
        Serial.println("Error: results array is null.");
        return;
    }

    cups_on_platform = 0; // Reset cup count

    configureDriver();
    stepper.setMaxSpeed(max_speed);
    stepper.setAcceleration(max_acc);
    digitalWrite(driver_en, LOW); // Ensure driver is enabled

    for (int i = 0; i < n_cups; i++) {

        Serial.printf("Scanning position %d...\n", i);
        delay(300); // Allow any vibration to settle

        bool cupDetected = updateCupPresence();
        if (cupDetected) {
            Serial.printf("Cup detected at position %d\n", i);
            results[i] = 1;
            cups_on_platform++;
        } else {
            Serial.printf("No cup at position %d\n", i);
            results[i] = 0;
        }

        Serial.printf("cups_on_platform count: %d\n", cups_on_platform);

        if (i < n_cups - 1) {
            if (disable_after_moving) {
                digitalWrite(driver_en, LOW);
                Serial.println("Driver enabled before move to next cup.");
                configureDriver();
                stepper.setMaxSpeed(max_speed);
                stepper.setAcceleration(max_acc);
            }

            moveToNextCup();

            Serial.printf("Stepper starting move. Distance to go: %ld\n", stepper.distanceToGo());
            int safeguard_counter = 0;
            while (stepper.distanceToGo() != 0) {
                bool running = stepper.run();
                if (!running) {
                    Serial.println("Stepper stopped itself.");
                    break;
                }
                safeguard_counter++;
                if (safeguard_counter > 10000) {
                    Serial.println("Stepper safeguard limit reached, breaking loop.");
                    break;
                }
            }

            if (disable_after_moving) {
                digitalWrite(driver_en, HIGH);
                Serial.println("Driver disabled after movement.");
            }

            delay(300); // Stabilization delay before next detection
        }
    }

    if (disable_after_moving) {
        digitalWrite(driver_en, HIGH);
    }

    Serial.printf("Scan completed. Total cups detected: %d\n", cups_on_platform);
}
