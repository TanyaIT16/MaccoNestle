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

#include <main_tanyaTMC.h>

//MqttConnection mqtt(mqtt_server, client_name, cmd_topic, n_cups_topic, update_topic);


// Setup of the ESP32
void setup()
{
    Serial.begin(9600);
    SERIAL_PORT.begin(9600);
    delay(1000);


    // Load config
    loadConfig();

    WifiConnection wifi(ssid,password,wifi_timeout);

    // Setup wifi
    //wifiConnect();

    wifi.wifiConnect();

    // Setup mqtt topics depending on kiosk and client names
    //id = 1; // Platform id
    // id 1 for taking empty clean cups
    // id 2 for serving full cups
    // id 3 dfor taking empty clean cocktail cups
    // Publishers
    //sensor_topic = kiosk_name + "/" + client_name + "/platform/" + id + "/sensor";
    state_topic = kiosk_name + "/" + client_name + "/platform/" + id + "/state";
    feedback_topic = kiosk_name + "/" + client_name + "/platform/" + id + "/feedback";
    //platform_cups = kiosk_name + "/" + client_name + "/platform/" + id + "/cups";
    medida_sensor = kiosk_name + "/" + client_name + "/platform/" + id + "/medida_sensor";
    // Subscribers
    cmd_topic = kiosk_name + "/" + client_name + "/platform/" + id + "/cmd";
    update_topic = kiosk_name + "/" + client_name + "/platform/" + id + "/update";
    //n_cups_topic = kiosk_name + "/" + client_name + "/platform/" + id + "/n_cups";
    refill_topic = kiosk_name + "/" + client_name + "/platform/" + id + "/refill_button";
    //finnished_order = kiosk_name + "/" + client_name + "/platform/" + id + "/finnished_order";

    // Setup mqtt
    client.setServer(mqtt_server.c_str(), 1883);

    mqttConnect();
    //mqtt.mqttConnect();

    // Set callback for mqtt messages
    client.setCallback(callback);

    // OTA setup
    OTADRIVE.setInfo(APIKEY, FW_VER);
    OTADRIVE.onUpdateFirmwareProgress(onUpdateProgress);

    stableCupCount = 0;

    //Select turning direction
    if(digitalRead(DIR_PIN) == HIGH)
    {
        turn_direction = 1; // Clockwise
    }
    else
    {
        turn_direction = -1; // Counter clockwise
    }

    Serial.println("DIRECTION: " + String(turn_direction));

    // Setup motor
    pinMode(driver_pul, OUTPUT);
    pinMode(driver_dir, OUTPUT);
    pinMode(driver_en, OUTPUT);
    pinMode(DIR_PIN, INPUT_PULLUP);

    // Setup limit switch
    pinMode(LIMIT_SWITCH, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(LIMIT_SWITCH), stopMotor, RISING); // Attach interrupt to stop motor when limit switch is pressed

    // Setup ultrasonic sensors
    pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
    pinMode(echoPin, INPUT); // Sets the echoPin as an Input

    // Initialize cups_on_platform
    if(id==1)
    {
        cups_on_platform = 8; // Specific number of cups depending on how many fit, for down platforms
        limitDistance = 17.0; // cm, distance between the sensor and the cup when the cup is on the platform
    }
    else if (id==2)
    {
        cups_on_platform = 0; // For sserving platforms
        limitDistance = 12.0; // cm, distance between the sensor and the cup when the cup is on the platform
    }
    else if (id==3)
    {
        cups_on_platform = 10; // Srecific number of cups depending on how many fit, for up platforms
        limitDistance = 14.0; // cm, distance between the sensor and the cup when the cup is on the platform
    }
    
    // Enable motor
    digitalWrite(driver_en, LOW);

    
    // Setup stepper speed
    stepper.setMaxSpeed(1500);      // From rev/min to step/s
    stepper.setAcceleration(200);    // From rev/min2 to step/s2


    // Setup TMC2209 driver
    configureDriver();

    // Move one complete rev at startup just in case some cups where not completely served before
    // shutdown
    Serial.println("Commanding one whole rev turn just in case there are cups left to be served");
    if (id == 1 || id == 3) 
    {
        // Comienza el movimiento de 360 grados
        if (digitalRead(LIMIT_SWITCH) == HIGH) {
            stepper.moveTo(-(stepper.currentPosition()+steps_per_revolution*(45)/360*turn_direction));
            while (stepper.distanceToGo() != 0) 
            {
                stepper.run();
            }
        }
        limitReached = false; // Reinicia el estado del límite
        stepper.move((steps_per_revolution*turn_direction));
        // Ejecuta el movimiento mientras no se alcance el límite
        while (!limitReached) {
            stepper.run(); // Ejecuta el movimiento paso a paso

            // Verifica si el motor ha alcanzado su objetivo
            if (stepper.distanceToGo() == 0) {
                Serial.println("Motor completed the movement without reaching the limit switch.");
                break; // Sal del bucle si el motor termina el movimiento
            }
        }

        // Si se alcanzó el límite, detén el motor
        if (limitReached) {
            stepper.stop();
            Serial.println("Limit switch reached. Stopping motor.");
            digitalWrite(driver_en, HIGH);

            // Reinicia el estado del límite
            limitReached = false;
        }
    }   
    else 
    {
        // Lógica existente para otros IDs
        stepper.move(steps_per_revolution * turn_direction); // moveTo si esto no funciona
        while (stepper.distanceToGo() != 0) {
            stepper.run();
        }
        cup = true;
    }


    Serial.println("Whole rev finnished");

    current_command = "init";
    
    Serial.println("Setup finished!");

    state = READY;
    sendState(state);
    Serial.println("State " + getStateString(state));


}


