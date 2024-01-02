/*
 * 1st Jan 2024
 * James Fotherby
 * Energy Monitor
 * 
 * This software runs on an ESP32 Feather (v1) and logs our electricity consumption to an influxDB that runs on a RPi on the local network
 * A photoresistor is bluetacked to the electricity meter's IMP light which flashes 1000 times per KwH
 * The photoresistor pulls up pin A2 to 3.3v and there's a 47K pull down resistor and a 10nF smoothing capacitor.
 * 
 * So how does this work?
 *  - The ADC samples continuously. When a sample is above a threshold FLASH_ADC_THRESHOLD then we detect a flash.
 *  - We measure the timestamp of the flash and compare it to the previous timestamp to discover the flash period 
 *  - If it's been more than 5 seconds since we recorder the last power usage then record another sample inbto influxDB
 *  - Wait 100ms and wait until the ADC voltage has dropped below threshold FLASH_ADC_THRESHOLD_L
 *  - We then restart sampling the ADC waiting for the next flash
 *  
 *  - If drawing 15 kW then with a 1000 imp/kwh meter our flash period would be: 240ms
 *  - If drawing 10 W then the flash period would be 360s
 *
 *  Data is logged to an InfluxDB database and can be viewed on grafana.
 */

#include <InfluxDbClient.h>
#include <WiFi.h>
#include <WiFiMulti.h>

//--- Pin Definitions ---------------------------------------------------------------
#define         ADC_PIN                       A2

#define         FLASH_ADC_THRESHOLD           3000
#define         FLASH_ADC_THRESHOLD_L         2800

//--- Other Defines -------------------------------------------------------------------
#define         DEVICE                        "ESP32"

#define         WIFI_SSID                     "xxxxxx"
#define         WIFI_PASSWORD                 "xxxxxx"
#define         INFLUXDB_URL                  "http://192.168.1.151:8086"
#define         INFLUXDB_DB_NAME              "Cheyney_DB"

//--- Function prototypes --------------------------------------------------------------
void startWifi();
void Save_Electric_Influx_Point(uint16_t Watts);

//--- Variables --------------------------------------------------------------------------
WiFiMulti       wifiMulti;
InfluxDBClient  client(INFLUXDB_URL, INFLUXDB_DB_NAME);
Point           Electric_Sensor_Point("Electricity_Measurements");

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//#########################################################################################################################################################################
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup() 
{
  Serial.begin(115200);
  while (!Serial); 

  startWifi();  

  pinMode(ADC_PIN, INPUT);                                                                                      // This is our analogue input
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//#########################################################################################################################################################################
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void loop() 
{
  static unsigned long Flash_Timestamp = 0, Previous_Flash_Timestamp = 0, Flash_Period = 0;
  static unsigned long Previous_Tx_Timestamp = 0, Tx_Period = 0;
  static unsigned long Current_Watts;
  
  // Wait for a flash to be detected
  while(analogRead(ADC_PIN) < FLASH_ADC_THRESHOLD) {
    delayMicroseconds(100);
  }

  // To reach here a flash must have just been detected. Record the time. Find the period and calculate the current power usage
  Previous_Flash_Timestamp = Flash_Timestamp;
  Flash_Timestamp = millis();
  Flash_Period = Flash_Timestamp - Previous_Flash_Timestamp;
  Current_Watts = 3600000 / Flash_Period;

  // Only log a new power reading if it's been at least 5s
  Tx_Period = millis() - Previous_Tx_Timestamp;
  if(Tx_Period >= 5000)  {
      Save_Electric_Influx_Point(Current_Watts);
      Previous_Tx_Timestamp = millis();  
    }

  delay(100);
  Serial.println(Current_Watts);
                                           
  while(analogRead(ADC_PIN) > FLASH_ADC_THRESHOLD_L) {
    delay(5);
  }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//#########################################################################################################################################################################
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void startWifi()
{
  // Setup wifi
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to wifi");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();

  // Check influx server connection
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//#########################################################################################################################################################################
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void Save_Electric_Influx_Point(uint16_t Watts)
{
  // Store measured value into point
  Electric_Sensor_Point.clearFields();
  Electric_Sensor_Point.addField("Electricity_Consumption", Watts);

  // Print what are we exactly writing
  Serial.print("Writing: ");
  Serial.println(client.pointToLineProtocol(Electric_Sensor_Point));

  // If no Wifi signal, try to reconnect it
  if (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("Wifi connection lost");
  }

  // Write point
  if (!client.writePoint(Electric_Sensor_Point)) {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
  }
  else  {
    Serial.println("Electric Point added to database");
  }
}



