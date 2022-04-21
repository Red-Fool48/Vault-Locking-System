# 1 "C:\\Users\\Anirudh\\Documents\\Arduino\\firmataArd\\firmataArd.ino"
/*

  Firmata is a generic protocol for communicating with microcontrollers

  from software on a host computer. It is intended to work with

  any host computer software package.



  To download a host software package, please click on the following link

  to open the list of Firmata client libraries in your default browser.



  https://github.com/firmata/arduino#firmata-client-libraries



  Copyright (C) 2006-2008 Hans-Christoph Steiner.  All rights reserved.

  Copyright (C) 2010-2011 Paul Stoffregen.  All rights reserved.

  Copyright (C) 2009 Shigeru Kobayashi.  All rights reserved.

  Copyright (C) 2009-2016 Jeff Hoefs.  All rights reserved.



  This library is free software; you can redistribute it and/or

  modify it under the terms of the GNU Lesser General Public

  License as published by the Free Software Foundation; either

  version 2.1 of the License, or (at your option) any later version.



  See file LICENSE.txt for further informations on licensing terms.



  Last updated August 17th, 2017

*/
# 26 "C:\\Users\\Anirudh\\Documents\\Arduino\\firmataArd\\firmataArd.ino"
# 27 "C:\\Users\\Anirudh\\Documents\\Arduino\\firmataArd\\firmataArd.ino" 2
# 28 "C:\\Users\\Anirudh\\Documents\\Arduino\\firmataArd\\firmataArd.ino" 2
# 29 "C:\\Users\\Anirudh\\Documents\\Arduino\\firmataArd\\firmataArd.ino" 2
# 30 "C:\\Users\\Anirudh\\Documents\\Arduino\\firmataArd\\firmataArd.ino" 2
# 42 "C:\\Users\\Anirudh\\Documents\\Arduino\\firmataArd\\firmataArd.ino"
// the minimum interval for sampling analog input



/*==============================================================================

 * GLOBAL VARIABLES

 *============================================================================*/



/* analog inputs */
int analogInputsToReport = 0; // bitwise array to store pin reporting

/* digital input ports */
byte reportPINs[((20 /* 14 digital + 6 analog*/ + 7) / 8)]; // 1 = report this port, 0 = silence
byte previousPINs[((20 /* 14 digital + 6 analog*/ + 7) / 8)]; // previous 8 bits sent

/* pins configuration */
byte portConfigInputs[((20 /* 14 digital + 6 analog*/ + 7) / 8)]; // each bit: 1 = pin in INPUT, 0 = anything else

/* timer variables */
unsigned long currentMillis; // store the current value from millis()
unsigned long previousMillis; // for comparison with currentMillis
unsigned int samplingInterval = 19; // how often to run the main loop (in ms)

/* i2c data */
struct i2c_device_info {
  byte addr;
  int reg;
  byte bytes;
  byte stopTX;
};

/* for i2c read continuous more */
i2c_device_info query[8];

byte i2cRxData[64];
boolean isI2CEnabled = false;
signed char queryIndex = -1;
// default delay time between i2c read request and Wire.requestFrom()
unsigned int i2cReadDelayTime = 0;

Servo servos[(_Nbr_16timers * 12 /* the maximum number of servos controlled by one timer */)];
byte servoPinMap[20 /* 14 digital + 6 analog*/];
byte detachedServos[(_Nbr_16timers * 12 /* the maximum number of servos controlled by one timer */)];
byte detachedServoCount = 0;
byte servoCount = 0;

boolean isResetting = false;

// Forward declare a few functions to avoid compiler errors with older versions
// of the Arduino IDE.
void setPinModeCallback(byte, int);
void reportAnalogCallback(byte analogPin, int value);
void sysexCallback(byte, byte, byte*);

/* utility functions */
void wireWrite(byte data)
{

  Wire.write((byte)data);



}

byte wireRead(void)
{

  return Wire.read();



}

/*==============================================================================

 * FUNCTIONS

 *============================================================================*/
# 123 "C:\\Users\\Anirudh\\Documents\\Arduino\\firmataArd\\firmataArd.ino"
void attachServo(byte pin, int minPulse, int maxPulse)
{
  if (servoCount < (_Nbr_16timers * 12 /* the maximum number of servos controlled by one timer */)) {
    // reuse indexes of detached servos until all have been reallocated
    if (detachedServoCount > 0) {
      servoPinMap[pin] = detachedServos[detachedServoCount - 1];
      if (detachedServoCount > 0) detachedServoCount--;
    } else {
      servoPinMap[pin] = servoCount;
      servoCount++;
    }
    if (minPulse > 0 && maxPulse > 0) {
      servos[servoPinMap[pin]].attach((pin), minPulse, maxPulse);
    } else {
      servos[servoPinMap[pin]].attach((pin));
    }
  } else {
    Firmata.sendString("Max servos attached");
  }
}