// Main loop continuosly running
void loop()
{


    // MQTT loop
    if(millis() - time_last_mqtt_loop > period_mqtt_loop)
    {
        time_last_mqtt_loop = millis();

        // If mqtt is not connected, retry
        
        if(!client.connected())
        {
            mqttConnect();
        }

        // Launch mqtt client loop
        client.loop();

        // State update
        if(millis() - time_last_msg_state > period_state_pub)
        {
            sendState(state);
            // Serial.println("State " + getStateString(state));
        }
    }



    currentTime = millis();


    // Initializing ultrasonic measurement every second
    if (currentTime - lastMeasurementTime >1000)
    {
        // Clears the trigPin
        digitalWrite(trigPin, LOW);
        delayMicroseconds(2);
        // Sets the trigPin on HIGH state for 10 micro seconds
        digitalWrite(trigPin, HIGH);
        delayMicroseconds(10);
        digitalWrite(trigPin, LOW);

        // Measuring echo pulse duration using pulseIn
        duration = pulseIn(echoPin, HIGH);
        distanceCm = duration * SOUND_SPEED / 2;
        Serial.print("Distance (cm): ");
        sendSensorData(distanceCm);
        Serial.println(distanceCm);

        // Variable to avoid possible mistakes during the cup detection
        previousCup = cup;


        if(distanceCm < limitDistance) // 12 cm is the distance between the sensor and the cup when the cup is on the platform
        {
            // Counter to ensure the presence of a cup
            stableCupCount++;
            //Serial.print("stableCupCount: ");
            //Serial.println(stableCupCount);
        }
        else
        {
            stableCupCount = 0;
        }

        if(stableCupCount >= stableCupThreshold)
        {
            cup = true;
        }
        else
        {
            cup = false;
        }

        lastMeasurementTime = currentTime;

        //Serial.print("STABLE CUP COUNT: ");
        //Serial.println(stableCupCount);

    }

    // There are 2 different state machines, depending on which platform is being used
    if(id==1)
    {
        // State machine
        switch(state)
        {
            case READY:
                
                //Check if there is a cup to update state
                if(cup)
                {
                    Serial.print("Number of cups: ");
                    Serial.println(cups_on_platform);
                    Serial.println("CUP");
                    state = WAITING_FOR_TAKE;
                    sendState(state);
                    Serial.println("New state " + getStateString(state));

                    // Check if the stepper is moving
                    if(stepper.isRunning())
                    {
                        //Serial.println("It is running");
                        float rev_to_go = (float)(stepper.distanceToGo()) / steps_per_revolution;

                        if(current_command == "moveorder" && rev_to_go < (1.0 / 12.0))
                        {
                            //cups_on_platform = 8;
                            sendFeedback(current_command, "FINISHED");
                            current_command = "none";
                        }
                        else
                        {
                            // Save distance to go in case of error
                            last_distance_to_go = stepper.distanceToGo();
                        }

                        // Stop the motor
                        stepper.stop();
                        if (disable_after_moving) digitalWrite(driver_en, HIGH);
                    }


                }

                break;

            case WAITING_FOR_TAKE:

                //Waiting the robot to take off the cup
                //Check if there is not a cup and previously there was one
                /*
                Serial.println("cup: ");
                Serial.print(cup);
                Serial.println("previousCup: ");
                Serial.print(previousCup);
                */

                digitalWrite(driver_en, LOW);
                //driver.ihold(16);
                configureDriver();
                driver.freewheel(0); 

                if(!cup && previousCup)
                {
                    driver.freewheel(1);
                    //driver.ihold(0);
                    digitalWrite(driver_en, HIGH);
                    state = ROTATE;
                    sendState(state);
                    Serial.println("New state " + getStateString(state));

                    // Set the current command to "movecup" to move the platform to the next position
                    current_command = "movecup";
                    if(cups_on_platform >0)
                    {
                        cups_on_platform--;
                    }

                    if(cups_on_platform == 0)
                    {
                        state = REFILL;
                        sendState(state);
                        Serial.println("New state " + getStateString(state));
                    }else{

                        // Enable motor and move the stepper
                        delay(take_platform_delay);
                        if(disable_after_moving) digitalWrite(driver_en,LOW);
                        configureDriver();
                        stepper.move(steps_per_revolution*(45)/360*turn_direction); //Assuming one position move is 36 degrees
                        

                    }   
                }
                previousCup = cup;
                
                break;

            case ROTATE:

                // Check if stepper is moving
                if(!stepper.isRunning())
                {
                    // Movement finished
                    state = READY;
                    sendState(state);
                    Serial.println("New state " + getStateString(state));


                    // Disable motor
                    if(disable_after_moving) digitalWrite(driver_en, HIGH);
                    
                    // Send feedback if the movement was commanded externally
                    if(current_command == "movecup")
                    {
                        sendFeedback(current_command,"FINISHED");
                        current_command = "none";
                    }
                }

                break;

                case REFILL:
                    state = REFILL;
                    sendState(state);
                    Serial.println("New state " + getStateString(state));

                    if(refill_button == 1 && previous_refill_button == 0) { // Asumiendo que LOW es pulsado
                        Serial.println("Refill button pressed. Exiting REFILL state.");
                        cups_on_platform = 8; // Reset cups on platform
                        if(disable_after_moving) digitalWrite(driver_en,LOW);
                        // Configure TMC2209 driver
                        configureDriver();
                        limitReached = false;
                        // Move stepper 360 degrees
                        stepper.moveTo(stepper.currentPosition() + (steps_per_revolution*turn_direction));  
                        //current_command = "moveorder";
                        start_time = millis();
                        // Update state
                        state = ROTATING_TO_LIMIT;
                        sendState(state);
                        Serial.println("New state " + getStateString(state));
                    }


                    /*
                    if (refill_button == 1 && previous_refill_button == 0) {
                        // Verify there are less than 8 cups on the platform
                        if (cups_on_platform < 8) {
                            //state = REFILL;
                            sendState(state);
                            Serial.println("New state " + getStateString(state));
                
                            // Increment the number of cups on the platform
                            cups_on_platform++;
                            Serial.print("Number of cups: ");
                            Serial.println(cups_on_platform);

                            //current_command = "movecup";

                
                            // Configure TMC2209 driver
                            if(disable_after_moving) digitalWrite(driver_en,LOW);
                            configureDriver();
                            // Move stepper 45 degrees
                            stepper.move(steps_per_revolution * (45) / 360 * turn_direction);
                
                        }

                        // Check if all cups have been placed
                        if(cups_on_platform == 8)
                        {   

                            // Configure TMC2209 driver
                            if(disable_after_moving) digitalWrite(driver_en,LOW);
                            configureDriver();
                            limitReached = false;
                            // Move stepper 360 degrees
                            stepper.moveTo(stepper.currentPosition() + (steps_per_revolution*turn_direction));  
                            //current_command = "moveorder";
                            start_time = millis();
                            // Update state
                            state = ROTATING_TO_LIMIT;
                            sendState(state);
                            Serial.println("New state " + getStateString(state));
                            
                        }
                            
                    }
                    
                    */
                    // Update previous refill button state
                    previous_refill_button = refill_button;
                    refill_button = 0;


                break;
            
            case ROTATING_TO_LIMIT:
                    stepper.run();
                    // Check if the limit switch has been reached
                    if(limitReached)
                    {
                        // Stop the motor
                        stepper.stop();
                        state = READY;
                        sendState(state);
                        Serial.println("New state " + getStateString(state));
                        if(disable_after_moving) digitalWrite(driver_en, HIGH);
                        limitReached = false;
                    }
                    
                    // Check if the motor has completed 360° without reaching the limit switch, enter ERROR
                    if (!stepper.isRunning() && !limitReached) {
                        Serial.println("ERROR: Motor completed 360° without reaching limit switch.");
                        state = ERROR;
                        sendState(state);
                        Serial.println("New state " + getStateString(state));
                    }

                    // If the motor takes too long to reach the limit switch, enter ERROR
                    if (millis() - start_time > 20000) {  // 20 seconds
                        Serial.println("ERROR: Motor took too long to reach limit switch.");
                        stepper.stop();
                        state = ERROR;
                        sendState(state);
                        Serial.println("New state " + getStateString(state));
                    }
                    
                break;
            

            default:
            
                state = ERROR;
                sendState(state);

        }
    }

    if(id==2)
    {

        switch (state)
        {
            case READY:
                //Serial.println("State: READYYYYYYY");
                //Serial.println("cup:" + String(cup));
                //Serial.println("previousCup:" + String(previousCup));
                //Check if there is a cup to update state
                if(!cup)
                {   
                    if (cupConfirmationStartTime == 0) 
                    {
                        cupConfirmationStartTime = millis(); // Inicia el temporizador
                    }

                    if (millis() - cupConfirmationStartTime > cupConfirmationDelay) 
                    {
                        Serial.println("No cup detected for 3 seconds. Transitioning to WAITING_FOR_DROP.");
                        digitalWrite(driver_en, LOW);
                        configureDriver();
                        driver.freewheel(0);
                        Serial.print("Number of cups: ");
                        Serial.println(cups_on_platform);
                        Serial.println("CUP");
                        state = WAITING_FOR_DROP;
                        sendState(state);
                        Serial.println("New state " + getStateString(state));
                    } 
                    else 
                    {
                        Serial.println("Waiting for cup confirmation...");
                    }
                }
                else 
                {
                    cupConfirmationStartTime = 0; // Reinicia el temporizador si se detecta un vaso
                }
                    // Check if stepper is moving
                    /*
                    if(stepper.isRunning())
                    {
                        float rev_to_go = (float)(stepper.distanceToGo()) / steps_per_revolution;

                        if(current_command == "moveorder" && rev_to_go < (1.0 / 12.0))
                        {
                            cups_on_platform = 0;
                            sendFeedback(current_command, "FINISHED");
                            current_command = "none";
                        }
                        else
                        {
                            // Save distance to go in case of error
                            last_distance_to_go = stepper.distanceToGo();
                        }

                        // Stop the motor
                        stepper.stop();
                        if (disable_after_moving) digitalWrite(driver_en, HIGH);
                    }
                    */

                
                break;

            case WAITING_FOR_DROP:

                //Waiting the robot to drop the cup
                //Check if there is a cup
                /*
                if(cup)
                {
                    state = ROTATE;
                    sendState(state);
                    Serial.println("New state " + getStateString(state));



                    if(cups_on_platform < desired_cups)
                    {
                        // Set the current command to "movecup" to move the platform to the next position
                        current_command = "movecup";
                        cups_on_platform++;
                         // Enable motor and move the stepper
                            delay(serve_platform_delay);
                            if(disable_after_moving) digitalWrite(driver_en,LOW);
                            configureDriver();
                            stepper.move(steps_per_revolution*(45)/360*turn_direction); //Assuming one position move is 36 degrees 
                    }
                    else
                    {
                        // Set the current command to "movecup" to move the platform to the next position
                        current_command = "moveorder";
                        Serial.println("All the requested cups have been served");
                         // Enable motor and move the stepper
                        if(disable_after_moving) digitalWrite(driver_en,LOW);
                        configureDriver();
                        stepper.move(steps_per_revolution*(45)/360*turn_direction); //Assuming one position move is 36 degrees 
                    }

 

                }*/
                digitalWrite(driver_en, LOW);
                configureDriver();
                driver.freewheel(0);
                if (finnish_command==1)
                {
                    driver.freewheel(1); 
                    state = ROTATE;
                    sendState(state);
                    Serial.println("New state " + getStateString(state));

                    // Set the current command to "movecup" to move the platform to the next position
                    current_command = "movecup";

                    // Enable motor and move the stepper
                    delay(serve_platform_delay);
                    if(disable_after_moving) digitalWrite(driver_en,LOW);
                    configureDriver();
                    stepper.move(steps_per_revolution*(45)/360*turn_direction); //Assuming one position move is 45 degrees 
                    
                    finnish_command=0;
                }


                break;

            case ROTATE:

                // Check if stepper is moving
                if(!stepper.isRunning())
                {
                    // Movement finished
                    state = READY;
                    sendState(state);

                    // Disable motor
                    if(disable_after_moving) digitalWrite(driver_en, HIGH);

                    // Send feedback if the movement was commanded externally
                    if(current_command == "movecup")
                    {
                        sendFeedback(current_command,"FINISHED");
                        current_command = "none";
                    }
                }

                break;

            default:
                state = ERROR;
                sendState(state);


        }

    }

    if(id==3)
    {
        // State machine
        switch(state)
        {
            case READY:
                
                //Check if there is a cup to update state
                if(cup)
                {
                    Serial.print("Number of cups: ");
                    Serial.println(cups_on_platform);
                    Serial.println("CUP");
                    state = WAITING_FOR_TAKE;
                    sendState(state);
                    Serial.println("New state " + getStateString(state));

                    // Check if the stepper is moving
                    if(stepper.isRunning())
                    {
                        //Serial.println("It is running");
                        float rev_to_go = (float)(stepper.distanceToGo()) / steps_per_revolution;

                        if(current_command == "moveorder" && rev_to_go < (1.0 / 12.0))
                        {
                            //cups_on_platform = 10;
                            sendFeedback(current_command, "FINISHED");
                            current_command = "none";
                        }
                        else
                        {
                            // Save distance to go in case of error
                            last_distance_to_go = stepper.distanceToGo();
                        }

                        // Stop the motor
                        stepper.stop();
                        if (disable_after_moving) digitalWrite(driver_en, HIGH);
                    }


                }

                break;

            case WAITING_FOR_TAKE:

                //Waiting the robot to take off the cup
                //Check if there is not a cup and previously there was one
                /*
                Serial.println("cup: ");
                Serial.print(cup);
                Serial.println("previousCup: ");
                Serial.print(previousCup);
                */
                digitalWrite(driver_en, LOW);
                //driver.ihold(16);
                configureDriver();
                driver.freewheel(0); 

                if(!cup && previousCup)
                {
                    driver.freewheel(1);
                    //driver.ihold(0);
                    digitalWrite(driver_en, HIGH);
                    state = ROTATE;
                    sendState(state);
                    Serial.println("New state " + getStateString(state));


                    // Set the current command to "movecup" to move the platform to the next position
                    current_command = "movecup";
                    if(cups_on_platform >0)
                    {
                        cups_on_platform--;
                    }

                    if(cups_on_platform == 0)
                    {
                        state = REFILL;
                        sendState(state);
                        Serial.println("New state " + getStateString(state));
                    }else{

                        // Enable motor and move the stepper
                        delay(take_platform_delay);
                        if(disable_after_moving) digitalWrite(driver_en,LOW);
                        configureDriver();
                        stepper.move(steps_per_revolution*(36)/360*turn_direction); //Assuming one position move is 36 degrees
                        

                    }   
                }
                previousCup = cup;
                
                break;

            case ROTATE:

                // Check if stepper is moving
                if(!stepper.isRunning())
                {
                    // Movement finished
                    state = READY;
                    sendState(state);
                    Serial.println("New state " + getStateString(state));


                    // Disable motor
                    if(disable_after_moving) digitalWrite(driver_en, HIGH);
                    
                    // Send feedback if the movement was commanded externally
                    if(current_command == "movecup")
                    {
                        sendFeedback(current_command,"FINISHED");
                        current_command = "none";
                    }
                }

                break;

                case REFILL:

                    state = REFILL;
                    sendState(state);
                    Serial.println("New state " + getStateString(state));

                    if(refill_button == 1 && previous_refill_button == 0) { // Asumiendo que LOW es pulsado
                        Serial.println("Refill button pressed. Exiting REFILL state.");
                        cups_on_platform = 10; // Reset cups on platform
                        if(disable_after_moving) digitalWrite(driver_en,LOW);
                        // Configure TMC2209 driver
                        configureDriver();
                        limitReached = false;
                        // Move stepper 360 degrees
                        stepper.moveTo(stepper.currentPosition() + (steps_per_revolution*turn_direction));  
                        //current_command = "moveorder";
                        start_time = millis();
                        // Update state
                        state = ROTATING_TO_LIMIT;
                        sendState(state);
                        Serial.println("New state " + getStateString(state));
                    }

                    /*
                    if (refill_button == 1 && previous_refill_button == 0) {
                        // Verify if there are less than 10 cups on the platform
                        if (cups_on_platform < 10) {
                            //state = REFILL;
                            sendState(state);
                            Serial.println("New state " + getStateString(state));
                
                            // Increment the number of cups on the platform
                            cups_on_platform++;
                            Serial.print("Number of cups: ");
                            Serial.println(cups_on_platform);

                            //current_command = "movecup";

                
                            // Configure TMC2209 driver
                            if(disable_after_moving) digitalWrite(driver_en,LOW);
                            configureDriver();
                            // Move stepper 36 degrees
                            stepper.move(steps_per_revolution * (36) / 360 * turn_direction);
                
                        }

                        if(cups_on_platform == 10)
                        {
                            // Configure TMC2209 driver
                            if(disable_after_moving) digitalWrite(driver_en,LOW);
                            configureDriver();
                            limitReached = false;
                            // Move stepper 360 degrees
                            stepper.moveTo(stepper.currentPosition() + (steps_per_revolution*turn_direction));  
                            //current_command = "moveorder";
                            start_time = millis();
                            // Update state
                            state = ROTATING_TO_LIMIT;
                            sendState(state);
                            Serial.println("New state " + getStateString(state));
                            
                        }
                            
                    }
            
                    */
                    // Update previous refill button state
                    previous_refill_button = refill_button;
                    refill_button = 0;

                break;

            case ROTATING_TO_LIMIT:
                    stepper.run();
                    // Check if the limit switch has been reached
                    if(limitReached)
                    {
                        stepper.stop();
                        state = READY;
                        sendState(state);
                        Serial.println("New state " + getStateString(state));
                        if(disable_after_moving) digitalWrite(driver_en, HIGH);
                        limitReached = false;
                    }
                        // Check if the motor has completed 360° without reaching the limit switch, enter ERROR
                    if (!stepper.isRunning() && !limitReached) {
                        Serial.println("ERROR: Motor completed 360° without reaching limit switch.");
                        state = ERROR;
                        sendState(state);
                        Serial.println("New state " + getStateString(state));
                    }

                    // If the motor takes too long to reach the limit switch, enter ERROR
                    if (millis() - start_time > 20000) {  // 20 seconds
                        Serial.println("ERROR: Motor took too long to reach limit switch.");
                        stepper.stop();
                        state = ERROR;
                        sendState(state);
                        Serial.println("New state " + getStateString(state));
                    }
                    
                break;

            default:
            
                state = ERROR;
                sendState(state);

        }
    }


     // Check if movement is due
    if(stepper.isRunning())
    {
        // Run stepper
        bool still_running = stepper.run();

        // Check when movement is finished (detecting movement falling edge)
        if(!still_running)
        {
             Serial.println("Movement finished");

            // Disable motor
            if(disable_after_moving) digitalWrite(driver_en, HIGH);

            // Send feedback if the movement was commanded externally
            if(current_command != "none" && last_distance_to_go == 0)
            {
                Serial.println("Finished command " + current_command);

                // Reset cups on platform if command is moveorder
                if(current_command == "moveorder" && (id==1)) cups_on_platform = 3;
                // Reset cups on platform if command is moveorder
                if(current_command == "moveorder" && (id==2)) cups_on_platform = 0;

                sendFeedback(current_command, "FINISHED");
                current_command = "none";
            }
        }
    }
}

