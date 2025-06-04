// WIFI_AP settings.
const char* AP_SSID = "ESP32_DEV";
const char* AP_PWD  = "12345678";

// WIFI_STA settings.
const char* STA_SSID = "MOVISTAR_4208";
const char* STA_PWD  = "m22KgLdyy2hUxbX5Y2Ca";

// the MAC address of the device you want to ctrl.
//uint8_t broadcastAddress[] = {0x08, 0x3A, 0xF2, 0x93, 0x5F, 0xA8};
// uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t broadcastAddress[] = {0xCC, 0x7B, 0x5C, 0xBC, 0x5C, 0x68};

typedef struct struct_message {
  int ID_send;
  int POS_send;
  int Spd_send;
} struct_message;

// Create a struct_message called myData
struct_message myData;


// set the default role here.
// 0 as normal mode.
// 1 as leader, ctrl other device via ESP-NOW.
// 2 as follower, can be controled via ESP-NOW.
#define DEFAULT_ROLE 0


// set the default wifi mode here.
// 1 as [AP] mode, it will not connect other wifi.
// 2 as [STA] mode, it will connect to know wifi.
#define DEFAULT_WIFI_MODE 1

// the uart used to control servos.
// GPIO 18 - S_RXD, GPIO 19 - S_TXD, as default.
#define S_RXD 18
#define S_TXD 19

// the IIC used to control OLED screen.
// GPIO 21 - S_SDA, GPIO 22 - S_SCL, as default.
#define S_SCL 22
#define S_SDA 21

// the GPIO used to control RGB LEDs.
// GPIO 23, as default.
#define RGB_LED   23
#define NUMPIXELS 10

// set the max ID.
int MAX_ID = 253;

// modeSelected.
// set the SERIAL_FORWARDING as true to control the servos with USB.
bool SERIAL_FORWARDING = true;

// OLED Screen Dispaly.
// Row1: MAC address.
// Row2: VCC --- IP address.
// Row3: MODE:Leader/Follower  [AP]/[STA][RSSI]
//       DEFAULT_ROLE: 1-Leader(L)/ 2-Follower(F).
//       DEFAULT_WIFI_MODE: 1-[AP]/ 2-[STA][RSSI] / 3-[TRY:SSID].
//       (no matter what wifi mode you select, you can always ctrl it via ESP-NOW.)
// Row4: the position of servo 1, 2 and 3.
String MAC_ADDRESS;
IPAddress IP_ADDRESS;
byte   SERVO_NUMBER;
byte   DEV_ROLE;
byte   WIFI_MODE;
int    WIFI_RSSI;

// set the interval of the threading.
#define threadingInterval 600
#define clientInterval    10

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

#include "RGB_CTRL.h"
#include "STSCTRL.h"
#include "CONNECT.h"
#include "BOARD_DEV.h"

int posObjetivo = 0;
bool posicionAlcanzada = true;
int servoID = 1; // ID del servo por defecto
int actualPos = 0;
int velocidad = 50;



void setup() {
  Serial.begin(115200);
  while(!Serial) {}
  InitRGB();
  EEPROM.begin(512);//************************************************************************
  espNowInit();

  getMAC();

  boardDevInit();

  RGBcolor(0, 64, 255);

  servoInit();
  setMode(activeNumInList, 0);
  st.EnableTorque(SERVO_NUMBER, 1);
  wifiInit();

  webServerSetup();

  RGBoff();

  delay(1000);
  pingAll(true);

  threadInit();
}
void procesarMensaje() {
  String mensaje = Serial.readStringUntil('\n');
    int id, pos, speed;
    if (sscanf(mensaje.c_str(), "%d %d %d", &id, &pos, &speed) == 3) {

        int velocidad=speed;
        int idd=id;
        Serial.println(mensaje);
        
        st.WritePosEx(idd, pos, velocidad,0);
        //delay (1000);
        //int actual_pos=st.ReadPos(idd);
        //Serial.println("Pos actual es: " + String(actual_pos));
        

        } else {
            Serial.println("Formato incorrecto. Usa: ID POS SPEED");
        }
}


void loop() {
      if (Serial.available() > 0) {
        procesarMensaje();
    } // Verifica cada 100ms
        
    
}









/*
void loop() {

if (Serial.available() > 0) {
        String mensaje = Serial.readStringUntil('\n'); // Leer el mensaje recibido

        // Procesar el mensaje recibido
        int id, pos, speed;
        if (sscanf(mensaje.c_str(), "%d %d %d", &id, &pos, &speed) == 3) {
            servoID = id; // Actualizar el ID del servo
            posObjetivo = pos; // Actualizar la posición objetivo
            velocidad = speed;
            // Mover el servo al objetivo especificado
            Serial.printf("Moviendo servo ID %d a posición %d con velocidad %d\n", id, pos, velocidad);
            sc.WritePosEx(id, pos, speed, 0);
            posicionAlcanzada = false;
        } else {
            Serial.println("Formato incorrecto. Usa: ID POS SPEED");
        }
    }

    // Verificar si el servo alcanzó la posición objetivo
    if (!posicionAlcanzada) {
        actualPos = sc.ReadPos(servoID); // Leer posición actual del servo

        int error = posObjetivo - actualPos;
        if (abs(error) <= 5) { // Verificar si el error es pequeño
            Serial.printf("El servo ha alcanzado la posición objetivo: %d\n", actualPos);
            posicionAlcanzada = true;
        }
    }
}
*/
/*
   if (Serial.available() > 0) {
    char command = Serial.read();  // Lee un solo carácter del puerto serie

    if (command == 's') {  // Cambiar el modo del servo
      setMode(activeNumInList, 0);
      Serial.println("Servo mode set to 0");
    }
    else if (command == 'e') {  // Enable torque
      st.EnableTorque(SERVO_NUMBER, 1);
      Serial.println("Torque enabled");
    }
    else if (command == 'i') {  // Cambiar el ID del servo
      byte newID = 5;  // Nuevo ID para el servo
      setID(activeNumInList, newID);
      Serial.print("Servo ID changed to ");
      Serial.println(newID);
    }
    // Añadir más comandos según sea necesario
  }*/



// > > > > > > > > > DOC < < < < < < < < <
// === Develop Board Ctrl ===
// get the MAC address and save it in MAC_ADDRESS;
// getMAC();

// Init GPIO.
// pinMode(PIN_NUM, OUTPUT);
// pinMode(PIN_NUM, INPUT_PULLUP);

// set the level of GPIO.
// digitalWrite(PIN_NUM, LOW);
// digitalWrite(PIN_NUM, HIGH);

// PWM output(GPIO).
// int freq = 50;
// resolution = 12;
// ledcSetup(PWM_NUM, frep, resolution);
// ledcAttachPin(PIN_NUM, PWM_NUM);
// ledcWrite(PWM_NUM, PWM);


// === Servo Ctrl ===
// GPIO 18 as RX, GPIO 19 as TX, init the serial and the servos.
// servoInit();

// set the position as middle pos.
// setMiddle(servoID);
// st.WritePosEx(ID, position, speed, acc);



// === Devices Ctrl ===
// ctrl the RGB.
// 0 < (R, G, B) <= 255
// setSingleLED(LED_ID, matrix.Color(R, G, B));

// init the OLED screen, RGB_LED.
// boardDevInit();

// dispaly the newest information on the screen.
// screenUpdate();