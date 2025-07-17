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

#include "PlatformStateMachine.h"
#include "Platform.h"
#include <vector>


Platform platform;
std::vector<int> scan_results;

void setup() {

    Serial.begin(9600);
    SERIAL_PORT.begin(9600);
    delay(1000);

    loadConfig();

    platform.attachPlatform(config.id, config.n_cups, config.turn_direction, config.max_speed, config.max_acc, config.disable_after_moving);
    scan_results.assign(config.n_cups, 0); // Initialize scan results with zeros
    
    platform.begin();


    WifiConnection wifi(ssid,password,wifi_timeout);
    wifi.wifiConnect();

    // Publishers
    state_topic = kiosk_name + "/" + client_name + "/platform/" + platform.id + "/state";
    feedback_topic = kiosk_name + "/" + client_name + "/platform/" + platform.id + "/feedback";
    medida_sensor = kiosk_name + "/" + client_name + "/platform/" + platform.id + "/medida_sensor";
    // Subscribers
    cmd_topic = kiosk_name + "/" + client_name + "/platform/" + platform.id + "/cmd";
    update_topic = kiosk_name + "/" + client_name + "/platform/" + platform.id + "/update";
    refill_topic = kiosk_name + "/" + client_name + "/platform/" + platform.id + "/refill_button";

    // Setup mqtt
    client.setServer(mqtt_server.c_str(), 1883);

    mqttConnect();
    //mqtt.mqttConnect();

    // Set callback for mqtt messages
    client.setCallback(callback);

    // Perform initial calibration
    platform.calibrate();

    delay(1000); 

    state = INIT;
    sendState(state);
    Serial.println("State " + getStateString(state));

    

    // OTA setup
    OTADRIVE.setInfo(APIKEY, FW_VER);
    OTADRIVE.onUpdateFirmwareProgress(onUpdateProgress);

}

