/*
    This sketch demonstrates how to scan WiFi networks.
    The API is almost the same as with the WiFi Shield library,
    the most obvious difference being the different file you need to include:
*/
#include "EEPROM.h"
#include "WiFi.h"
#include "time.h"
#include <HTTPClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

OneWire ds(25);  //data wire connected to GPIO15
DeviceAddress sensor1 = { 0x28, 0xFF, 0x45, 0x4C, 0x74, 0x15, 0x3, 0x1 }; //hot water
DeviceAddress sensor2 = { 0x28, 0xFF, 0xA7, 0xF2, 0x81, 0x15, 0x1, 0xD8}; //cold water
DallasTemperature sensors(&ds);
float temp[8]; //temperature array

const char* ssid       = "Redmi";
const char* password   = "micromax";

const char* ntpServer = "pool.ntp.org";
const char* httpSettings = "http://test.pollutants.eu/setup.php?set_flag=1";

const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

int RTC_DATA_ATTR LoopCount = 0;

//static const uint8_t LED_BUILTIN = 32;
#define HEAT_RELAY  27 // backward compatibility
#define PUMP1  26 // backward compatibility
#define PUMP2  32 // backward compatibility
#define COOLER 33 // backward compatibility

#define EEPROM_SIZE 32
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  5        /* Time ESP32 will go to sleep (in seconds) */

signed char TemperatureArray[EEPROM_SIZE] = {0, 1, 2, 3, 4, 5, 6, 7}; // temporal temperature array
signed char TimeArray[EEPROM_SIZE] = {0, 1, 2, 3, 4, 5, 6, 7}; // temporal temperature array

//signed char RTC_DATA_ATTR TemperatureArrayRTC[EEPROM_SIZE];
//signed char RTC_DATA_ATTR TimeArrayRTC[EEPROM_SIZE];

signed char RTC_DATA_ATTR TimeArrayRTC[EEPROM_SIZE];
struct RTC_DATA_ATTR tm timeinfo;

int RTC_DATA_ATTR loopCounter = 0; // main loop counter

int addr = 0;
#define ARRAY_SIZE_PARMS 8
int arrayParams[ARRAY_SIZE_PARMS];

void parseParms(String strToParse) {
  bool flag = false;
  int arrayIndex = 0;
  int numbIndex = 0;
  int numb[4];

  for (int i = 0; i < 26; i++) {
    if (strToParse[i] == ':') {
      if (flag) {
        switch (numbIndex) {
          case 1:
            arrayParams[arrayIndex++] = numb[0];
            break;
          case 2:
            arrayParams[arrayIndex++] = (numb[0]) * 10 + numb[1];
            break;
          case 3:
            arrayParams[arrayIndex++] = (numb[0]) * 100 + (numb[1]) * 10 + numb[2];
            break;
          case 4:
            arrayParams[arrayIndex++] = (numb[0]) * 100 + (numb[1]) * 100 + (numb[2]) * 10 + numb[3];
            break;
        }

        if (arrayIndex == ARRAY_SIZE_PARMS) return;
        for (int j = 0; j < 4 ; j++) {
          numb[j] = 0;
          numbIndex = 0;
        }
      }

      flag = true;
    }
    else {
      if (flag && ((int)strToParse[i] > 47)) {
        numb[numbIndex] = (int)strToParse[i] - 48;
        numbIndex++;
      }
    }
  }
}

void printLocalTime()
{
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void takeDataWeb() {
  int i = 0;
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  //connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    i++;
    if (i > 20) break;
  }
  Serial.println(" CONNECTED");

  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
  ///HTTP
  HTTPClient http;
  WiFiClient cli;

  Serial.print("[HTTP] begin...\n");
  http.begin(cli, httpSettings); //HTTP
  Serial.print("[HTTP] GET...\n");
  // start connection and send HTTP header
  int httpCode = http.GET();

  // httpCode will be negative on error
  if (httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);

    // file found at server
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println("---------------------------------------");
      Serial.println(payload);
      parseParms(payload);
      Serial.println("---------------------------------------");
      for (int i = 0; i < 6; i++) {
        Serial.print(arrayParams[i]);
        Serial.print(" : ");
      }
      Serial.println("---------------------------------------");
    }
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();

  ////////////////
  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}
//----------------------------------
void setup()
{
  Serial.begin(115200);

  pinMode (HEAT_RELAY, OUTPUT);
  digitalWrite(HEAT_RELAY, HIGH);
  pinMode (PUMP1, OUTPUT);
  digitalWrite(PUMP1, LOW);
  pinMode (PUMP2, OUTPUT);
  digitalWrite(PUMP2, LOW);
  pinMode (COOLER, OUTPUT);
  digitalWrite(COOLER, LOW);

  Serial.println(loopCounter);
  //if (LoopCount++ % 60 == 0) {
  //  takeDataWeb();
  // takeDataWeb();

  if (!EEPROM.begin(EEPROM_SIZE))
  {
    Serial.println("failed to initialise EEPROM"); delay(1000000);
  }
  Serial.println(" bytes read from Flash . Values are:");
  for (int i = 0; i < EEPROM_SIZE; i++)
  {
    Serial.print(byte(EEPROM.read(i))); Serial.print(" ");
  }
  Serial.println("Setup done");
  // }
  sensors.begin();
}

void loop()
{
  byte i;
  byte addr[8];

  loopCounter++;

  sensors.begin();

  Serial.println(" ");
  printLocalTime();

  Serial.println(" ");
  if (loopCounter == 1)takeDataWeb();
  // if (LoopCount > 1){
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.print("Sensor 1(*C): ");
  Serial.print(sensors.getTempC(sensor1));
  Serial.print(" Sensor 2(*C): ");
  Serial.print(sensors.getTempC(sensor2));
  //}
  //reload
  // esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);


  Serial.flush();


  //if (loopCounter % 10 == 0){
  //  WiFi.mode(WIFI_STA);
  //  takeDataWeb();
  //  }

  delay(1000);
  //digitalWrite(HEAT_RELAY, LOW);
  //delay(1000);
  //  esp_wifi_stop();
  // esp_light_sleep_start();
  // esp_deep_sleep_start();
  //Serial.println("This will never be printed");
}