void detachServo(byte pin)
{
  servos[servoPinMap[pin]].detach();
  // if we're detaching the last servo, decrement the count
  // otherwise store the index of the detached servo
  if (servoPinMap[pin] == servoCount && servoCount > 0) {
    servoCount--;
  } else if (servoCount > 0) {
    // keep track of detached servos because we want to reuse their indexes
    // before incrementing the count of attached servos
    detachedServoCount++;
    detachedServos[detachedServoCount - 1] = servoPinMap[pin];
  }

  servoPinMap[pin] = 255;
}

void enableI2CPins()
{
  byte i;
  // is there a faster way to do this? would probaby require importing
  // Arduino.h to get SCL and SDA pins
  for (i = 0; i < 20 /* 14 digital + 6 analog*/; i++) {
    if (((i) == 18 || (i) == 19)) {
      // mark pins as i2c so they are ignore in non i2c data requests
      setPinModeCallback(i, firmata::PIN_MODE_I2C /* pin included in I2C setup*/);
    }
  }

  isI2CEnabled = true;

  Wire.begin();
}

/* disable the i2c pins so they can be used for other functions */
void disableI2CPins() {
  isI2CEnabled = false;
  // disable read continuous mode for all devices
  queryIndex = -1;
}

void readAndReportData(byte address, int theRegister, byte numBytes, byte stopTX) {
  // allow I2C requests that don't require a register read
  // for example, some devices using an interrupt pin to signify new data available
  // do not always require the register read so upon interrupt you call Wire.requestFrom()
  if (theRegister != -1) {
    Wire.beginTransmission(address);
    wireWrite((byte)theRegister);
    Wire.endTransmission(stopTX); // default = true
    // do not set a value of 0
    if (i2cReadDelayTime > 0) {
      // delay is necessary for some devices such as WiiNunchuck
      delayMicroseconds(i2cReadDelayTime);
    }
  } else {
    theRegister = 0; // fill the register with a dummy value
  }

  Wire.requestFrom(address, numBytes); // all bytes are returned in requestFrom

  // check to be sure correct number of bytes were returned by slave
  if (numBytes < Wire.available()) {
    Firmata.sendString("I2C: Too many bytes received");
  } else if (numBytes > Wire.available()) {
    Firmata.sendString("I2C: Too few bytes received");
  }

  i2cRxData[0] = address;
  i2cRxData[1] = theRegister;

  for (int i = 0; i < numBytes && Wire.available(); i++) {
    i2cRxData[2 + i] = wireRead();
  }

  // send slave address, register and received bytes
  Firmata.sendSysex(0x77 /* same as I2C_REPLY*/, numBytes + 2, i2cRxData);
}

void outputPort(byte portNumber, byte portValue, byte forceSend)
{
  // pins not configured as INPUT are cleared to zeros
  portValue = portValue & portConfigInputs[portNumber];
  // only send if the value is different than previously sent
  if (forceSend || previousPINs[portNumber] != portValue) {
    Firmata.sendDigitalPort(portNumber, portValue);
    previousPINs[portNumber] = portValue;
  }
}

/* -----------------------------------------------------------------------------

 * check all the active digital inputs for change of state, then add any events

 * to the Serial output queue using Serial.print() */
