/*
 *  This sketch demonstrates how to scan WiFi networks.
 *  The API is almost the same as with the WiFi Shield library,
 *  the most obvious difference being the different file you need to include:
 */
#include "EEPROM.h"
#include "WiFi.h"
#include "time.h"

const char* ssid       = "Redmi";
const char* password   = "micromax";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

int RTC_DATA_ATTR LoopCount = 0;

static const uint8_t LED_BUILTIN = 32;
#define BUILTIN_LED  LED_BUILTIN // backward compatibility
#define EEPROM_SIZE 8
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  5        /* Time ESP32 will go to sleep (in seconds) */


signed char TemperatureArray[EEPROM_SIZE]={0,1,2,3,4,5,6,7};  // temporal temperature array

int loopCounter = 0; // main loop counter

int addr = 0;

void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void takeDataWeb(){
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

  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  }
//----------------------------------
void setup()
{
    Serial.begin(115200);
    pinMode (LED_BUILTIN, OUTPUT);
    
    if(LoopCount++ % 60 == 0){
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
  int i = 0;
  loopCounter++;  
   

    // Wait a bit before scanning again
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    esp_deep_sleep_start();
    Serial.println(" ");
    printLocalTime();
    Serial.println(" ");
}
