/*
 * 27th February 2021
 * James Fotherby
 * Energy Monitoring Lamp TX
 * 
 * This software is the transmitter for the energy monitoring lamp. A photoresistor is bluetacked to the electricity meter's IMP light. It flashes 1000 times per KwH
 * The photoresistor pulls up pin A0 to 5v and there's a 47K pull down resistor.
 * 
 * So how does this work?
 *  - The ADC samples at ~4KHz. When a sample is above a threshold FLASH_ADC_THRESHOLD then we detect a flash.
 *  - We transmit an "F" and wait 100ms
 *  - We then restart sampling the ADC waiting for the next flash
 *  
 *  - If drawing 7.5 kW then with a 2000 imp/kwh meter our flash period would be: 240ms
 * 
 */

#define     FLASH_ADC_THRESHOLD     750
#define     FLASH_ADC_THRESHOLD_L   700

void setup() {
  Serial.begin(115200);
  pinMode(A0, INPUT);                                             // This is our analogue input
  pinMode(11, OUTPUT); digitalWrite(11, LOW);                     // Oh for lack of convenient GND pins the 220K pulldown resistor is grounded by this pin...
  pinMode(13, OUTPUT); digitalWrite(13, LOW);                     // The Pro Mini's onboard LED for debugging
}

void loop() {
  // Wait for a flash to be detected
  while(analogRead(A0) < FLASH_ADC_THRESHOLD) {
    delayMicroseconds(200);
  }

  // Now a flash has been detected transmit an "F"
  Serial.println("F");
  
  digitalWrite(13, HIGH);
  delay(100);
  
  digitalWrite(13, LOW);                                          // Photoresistors have a slow response time
  while(analogRead(A0) > FLASH_ADC_THRESHOLD_L) {
    delay(5);
  }
}
