/*
 * 27th February 2021
 * James Fotherby
 * Energy Monitoring RGB Lamp
 * 
 * This software runs the energy monitoring light. It receives flash signals wirelessly from the transmitter at the electricity meter and 
 * controls the PWM duty cycle to an RGB LED. The Hue is set according to the current power consumption. Blue to Red
 * 
 * Finer details:
 *  - A logarithmic scale is used to select the Hue. ie. Hue = ln(Power)
 *  - We transmit the current minute's flash count over serial to a receiving raspberry pi
 * 
 */

#include "Average.h"

#define     BLUE_LED        9
#define     RED_LED         10
#define     GREEN_LED       11
#define     PRO_MINI_LED    13                                                  // Flashes for each received packet (for debugging)    

#define     POWER_USAGE_K   1.8e6
#define     RPI_TX_PERIOD   60000

#define     R_PWM           OCR1B
#define     G_PWM           OCR2A
#define     B_PWM           OCR1A

typedef struct {
    double r;                                                                   // a fraction between 0 and 1
    double g;                                                                   // a fraction between 0 and 1
    double b;                                                                   // a fraction between 0 and 1
} rgb;

typedef struct {
    double h;                                                                   // angle in degrees
    double s;                                                                   // a fraction between 0 and 1
    double v;                                                                   // a fraction between 0 and 1
} hsv;

static  hsv   rgb2hsv(rgb in);
static  rgb   hsv2rgb(hsv in);

void    Update_Lamp_Colour(float Power);
void    set_colour(byte R, byte G, byte B);

//-----------------------------------------------------------------------------------------------------------------------------
//#############################################################################################################################
//-----------------------------------------------------------------------------------------------------------------------------
void setup() {
  pinMode(RED_LED, OUTPUT);  
  pinMode(GREEN_LED, OUTPUT);  
  pinMode(BLUE_LED, OUTPUT);  
  pinMode(PRO_MINI_LED, OUTPUT);
  
  TCCR1A = 0b10100001;
  TCCR1B = 0b00001001;                                                          // Timer1 Fast PWM, No Prescaling = 62.5KHz PWM

  TCCR2A = 0b10000011;
  TCCR2B = 0b00000001;                                                          // Timer 2Fast PWM, No Prescaling = 62.5KHz PWM

  Serial.begin(115200);                                                         // Used to recieve UART from the RF transceiver and transmit to the RPi 
}

//-----------------------------------------------------------------------------------------------------------------------------
//#############################################################################################################################
//-----------------------------------------------------------------------------------------------------------------------------
void loop() 
{    
  static unsigned long Last_Flash_Timestamp = 0, Next_Expected_Flash_Time = 0, Flash_Interval_Time = 0;
  static unsigned long Next_RPi_Tx_Time = millis(), Flash_Count = 0;
  static float Current_Power_Consumption;

  // Wait for a flash impulse, whilst waiting, if we pass the expected flash time we update the current power every 1000ms based on the latest wait
  while(!Serial.available())  {
    if(millis() > Next_Expected_Flash_Time)                                     // Improtant, otherwise if energy usuage suddenly decreased it'd take ages to update
    {
      Next_Expected_Flash_Time += 1000;
      
      Flash_Interval_Time = millis() - Last_Flash_Timestamp;
      Current_Power_Consumption = POWER_USAGE_K / (float)Flash_Interval_Time;
      Update_Lamp_Colour(Current_Power_Consumption);
    }

    if(millis() >= Next_RPi_Tx_Time)
    {
      Next_RPi_Tx_Time += RPI_TX_PERIOD;      
      Serial.println(Flash_Count);
    }

    delay(5);    
  }

  // We get here if a flash has just occurred. Update all our metrics, clear the RX buffer and update the lamp colour
  Flash_Interval_Time       = millis() - Last_Flash_Timestamp;
  Next_Expected_Flash_Time  = millis() + Flash_Interval_Time;
  Last_Flash_Timestamp      = millis();  

  digitalWrite(PRO_MINI_LED, HIGH);
  Flash_Count++;                                                                // Keep a count of flashes which we transmit every 60 seconds 
  Serial.println(Flash_Count);
  
  while(Serial.available())  {
    delay(5);
    Serial.read();
  }
  digitalWrite(PRO_MINI_LED, LOW);

  Current_Power_Consumption = POWER_USAGE_K / (float)Flash_Interval_Time;
  Update_Lamp_Colour(Current_Power_Consumption);
}