void loop() {

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
    platform.previousCup = platform.cup;  // Guardamos el estado anterior
    platform.cup = platform.updateCupPresence();  // Leemos el nuevo estado


    // Ultrasonic sensor measurement
    if (millis() - time_last_msg_sensor > period_sensor_pub) {
        sendSensorData(platform.distanceCm);
        time_last_msg_sensor = millis();
    }

    
    // State Machines
    if(platform.id==1)
    {
        switch(state)
        {
            case INIT:
                //Serial.println("Platform is initializing");
                //Serial.print("ACTUAL POSITION AAAAAAAA: ");
                //Serial.println(ActualPosition);
                platform.configureDriver();
                platform.cup = false;
                platform.previousCup = false;
                state = CHECK;
                sendState(state);
                Serial.println("New state " + getStateString(state));
                break;
            case READY:
                //Serial.print("ACTUAL POSITION AAAAAAAA: ");
                //Serial.println(ActualPosition);
                if(platform.cup)
                {
                    Serial.print("Number of cups: ");
                    Serial.println(platform.cups_on_platform);
                    Serial.println("CUP");
                    state = WAITING_FOR_TAKE;
                    sendState(state);
                    Serial.println("New state " + getStateString(state));

                    // Check if the stepper is moving
                    if(platform.stepper.isRunning())
                    {
                        //Serial.println("It is running");
                        float rev_to_go = (float)(platform.stepper.distanceToGo()) / platform.steps_per_revolution;

                        if(current_command == "moveorder" && rev_to_go < (1.0 / 12.0))
                        {
                            //cups_on_platform = 10;
                            sendFeedback(current_command, "FINISHED");
                            current_command = "none";
                        }
                        else
                        {
                            // Save distance to go in case of error
                            platform.last_distance_to_go = platform.stepper.distanceToGo();
                        }

                        // Stop the motor
                        platform.stepper.stop();
                        if (platform.disable_after_moving) digitalWrite(platform.driver_en, HIGH);
                    }
                }
                if(ActualPosition < scan_results.size() && scan_results[ActualPosition] == 0) {
                    Serial.println("There is no cup, moving to next position");
                    if(platform.disable_after_moving) digitalWrite(platform.driver_en, LOW);
                    platform.configureDriver();
                    platform.moveToNextCup();  // inicia el movimiento
                    state = ROTATE;
                    sendState(state);
                    Serial.println("New state " + getStateString(state));
                }

                break;
            case CHECK:
                //Serial.print("ACTUAL POSITION AAAAAAAA: ");
                //Serial.println(ActualPosition);
                // Check if the platform is in the initial position, if it is pressing the limit switch
                if(platform.limitReached || digitalRead(Platform::limitSwitchPin) == HIGH)
                {
                    Serial.println("Platform is in the initial position");
                    delay(1000);  // Espera para evitar movimientos inmediatos
                    platform.scan(scan_results.data());
                    state = READY;
                    sendState(state);
                    Serial.println("New state " + getStateString(state));
                }
                else
                {
                    Serial.println("Platform is not in the initial position");
                    state = ROTATING_TO_LIMIT;
                    sendState(state);
                    Serial.println("New state " + getStateString(state));
                }

                //Serial.print("ACTUAL POSITION AAAAAAAA: ");
                //Serial.println(ActualPosition);

                break;
            case WAITING_FOR_TAKE:
                digitalWrite(platform.driver_en, LOW);
                //driver.ihold(16);
                platform.configureDriver();
                platform.driver.freewheel(0); 

                if(!platform.cup && platform.previousCup)
                {
                    platform.driver.freewheel(1);
                    digitalWrite(platform.driver_en, HIGH);
                    state = ROTATE;
                    sendState(state);
                    Serial.println("New state " + getStateString(state));

                    // Set the current command to "movecup" to move the platform to the next position
                    current_command = "movecup";

                    if(platform.cups_on_platform >0)
                    {
                        platform.cups_on_platform--;
                    }

                    if(platform.cups_on_platform == 0)
                    {
                        state = REFILL;
                        sendState(state);
                        Serial.println("New state " + getStateString(state));
                    }
                    else
                    {
                        // Enable motor and move the stepper
                        delay(platform.take_platform_delay);
                        if(platform.disable_after_moving) digitalWrite(platform.driver_en,LOW);
                        platform.configureDriver();
                        platform.moveToNextCup();
                    }
                    platform.previousCup = platform.cup; // Update previous cup state
                }
                break;
            case ROTATE:
                if(!platform.stepper.isRunning()) {
                    ActualPosition++;
                    //Serial.print("ACTUAL POSITION AAAAAAAA: ");
                    //Serial.println(ActualPosition);
                    state = READY;
                    sendState(state);
                    Serial.println("New state " + getStateString(state));

                    if(platform.disable_after_moving) digitalWrite(platform.driver_en, HIGH);

                    if(current_command == "movecup") {
                        sendFeedback(current_command, "FINISHED");
                        current_command = "none";
                    }
                }
                    break;
            case REFILL:
                state = REFILL;
                sendState(state);
                Serial.println("New state " + getStateString(state));
                if(platform.refill_button==1 && platform.previous_refill_button==0)
                {
                    Serial.println("Refill button pressed");
                    // Refill cups
                    platform.startRefillCycle();

                    state = CHECK;
                    sendState(state);
                    Serial.println("New state " + getStateString(state));

                }

                // Update previous refill button state
                platform.previous_refill_button = platform.refill_button;
                platform.refill_button = 0;

                break;
            case ROTATING_TO_LIMIT:
                // Move the platform to the limit switch
                Platform::RotationStatus status;
                status = platform.calibrate();
                switch (status)
                {
                    case Platform::LIMIT_REACHED:
                        state = CHECK;
                        sendState(state);
                        Serial.println("New state " + getStateString(state));
                        ActualPosition = 0; // Reset actual position

                        break;

                    case Platform::ROTATION_ERROR:
                        state = ERROR;
                        sendState(state);
                        Serial.println("New state " + getStateString(state));
                        break;
                    case Platform::ROTATION_TIMEOUT:
                            state = ERROR;
                            sendState(state);
                            Serial.println("New state " + getStateString(state));
                            break;
                    case Platform::ROTATING:
                        // Todavía rotando, no hacer nada.
                        break;
                }
                break;
            case ERROR:
                Serial.println("Platform is in ERROR state. Waiting for MQTT command to reset.");
                /*
                // Wait for MQTT command to reset the state
                if(current_command == "init")
                {
                    Serial.println("Resetting platform state from ERROR to INIT");
                    state = INIT;
                    sendState(state);
                    Serial.println("New state " + getStateString(state));
                    current_command = "none";
                }*/
                break;
        }
    }

    // Platform 2 state machine (sin probar)
    if(platform.id==2)
    {
        switch(state)
        {
            case INIT:
                Serial.println("Platform is initializing");
                platform.configureDriver();
                platform.cup = false;
                platform.previousCup = false;
                state = CHECK;
                sendState(state);
                Serial.println("New state " + getStateString(state));
                break;
            case READY:
                if(!platform.cup)
                {
                    if(cupConfirmationStartTime == 0) 
                    {
                        cupConfirmationStartTime = millis(); // Start the timer
                    } 

                    if (millis() - cupConfirmationStartTime > cupConfirmationDelay) 
                    {
                        // If the cup is still not detected after the delay, update the state
                        Serial.println("No cup detected after confirmation delay");
                        digitalWrite(Platform::driver_en, LOW);
                        platform.configureDriver();
                        platform.driver.freewheel(0); // Freewheel the driver
                        Serial.print("Number of cups: ");
                        Serial.println(platform.cups_on_platform);
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
                    cupConfirmationStartTime = 0; // Reset the timer if a cup is detected
                }
                
                break;
            case WAITING_FOR_DROP:
                digitalWrite(Platform::driver_en, LOW);
                platform.configureDriver();
                platform.driver.freewheel(0); // Freewheel the driver
                if(finnish_command == 1)
                {
                    platform.driver.freewheel(1); // Freewheel the driver
                    state = ROTATE;
                    sendState(state);
                    Serial.println("New state " + getStateString(state));

                    current_command = "movecup";

                    delay(platform.serve_platform_delay);
                    if(platform.disable_after_moving) digitalWrite(Platform::driver_en, HIGH);
                    platform.configureDriver();
                    platform.moveToNextCup(); // Move to the next cup position
                    
                    finnish_command = 0; // Reset the command flag
                }
                break;
            case ROTATE:
                if(!platform.stepper.isRunning()) {
                    state = READY;
                    sendState(state);
                    Serial.println("New state " + getStateString(state));

                    if(platform.disable_after_moving) digitalWrite(platform.driver_en, HIGH);

                    if(current_command == "movecup") {
                        sendFeedback(current_command, "FINISHED");
                        current_command = "none";
                    }
                }
                break;
            case ERROR:
                Serial.println("Platform is in ERROR state. Waiting for MQTT command to reset.");
                break;
        }
    }

    if(platform.id==3)
    {
        switch(state)
        {
            case INIT:
                //Serial.println("Platform is initializing");
                //Serial.print("ACTUAL POSITION AAAAAAAA: ");
                //Serial.println(ActualPosition);
                platform.configureDriver();
                platform.cup = false;
                platform.previousCup = false;
                state = CHECK;
                sendState(state);
                Serial.println("New state " + getStateString(state));
                break;
            case READY:
                break;
            case CHECK:
                break;
            case WAITING_FOR_TAKE:
                break;
            case WAITING_FOR_DROP:
                break;
            case WAITING_FOR_ORDER:
                break;
            case ROTATE:
                break;
            case REFILL:
                break;
            case ROTATING_TO_LIMIT:
                break;
            case ERROR:
                break;
        }
    }

    // Keep stepper running outside of the state machine
    if(platform.stepper.isRunning()) {
        bool still_running = platform.stepper.run();
        if(!still_running) {
            Serial.println("Movement finished");
            if(platform.disable_after_moving) digitalWrite(Platform::driver_en, HIGH);

            if(current_command != "none" && platform.last_distance_to_go == 0) {
                Serial.println("Finished command " + current_command);

                if(current_command == "moveorder" && platform.id == 1) platform.cups_on_platform = 3;
                if(current_command == "moveorder" && platform.id == 2) platform.cups_on_platform = 0;

                sendFeedback(current_command, "FINISHED");
                current_command = "none";
            }
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
            JsonDocument doc;

            DeserializationError result = deserializeJson(doc, configFile);
            if(result.c_str() != "Ok")
            {
                Serial.print(F("Deserialization of the config json failed with code "));
                Serial.println(result.f_str());

                useDefaultConfig();
            }
            else
            {
                // Get a reference to the root object
                JsonObject obj = doc.as<JsonObject>();

                // Loop through all the key-value pairs in obj
                Serial.println("Available keys in json obj: ");    // is a JsonString
                for(JsonPair p : obj)
                    Serial.println(p.key().c_str());    // is a JsonString

                ssid = doc["ssid"].as<String>();
                if(ssid != nullptr)
                {
                    Serial.print("Loaded config for ssid: ");
                    Serial.println(ssid);
                }
                else
                {
                    Serial.println("No SSID value found in config file");
                    exit(1);
                }

                password = doc["password"].as<String>();
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

                mqtt_server = doc["mqtt_server"].as<String>();
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

                client_name = doc["client_name"].as<String>();
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

                kiosk_name = doc["kiosk_name"].as<String>();
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

                config.id = doc["id"];
                if(config.id != 0)
                {
                    Serial.print("Loaded config for id: ");
                    Serial.println(config.id);
                }
                else
                {
                    Serial.println("Could not find id config, using default id = 1");
                    config.id = 1;
                }

                config.n_cups = doc["n_cups"] | 8;
                Serial.print("Loaded config for n_cups: ");
                Serial.println(config.n_cups);

                period_mqtt_loop = doc["period_mqtt_loop"];
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

                period_state_pub = doc["period_state_pub"];
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

                period_sensor_pub = doc["period_sensor_pub"];
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

                config.max_speed = doc["max_speed"];
                if(config.max_speed != 0)
                {
                    Serial.print("Loaded config for max_speed: ");
                    Serial.println(config.max_speed);
                }
                else
                {
                    Serial.println("Could not find max_speed config");

                    config.max_speed = 1500;    // [rev/min]

                    Serial.print("Using default config for max_speed: ");
                    Serial.println(config.max_speed);
                }

                config.max_acc = doc["max_acc"];
                if(config.max_acc != 0)
                {
                    Serial.print("Loaded config for max_acc: ");
                    Serial.println(config.max_acc);
                }
                else
                {
                    Serial.println("Could not find max_acc config");

                    config.max_acc = 200;    // [rev/min2]

                    Serial.print("Using default config for max_acc: ");
                    Serial.println(config.max_acc);
                }
                config.turn_direction = doc["turn_direction"];
                if(config.turn_direction != 0)
                {
                    Serial.print("Loaded config for turn_direction: ");
                    Serial.println(config.turn_direction);

                    if(abs(config.turn_direction) != 1)
                    {
                        Serial.print("Wrong value of turn direction, only 1 or -1 allowed. Using "
                                     "only sign...");
                        config.turn_direction = (config.turn_direction >= 0) - (config.turn_direction < 0);
                    }
                }
                else
                {
                    Serial.println("Could not find turn_direction config");

                    config.turn_direction = 1;    // 1 clockwise, -1 counterclockwise
                    

                    Serial.print("Using default config for turn_direction: ");
                    Serial.println(config.turn_direction);
                }

                String disable = doc["disable_after_moving"].as<String>();
                if(disable != "null")
                {
                    if(disable == "true")
                        config.disable_after_moving = true;
                    else
                        config.disable_after_moving = false;

                    Serial.print("Loaded config for disable_after_moving: ");
                    Serial.println(config.disable_after_moving);
                }
                else
                {
                    Serial.println("Could not find disable_after_moving config");

                    config.disable_after_moving = false;

                    Serial.print("Using default config for disable_after_moving: ");
                    Serial.println(config.disable_after_moving);
                }

                platform.take_platform_delay = doc["take_platform_delay"];
                if (platform.take_platform_delay != 0)
                {
                    Serial.print("Loaded config for take_platform_delay: ");
                    Serial.println(platform.take_platform_delay);
                }
                else
                {
                    Serial.println("Could not find take_platform_delay config");
                    platform.take_platform_delay = 5000; // Valor predeterminado
                    Serial.print("Using default config for take_platform_delay: ");
                    Serial.println(platform.take_platform_delay);
                }

                platform.serve_platform_delay = doc["serve_platform_delay"];
                if (platform.serve_platform_delay != 0)
                {
                
                    Serial.print("Loaded config for serve_platform_delay: ");
                    Serial.println(platform.serve_platform_delay);
                }
                else
                {
                    Serial.println("Could not find serve_platform_delay config");
                    platform.serve_platform_delay = 8000; // Valor predeterminado
                    Serial.print("Using default config for serve_platform_delay: ");
                    Serial.println(platform.serve_platform_delay);
                }

                if (doc["bottles"].is<JsonArray>()) {
                JsonArray arr = doc["bottles"].as<JsonArray>();
                for(int i=0; i<arr.size() && i<NUM_BOTTLES; i++) {
                    bottles[i].bottle_name = arr[i]["name"].as<String>();
                    bottles[i].pos = arr[i]["position"];
                    bottles[i].isBottle = true;
                    bottles[i].isEmpty = false;
                }
                Serial.println("Loaded bottles configuration:");}
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
    config.id = 1;
    config.n_cups = 8;
    period_mqtt_loop = 100;      // [ms]
    period_state_pub = 15000;    // [ms]
    period_sensor_pub = 15000;
    config.max_speed = 1500;               // [rev/min]
    config.max_acc = 200;                 // [rev/min2]
    config.disable_after_moving = false;
    platform.take_platform_delay = 5000;
    platform.serve_platform_delay = 8000;


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
    Serial.println(platform.max_speed);

    Serial.print("max_acc ");
    Serial.println(platform.max_acc);

    Serial.print("turn_direction ");
    Serial.println(platform.turn_direction);

    Serial.print("disable_after_moving ");
    Serial.println(platform.disable_after_moving);
}

// MQTT subscription callback
void callback(char * topic, byte * message, unsigned int length)
{
    String topicStr = String(topic);

    // Extract command/message
    char buffer[length + 1];
    for(int i = 0; i < length; i++) {
        buffer[i] = (char)message[i];
    }
    buffer[length] = '\0';
    String command(buffer);

    Serial.println("Message received on topic: " + topicStr + " | Command: " + command);

    // --- COMMAND TOPIC ---
    if(topicStr == cmd_topic)
    {
        // Special command: init to reset state machine
        if (command == "init") {
            Serial.println("MQTT command INIT: resetting to INIT state.");
            state = INIT;
            sendState(state);
            return;
        }

        // Special command: init to reset state machine

        if (command == "calib") {
            Serial.println("MQTT command CALIB: starting calibration");
            platform.calibrate();
            sendFeedback("calib", "DONE");
            return;
        }
        if (command == "scan") {
            Serial.println("MQTT command SCAN: scanning cups");
            platform.scan(scan_results.data());
            String msg = "";
            for(size_t i=0;i<scan_results.size();++i){
                msg += String(scan_results[i]);
                if(i < scan_results.size()-1) msg += ",";
            }
            sendFeedback("scan", msg);
            return;
        }


        // Avoid processing the same command twice
        if(command == current_command) {
            Serial.println("Duplicate command received (QoS 1 duplicate): " + command);
            return;
        }

        // Check for bottle request
        for(int i=0; i<NUM_BOTTLES; i++) {
            if(bottles[i].bottle_name == command && bottles[i].isBottle) {
                platform.requested_bottle = command;
                platform.bottle_request_pending = true;
                Serial.print("Bottle request received: ");
                Serial.println(command);
                return;
            }
        }

        // Process platform commands
        if(command == "1") {
            finnish_command = atoi(buffer);
            Serial.println("Received finish command");
        }
        else if(command == "movecup") {
            current_command = command;

            if(platform.disable_after_moving) digitalWrite(platform.driver_en, LOW);
            platform.moveToNextCup();

            if(platform.id == 2) {
                platform.cups_on_platform++;
            } else {
                platform.cups_on_platform--;
            }
            Serial.println("Executed movecup command");
        }
        else if(command == "moveorder") {
            current_command = command;

            if(platform.disable_after_moving) digitalWrite(platform.driver_en, LOW);

            float rev_till_end = 1.0 - 3.0 / 12.0 - platform.cups_on_platform / 12.0;
            platform.stepper.move((long)(rev_till_end * platform.steps_per_revolution * platform.turn_direction));

            Serial.println("Executed moveorder command");
        }
        else {
            Serial.println("Unknown command: " + command);
            sendFeedback(command, "IGNORED");
        }
    }

    // --- REFILL TOPIC ---
    else if(topicStr == refill_topic)
    {
        platform.refill_button = atoi(buffer);
        Serial.println("Refill button state: " + String(platform.refill_button));
    }

    // --- UPDATE TOPIC for OTA ---
    else if(topicStr == update_topic)
    {
        Serial.println("Requested OTA update");

        if(SPIFFS.begin()) {
            Serial.println("Synchronizing resources...");
            OTADRIVE.setFileSystem(&SPIFFS);
            OTADRIVE.syncResources();
            SPIFFS.end();
            Serial.println("Resources updated");
        }

        auto inf = OTADRIVE.updateFirmwareInfo();
        if(inf.available) {
            Serial.println("New firmware version available: " + String(inf.version.c_str()));
            OTADRIVE.updateFirmware();
            Serial.println("Firmware updated!");
        } else {
            Serial.println("No newer firmware version available.");
        }

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
            case INIT:
                return "INIT";
            case CHECK:
                return "CHECK";
            case WAITING_FOR_ORDER:
                return "WAITING_FOR_ORDER";
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
            JsonDocument doc;
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