# 236 "C:\\Users\\Anirudh\\Documents\\Arduino\\firmataArd\\firmataArd.ino"
void checkDigitalInputs(void)
{
  /* Using non-looping code allows constants to be given to readPort().

   * The compiler will apply substantial optimizations if the inputs

   * to readPort() are compile-time constants. */
# 241 "C:\\Users\\Anirudh\\Documents\\Arduino\\firmataArd\\firmataArd.ino"
  if (((20 /* 14 digital + 6 analog*/ + 7) / 8) > 0 && reportPINs[0]) outputPort(0, readPort(0, portConfigInputs[0]), false);
  if (((20 /* 14 digital + 6 analog*/ + 7) / 8) > 1 && reportPINs[1]) outputPort(1, readPort(1, portConfigInputs[1]), false);
  if (((20 /* 14 digital + 6 analog*/ + 7) / 8) > 2 && reportPINs[2]) outputPort(2, readPort(2, portConfigInputs[2]), false);
  if (((20 /* 14 digital + 6 analog*/ + 7) / 8) > 3 && reportPINs[3]) outputPort(3, readPort(3, portConfigInputs[3]), false);
  if (((20 /* 14 digital + 6 analog*/ + 7) / 8) > 4 && reportPINs[4]) outputPort(4, readPort(4, portConfigInputs[4]), false);
  if (((20 /* 14 digital + 6 analog*/ + 7) / 8) > 5 && reportPINs[5]) outputPort(5, readPort(5, portConfigInputs[5]), false);
  if (((20 /* 14 digital + 6 analog*/ + 7) / 8) > 6 && reportPINs[6]) outputPort(6, readPort(6, portConfigInputs[6]), false);
  if (((20 /* 14 digital + 6 analog*/ + 7) / 8) > 7 && reportPINs[7]) outputPort(7, readPort(7, portConfigInputs[7]), false);
  if (((20 /* 14 digital + 6 analog*/ + 7) / 8) > 8 && reportPINs[8]) outputPort(8, readPort(8, portConfigInputs[8]), false);
  if (((20 /* 14 digital + 6 analog*/ + 7) / 8) > 9 && reportPINs[9]) outputPort(9, readPort(9, portConfigInputs[9]), false);
  if (((20 /* 14 digital + 6 analog*/ + 7) / 8) > 10 && reportPINs[10]) outputPort(10, readPort(10, portConfigInputs[10]), false);
  if (((20 /* 14 digital + 6 analog*/ + 7) / 8) > 11 && reportPINs[11]) outputPort(11, readPort(11, portConfigInputs[11]), false);
  if (((20 /* 14 digital + 6 analog*/ + 7) / 8) > 12 && reportPINs[12]) outputPort(12, readPort(12, portConfigInputs[12]), false);
  if (((20 /* 14 digital + 6 analog*/ + 7) / 8) > 13 && reportPINs[13]) outputPort(13, readPort(13, portConfigInputs[13]), false);
  if (((20 /* 14 digital + 6 analog*/ + 7) / 8) > 14 && reportPINs[14]) outputPort(14, readPort(14, portConfigInputs[14]), false);
  if (((20 /* 14 digital + 6 analog*/ + 7) / 8) > 15 && reportPINs[15]) outputPort(15, readPort(15, portConfigInputs[15]), false);
}

// -----------------------------------------------------------------------------
/* sets the pin mode to the correct state and sets the relevant bits in the

 * two bit-arrays that track Digital I/O and PWM status

 */
