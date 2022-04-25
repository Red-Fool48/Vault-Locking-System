#include <Arduino.h>
#line 1 "C:\\Users\\Anirudh\\Documents\\Arduino\\sketch_jan11b\\sketch_jan11b.ino"
#define BLYNK_DEVICE_NAME           "arduino"
#define BLYNK_AUTH_TOKEN            "UiDJeWzDXWYtfzvnFBxCkomOhbYWcu3w"


// Comment this out to disable prints and save space
#define BLYNK_PRINT SwSerial


#include <SoftwareSerial.h>
SoftwareSerial SwSerial(10, 10); // RX, TX

#include <BlynkSimpleStream.h>

char auth[] = BLYNK_AUTH_TOKEN;


// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).


#line 21 "C:\\Users\\Anirudh\\Documents\\Arduino\\sketch_jan11b\\sketch_jan11b.ino"
void setup();
#line 30 "C:\\Users\\Anirudh\\Documents\\Arduino\\sketch_jan11b\\sketch_jan11b.ino"
void BlynkWidgetWrite1(BlynkReq __attribute__((__unused__)) &request, const BlynkParam __attribute__((__unused__)) &param);
#line 40 "C:\\Users\\Anirudh\\Documents\\Arduino\\sketch_jan11b\\sketch_jan11b.ino"
void loop();
#line 21 "C:\\Users\\Anirudh\\Documents\\Arduino\\sketch_jan11b\\sketch_jan11b.ino"
void setup()
{
  Serial.begin(9600);
  Blynk.begin(Serial,auth);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
}

BLYNK_WRITE(V1) //Button Widget is writing to pin V1
{
  int pinData = param.asInt();
  if (pinData == 1) {
    digitalWrite(11, HIGH);
  } else {
    digitalWrite(11, LOW);
  }
}

void loop()
{
  Blynk.run();
}
//}
/*
 * #define BLYNK_DEVICE_NAME           "arduino"
#define BLYNK_AUTH_TOKEN            "Osow1FIH6SLyzn_spyqwohLysqFbkScb"


// Comment this out to disable prints and save space
#define BLYNK_PRINT SwSerial


#include <SoftwareSerial.h>
SoftwareSerial SwSerial(10, 10); // RX, TX

#include <BlynkSimpleStream.h>

char auth[] = BLYNK_AUTH_TOKEN;


void setup()
{
  // Debug console
  SwSerial.begin(115200);

  // Blynk will work through Serial
  // Do not read or write this serial manually in your sketch
  Serial.begin(9600);
  Blynk.begin(Serial, auth);
}

void loop()
{
  Blynk.run();
  // You can inject your own code or combine it with other sketches.
  // Check other examples on how to communicate with Blynk. Remember
  // to avoid delay() function!
}
 */