//-----------------------------------------------------------------------------------------------------------------------------
//#############################################################################################################################
//-----------------------------------------------------------------------------------------------------------------------------
void Update_Lamp_Colour(float Power_f)
{
  static Average Average(10);                                                   // Configures a 10 sample rolling average filter
 
  int Avg_Power_I = (int)Power_f;
  Avg_Power_I = constrain(Avg_Power_I, 0, 7200);                                // If we're using >7.2kW we just remain cherry red
  float Avg_Power_f = (float)Average.Rolling_Average(Avg_Power_I);
  
  float Scaled_Power_f = 72.5 * log(1.0 + (0.005 * Avg_Power_f));               // This applies our logarithmic scaling

  Scaled_Power_f = 250.0 - Scaled_Power_f;                                      // This makes our Hue go from Blue to Red rather than vice versa
  if(Scaled_Power_f < 0.0)  {                                                   // Hue is an angle so keep it in the 0-360 range
    Scaled_Power_f = Scaled_Power_f + 360.0;
  }

  rgb rgb_out;
  hsv hsv_in;

  hsv_in.h = Scaled_Power_f;
  hsv_in.s = 1.0;
  hsv_in.v = 1.0;

  rgb_out = hsv2rgb(hsv_in);

  byte R_Fade, G_Fade, B_Fade;
  R_Fade = (byte)(rgb_out.r * 100.0);                                           // Due to different forward voltages of the R/G/B LEDs we apply different scalings
  G_Fade = (byte)(rgb_out.g * 110.0);                                           // with these PWM duties I was getting max drive currents ~500mA
  B_Fade = (byte)(rgb_out.b * 120.0);

  set_colour(R_Fade, G_Fade, B_Fade);  
}

void set_colour(byte R, byte G, byte B)
{
  R_PWM = constrain(R, 0, 100);                                                 // It's important not to overdrive our LEDs.
  G_PWM = constrain(G, 0, 110);                                                 // These constraints were decided upon following some testing with a multimeter 
  B_PWM = constrain(B, 0, 120);
}

hsv rgb2hsv(rgb in)
{
    hsv         out;
    double      min, max, delta;

    min = in.r < in.g ? in.r : in.g;
    min = min  < in.b ? min  : in.b;

    max = in.r > in.g ? in.r : in.g;
    max = max  > in.b ? max  : in.b;

    out.v = max;                                                                // v
    delta = max - min;
    if (delta < 0.00001)
    {
        out.s = 0;
        out.h = 0; // undefined, maybe nan?
        return out;
    }
    if( max > 0.0 ) { // NOTE: if Max is == 0, this divide would cause a crash
        out.s = (delta / max);                                                  // s
    } else {
        // if max is 0, then r = g = b = 0              
        // s = 0, h is undefined
        out.s = 0.0;
        out.h = NAN;                                                            // its now undefined
        return out;
    }
    if( in.r >= max )                                                           // > is bogus, just keeps compilor happy
        out.h = ( in.g - in.b ) / delta;                                        // between yellow & magenta
    else
    if( in.g >= max )
        out.h = 2.0 + ( in.b - in.r ) / delta;                                  // between cyan & yellow
    else
        out.h = 4.0 + ( in.r - in.g ) / delta;                                  // between magenta & cyan

    out.h *= 60.0;                                                              // degrees

    if( out.h < 0.0 )
        out.h += 360.0;

    return out;
}

rgb hsv2rgb(hsv in)
{
    double      hh, p, q, t, ff;
    long        i;
    rgb         out;

    if(in.s <= 0.0) {                                                           // < is bogus, just shuts up warnings
        out.r = in.v;
        out.g = in.v;
        out.b = in.v;
        return out;
    }
    hh = in.h;
    if(hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = in.v * (1.0 - in.s);
    q = in.v * (1.0 - (in.s * ff));
    t = in.v * (1.0 - (in.s * (1.0 - ff)));

    switch(i) {
    case 0:
        out.r = in.v;
        out.g = t;
        out.b = p;
        break;
    case 1:
        out.r = q;
        out.g = in.v;
        out.b = p;
        break;
    case 2:
        out.r = p;
        out.g = in.v;
        out.b = t;
        break;

    case 3:
        out.r = p;
        out.g = q;
        out.b = in.v;
        break;
    case 4:
        out.r = t;
        out.g = p;
        out.b = in.v;
        break;
    case 5:
    default:
        out.r = in.v;
        out.g = p;
        out.b = q;
        break;
    }
    return out;     
}
