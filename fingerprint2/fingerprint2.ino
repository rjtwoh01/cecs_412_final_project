/*************************************************** 
  This is an example sketch for our optical Fingerprint sensor

  Designed specifically to work with the Adafruit BMP085 Breakout 
  ----> http://www.adafruit.com/products/751

  These displays use TTL Serial to communicate, 2 pins are required to 
  interface
  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/


#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <ctype.h>
#include "Adafruit_Trellis.h"

#define bit9600Delay 84  
#define halfBit9600Delay 42
#define bit4800Delay 188 
#define halfBit4800Delay 94
#define MOMENTARY 0
#define LATCHING 1
// set the mode here
#define MODE LATCHING

Adafruit_Trellis matrix0 = Adafruit_Trellis();
Adafruit_TrellisSet trellis =  Adafruit_TrellisSet(&matrix0);

#define NUMTRELLIS 1
#define numKeys (NUMTRELLIS * 16)

byte rx = 6;
byte tx = 7;
byte SWval;

int getFingerprintIDez();

int getKeyPadButton();

int runKeypad();

void updateTime();

int keypadLoop();

void updateUserCombination(int result);

bool checkValid();

int correctCombination[4] = { 4, 1, 2, 1 };
int userCombination[4] = { 0, 0, 0, 0 };
int keypadLoop();
int index = 0;
int globalFingerResult = 0;
int success = 0;
bool youHaveTime = false;
unsigned long timer;



// pin #2 is IN from sensor (GREEN wire)
// pin #3 is OUT from arduino  (WHITE wire)
SoftwareSerial mySerial(2, 3);



Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
#define INTPIN A2

void setup()  
{
  Serial.begin(9600);
  //Serial.println("fingertest");

  // set the data rate for the sensor serial port
  finger.begin(57600);
  trellis.begin(0x70);  // only one

  // INT pin requires a pullup
  pinMode(INTPIN, INPUT);
  digitalWrite(INTPIN, HIGH);
}

// Numpad first then fingerprint
// You have 8 seconds from first key press to fingerprint scan 
void loop() // run over and over again
{
  //Wait for numpad press
  int result = keypadLoop();
//  int fingerResult = getFingerprintIDez();
//  if (fingerResult > 0) {
//    globalFingerResult = fingerResult; 
//  }
  if(result > -1){
    timer = millis();
    updateUserCombination(result);
    youHaveTime = true;
    while( youHaveTime == true){
      result = keypadLoop();
      if(result > 0){
        updateUserCombination(result);
        Serial.print(result-1);
      }
      int fingerResult = getFingerprintIDez();
      if (fingerResult > 0) {
        globalFingerResult = fingerResult;
        Serial.print(fingerResult);
      }
      bool isValid = checkValid();
      if(isValid){
        Serial.write(finger.fingerID);
        success = 0;
        globalFingerResult = 0;
        updateTime();
      }
      delay(50);
    }
  }
  
  delay(50);            //don't ned to run this at full speed.
}

int keypadLoop() {
  delay(30); // 30ms delay is required, dont remove me!
  
  if (MODE == MOMENTARY) {
    // If a button was just pressed or released...
    if (trellis.readSwitches()) {
      // go through every button
      for (uint8_t i=0; i<numKeys; i++) {
        // if it was pressed, turn it on
        if (trellis.justPressed(i)) {
//          Serial.print("v"); Serial.println(i);
          return i+1;
          trellis.setLED(i);
        } 
        // if it was released, turn it off
        if (trellis.justReleased(i)) {
//          Serial.print("^"); Serial.println(i);
          return i+1;
          trellis.clrLED(i);
        }
      }
      // tell the trellis to set the LEDs we requested
      trellis.writeDisplay();
    }
  }

  if (MODE == LATCHING) {
    // If a button was just pressed or released...
    if (trellis.readSwitches()) {
      // go through every button
      for (uint8_t i=0; i<numKeys; i++) {
        // if it was pressed...
        if (trellis.justPressed(i)) {
//        Serial.print("v"); Serial.println(i);
        return i+1;
        // Alternate the LED
        if (trellis.isLED(i))
          trellis.clrLED(i);
        else
          trellis.setLED(i);
        } 
      }
      // tell the trellis to set the LEDs we requested
      trellis.writeDisplay();
    }
  }
  return -1;
}

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      //Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      //Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      //Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      //Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      //Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      //Serial.println("Could not find fingerprint features");
      return p;
    default:
      //Serial.println("Unknown error");
      return p;
  }
  
  // OK converted!
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    //Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    //Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return p;
  } else {
    //Serial.println("Unknown error");
    return p;
  }   
  
  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID); 
  Serial.print(" with confidence of "); Serial.println(finger.confidence); 
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;
  
  // found a match!
  /*Serial.print("");*/ //Serial.write(finger.fingerID); 
   
  return finger.fingerID; 
}

//return 1 if success, 0 if fail
int runKeypad() {
  int numOfKeysRequired = 4;
  int index = 0;
  while (index < numOfKeysRequired) {
    int key = keypadLoop();
    userCombination[index] = key;
    //Serial.println(userCombination[index]);
    index++;
  }


  return 1;
}

void updateTime(){
  youHaveTime = false;
}

void updateUserCombination(int result){
  userCombination[0] = userCombination[1];
  userCombination[1] = userCombination[2];
  userCombination[2] = userCombination[3];
  userCombination[3] = result - 1;
}

bool checkValid(){
  if((millis()-timer) >8000){
    youHaveTime = false;
  }
  for (int i = 0; i <= 3; i++) {
    if (userCombination[0] == correctCombination[0] && userCombination[1] == correctCombination[1] && userCombination[2] == correctCombination[2] && userCombination[3] == correctCombination[3] && globalFingerResult > 0) {
      success = 1;
      for (int j = 0; j <= 3; j++) {
        updateUserCombination(0);
      }
      return true;
    } else {
      success = 0;
      return false;
    }
  }
}

