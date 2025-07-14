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

    // Default limit distance until configuration is loaded
    limitDistance = 10.0; // [cm]
}

void Platform::begin() {
    // Configure GPIOs
    pinMode(driver_pul, OUTPUT);
    pinMode(driver_dir, OUTPUT);
    pinMode(driver_en, OUTPUT);

    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);

    pinMode(limitSwitchPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(limitSwitchPin), Platform::stopMotor, RISING);

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
    
    // Calculate the distance in cm
    float distanceCm = (duration * SOUND_SPEED) / 2.0; // Divide by 2 for round trip

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
    if (disable_after_moving) digitalWrite(driver_en, LOW);
    configureDriver();
    stepper.move(steps);
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

void IRAM_ATTR Platform::stopMotor() {
    limitReached = true;
}