# 263 "C:\\Users\\Anirudh\\Documents\\Arduino\\firmataArd\\firmataArd.ino"
void setPinModeCallback(byte pin, int mode)
{
  if (Firmata.getPinMode(pin) == firmata::PIN_MODE_IGNORE /* pin configured to be ignored by digitalWrite and capabilityResponse*/)
    return;

  if (Firmata.getPinMode(pin) == firmata::PIN_MODE_I2C /* pin included in I2C setup*/ && isI2CEnabled && mode != firmata::PIN_MODE_I2C /* pin included in I2C setup*/) {
    // disable i2c so pins can be used for other functions
    // the following if statements should reconfigure the pins properly
    disableI2CPins();
  }
  if (((pin) >= 2 && (pin) <= 19) && mode != firmata::PIN_MODE_SERVO /* digital pin in Servo output mode*/) {
    if (servoPinMap[pin] < (_Nbr_16timers * 12 /* the maximum number of servos controlled by one timer */) && servos[servoPinMap[pin]].attached()) {
      detachServo(pin);
    }
  }
  if (((pin) >= 14 && (pin) < 14 + 6)) {
    reportAnalogCallback(((pin) - 14), mode == firmata::PIN_MODE_ANALOG /* analog pin in analogInput mode*/ ? 1 : 0); // turn on/off reporting
  }
  if (((pin) >= 2 && (pin) <= 19)) {
    if (mode == 0x0 || mode == firmata::PIN_MODE_PULLUP /* enable internal pull-up resistor for pin*/) {
      portConfigInputs[pin / 8] |= (1 << (pin & 7));
    } else {
      portConfigInputs[pin / 8] &= ~(1 << (pin & 7));
    }
  }
  Firmata.setPinState(pin, 0);
  switch (mode) {
    case firmata::PIN_MODE_ANALOG /* analog pin in analogInput mode*/:
      if (((pin) >= 14 && (pin) < 14 + 6)) {
        if (((pin) >= 2 && (pin) <= 19)) {
          pinMode((pin), 0x0); // disable output driver




        }
        Firmata.setPinMode(pin, firmata::PIN_MODE_ANALOG /* analog pin in analogInput mode*/);
      }
      break;
    case 0x0:
      if (((pin) >= 2 && (pin) <= 19)) {
        pinMode((pin), 0x0); // disable output driver




        Firmata.setPinMode(pin, 0x0);
      }
      break;
    case firmata::PIN_MODE_PULLUP /* enable internal pull-up resistor for pin*/:
      if (((pin) >= 2 && (pin) <= 19)) {
        pinMode((pin), 0x2);
        Firmata.setPinMode(pin, firmata::PIN_MODE_PULLUP /* enable internal pull-up resistor for pin*/);
        Firmata.setPinState(pin, 1);
      }
      break;
    case 0x1:
      if (((pin) >= 2 && (pin) <= 19)) {
        if (Firmata.getPinMode(pin) == firmata::PIN_MODE_PWM /* digital pin in PWM output mode*/) {
          // Disable PWM if pin mode was previously set to PWM.
          digitalWrite((pin), 0x0);
        }
        pinMode((pin), 0x1);
        Firmata.setPinMode(pin, 0x1);
      }
      break;
    case firmata::PIN_MODE_PWM /* digital pin in PWM output mode*/:
      if (((pin) == 3 || (pin) == 5 || (pin) == 6 || (pin) == 9 || (pin) == 10 || (pin) == 11)) {
        pinMode((pin), 0x1);
        analogWrite((pin), 0);
        Firmata.setPinMode(pin, firmata::PIN_MODE_PWM /* digital pin in PWM output mode*/);
      }
      break;
    case firmata::PIN_MODE_SERVO /* digital pin in Servo output mode*/:
      if (((pin) >= 2 && (pin) <= 19)) {
        Firmata.setPinMode(pin, firmata::PIN_MODE_SERVO /* digital pin in Servo output mode*/);
        if (servoPinMap[pin] == 255 || !servos[servoPinMap[pin]].attached()) {
          // pass -1 for min and max pulse values to use default values set
          // by Servo library
          attachServo(pin, -1, -1);
        }
      }
      break;
    case firmata::PIN_MODE_I2C /* pin included in I2C setup*/:
      if (((pin) == 18 || (pin) == 19)) {
        // mark the pin as i2c
        // the user must call I2C_CONFIG to enable I2C for a device
        Firmata.setPinMode(pin, firmata::PIN_MODE_I2C /* pin included in I2C setup*/);
      }
      break;
    case firmata::PIN_MODE_SERIAL /* pin configured for serial communication*/:



      break;
    default:
      Firmata.sendString("Unknown pin mode"); // TODO: put error msgs in EEPROM
  }
  // TODO: save status to EEPROM here, if changed
}

/*

 * Sets the value of an individual pin. Useful if you want to set a pin value but

 * are not tracking the digital port state.

 * Can only be used on pins configured as OUTPUT.

 * Cannot be used to enable pull-ups on Digital INPUT pins.

 */
# 370 "C:\\Users\\Anirudh\\Documents\\Arduino\\firmataArd\\firmataArd.ino"
void setPinValueCallback(byte pin, int value)
{
  if (pin < 20 /* 14 digital + 6 analog*/ && ((pin) >= 2 && (pin) <= 19)) {
    if (Firmata.getPinMode(pin) == 0x1) {
      Firmata.setPinState(pin, value);
      digitalWrite((pin), value);
    }
  }
}

void analogWriteCallback(byte pin, int value)
{
  if (pin < 20 /* 14 digital + 6 analog*/) {
    switch (Firmata.getPinMode(pin)) {
      case firmata::PIN_MODE_SERVO /* digital pin in Servo output mode*/:
        if (((pin) >= 2 && (pin) <= 19))
          servos[servoPinMap[pin]].write(value);
        Firmata.setPinState(pin, value);
        break;
      case firmata::PIN_MODE_PWM /* digital pin in PWM output mode*/:
        if (((pin) == 3 || (pin) == 5 || (pin) == 6 || (pin) == 9 || (pin) == 10 || (pin) == 11))
          analogWrite((pin), value);
        Firmata.setPinState(pin, value);
        break;
    }
  }
}