// MQTT subscription callback
void callback(char * topic,byte * message, unsigned int length)
{
    // Serial.print("Message arrived on topic: ");
    // Serial.print(topic);
    // Serial.println();

    // If the topic contains a command for this device
    if(String(topic) == cmd_topic)
    {
        // Extract command
        char buffer[length + 1];

        // Serial.print("Message: ");

        for(int i = 0; i < length; i++)
        {
            // Serial.print((char)message[i]);
            buffer[i] = (char)message[i];
        }

        // Serial.println();

        // Set 0 as string terminator
        buffer[length] = 0;

        // Convert command to string
        String command((char *)buffer);


        // Check if the same command has been received twice, maybe because of MQTT QoS 1
        if(command == current_command)
        {
            // Serial.println("Received same command twice: " + command +
            //                ", assuming it is because of MQTT QoS 1");

            // IMPORTANT: Feedback to this command is not sent since it is assumed its duplicated
            // Otherwise it will be interpreted as the feedback for the firstly received command

            return;
        }

        // Check if the device can currently accept commands
        /*
        if(state != READY)
        {
            // Serial.println("Cannot run command " + command + " since device is in state " +
            //                getStateString(state) + "...");

            sendFeedback(command, "IGNORED");

            return;
        }*/

        // Differentiate commands
        if(command == "1")
        {
            finnish_command = atoi(buffer);
            Serial.println("Received finnish command through mqtt cmd_topic");

        }
        else if(command == "movecup")
        {
            // Serial.println("Command " + command + " accepted, starting...");

            // Save current command
            current_command = command;

            // Enable motor
            if(disable_after_moving) digitalWrite(driver_en, LOW);

            // Move 30º (1/12 rev)
            //stepper.move((long)(steps_per_revolution / 12.0) * turn_direction);
            stepper.move(steps_per_revolution*(45)/360*turn_direction);

            if(id==2)
            {
                // Increase the number of cups on the platform
                cups_on_platform++;
            }
            else
            {
                cups_on_platform--;
            }

            //if(cups_on_platform > 6) cups_on_platform = 6;
        }
        else if(command == "moveorder")
        {
            // Serial.println("Command " + command + " accepted, starting...");

            // Save current command
            current_command = command;

            // Enable motor
            if(disable_after_moving) digitalWrite(driver_en, LOW);

            // Move until end considering the current number of cups on the platform and the 90º
            // (3/12 rev) between the sensors
            float rev_till_end = 1.0 - 3.0 / 12.0 - cups_on_platform / 12.0;

            // Serial.println("Since there are " + String(cups_on_platform + 1) +
            //                " cups on top of the service platform, rotating " +
            //                String(rev_till_end) +
            //                " rev to place the first cup in the retrieval position");

            stepper.move((long)(rev_till_end * steps_per_revolution * turn_direction));
        }
        else
        {
            // Serial.println("Received unknown command " + command + ", ignoring...");

            sendFeedback(command, "IGNORED");

            return;
        }
    }
    /*else if(String(topic) == n_cups_topic)
    {
        // Extract the numer of requested cups
        char buffer[length+1];
        for(int i=0;i<length;i++)
        {
            buffer[i]=(char)message[i];
        }
        buffer[length] = 0;
        desired_cups = atoi(buffer);
        cups_on_platform = desired_cups;
        Serial.println("Received number of cups: ");
        Serial.println(desired_cups);
    }*/
    else if(String(topic) == refill_topic)
    {
        // Extract button information
        char buffer[length+1];
        for(int i=0;i<length;i++)
        {
            buffer[i]=(char)message[i];
        }
        buffer[length] = 0;
        refill_button = atoi(buffer);
        Serial.println("Refill button: ");
        Serial.println(refill_button);
    }/*
    else if(String(topic) == finnished_order)
    {
        // Extract information
        char buffer[length+1];
        for(int i=0;i<length;i++)
        {
            buffer[i]=(char)message[i];
        }
        buffer[length] = 0;
        finnish_command = atoi(buffer);
        Serial.println("Finnished order: ");
        Serial.println(finnish_command);

    }*/
    else if(String(topic) == update_topic)
    {
        Serial.println("Requested OTA update");

        // Sync SPIFFS files with the OTAdrive server
        if(SPIFFS.begin())
        {
            Serial.println("Syncronyzing resources...");
            OTADRIVE.setFileSystem(&SPIFFS);
            OTADRIVE.syncResources();
            SPIFFS.end();
            Serial.println("Resources updated");
        }

        // Retrieve firmware info from OTAdrive server
        auto inf = OTADRIVE.updateFirmwareInfo();

        // Update firmware if newer available
        if(inf.available)
        {
            Serial.println("New version available");
            Serial.println("Size: ");
            Serial.println(inf.size);
            Serial.println("Version: ");
            Serial.println(inf.version.c_str());

            OTADRIVE.updateFirmware();

            Serial.println("Updated!");
        }
        else
        {
            Serial.println("No newer version");
        }

        // Reboot ESP
        ESP.restart();
    }

}

