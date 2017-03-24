/*********************************************************************
 This is an example for our Feather Bluefruit modules

 Pick one up today in the adafruit shop!

 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/

/*
 * This sketch use all 12 channels of 3 Hardware PWM.
 * Also change the resolution to max 15-bit.
 * Each PWM module (4 pin) also run with different frequency
 * - PWM0 : clock/1  ~ 16Mhz
 * - PWM1 : clock/4  ~ 4Mhz
 * - PMW2 : clock/16 ~ 1Mhz
 * 
 * As the result LED BLUE will blink while fading unline LED RED which looks 
 * more solid
 */

#include <Arduino.h>

// Maximum 12 pins can be used for 3 PWM module ( 4 channel each )
int pins[12] = 
{ 
  PIN_A0 , PIN_A1  , PIN_A2      , PIN_A3, 
  PIN_A4 , PIN_A5  , PIN_A6      , LED_RED, /* avoid A7 (VBAT)  */
  27     , LED_BLUE, PIN_WIRE_SDA, PIN_WIRE_SCL
};

/**************************************************************************/
/*!
    @brief  The setup function runs once when reset the board
*/
/**************************************************************************/
void setup()
{
  Serial.begin(115200);

  // Add 4 pins into a group
  // It is better to add Pin before call .begin()
  for (int i=0; i<12; i++)
  {
    PWMx[i/4]->addPin( pins[i] );
  }

  // Enable all 3 PWM modules with 15-bit resolutions(max) but different clock div
  PWM0.begin();
  PWM0.setResolution(15);
  PWM0.setClockDiv(PWM_PRESCALER_PRESCALER_DIV_1); // default : freq = 16Mhz
  
  PWM1.begin();
  PWM1.setResolution(15);
  PWM1.setClockDiv(PWM_PRESCALER_PRESCALER_DIV_4); // default : freq = 4Mhz
  
  PWM2.begin();
  PWM2.setResolution(15);
  PWM2.setClockDiv(PWM_PRESCALER_PRESCALER_DIV_16); // default : freq = 1Mhz

  PWM0.printInfo();
  PWM1.printInfo();
  PWM2.printInfo();
}

/**************************************************************************/
/*!
    @brief  The loop function runs over and over again forever
*/
/**************************************************************************/
void loop()
{
  const int maxValue = bit(15) - 1;

  // fade in from min to max in increments of 5 points:
  for (int fadeValue = 0 ; fadeValue <= maxValue; fadeValue += 512) 
  {
    for (int i=0; i<12; i++)
    {
      PWMx[i/4]->writePin( pins[i], fadeValue);
    }
    
    // wait for 30 milliseconds to see the dimming effect
    delay(30);
  }

  // fade out from max to min in increments of 5 points:
  for (int fadeValue = maxValue ; fadeValue >= 0; fadeValue -= 512) 
  {
    for (int i=0; i<12; i++)
    {
      PWMx[i/4]->writePin( pins[i], fadeValue);
    }
    
    // wait for 30 milliseconds to see the dimming effect
    delay(30);
  }
}
