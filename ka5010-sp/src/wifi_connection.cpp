#include <wifi_connection.h>


WifiConnection::WifiConnection(String ssid, String password, int wifi_timeout){

    this-> ssid=ssid;
    this-> password=password;
    this-> wifi_timeout=wifi_timeout;
}

void WifiConnection::wifiConnect(){
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
}