// Send device state through mqtt topic
void sendState(String msg)
{
    // Filter quick state changes
    if(millis() - time_last_msg_state > 500)
    {
        client.publish(state_topic.c_str(), msg.c_str());
        // Serial.println(state_topic + " sent: " + msg);
        // Update time last state msg has been sent
        time_last_msg_state = millis();
    }
}

void sendState(int state)
{
    // Filter quick state changes
    if(millis() - time_last_msg_state > 500)
    {
        // Get the a state string depending on the current state
        String state_msg = getStateString(state);

        client.publish(state_topic.c_str(), state_msg.c_str());
        // Serial.println(state_topic + " sent: " + state_msg);

        // Update time last state msg has been sent
        time_last_msg_state = millis();
    }
}


// Get state string msg
String getStateString(int state)
{
        // Get the a state string depending on the current state
        switch(state)
        {
            case READY:
                return "READY";
            case WAITING_FOR_TAKE:
                return "WAITING_FOR_TAKE";
            case WAITING_FOR_DROP:
                return "WAITING_FOR_DROP";
            case ROTATE:
                return "ROTATE";
            case REFILL:
                return "REFILL";
            case ROTATING_TO_LIMIT:
                return "ROTATING_TO_LIMIT";
            case ERROR:
                return "ERROR";
            default:
                return "UNEXPECTED";
        }

}

