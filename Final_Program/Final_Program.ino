// SIT107 Final Project
// Tim Tattersall 
// 217 288 933

#include <Servo.h>
#include <stdlib.h>
#include <string.h>
#include <SD.h>
#include <HX711.h>
#include <Wire.h>
#include <RTClib.h>


// Constant Pin Variables
const int buzzerPin = 8;
const int ledPin = 13;
const int irTransPin = 4;
const int irRecPin = A0;
const int dispensorPin = 9;
const int loadRXPin = 0;
const int loadTXPin = 1;
const int sdPin = 10;

// Constant Calibration Variables
const int irBaseLevel = 10;
const double speedOfLight = 299792458;
const float repositoryWidth = 0.5;
const float calibrationFactor = -0.00045;
const int feedTimesCount = 2;
const int feedTimes[feedTimesCount][2] = {
  {8, 30},
  {18, 30}
};

// Global Variables
int dispensorPos = 0;
char fileName[] = "DISP000.csv";
bool foodDispensed[feedTimesCount] = {false, false};

// Declare Library Classes
Servo dispensor;
RTC_DS1307 rtc;
HX711 scale(loadRXPin, loadTXPin);
File dataFile;

void setup() {
  Serial.begin(9600);
  
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(irTransPin, OUTPUT);
  
  pinMode(irRecPin, INPUT);

  dispensor.attach(dispensorPin);

  scale.set_scale(calibrationFactor);
  scale.tare();

  Wire.begin();
  if (!rtc.begin()) {
    Serial.println("ERROR: RTC had problems initializing");
  }

  if (!SD.begin(sdPin)) {
    Serial.println("ERROR: SD Card had problems initializing");
  }

  for (uint8_t i = 0; i < 999; i++) {
    fileName[5] = i / 10 + '0';
    fileName[6] = i % 10 + '0';
    if (!SD.exists(fileName)) {
      Serial.print("MESSAGE: File Created: ");
      Serial.println(String(fileName));
      dataFile = SD.open(fileName, FILE_WRITE);

      Serial.println("MESSAGE: Adding File Headings");
      dataFile.println("unix,utc,food level,bowl mass");
    }
  }
}

void loop() {
  DateTime now = rtc.now();
  bool foodLevel = foodAboveLevel();
  
  if (now.hour() == 0 && now.minute() == 0) {
    Serial.println("MESSAGE: Resetting the food dispensed flags");
    for (int i = 0; i < feedTimesCount; i++) {
      foodDispensed[i] = false;
    }
  }
  
  for (int i = 0; i < feedTimesCount; i++) {
    if (now.hour() == feedTimes[i][0] && now.minute() == feedTimes[i][1] && foodDispensed[i] == false) {
      Serial.println("MESSAGE: Dispensing Food");
      dispenseFood();
      foodDispensed[i] = true;
    }
  }

  if (foodLevel == false) {
    Serial.println("WARNING: Food repository needs to be topped up");
    digitalWrite(ledPin, HIGH);
  } else {
    digitalWrite(ledPin, LOW);
  }

  dataFile.print(now.unixtime());
  dataFile.print(",");
  dataFile.print(now.year(), DEC);
  dataFile.print("-");
  dataFile.print(now.month(), DEC);
  dataFile.print("-");
  dataFile.print(now.day(), DEC);
  dataFile.print(" ");
  dataFile.print(now.hour(), DEC);
  dataFile.print(":");
  dataFile.print(now.minute(), DEC);
  dataFile.print(":");
  dataFile.print(now.second(), DEC);
  dataFile.print(",");
  dataFile.print(foodLevel);
  dataFile.print(",");
  dataFile.println(getWeight());

  delay(1000);
}

float roundUp(float number, int places) {
  int power = pow(10, places);
  number = number * power;
  number = round(number);
  number = number / power;
  return number;
}

void dispenseFood() {
  dispensorPos += 90;
  dispensor.write(dispensorPos);
}

float getWeight() {
  return scale.get_units();
}

bool foodAboveLevel() {
  int timeTaken = 0;

  // Send IR pulse
  digitalWrite(irTransPin, HIGH);

  while (analogRead(irRecPin) <= irBaseLevel) {
    if (timeTaken >= 2000) break;
    timeTaken += 10;
    delay(10);
  }

  digitalWrite(irTransPin, LOW);

  // Calculate Distance
  float distance = (timeTaken * speedOfLight) / 2;
  distance = roundUp(distance, 3);

  if (distance <= repositoryWidth) {
    return true;
  }
  return false;
}