void digitalWriteCallback(byte port, int value)
{
  byte pin, lastPin, pinValue, mask = 1, pinWriteMask = 0;

  if (port < ((20 /* 14 digital + 6 analog*/ + 7) / 8)) {
    // create a mask of the pins on this port that are writable.
    lastPin = port * 8 + 8;
    if (lastPin > 20 /* 14 digital + 6 analog*/) lastPin = 20 /* 14 digital + 6 analog*/;
    for (pin = port * 8; pin < lastPin; pin++) {
      // do not disturb non-digital pins (eg, Rx & Tx)
      if (((pin) >= 2 && (pin) <= 19)) {
        // do not touch pins in PWM, ANALOG, SERVO or other modes
        if (Firmata.getPinMode(pin) == 0x1 || Firmata.getPinMode(pin) == 0x0) {
          pinValue = ((byte)value & mask) ? 1 : 0;
          if (Firmata.getPinMode(pin) == 0x1) {
            pinWriteMask |= mask;
          } else if (Firmata.getPinMode(pin) == 0x0 && pinValue == 1 && Firmata.getPinState(pin) != 1) {
            // only handle INPUT here for backwards compatibility

            pinMode(pin, 0x2);




          }
          Firmata.setPinState(pin, pinValue);
        }
      }
      mask = mask << 1;
    }
    writePort(port, (byte)value, pinWriteMask);
  }
}


// -----------------------------------------------------------------------------
/* sets bits in a bit array (int) to toggle the reporting of the analogIns

 */
# 436 "C:\\Users\\Anirudh\\Documents\\Arduino\\firmataArd\\firmataArd.ino"
//void FirmataClass::setAnalogPinReporting(byte pin, byte state) {
//}
void reportAnalogCallback(byte analogPin, int value)
{
  if (analogPin < 6) {
    if (value == 0) {
      analogInputsToReport = analogInputsToReport & ~ (1 << analogPin);
    } else {
      analogInputsToReport = analogInputsToReport | (1 << analogPin);
      // prevent during system reset or all analog pin values will be reported
      // which may report noise for unconnected analog pins
      if (!isResetting) {
        // Send pin value immediately. This is helpful when connected via
        // ethernet, wi-fi or bluetooth so pin states can be known upon
        // reconnecting.
        Firmata.sendAnalog(analogPin, analogRead(analogPin));
      }
    }
  }
  // TODO: save status to EEPROM here, if changed
}

void reportDigitalCallback(byte port, int value)
{
  if (port < ((20 /* 14 digital + 6 analog*/ + 7) / 8)) {
    reportPINs[port] = (byte)value;
    // Send port value immediately. This is helpful when connected via
    // ethernet, wi-fi or bluetooth so pin states can be known upon
    // reconnecting.
    if (value) outputPort(port, readPort(port, portConfigInputs[port]), true);
  }
  // do not disable analog reporting on these 8 pins, to allow some
  // pins used for digital, others analog.  Instead, allow both types
  // of reporting to be enabled, but check if the pin is configured
  // as analog when sampling the analog inputs.  Likewise, while
  // scanning digital pins, portConfigInputs will mask off values from any
  // pins configured as analog
}

/*==============================================================================

 * SYSEX-BASED commands

 *============================================================================*/