// Send feedback through mqtt topic
void sendFeedback(String command, String feedback)
{
    String msg = command + ">" + feedback;
    client.publish(feedback_topic.c_str(), msg.c_str());
    // Serial.println("Sending feedback: " + msg);
}

// Send sensor data through mqtt topic
void sendSensorData(float distanceCm)
{
    // Filter quick sensor changes
    if(millis() - time_last_msg_sensor > 500)
    {
        // Check if the MQTT client is connected before sending data
        if (client.connected()) 
        {
            StaticJsonDocument<100> doc;
            doc["distance_cm"] = distanceCm;  // Add sensor data to JSON document
            
            char buffer[128];
            size_t n = serializeJson(doc, buffer); // Serialize JSON to buffer
            /*
            // Publish the sensor data to the MQTT topic
            if (client.publish(sensor_topic.c_str(), buffer, n)) {
                //Serial.println("Sensor data sent successfully");
            } else {
                Serial.println("Failed to send sensor data");
            }*/

            // Publish the distanceCm to the medida_sensor topic
            if (client.publish(medida_sensor.c_str(), String(distanceCm).c_str())) {
                //Serial.println("Distance data sent successfully");
            } else {
                Serial.println("Failed to send distance data");
            }

            // Update time last sensor data was sent
            time_last_msg_sensor = millis(); // Reset timer
        } 
        else 
        {
            Serial.println("MQTT client not connected. Unable to send sensor data.");
        }
    }
}

