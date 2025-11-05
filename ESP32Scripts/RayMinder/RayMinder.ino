/*
  DES222 - Task 3
  By Annika Kaul and Jofel Guevarra

  This is the official Script of the RayMinder: UV Smart Caps for Friends, which was created as Task 3 submission for DES222. The code is run on an ESP32-C3 Zero with 4 Buzzers,
  1 HMC5883L Compass, 1 Adafruit Analog UV Sensor, and a battery pack attached to it. The pins for each component can be found and changed in the code.
*/

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);

// Define pins for I2C communication with compass
#define SDA_PIN 6
#define SCL_PIN 7

// Define pins of the 4 buzzers
#define PIN_BUZZER0 10
#define PIN_BUZZER1 3
#define PIN_BUZZER2 4
#define PIN_BUZZER3 5

// Define pin of the uv sensor
#define PIN_UVSENSOR 2

#define UV_SAMPLE_SIZE 60

int uvMediumValue = 0;
int uvCurrentValue = 5; // Use 5 as an average starting value
int uvNextIndex = 1;
int skintype = 1;
int spf = 1;

int uvHistory[UV_SAMPLE_SIZE];

time_t timeNextApplication;
int timeToNextReapplication;
time_t now;
time_t timeLastApplication;

// Variables for compass
float lastDirection = 0;
float allowedAngleDifference = 2;

BLEServer* server = nullptr;
BLECharacteristic* characteristic = nullptr;

String message = "";

// Declare later used functions
void sendFacingDirection();
int getUVdata();
void changeBuzzer(int buzzerNr, int status);
void readAppMessagesForTest();
void turnOnBuzzersFromMessage();
void reapplySunscreenFromMessage();
void decreaseReaplication(int uvValue);
void setValuesToStart();

class RayMinderCharacteristicCallbacks : public BLECharacteristicCallbacks {
  // Receive message from phone
  void onWrite(BLECharacteristic* characteristic) override {
    message = characteristic->getValue();
    if (message.length() > 0) {
      Serial.print("Received message: ");
      Serial.println(message.c_str());

      // Turn on buzzer(s) from message
      if (message[0] == '0') {
        turnOnBuzzersFromMessage();

      } else if (message[0] == '1') {
        reapplySunscreenFromMessage();
      } else if (message[0] == '2') {
        setValuesToStart();
      }
    }
  }
};

void setup() {
  // Setup compass connection
  Serial.begin(115200);
  delay(200);
  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.println("Starting HMC5883 Magnetometer...");

  // Initialise the compass sensor
  if (!mag.begin()) {
    /* There was a problem detecting the HMC5883 ... check your connections */
    Serial.println("Ooops, no HMC5883 detected ... Check your wiring!");
    while (1)
      ;
  }


  timeNextApplication = time(NULL);
  timeLastApplication = time(NULL);

  for (int i = 0; i < UV_SAMPLE_SIZE; i++) {
    uvHistory[i] = uvCurrentValue;
  }


  // Initialaze BLE
  BLEDevice::init("ESP32-C3");
  server = BLEDevice::createServer();
  BLEService* service = server->createService("12345678-1234-1234-1234-123456789012");
  characteristic = service->createCharacteristic(
    "12341234-1234-1234-1234-123412341234",
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);

  characteristic->setCallbacks(new RayMinderCharacteristicCallbacks());
  characteristic->setValue("");
  service->start();
  server->getAdvertising()->start();

  Serial.println("BLE started, now connecting to the phone.");

  sendTimesToPhone();

  // Configure pin modes
  pinMode(PIN_BUZZER0, OUTPUT);
  pinMode(PIN_BUZZER1, OUTPUT);
  pinMode(PIN_BUZZER2, OUTPUT);
  pinMode(PIN_BUZZER3, OUTPUT);
  pinMode(PIN_UVSENSOR, INPUT);

  // Turn off buzzers (in case one is still on)
  for (int i = 0; i <= 3; i++) changeBuzzer(i, 0);
}