# 479 "C:\\Users\\Anirudh\\Documents\\Arduino\\firmataArd\\firmataArd.ino"
void sysexCallback(byte command, byte argc, byte *argv)
{
  byte mode;
  byte stopTX;
  byte slaveAddress;
  byte data;
  int slaveRegister;
  unsigned int delayTime;

  switch (command) {
    case firmata::I2C_REQUEST /* send an I2C read/write request*/:
      mode = argv[1] & 24;
      if (argv[1] & 32) {
        Firmata.sendString("10-bit addressing not supported");
        return;
      }
      else {
        slaveAddress = argv[0];
      }

      // need to invert the logic here since 0 will be default for client
      // libraries that have not updated to add support for restart tx
      if (argv[1] & 64) {
        stopTX = 0;
      }
      else {
        stopTX = 1; // default
      }

      switch (mode) {
        case 0:
          Wire.beginTransmission(slaveAddress);
          for (byte i = 2; i < argc; i += 2) {
            data = argv[i] + (argv[i + 1] << 7);
            wireWrite(data);
          }
          Wire.endTransmission();
          delayMicroseconds(70);
          break;
        case 8:
          if (argc == 6) {
            // a slave register is specified
            slaveRegister = argv[2] + (argv[3] << 7);
            data = argv[4] + (argv[5] << 7); // bytes to read
          }
          else {
            // a slave register is NOT specified
            slaveRegister = -1;
            data = argv[2] + (argv[3] << 7); // bytes to read
          }
          readAndReportData(slaveAddress, (int)slaveRegister, data, stopTX);
          break;
        case 16:
          if ((queryIndex + 1) >= 8) {
            // too many queries, just ignore
            Firmata.sendString("too many queries");
            break;
          }
          if (argc == 6) {
            // a slave register is specified
            slaveRegister = argv[2] + (argv[3] << 7);
            data = argv[4] + (argv[5] << 7); // bytes to read
          }
          else {
            // a slave register is NOT specified
            slaveRegister = (int)-1;
            data = argv[2] + (argv[3] << 7); // bytes to read
          }
          queryIndex++;
          query[queryIndex].addr = slaveAddress;
          query[queryIndex].reg = slaveRegister;
          query[queryIndex].bytes = data;
          query[queryIndex].stopTX = stopTX;
          break;
        case 24:
          byte queryIndexToSkip;
          // if read continuous mode is enabled for only 1 i2c device, disable
          // read continuous reporting for that device
          if (queryIndex <= 0) {
            queryIndex = -1;
          } else {
            queryIndexToSkip = 0;
            // if read continuous mode is enabled for multiple devices,
            // determine which device to stop reading and remove it's data from
            // the array, shifiting other array data to fill the space
            for (byte i = 0; i < queryIndex + 1; i++) {
              if (query[i].addr == slaveAddress) {
                queryIndexToSkip = i;
                break;
              }
            }

            for (byte i = queryIndexToSkip; i < queryIndex + 1; i++) {
              if (i < 8) {
                query[i].addr = query[i + 1].addr;
                query[i].reg = query[i + 1].reg;
                query[i].bytes = query[i + 1].bytes;
                query[i].stopTX = query[i + 1].stopTX;
              }
            }
            queryIndex--;
          }
          break;
        default:
          break;
      }
      break;
    case firmata::I2C_CONFIG /* config I2C settings such as delay times and power pins*/:
      delayTime = (argv[0] + (argv[1] << 7));

      if (argc > 1 && delayTime > 0) {
        i2cReadDelayTime = delayTime;
      }

      if (!isI2CEnabled) {
        enableI2CPins();
      }

      break;
    case firmata::SERVO_CONFIG /* set max angle, minPulse, maxPulse, freq*/:
      if (argc > 4) {
        // these vars are here for clarity, they'll optimized away by the compiler
        byte pin = argv[0];
        int minPulse = argv[1] + (argv[2] << 7);
        int maxPulse = argv[3] + (argv[4] << 7);

        if (((pin) >= 2 && (pin) <= 19)) {
          if (servoPinMap[pin] < (_Nbr_16timers * 12 /* the maximum number of servos controlled by one timer */) && servos[servoPinMap[pin]].attached()) {
            detachServo(pin);
          }
          attachServo(pin, minPulse, maxPulse);
          setPinModeCallback(pin, firmata::PIN_MODE_SERVO /* digital pin in Servo output mode*/);
        }
      }
      break;
    case firmata::SAMPLING_INTERVAL /* set the poll rate of the main loop*/:
      if (argc > 1) {
        samplingInterval = argv[0] + (argv[1] << 7);
        if (samplingInterval < 1) {
          samplingInterval = 1;
        }
      } else {
        //Firmata.sendString("Not enough data");
      }
      break;
    case firmata::EXTENDED_ANALOG /* analog write (PWM, Servo, etc) to any pin*/:
      if (argc > 1) {
        int val = argv[1];
        if (argc > 2) val |= (argv[2] << 7);
        if (argc > 3) val |= (argv[3] << 14);
        analogWriteCallback(argv[0], val);
      }
      break;
    case firmata::CAPABILITY_QUERY /* ask for supported modes and resolution of all pins*/:
      Firmata.write(firmata::START_SYSEX /* start a MIDI Sysex message*/);
      Firmata.write(firmata::CAPABILITY_RESPONSE /* reply with supported modes and resolution*/);
      for (byte pin = 0; pin < 20 /* 14 digital + 6 analog*/; pin++) {
        if (((pin) >= 2 && (pin) <= 19)) {
          Firmata.write((byte)0x0);
          Firmata.write(1);
          Firmata.write((byte)firmata::PIN_MODE_PULLUP /* enable internal pull-up resistor for pin*/);
          Firmata.write(1);
          Firmata.write((byte)0x1);
          Firmata.write(1);
        }
        if (((pin) >= 14 && (pin) < 14 + 6)) {
          Firmata.write(firmata::PIN_MODE_ANALOG /* analog pin in analogInput mode*/);
          Firmata.write(10); // 10 = 10-bit resolution
        }
        if (((pin) == 3 || (pin) == 5 || (pin) == 6 || (pin) == 9 || (pin) == 10 || (pin) == 11)) {
          Firmata.write(firmata::PIN_MODE_PWM /* digital pin in PWM output mode*/);
          Firmata.write(8);
        }
        if (((pin) >= 2 && (pin) <= 19)) {
          Firmata.write(firmata::PIN_MODE_SERVO /* digital pin in Servo output mode*/);
          Firmata.write(14);
        }
        if (((pin) == 18 || (pin) == 19)) {
          Firmata.write(firmata::PIN_MODE_I2C /* pin included in I2C setup*/);
          Firmata.write(1); // TODO: could assign a number to map to SCL or SDA
        }



        Firmata.write(127);
      }
      Firmata.write(firmata::END_SYSEX /* end a MIDI Sysex message*/);
      break;
    case firmata::PIN_STATE_QUERY /* ask for a pin's current mode and value*/:
      if (argc > 0) {
        byte pin = argv[0];
        Firmata.write(firmata::START_SYSEX /* start a MIDI Sysex message*/);
        Firmata.write(firmata::PIN_STATE_RESPONSE /* reply with pin's current mode and value*/);
        Firmata.write(pin);
        if (pin < 20 /* 14 digital + 6 analog*/) {
          Firmata.write(Firmata.getPinMode(pin));
          Firmata.write((byte)Firmata.getPinState(pin) & 0x7F);
          if (Firmata.getPinState(pin) & 0xFF80) Firmata.write((byte)(Firmata.getPinState(pin) >> 7) & 0x7F);
          if (Firmata.getPinState(pin) & 0xC000) Firmata.write((byte)(Firmata.getPinState(pin) >> 14) & 0x7F);
        }
        Firmata.write(firmata::END_SYSEX /* end a MIDI Sysex message*/);
      }
      break;
    case firmata::ANALOG_MAPPING_QUERY /* ask for mapping of analog to pin numbers*/:
      Firmata.write(firmata::START_SYSEX /* start a MIDI Sysex message*/);
      Firmata.write(firmata::ANALOG_MAPPING_RESPONSE /* reply with mapping info*/);
      for (byte pin = 0; pin < 20 /* 14 digital + 6 analog*/; pin++) {
        Firmata.write(((pin) >= 14 && (pin) < 14 + 6) ? ((pin) - 14) : 127);
      }
      Firmata.write(firmata::END_SYSEX /* end a MIDI Sysex message*/);
      break;

    case firmata::SERIAL_DATA /* communicate with serial devices, including other boards*/:



      break;
  }
}