// Load local config json file
void loadConfig()
{
    if(!SPIFFS.begin())
    {
        Serial.println("An Error has occurred while mounting filesystem");

        
        //useDefaultConfig();
        return;

    }
    else
    {
        File configFile = SPIFFS.open("/config.json", "r");

        if(!configFile)
        {
            Serial.println("An Error has occurred while reading config file");

            useDefaultConfig();
        }
        else
        {
            JsonDocument config;

            DeserializationError result = deserializeJson(config, configFile);

            if(result.c_str() != "Ok")
            {
                Serial.print(F("Deserialization of the config json failed with code "));
                Serial.println(result.f_str());

                useDefaultConfig();
            }
            else
            {
                // Get a reference to the root object
                JsonObject obj = config.as<JsonObject>();

                // Loop through all the key-value pairs in obj
                Serial.println("Available keys in json obj: ");    // is a JsonString
                for(JsonPair p : obj)
                    Serial.println(p.key().c_str());    // is a JsonString

                ssid = config["ssid"].as<String>();
                if(ssid != nullptr)
                {
                    Serial.print("Loaded config for ssid: ");
                    Serial.println(ssid);
                }
                else
                {
                    Serial.println("No SSID value found in config file");
                    exit(1);
                    
                    /*Serial.println("Could not find ssid config");

                    platform->wifi.ssid = "MaccoKA5010";

                    Serial.print("Using default config for ssid: ");
                    Serial.println(platform->wifi.ssid);*/
                }

                password = config["password"].as<String>();
                if(password != nullptr)
                {
                    Serial.print("Loaded config for wifi password: ");
                    Serial.println(password);
                }
                else
                {
                    Serial.println("Could not find wifi password config");

                    password = "Xv36Myf1";

                    Serial.print("Using default config for wifi password: ");
                    Serial.println(password);
                }

                mqtt_server = config["mqtt_server"].as<String>();
                if(mqtt_server != nullptr)
                {
                    Serial.print("Loaded config for mqtt_server: ");
                    Serial.println(mqtt_server);
                }
                else
                {
                    Serial.println("Could not find mqtt_server config");

                    mqtt_server = "192.168.10.187";

                    Serial.print("Using default config for mqtt_server: ");
                    Serial.println(mqtt_server);
                }

                client_name = config["client_name"].as<String>();
                if(client_name != "null")
                {
                    Serial.print("Loaded config for client_name: ");
                    Serial.println(client_name);
                }
                else
                {
                    Serial.println("Could not find client_name config");

                    client_name = "sp_empty_left_down";

                    Serial.print("Using default config for client_name: ");
                    Serial.println(client_name);
                }

                kiosk_name = config["kiosk_name"].as<String>();
                if(kiosk_name != "null")
                {
                    Serial.print("Loaded config for kiosk_name: ");
                    Serial.println(kiosk_name);
                }
                else
                {
                    Serial.println("Could not find kiosk_name config");

                    kiosk_name = "kiosk";

                    Serial.print("Using default config for kiosk_name: ");
                    Serial.println(kiosk_name);
                }

                id = config["id"];
                if(id != 0)
                {
                    Serial.print("Loaded config for id: ");
                    Serial.println(id);
                }
                else
                {
                    Serial.println("Could not find id config, using default id = 1");
                    id = 1;
                }

                period_mqtt_loop = config["period_mqtt_loop"];
                if(period_mqtt_loop != 0)
                {
                    Serial.print("Loaded config for period_mqtt_loop: ");
                    Serial.println(period_mqtt_loop);
                }
                else
                {
                    Serial.println("Could not find period_mqtt_loop config");

                    period_mqtt_loop = 100;    // [ms]

                    Serial.print("Using default config for period_mqtt_loop: ");
                    Serial.println(period_mqtt_loop);
                }

                period_state_pub = config["period_state_pub"];
                if(period_state_pub != 0)
                {
                    Serial.print("Loaded config for period_state_pub: ");
                    Serial.println(period_state_pub);
                }
                else
                {
                    Serial.println("Could not find period_state_pub config");

                    period_state_pub = 15000;    // [ms]

                    Serial.print("Using default config for period_state_pub: ");
                    Serial.println(period_state_pub);
                }

                period_sensor_pub = config["period_sensor_pub"];
                if(period_sensor_pub != 0)
                {
                    Serial.print("Loaded config for period_sensor_pub: ");
                    Serial.println(period_sensor_pub);
                }
                else
                {
                    Serial.println("Could not find period_sensor_pub config");

                    period_sensor_pub = 15000;    // [ms]

                    Serial.print("Using default config for period_sensor_pub: ");
                    Serial.println(period_sensor_pub);
                }

                max_speed = config["max_speed"];
                if(max_speed != 0)
                {
                    Serial.print("Loaded config for max_speed: ");
                    Serial.println(max_speed);
                }
                else
                {
                    Serial.println("Could not find max_speed config");

                    max_speed = 1500;    // [rev/min]

                    Serial.print("Using default config for max_speed: ");
                    Serial.println(max_speed);
                }

                max_acc = config["max_acc"];
                if(max_acc != 0)
                {
                    Serial.print("Loaded config for max_acc: ");
                    Serial.println(max_acc);
                }
                else
                {
                    Serial.println("Could not find max_acc config");

                    max_acc = 200;    // [rev/min2]

                    Serial.print("Using default config for max_acc: ");
                    Serial.println(max_acc);
                }
                /*
                turn_direction = config["turn_direction"];
                if(turn_direction != 0)
                {
                    Serial.print("Loaded config for turn_direction: ");
                    Serial.println(turn_direction);

                    if(abs(turn_direction) != 1)
                    {
                        Serial.print("Wrong value of turn direction, only 1 or -1 allowed. Using "
                                     "only sign...");
                        turn_direction = (turn_direction >= 0) - (turn_direction < 0);
                    }
                }
                else
                {
                    Serial.println("Could not find turn_direction config");

                    turn_direction = 1;    // 1 clockwise, -1 counterclockwise

                    Serial.print("Using default config for turn_direction: ");
                    Serial.println(turn_direction);
                }
                */

                String disable = config["disable_after_moving"].as<String>();
                if(disable != "null")
                {
                    if(disable == "true")
                        disable_after_moving = true;
                    else
                        disable_after_moving = false;

                    Serial.print("Loaded config for disable_after_moving: ");
                    Serial.println(disable_after_moving);
                }
                else
                {
                    Serial.println("Could not find disable_after_moving config");

                    disable_after_moving = false;

                    Serial.print("Using default config for disable_after_moving: ");
                    Serial.println(disable_after_moving);
                }

                take_platform_delay = config["take_platform_delay"];
                if (take_platform_delay != 0)
                {
                    Serial.print("Loaded config for take_platform_delay: ");
                    Serial.println(take_platform_delay);
                }
                else
                {
                    Serial.println("Could not find take_platform_delay config");
                    take_platform_delay = 5000; // Valor predeterminado
                    Serial.print("Using default config for take_platform_delay: ");
                    Serial.println(take_platform_delay);
                }

                serve_platform_delay = config["serve_platform_delay"];
                if (serve_platform_delay != 0)
                {
                    Serial.print("Loaded config for serve_platform_delay: ");
                    Serial.println(serve_platform_delay);
                }
                else
                {
                    Serial.println("Could not find serve_platform_delay config");
                    serve_platform_delay = 8000; // Valor predeterminado
                    Serial.print("Using default config for serve_platform_delay: ");
                    Serial.println(serve_platform_delay);
                }
            }
        }
    }

    // Unmount filesystem
    SPIFFS.end();
}

