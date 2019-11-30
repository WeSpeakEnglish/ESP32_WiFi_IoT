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
OneWire ds(25);  //data wire connected to GPIO15

const char* ssid       = "Redmi";
const char* password   = "micromax";

const char* ntpServer = "pool.ntp.org";
const char* httpSettings = "http://test.pollutants.eu/settings.php";

const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

int RTC_DATA_ATTR LoopCount = 0;

static const uint8_t LED_BUILTIN = 32;
#define BUILTIN_LED  LED_BUILTIN // backward compatibility

#define EEPROM_SIZE 8
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  5        /* Time ESP32 will go to sleep (in seconds) */

signed char TemperatureArray[EEPROM_SIZE] = {0, 1, 2, 3, 4, 5, 6, 7}; // temporal temperature array
signed char TimeArray[EEPROM_SIZE] = {0, 1, 2, 3, 4, 5, 6, 7}; // temporal temperature array

signed char RTC_DATA_ATTR TemperatureArrayRTC[EEPROM_SIZE];
signed char RTC_DATA_ATTR TimeArrayRTC[EEPROM_SIZE];
struct RTC_DATA_ATTR tm timeinfo;

int RTC_DATA_ATTR loopCounter = 0; // main loop counter

int addr = 0;

void printLocalTime()
{
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void takeDataWeb() {
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  //connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" CONNECTED");

  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
  ///HTTP
  HTTPClient http;
  WiFiClient cli;
  
  Serial.print("[HTTP] begin...\n");
  http.begin(cli,httpSettings); //HTTP
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
      Serial.println(payload);
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
  pinMode (LED_BUILTIN, OUTPUT);
  Serial.println(loopCounter);
  if (LoopCount++ % 60 == 0) {
    takeDataWeb();

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
  }


}

void loop()
{
  byte i;
  byte addr[8];
  
  loopCounter++;
  
  Serial.println(" ");
  printLocalTime();
  
  Serial.println(" ");
  Serial.println(" eeeee.");
  if (!ds.search(addr)) {
    Serial.println(" No more addresses.");
    Serial.println();
    ds.reset_search();
    delay(250);
    return;
  }
  Serial.print(" ROM =");
  for (i = 0; i < 8; i++) {
    Serial.write(' ');
    Serial.print(addr[i], HEX);
  }

  //reload
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);


  Serial.flush(); 
  
 //  esp_wifi_stop();
 // esp_light_sleep_start();
  esp_deep_sleep_start();
  Serial.println("This will never be printed");
}