/*==============================================================================

 * STRING RETURN FUNCTION

 *=============================================================================*/
# 702 "C:\\Users\\Anirudh\\Documents\\Arduino\\firmataArd\\firmataArd.ino"
 const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs,en,d4,d5,d6,d7);
int lastLine=0;
void stringCallBack(char *stringData)
{
  if(lastLine)
  {
    lastLine=0;
    lcd.clear();
  }
  else
  {
    lastLine=1;
    lcd.setCursor(0,1);
  }
  lcd.print(stringData);
//  String data=stringData;
//  if(data.equals("hello")) 
//  {
//    myservo.write(180);
//    delay(500);
//    myservo.write(0);
//  }
}

/*==============================================================================

 * SETUP()

 *============================================================================*/
# 731 "C:\\Users\\Anirudh\\Documents\\Arduino\\firmataArd\\firmataArd.ino"
void systemResetCallback()
{
  isResetting = true;

  // initialize a defalt state
  // TODO: option to load config from EEPROM instead of default





  if (isI2CEnabled) {
    disableI2CPins();
  }

  for (byte i = 0; i < ((20 /* 14 digital + 6 analog*/ + 7) / 8); i++) {
    reportPINs[i] = false; // by default, reporting off
    portConfigInputs[i] = 0; // until activated
    previousPINs[i] = 0;
  }

  for (byte i = 0; i < 20 /* 14 digital + 6 analog*/; i++) {
    // pins with analog capability default to analog input
    // otherwise, pins default to digital output
    if (((i) >= 14 && (i) < 14 + 6)) {
      // turns off pullup, configures everything
      setPinModeCallback(i, firmata::PIN_MODE_ANALOG /* analog pin in analogInput mode*/);
    } else if (((i) >= 2 && (i) <= 19)) {
      // sets the output to 0, configures portConfigInputs
      setPinModeCallback(i, 0x1);
    }

    servoPinMap[i] = 255;
  }
  // by default, do not report any analog inputs
  analogInputsToReport = 0;

  detachedServoCount = 0;
  servoCount = 0;

  /* send digital inputs to set the initial state on the host computer,

   * since once in the loop(), this firmware will only send on change */
# 773 "C:\\Users\\Anirudh\\Documents\\Arduino\\firmataArd\\firmataArd.ino"
  /*

  TODO: this can never execute, since no pins default to digital input

        but it will be needed when/if we support EEPROM stored config

  for (byte i=0; i < TOTAL_PORTS; i++) {

    outputPort(i, readPort(i, portConfigInputs[i]), true);

  }

  */
# 780 "C:\\Users\\Anirudh\\Documents\\Arduino\\firmataArd\\firmataArd.ino"
  isResetting = false;
}