// Set default config, used if config setup fails
void useDefaultConfig()
{
     Serial.println("Using default config: ");

    ssid = "MOVISTAR_4208";
    password = "m22KgLdyy2hUxbX5Y2Ca";
    mqtt_server = "192.168.1.39";
    client_name = "sp_empty_left_down";
    kiosk_name = "kiosk";
    id = 1;
    period_mqtt_loop = 100;      // [ms]
    period_state_pub = 15000;    // [ms]
    period_sensor_pub = 15000;
    max_speed = 1500;               // [rev/min]
    max_acc = 200;                 // [rev/min2]
    disable_after_moving = false;
    take_platform_delay = 5000;
    serve_platform_delay = 8000;


    Serial.print("ssid ");
    Serial.println(ssid);

    Serial.print("password ");
    Serial.println(password);

    Serial.print("mqtt_server ");
    Serial.println(mqtt_server);

    Serial.print("client_name ");
    Serial.println(client_name);

    Serial.print("kiosk_name ");
    Serial.println(kiosk_name);

    Serial.print("period_mqtt_loop ");
    Serial.println(period_mqtt_loop);

    Serial.print("period_state_pub ");
    Serial.println(period_state_pub);

    Serial.print("period_sensor_pub ");
    Serial.println(period_sensor_pub);


    Serial.print("max_speed ");
    Serial.println(max_speed);

    Serial.print("max_acc ");
    Serial.println(max_acc);

    Serial.print("turn_direction ");
    Serial.println(turn_direction);

    Serial.print("disable_after_moving ");
    Serial.println(disable_after_moving);
}

