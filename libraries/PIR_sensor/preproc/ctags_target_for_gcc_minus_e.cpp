# 1 "C:\\Users\\Anirudh\\Documents\\Arduino\\pirsensor\\pirsensor.ino"
/*

 * PIRMotionSensor.ino

 * Example sketch for the PIR motion sensor

 *

 * Copyright (c) 2013 seeed technology inc.

 * Website    : www.seeed.cc

 * Author     : FrankieChu

 * Create Time: Jan 21,2013

 * Change Log :

 *

 * The MIT License (MIT)

 *

 * Permission is hereby granted, free of charge, to any person obtaining a copy

 * of this software and associated documentation files (the "Software"), to deal

 * in the Software without restriction, including without limitation the rights

 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell

 * copies of the Software, and to permit persons to whom the Software is

 * furnished to do so, subject to the following conditions:

 *

 * The above copyright notice and this permission notice shall be included in

 * all copies or substantial portions of the Software.

 *

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR

 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,

 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE

 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER

 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,

 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN

 * THE SOFTWARE.

 */
# 33 "C:\\Users\\Anirudh\\Documents\\Arduino\\pirsensor\\pirsensor.ino"
/*****************************************************************************/
//  Function: If the sensor detects the moving people in it's detecting range,
//        the Grove - LED is turned on.Otherwise, the LED is turned off.
//  Hardware: Grove - PIR Motion Sensor, Grove - LED
//  Arduino IDE: Arduino-1.0
/*******************************************************************************/
/*macro definitions of PIR motion sensor pin and LED pin*/



void setup()
{
  pinsInit();
}

void loop()
{
  if(isPeopleDetected())//if it detects the moving people?
    turnOnLED();
  else
    turnOffLED();
}
void pinsInit()
{
  pinMode(2/*Use pin 2 to receive the signal from the module */, 0x0);
  pinMode(4/*the Grove - LED is connected to D4 of Arduino*/,0x1);
}
void turnOnLED()
{
  digitalWrite(4/*the Grove - LED is connected to D4 of Arduino*/,0x1);
}
void turnOffLED()
{
  digitalWrite(4/*the Grove - LED is connected to D4 of Arduino*/,0x0);
}
/***************************************************************/
/*Function: Detect whether anyone moves in it's detecting range*/
/*Return:-boolean, ture is someone detected.*/
boolean isPeopleDetected()
{
  int sensorValue = digitalRead(2/*Use pin 2 to receive the signal from the module */);
  if(sensorValue == 0x1)//if the sensor value is HIGH?
  {
    return true;//yes,return ture
  }
  else
  {
    return false;//no,return false
  }
}