void setup()
{
  Firmata.setFirmwareNameAndVersion("C:\\Users\\Anirudh\\Documents\\Arduino\\firmataArd\\firmataArd.ino", firmata::FIRMWARE_MAJOR_VERSION, firmata::FIRMWARE_MINOR_VERSION);
  lcd.begin(16,2);
  Firmata.attach(firmata::ANALOG_MESSAGE /* send data for an analog pin (or PWM)*/, analogWriteCallback);
  Firmata.attach(firmata::DIGITAL_MESSAGE /* send data for a digital port (collection of 8 pins)*/, digitalWriteCallback);
  Firmata.attach(firmata::REPORT_ANALOG /* enable analog input by pin #*/, reportAnalogCallback);
  Firmata.attach(firmata::REPORT_DIGITAL /* enable digital input by port pair*/, reportDigitalCallback);
  Firmata.attach(firmata::SET_PIN_MODE /* set a pin to INPUT/OUTPUT/PWM/etc*/, setPinModeCallback);
  Firmata.attach(firmata::SET_DIGITAL_PIN_VALUE /* set value of an individual digital pin*/, setPinValueCallback);
  Firmata.attach(firmata::START_SYSEX /* start a MIDI Sysex message*/, sysexCallback);
  Firmata.attach(firmata::SYSTEM_RESET /* reset from MIDI*/, systemResetCallback);
  Firmata.attach(firmata::STRING_DATA /* a string message with 14-bits per char*/,stringCallBack);
  // to use a port other than Serial, such as Serial1 on an Arduino Leonardo or Mega,
  // Call begin(baud) on the alternate serial port and pass it to Firmata to begin like this:
  // Serial1.begin(57600);
  // Firmata.begin(Serial1);
  // However do not do this if you are using SERIAL_MESSAGE

  Firmata.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for ATmega32u4-based boards and Arduino 101
  }

  systemResetCallback(); // reset to default config
}

/*==============================================================================

 * LOOP()

 *============================================================================*/
# 813 "C:\\Users\\Anirudh\\Documents\\Arduino\\firmataArd\\firmataArd.ino"
void loop()
{
  byte pin, analogPin;

  /* DIGITALREAD - as fast as possible, check for changes and output them to the

   * FTDI buffer using Serial.print()  */
# 819 "C:\\Users\\Anirudh\\Documents\\Arduino\\firmataArd\\firmataArd.ino"
  checkDigitalInputs();

  /* STREAMREAD - processing incoming messagse as soon as possible, while still

   * checking digital inputs.  */
# 823 "C:\\Users\\Anirudh\\Documents\\Arduino\\firmataArd\\firmataArd.ino"
  while (Firmata.available())
    Firmata.processInput();

  // TODO - ensure that Stream buffer doesn't go over 60 bytes

  currentMillis = millis();
  if (currentMillis - previousMillis > samplingInterval) {
    previousMillis += samplingInterval;
    /* ANALOGREAD - do all analogReads() at the configured sampling interval */
    for (pin = 0; pin < 20 /* 14 digital + 6 analog*/; pin++) {
      if (((pin) >= 14 && (pin) < 14 + 6) && Firmata.getPinMode(pin) == firmata::PIN_MODE_ANALOG /* analog pin in analogInput mode*/) {
        analogPin = ((pin) - 14);
        if (analogInputsToReport & (1 << analogPin)) {
          Firmata.sendAnalog(analogPin, analogRead(analogPin));
        }
      }
    }
    // report i2c data for all device with read continuous mode enabled
    if (queryIndex > -1) {
      for (byte i = 0; i < queryIndex + 1; i++) {
        readAndReportData(query[i].addr, query[i].reg, query[i].bytes, query[i].stopTX);
      }
    }
  }




}
