/*
  RayMinder

  This is going to be the script for the RayMinders. Right now, it's still under construction.
*/

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>

Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);

// Define pins for I2C communication with compass
#define SDA_PIN 4
#define SCL_PIN 5

// Define pins of the 4 buzzers
#define PIN_BUZZER0 6
#define PIN_BUZZER1 7
#define PIN_BUZZER2 8
#define PIN_BUZZER3 9

// Define pin of the uv sensor
#define PIN_UVSENSOR 10

#define UV_SAMPLE_SIZE 60

int uvMediumValue = 0;
int uvCurrentValue;
int uvNextIndex = 1;
int skintype;
int spf;

int uvHistory[UV_SAMPLE_SIZE];

time_t timeNextApplication;
int timeToNextReapplication;
time_t now;

// Variables for compass
float lastDirection = 0;
float allowedAngleDifference = 2;

// Declare later used functions
void sendFacingDirection();
int getUVdata();
void changeBuzzer(int buzzerNr, int status);
void readAppMessages();
void turnOnBuzzersFromMessage();
void reapplySunscreenFromMessage();
void decreaseReaplication(int uvValue);

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

  for (int i = 0; i < UV_SAMPLE_SIZE; i++) {
    uvHistory[i] = uvCurrentValue;
  }

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
  if (Serial.available()) readAppMessages();

  now = time(NULL);
  timeToNextReapplication = (int)difftime(timeNextApplication, now); // in seconds

  if (uvMediumValue > 0 && timeToNextReapplication < 0) {
    Serial.println("Time to reapply!");

    int buzzerStatusOn[4] = {1,1,1,1};
    changeMultipleBuzzers(buzzerStatusOn);
    delay(200);
    int buzzerStatusOff[4] = {1,1,1,1};
    int* buzzerStatus = buzzerStatusOff;
    changeMultipleBuzzers(buzzerStatus);
  }


  int timeHours = timeToNextReapplication / 3600;
  int timeMins = (timeToNextReapplication % 3600) / 60;
  int timeSeconds = timeToNextReapplication % 60;
  
  Serial.printf("Next reapplication in %d:%02d:%02d, current UV: %d\n", timeHours,timeMins, timeSeconds, uvMediumValue);
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

  if (uvMediumValue != previousUVMedium) 
    recalculateTimeAfterUVChange((float)previousUVMedium);
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
}


// Time (in mins) to burn without sunscreen for uv index 10:
// 1 -> 10; 2 -> 10-20 (~=15); 3 -> 20; 4 -> 30; 5 -> 60; 6 -> 90
float timesToBurnWithoutSunscreenForUV10[6] = {10, 15, 20, 30, 60, 90};

// Calculate time for different uvIndexes and skintypes (Time is proportional to uv index)
float timeToNextReapplicationForUVAndSkintype() {
  float uvValue = uvMediumValue > 0 ? (float)uvMediumValue : 0.1; // Use small value as "infinite" seconds

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

  // Compass is parallel to ground at ~z=45 (Only use these values, as the degrees differ to much when it is tilted)
  if (event.magnetic.z > 45 - allowedAngleDifference && event.magnetic.z < 45 + allowedAngleDifference) {
    lastDirection = headingDegrees;
    Serial.print("Direction: ");
    Serial.print(headingDegrees);
    Serial.println("°");

    // TODO: Send a message to the phone here with the direction the cap is currently facing
    // Serial.write();
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
String message;
char c;

// Read & decode messages send by phone
void readAppMessages() {
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
  }
}

// Reapply Sunscreen from the message
void reapplySunscreenFromMessage() {
  spf = (((int)message[1]) - 48) * 10;
  spf += ((int)message[2]) - 48;
  skintype = ((int)message[3]) - 48;

  float spfFactor = spf > 0 ? spf : 0.1;
  float factorForBadApplication = 0.6;

  Serial.printf("Reapplied sunscreen with SPF %d, for skintype %d\n", (int)spfFactor, skintype);
  int newTimeToNext = (int)(timeToNextReapplicationForUVAndSkintype() * spfFactor * factorForBadApplication);
  Serial.printf("Added %dsec.\n", newTimeToNext);
  timeNextApplication = now + newTimeToNext;
}

// After message, that buzzers should go off was received: Turn on these buzzers
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
}
