#ifndef WIFI_CONECTION_H
#define WIFICONNECTION_H

#include <WiFi.h>
#include <Arduino.h>
#include <EEPROM.h>


class WifiConnection {

    public:
        WifiConnection(String ssid, String password, int wifi_timeout);
        void wifiConnect();

    private:
        // WiFi Config
        String ssid;
        String password;
        int wifi_timeout=8000;    // Time to restart if wifi does not connect [ms]
};


#endif