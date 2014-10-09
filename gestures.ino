/*
 *
 * This sketch makes the use of the compass and accelerometer LSM303DLHC
 * breakout board from adafruit. It also uses the libraries for this breakout
 * board.  It recognizes gestures in a simplified maner by checking if certain
 * thresholds is breached.
 *
 * It also uses an adapted smoothing method, smoothing all 6 axis from the
 * LSM303 sensor. It puts several readings in an array and calculates the
 * average.
*/

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>



Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(54321);
Adafruit_LSM303_Mag_Unified mag = Adafruit_LSM303_Mag_Unified(12345);

float xmax = 0;
float x = 0;
const float eGravity = 9.78;
const float compass = 60.0;
/*
 * variables for smoothing of the readings.
 */
const int numberOfAxis = 6;
const int numberOfReadings = 36; // number of readings to run average from.
int index = 0;

// readings from the calibration run. Using the calibration sketch from Adafruit
// to find these.
//https://learn.adafruit.com/lsm303-accelerometer-slash-compass-breakout/calibration
// the order of the readings stored in the array:
// acc x, y z, mag x, y, z:
// calibration run lowest readings.
float minReadings[numberOfAxis] = 
  {-13.45, -13.18, -10.83, -43.27, -50.0, -56.12};
// calibration run highest readings.
float maxReadings[numberOfAxis] = 
  {13.61, 11.96, 11.34, 57.09, 49.73, 52.04};
float readings[numberOfAxis];
float readingSums[numberOfAxis];
float sreadings[numberOfAxis][numberOfReadings];


struct RGBpins {
  int blue;
  int green;
  int red;
};

RGBpins ledpins = { 11, 9, 10 };

double ledPulseCounter = 0;

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

  if (!accel.begin() || !mag.begin()) {
    Serial.println("din't work out. check your LSM303DLHC wiring.");
    Serial.println("going into a infinite loop now. press reset after rewiring.");
    while(1);
  }

  pinMode(ledpins.blue, OUTPUT);
  pinMode(ledpins.green, OUTPUT);
  pinMode(ledpins.red, OUTPUT);
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


  if (liftUp()) {
    runLiftUpSequence();
  }

  if (timeOut()) {
    runTimeOutSequence();
  }

  if (wrongPassTimeout()) {
    wrongPassReset();
  }

  if (correctGesture()) {
    unlock();
  }

  //printReadings();

  if (!liftedUp && !unlocked && !wrongPass) {
    ledPulse();
  }

  ///*
  Serial.print(liftedUp); Serial.print(" : ");
  Serial.print(unlocked); Serial.print(" : ");
  Serial.print(wrongPass); Serial.println();
  //*/
}



/* checks if the sensor is tilted the right way (earth gravity on the x axis > 6
 * (check your readings with uncomment of printReadings())) and if it's shaken
 * at the same time (above or below 3 on the y axis)
 */
boolean correctGesture() {
  return map(0, eGravity) > 6 
  && (map(1, eGravity) > 3 || map(1, eGravity) < -3);
}


void unlock() {
    firstLiftedUpTime = 0;
    greenLight(true);
    unlocked = true;
}

boolean timeOut() {
  int timeoutLength = 10000;
  return !unlocked && firstLiftedUpTime != 0 
    && millis() - firstLiftedUpTime > timeoutLength;
}

void runTimeOutSequence() {
    orangeLight(false);
    redLight(true);
    wrongPass = true;
    wrongpassTime = millis();
    firstLiftedUpTime = 0;
    liftedUp = false;
}


boolean wrongPassTimeout() {
  int timeoutLength = 4000;
  return !unlocked && wrongPass && millis() - wrongpassTime > timeoutLength;
}
void wrongPassReset() {
  firstLiftedUpTime = 0;
  redLight(false);
  wrongPass = false;
}


/* checks if the z-axis (the '2' variable in the readings array) is larger than
 * earth's gravity (9.2 is not larger than 9.8, but is like that because of
 * inaccurate calibration - uncomment the printReadings() call above to look at
 * your readings.
 */
boolean liftUp() {
  return map(2, eGravity) > 9.2f && !unlocked && !wrongPass;
}

void runLiftUpSequence() {
  orangeLight(true);
  liftedUpTime = millis();
  if (firstLiftedUpTime == 0) {
    firstLiftedUpTime = liftedUpTime;
  }
  liftedUp = true;
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



/*
 *
 */
float reading(int lindex) {
  return readingSums[lindex]/numberOfReadings;
}



void orangeLight(boolean onoff) {
  if (onoff) {
    analogWrite(ledpins.green,250);
    analogWrite(ledpins.red, 225);
    analogWrite(ledpins.blue, 0);
  } else {
    analogWrite(ledpins.green, 0);
    analogWrite(ledpins.red, 0);
  }
}

void redLight(boolean onoff) {
  if (onoff) {
    analogWrite(ledpins.red, 225);
    analogWrite(ledpins.blue, 0);
    analogWrite(ledpins.green, 0);
  } else {
    analogWrite(ledpins.red, 0);
  }
}

void greenLight(boolean onoff) {
  if (onoff) {
    analogWrite(ledpins.green, 255);
    analogWrite(ledpins.blue, 0);
    analogWrite(ledpins.red, 0);
  } else {
    analogWrite(ledpins.green, 0);
  }
}



/*
 * method that pulses the led when in idle mode.
 */
void ledPulse() {
    analogWrite(ledpins.blue, 
      map((long)(sin(ledPulseCounter++/200)*100), -100, 100, 0, 255));
}



/*
 * prints the readings to the serial port. Use a serial monitor to look at
 * readings. remember to match the baud rate of this sketch in your serial
 * monitor.
 */
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