// the loop function runs over and over again forever
void loop() {
  // Send facing direction to phone
  sendFacingDirection();

  // Get UV sensor data
  uvCurrentValue = getUVdata();
  if (uvCurrentValue > 10) uvCurrentValue = 10;

  updateUVMedium();

  // Receive messages from app that triggers buzzer in direction of friend OR that sunscreen was reapplied
  if (Serial.available()) readAppMessagesForTest(); // Just for testing purposes

  now = time(NULL);
  timeToNextReapplication = (int)difftime(timeNextApplication, now);  // in seconds

  if (uvMediumValue > 0 && timeToNextReapplication < 0) {
    Serial.println("Time to reapply!");

    int buzzerStatusOn[4] = { 1, 1, 1, 1 };
    int buzzerStatusOff[4] = { 0, 0, 0, 0 };
    changeMultipleBuzzers(buzzerStatusOn);
    delay(400);
    changeMultipleBuzzers(buzzerStatusOff);
    delay(400);
    changeMultipleBuzzers(buzzerStatusOn);
    delay(400);
    changeMultipleBuzzers(buzzerStatusOff);
    delay(400);
    changeMultipleBuzzers(buzzerStatusOn);
    delay(400);
    changeMultipleBuzzers(buzzerStatusOff);
  }


  int timeHours = timeToNextReapplication / 3600;
  int timeMins = (timeToNextReapplication % 3600) / 60;
  int timeSeconds = timeToNextReapplication % 60;

  Serial.printf("Next reapplication in %d:%02d:%02d, current UV: %d\n", timeHours, timeMins, timeSeconds, uvMediumValue);
  delay(2000);
}

void changeMultipleBuzzers(int buzzerStatus[4]) {
  for (int i = 0; i < 4; i++) {
    changeBuzzer(i, buzzerStatus[i]);
  }
}

void updateUVMedium() {
  int previousUVMedium = uvMediumValue;
  uvHistory[uvNextIndex] = uvCurrentValue;
  uvNextIndex = (uvNextIndex + 1) % UV_SAMPLE_SIZE;

  int uvSum = 0;
  for (int i = 0; i < UV_SAMPLE_SIZE; i++) {
    uvSum += uvHistory[i];
  }

  uvMediumValue = (int)(uvSum / UV_SAMPLE_SIZE);

  if (uvMediumValue != previousUVMedium) {
    recalculateTimeAfterUVChange((float)previousUVMedium);

    // Send message to phone with 3 digits
    // 0 -> Sending uv value
    //  XX -> UV value in 2 digits
    String message = "0";
    if (uvMediumValue < 10) message += '0';
    message += uvMediumValue;
    characteristic->setValue(message.c_str());
    characteristic->notify();
  }
}

void recalculateTimeAfterUVChange(float previousUVMedium) {
  if (uvMediumValue == 0) {
    timeNextApplication = now;
    return;
  }

  if (previousUVMedium < 1) previousUVMedium = 0.1;

  float ratio = previousUVMedium / (float)uvMediumValue;
  timeToNextReapplication = (int)((float)timeToNextReapplication * ratio);
  Serial.printf("UV change detected. Recalculating time.");
  timeNextApplication = now + (time_t)timeToNextReapplication;

  sendTimesToPhone();
}

// Send times to phone
void sendTimesToPhone() {
  // Send last application time
  String message = "1";
  message += (int)timeLastApplication;
  characteristic->setValue(message.c_str());
  characteristic->notify();

  // Send next application time
  message = "2";
  message += (int)timeToNextReapplication;
  characteristic->setValue(message.c_str());
  characteristic->notify();
}


// Time (in mins) to burn without sunscreen for uv index 10:
// 1 -> 10; 2 -> 10-20 (~=15); 3 -> 20; 4 -> 30; 5 -> 60; 6 -> 90
float timesToBurnWithoutSunscreenForUV10[6] = { 10, 15, 20, 30, 60, 90 };

// Calculate time for different uvIndexes and skintypes (Time is proportional to uv index)
float timeToNextReapplicationForUVAndSkintype() {
  float uvValue = uvMediumValue > 0 ? (float)uvMediumValue : 0.1;  // Use small value as "infinite" seconds

  return timesToBurnWithoutSunscreenForUV10[skintype - 1] * 60 * (10 / uvValue);
}

// Calculate the direction the cap is facing and send it to the phone
void sendFacingDirection() {
  /* Get a new sensor event */
  sensors_event_t event;
  mag.getEvent(&event);

  // Calculate heading when the magnetometer is level
  float heading = atan2(event.magnetic.y, event.magnetic.x);

  // 'Declination Angle / 'Error' of the magnetic field in our location (Sunshine Coast)
  float declinationAngle = 0.1894;
  heading -= declinationAngle;

  // Correct for when signs are reversed.
  if (heading < 0)
    heading += 2 * PI;

  // Check for wrap due to addition of declination.
  if (heading > 2 * PI)
    heading -= 2 * PI;

  // Convert radians to degrees for readability.
  float headingDegrees = heading * 180 / M_PI;

  // North is at 180, so we shift all degrees
  // TODO: If we install the compass in a different angle, this angle has to be shifted
  headingDegrees -= 180;
  if (headingDegrees < 0) headingDegrees += 360;

  float bestAngle = 45;
  // Compass is parallel to ground at ~z=45 (Only use these values, as the degrees differ to much when it is tilted)
  if (event.magnetic.z > bestAngle - allowedAngleDifference && event.magnetic.z < bestAngle + allowedAngleDifference) {
    lastDirection = headingDegrees;
    Serial.print("Sending direction to phone: ");
    Serial.print(headingDegrees);
    Serial.println("°");

    // Send message with 4 digits to phone
    // 1 -> For sending facing direction
    //  XXX -> degree cap is facing
    String message = "3";
    if (lastDirection < 100) message += '0';
    if (lastDirection < 10) message += '0';
    message += ((int)lastDirection);

    characteristic->setValue(message.c_str());
    characteristic->notify();
  }
}