// Connect to wifi
/*
void wifiConnect()
{
    // Attempt connection to WiFi network

    Serial.println();
    Serial.print("Attempting connection to WiFi with ssid: ");
    Serial.println(ssid);

    Serial.print("with pass: ");
    Serial.println(password);

    // Save init time of wifi setup for restart watchdog
    unsigned long int time_init_wifi_setup = millis();

    WiFi.begin(ssid.c_str(), password.c_str());

    while(WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");

        // If it doesn't connect before x seconds, the esp32 will be restarted
        if(millis() - time_init_wifi_setup > wifi_timeout)
        {
            Serial.println();
            Serial.println("WiFi network not reachable, restarting...");
            ESP.restart();
        }
    }

    Serial.println("");
    Serial.println("WiFi correctly connected!");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    // Make WiFi reconnect automatically in case of disconnection
    WiFi.setAutoReconnect(true);
}*/

// Connect to mqtt broker

void mqttConnect()
{
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
            //client.subscribe(n_cups_topic.c_str(), 1); // QoS 1
            client.subscribe(refill_topic.c_str(), 1);    // QoS 1
            //client.subscribe(finnished_order.c_str(), 1);    // QoS 1
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


// OTA send update progress through serial
void onUpdateProgress(int progress, int totalt)
{
    static int last = 0;
    int progressPercent = (100 * progress) / totalt;
    Serial.print("*");
    if(last != progressPercent && progressPercent % 10 == 0)
    {
        // print every 10%
        Serial.printf("%d", progressPercent);
    }
    last = progressPercent;
}

void IRAM_ATTR stopMotor()
{

    limitReached = true;
}

void configureDriver()
{
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