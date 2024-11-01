#include <TinyGPSPlus.h>
#include <MKRWAN.h>
#include <DHT.h>
#include <Wire.h>
#include "Seeed_BMP280.h"
#include "rgb_lcd.h"

TinyGPSPlus gps;
rgb_lcd lcd;
LoRaModem modem;
DHT dht(7, DHT22);
BMP280 bmp280;
const unsigned int time_interval = 900000;
unsigned long time_current = 0;
const unsigned int alarm_interval = 90000;
unsigned long alarm_next = 0;
String appEui = "32B1C442EC756839";
String appKey = "066E14E8E56C7F1DB4C732419E05E090";
const int lora_timeout = 600000;
byte atil[] = {B01110, B00000, B01110, B00001, B01111, B10001, B01111, B00000};
byte cdila[] = {B00000, B00000, B01110, B10000 ,B10000, B10001, B01110, B01000};
bool start_up = true;
bool inibit = false;

float Max_T = 24.0; 
float Min_T = 18.0;

void setup(){
  lcd.begin(16, 2);
  lcd.createChar(1, atil);
  lcd.createChar(2, cdila);
  lcd.setCursor(0,0);
  lcd.print("Inicializa\2\1o.");
  Serial.begin(9600);
  Serial1.begin(9600);
  delay(500);
  modem.begin(EU868);
  int connect = modem.joinOTAA(appEui, appKey, lora_timeout);
  if (!connect) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Erro conex\1o."); 
    while (1) {}
  }
  modem.dataRate(3);
  modem.minPollInterval(60);
  dht.begin();
  bmp280.init();
  lcd.clear();
  lcd.print("Iniciado.");
}

void loop(){
  while (Serial1.available() > 0){
    gps.encode(Serial1.read());
  }
  if ((millis() >= time_current + time_interval) || (start_up == true)) {
    displayInfo();
    time_current += time_interval;
    start_up = false;
  } 
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Temp.:" + String(dht.readTemperature()));
  chek_parm();
}

void chek_parm(){
  if ((dht.readTemperature() < Min_T) || (dht.readTemperature() > Max_T)){
    if ((millis() > alarm_next) && (inibit == false)) {
        modem.beginPacket();
        modem.print("A");
        modem.endPacket();
        inibit = true;
        alarm_next = millis() + alarm_interval;
    }
  }
  if ((alarm_next > millis()) && (inibit == true)){
      inibit = false;
  }
}

void displayInfo(){
  byte dataSender[18];
  uint16_t T = dht.readTemperature() * 100;
  uint16_t H = dht.readHumidity() * 100;
  uint16_t P = round(bmp280.getPressure() / 10.0);
  uint16_t V = gps.speed.kmph() * 100;
  uint16_t A = gps.altitude.meters() * 100;
  uint32_t LAT = gps.location.lat() * 1000000;
  uint32_t LNG = gps.location.lng() * -1000000;
  dataSender[0] = T >> 8;
  dataSender[1] = T & 0xFF;
  dataSender[2] = H >> 8;
  dataSender[3] = H & 0xFF;
  dataSender[4] = P >> 8;
  dataSender[5] = P & 0xFF;
  dataSender[6] = A >> 8;
  dataSender[7] = A & 0xFF;
  dataSender[8] = V >> 8;
  dataSender[9] = V & 0xFF;
  dataSender[10] = LAT;
  dataSender[11] = LAT >> 8;
  dataSender[12] = LAT >> 16;
  dataSender[13] = LAT >> 24;
  dataSender[14] = LNG;
  dataSender[15] = LNG >> 8;
  dataSender[16] = LNG >> 16;
  dataSender[17] = LNG >> 24;
  modem.beginPacket();
  modem.write(dataSender,18); 
  modem.endPacket();
}