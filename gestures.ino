//https://learn.adafruit.com/lsm303-accelerometer-slash-compass-breakout/calibration
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>



Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(54321);
Adafruit_LSM303_Mag_Unified mag = Adafruit_LSM303_Mag_Unified(12345);

float xmax = 0;
float x = 0;
const float eGravity = 9.78;
const float compass = 60.0;
const int numberOfAxis = 6;
const int numberOfReadings = 18 * 2;
int index = 0;
// readings from the calibration run.
//acc x, y z, mag x, y, z:
float minReadings[numberOfAxis] = 
  {-13.45, -13.18, -10.83, -43.27, -50.0, -56.12};
float maxReadings[numberOfAxis] = 
  {13.61, 11.96, 11.34, 57.09, 49.73, 52.04};
float readings[numberOfAxis];
float readingSums[numberOfAxis];
float sreadings[numberOfAxis][numberOfReadings];
unsigned long lastprint = 0;

int ledPinBlue = 11;
int ledPinGreen = 9;
int ledPinRed = 10;
double pi2 = 1570;

boolean orangeLightOn = false;
boolean greenLightOn = false;
boolean redLightOn = false;

unsigned long liftedUpTime = 0;
unsigned long firstLiftedUpTime = 0;
unsigned long wrongpassTime = 0;
boolean liftedUp = false;
boolean wrongPass = false;
boolean unlocked = false;




void setup (void) {
  Serial.begin(115200);
  if (!mag.begin()) { Serial.println("w: mag"); }
  if (!accel.begin()) { Serial.println("w: accel"); }
  if (!accel.begin() || !mag.begin()) {
    Serial.println("din't work out, did it?");
    while(1);
  }
  pinMode(ledPinBlue, OUTPUT);
  pinMode(ledPinGreen, OUTPUT);
  pinMode(ledPinRed, OUTPUT);
} 



void loop(void) {
  sensors_event_t accevent;
  sensors_event_t magevent;
  accel.getEvent(&accevent);
  mag.getEvent(&magevent);


  readings[0] = accevent.acceleration.x;
  readings[1] = accevent.acceleration.y;
  readings[2] = accevent.acceleration.z;
  readings[3] = magevent.magnetic.x;
  readings[4] = magevent.magnetic.y;
  readings[5] = magevent.magnetic.z;
  populateReadings();



  //    0  1 2      3  4  5
  //acc x, y z, mag x, y, z:
  if (map(2, eGravity) > 9.2f && !unlocked) {
    orangeLight(true);
    liftedUpTime = millis();
    Serial.print(firstLiftedUpTime);
    if (firstLiftedUpTime == 0) {
      Serial.println("Lifted up first time reg.");
      firstLiftedUpTime = liftedUpTime;
    }
    liftedUp = true;
  }

  if (!unlocked && millis() - liftedUpTime > 10000) {
    liftedUp = false;
  }
  if (!unlocked && firstLiftedUpTime != 0 
    && millis() - firstLiftedUpTime > 9000) {
    orangeLight(false);
    redLight(true);
    wrongPass = true;
    wrongpassTime = millis();
    firstLiftedUpTime = 0;
  }
  if (firstLiftedUpTime != 0 && millis() - firstLiftedUpTime > 20000) {
    Serial.println(firstLiftedUpTime);
    firstLiftedUpTime = 0;
  }
  if (!unlocked && wrongPass && millis() - wrongpassTime > 4000) {
    redLight(false);
    wrongPass = false;
  }
  if (map(0, eGravity) > 6 && (map(1, eGravity) > 3 || map(1, eGravity) < -3)) {
    greenLight(true);
    unlocked = true;
  }

  printReadings();
  if (!liftedUp && !unlocked && !wrongPass) ledPulse();
  /*
  Serial.print(liftedUp); Serial.print(" : ");
  Serial.print(unlocked); Serial.print(" : ");
  Serial.print(wrongPass); Serial.println();
  */
}


float map (int lindex, float multiplier) {
  return map(reading(lindex), minReadings[lindex], maxReadings[lindex],
    -multiplier, multiplier);
}



float map(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}



float populateReadings() {
  for (int i = 0; i < numberOfAxis; i++) {
    sreadings[i][index] = readings[i];
    if (index == numberOfReadings - 1) {
      readingSums[i] -= sreadings[i][0];
    }
    else {
      readingSums[i] -= sreadings[i][index + 1];
    }
    readingSums[i] += readings[i];
  }
  if (index++ >= numberOfReadings - 1) { 
    index = 0; 
  }
}



float reading(int lindex) {
  return readingSums[lindex]/numberOfReadings;
}



void orangeLight(boolean onoff) {
  if (onoff) {
    analogWrite(ledPinGreen,250);
    analogWrite(ledPinRed, 225);
    analogWrite(ledPinBlue, 0);
  } else {
    analogWrite(ledPinGreen, 0);
    analogWrite(ledPinRed, 0);
  }
}

void redLight(boolean onoff) {
  if (onoff) {
    analogWrite(ledPinRed, 225);
    analogWrite(ledPinBlue, 0);
    analogWrite(ledPinGreen, 0);
  } else {
    analogWrite(ledPinRed, 0);
  }
}

void greenLight(boolean onoff) {
  if (onoff) {
    analogWrite(ledPinGreen, 255);
    analogWrite(ledPinBlue, 0);
    analogWrite(ledPinRed, 0);
  } else {
    analogWrite(ledPinGreen, 0);
  }
}

void ledPulse() {
    analogWrite(ledPinBlue, map((long)(sin(pi2++/200)*100), -100, 100, 0, 255));
    //Serial.println( map((long)(sin(pi2++/100)*100), -100, 100, 0, 255));
}


boolean detectLiftUp() {
  orangeLightOn = true;
  return false;

}


void printReadings() {
  Serial.print(map(0, eGravity));
  Serial.print(":");
  Serial.print(map(1, eGravity));
  Serial.print(":");
  Serial.print(map(2, eGravity));
  Serial.print(":");
  Serial.print(map(3, compass));
  Serial.print(":");
  Serial.print(map(4, compass));
  Serial.print(":");
  Serial.print(map(5, compass));
  Serial.println();
}