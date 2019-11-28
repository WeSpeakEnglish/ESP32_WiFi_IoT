/*
 *  This sketch demonstrates how to scan WiFi networks.
 *  The API is almost the same as with the WiFi Shield library,
 *  the most obvious difference being the different file you need to include:
 */
#include "EEPROM.h"
#include "WiFi.h"
static const uint8_t LED_BUILTIN = 32;
#define BUILTIN_LED  LED_BUILTIN // backward compatibility

int TemperatureArray[8]={0,1,2,3,4,5,6,7};  // temporal temperature array
int loopCounter = 0; // main loop counter

int addr = 0;
#define EEPROM_SIZE 8


void setup()
{
    Serial.begin(115200);
    pinMode (LED_BUILTIN, OUTPUT);
    
    // Set WiFi to station mode and disconnect from an AP if it was previously connected
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    Serial.println("Setup done");

  if (!EEPROM.begin(EEPROM_SIZE))
  {
    Serial.println("failed to initialise EEPROM"); delay(1000000);
  }
  Serial.println(" bytes read from Flash . Values are:");
  for (int i = 0; i < EEPROM_SIZE; i++)
  {
    Serial.print(byte(EEPROM.read(i))); Serial.print(" ");
  } 
}

void loop()
{
  int i = 0;
  loopCounter++;  
    Serial.println("scan start");

    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
    Serial.println("scan done");
    

      digitalWrite(LED_BUILTIN, HIGH);
      digitalWrite(LED_BUILTIN, LOW);
      

      
    if (n == 0) {
        Serial.println("no networks found");
    } else {
        Serial.print(n);
        Serial.println(" networks found");
        for (int i = 0; i < n; ++i) {
          
            // Print SSID and RSSI for each network found
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.print(")");
            Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
            delay(10);
        }
    }
    Serial.println("");
    
    Serial.println(" bytes read from Flash . Values are:");
       for (int i = 0; i < EEPROM_SIZE; i++)
        {
           Serial.print(byte(EEPROM.read(i))); Serial.print(" ");
        } 
    // Wait a bit before scanning again
    delay(5000);
    for (i = 0; i < 8; i++)      TemperatureArray[i]++;
      
   if (loopCounter == 10) {

      Serial.println("FLASH data written");
      Serial.println(" bytes read from Flash . Values are:");
    for (i = 0; i < 8; i++){ 
     EEPROM.write(i, TemperatureArray[i]);
    }
    EEPROM.commit();
      for (int i = 0; i < EEPROM_SIZE; i++)
    {
    Serial.print(byte(EEPROM.read(i))); Serial.print(" ");
    }
    }
}
