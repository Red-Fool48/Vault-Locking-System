# 1 "C:\\Users\\Anirudh\\Documents\\Arduino\\sketch_jan11b\\sketch_jan11b.ino"




// Comment this out to disable prints and save space


# 9 "C:\\Users\\Anirudh\\Documents\\Arduino\\sketch_jan11b\\sketch_jan11b.ino" 2
# 10 "C:\\Users\\Anirudh\\Documents\\Arduino\\sketch_jan11b\\sketch_jan11b.ino" 2
SoftwareSerial SwSerial(10, 10); // RX, TX

# 13 "C:\\Users\\Anirudh\\Documents\\Arduino\\sketch_jan11b\\sketch_jan11b.ino" 2

char auth[] = "UiDJeWzDXWYtfzvnFBxCkomOhbYWcu3w";
Servo myServo;

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).


void setup()
{
  Serial.begin(9600);
  Blynk.begin(Serial,auth);
  myServo.attach(10);
  pinMode(11, 0x1);
  pinMode(12, 0x1);
  pinMode(13, 0x1);
}

void BlynkWidgetWrite1 (BlynkReq __attribute__((__unused__)) &request, const BlynkParam __attribute__((__unused__)) &param) //Button Widget is writing to pin V1
{
  int pinData = param.asInt();
  if (pinData == 1) {
    digitalWrite(11, 0x1);
  } else {
    digitalWrite(11, 0x0);
  }
}

void BlynkWidgetWrite2 (BlynkReq __attribute__((__unused__)) &request, const BlynkParam __attribute__((__unused__)) &param)
{
  int state=param.asInt();
  if(state==1)
  {
    myServo.write(180);
  }
  else
  {
    myServo.write(0);
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