int getUVdata() {
  uvCurrentValue = analogRead(PIN_UVSENSOR);
  return uvCurrentValue;
}

// Change buzzer status (off or on) by number
// buzzerNr: 0 -> back left, 1 -> front left, 2 -> front right, 3 -> back right
// status: 0 -> off, 1 -> on
void changeBuzzer(int buzzerNr, int status) {
  switch (buzzerNr) {
    case 0:
      digitalWrite(PIN_BUZZER0, status == 0 ? LOW : HIGH);
      break;
    case 1:
      digitalWrite(PIN_BUZZER1, status == 0 ? LOW : HIGH);
      break;
    case 2:
      digitalWrite(PIN_BUZZER2, status == 0 ? LOW : HIGH);
      break;
    case 3:
      digitalWrite(PIN_BUZZER3, status == 0 ? LOW : HIGH);
      break;
  }
}

/* Messages:
  0 -> Turn on buzzer
    0-3 -> Turn on buzzer 0-3
    4 -> Turn on buzzer 0+1
    5 -> Turn on buzzer 1+2
    6 -> Turn on buzzer 2+3
    7 -> Turn on buzzer 3+0

  1 -> Reapplied sunscreen
    2 digit SPF (e.g. 10 -> SPF 10; 25 -> SPF 25)
      1 Digit skin type of 1-6
*/
char c;

// For Testing purposes: Read & decode messages send by Serial input
void readAppMessagesForTest() {
  message = "";

  while (Serial.available()) {
    c = Serial.read();

    if (c == '\n' || c == '\0') {
      break;
    }

    message += c;
  }

  Serial.print("Received message: ");
  Serial.println(message);

  // Turn on buzzer(s) from message
  if (message[0] == '0') {
    turnOnBuzzersFromMessage();

  } else if (message[0] == '1') {
    reapplySunscreenFromMessage();
  } else if (message[0] == '3') {
    // Set small timer to show
  }
}

// Reapply Sunscreen from the message
void reapplySunscreenFromMessage() {
  spf = (((int)message[1]) - 48) * 10;
  spf += ((int)message[2]) - 48;
  skintype = ((int)message[3]) - 48;

  float spfFactor = spf > 10 ? spf * 0.1 : 1;
  float factorForBadApplication = 0.6;

  Serial.printf("Reapplied sunscreen with SPF %d, for skintype %d\n", (int)spfFactor, skintype);
  timeToNextReapplication = (time_t)((int)(timeToNextReapplicationForUVAndSkintype() * spfFactor * factorForBadApplication));
  Serial.printf("Added %dsec.\n", timeToNextReapplication);
  timeNextApplication = now + timeToNextReapplication;
  timeLastApplication = now;

  sendTimesToPhone();
}

// After message, that buzzers should go off was received: Turn on these buzzers 3x
void turnOnBuzzersFromMessage() {
  int buzzers = ((int)message[1]) - 48;

  Serial.print("Turning on buzzer ");
  Serial.print(buzzers % 4);

  changeBuzzer(buzzers % 4, 1);
  if (buzzers > 3) {
    Serial.print(" & ");
    Serial.print(buzzers + 1 % 4);

    changeBuzzer(buzzers + 1 % 4, 1);
  }
  Serial.println("");

  delay(400);

  changeBuzzer(buzzers % 4, 0);
  if (buzzers > 3) {
    changeBuzzer(buzzers + 1 % 4, 0);
  }

  delay(400);

  changeBuzzer(buzzers % 4, 1);
  if (buzzers > 3) {
    changeBuzzer(buzzers + 1 % 4, 1);
  }

  delay(400);

  changeBuzzer(buzzers % 4, 0);
  if (buzzers > 3) {
    changeBuzzer(buzzers + 1 % 4, 0);
  }

  delay(400);

  changeBuzzer(buzzers % 4, 1);
  if (buzzers > 3) {
    changeBuzzer(buzzers + 1 % 4, 1);
  }

  delay(400);

  changeBuzzer(buzzers % 4, 0);
  if (buzzers > 3) {
    changeBuzzer(buzzers + 1 % 4, 0);
  }
}

void setValuesToStart() {
  // Set start time
  message = "1011";
  reapplySunscreenFromMessage();
}
