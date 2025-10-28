// Little test program for calculation of reapplication when different spfs where applied and if the uv index changed

#include <stdio.h>
#include <time.h>

// Define global variable for time of next reapplication (in seconds)
int nextReapplication = 100;
int uvValue = 0;

float percentageTimeLeft = 0;

time_t timeLastApplication;
time_t timeNextApplication;


// Declare later used functions
void sendFacingDirection();
int getUVdata();
void changeBuzzer(int buzzerNr, int status);
void readAppMessages();
void setTimeFromMessage();
void reapplySunscreenFromMessage();
void decreaseReaplication(int uvValue);

void readMessages();

void setup() {
  // Setup compass connection
  Serial.begin(115200);
  delay(200);

  // Set time of last  & next application to now;
  timeLastApplication = time(NULL);
  timeNextApplication = time(NULL);
}

// the loop function runs over and over again forever
void loop() {
  if (Serial.available()) readMessages();

  time_t seconds = time(NULL);
  Serial.printf("%ld seconds", seconds);
  delay(1500);
}


/* Messages:
  0 -> set time
    XXX -> time
  1 -> Reapplied sunscreen
    XX -> 2 digit SPF (e.g. 10 -> SPF 10; 25 -> SPF 25)
      X -> 1 Digit skin type of 1-6
  2 -> set uv index
    X -> uv index
*/
String message;
char c;

// Read & decode messages send by phone
void readMessages() {
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
    setTimeFromMessage(); // Simply for testing purposes

  } else if (message[0] == '1') {
    reapplySunscreenFromMessage();
  } else if (message[0] == '2') {
    setUVvalueFromMessage(); // Simply for testing purposes
  }
}

// After message, that buzzers should go off was received: Turn on these buzzers
void setTimeFromMessage() {
  int time = (((int)message[1]) - 48) * 100;
  time += (((int)message[2]) - 48) * 10;
  time += (((int)message[3]) - 48);

  nextReapplication = time;
  Serial.print("Set next reaplication time to ");
  Serial.println(nextReapplication);
}

// Reapply Sunscreen from the message
void reapplySunscreenFromMessage() {
  int spf = (((int)message[1]) - 48) * 10;
  spf += ((int)message[2]) - 48;
  int skintype = ((int)message[3]) - 48;


  Serial.print("Reapply sunscreen with SPF");
  Serial.print(spf);
  Serial.print(", for skintype ");
  Serial.println(skintype);

  float timeToAdd = timeToNextReapplicationForUVAndSkintype(uvValue, skintype);
  Serial.print("Time to be added: ");
  Serial.println(timeToAdd);

  // TODO: Increase the next time for reapplying
}

void setUVvalueFromMessage() {
  uvValue = ((int)message[1]) - 48;
  Serial.print("Set uv index to ");
  Serial.println(uvValue);
}

// Decrease Reapplication accoring to the uvValue
void decreaseReaplication(int uvValue) {
  int decrease = uvValue;  // TODO: Change to a value that fits to the time
  nextReapplication -= decrease;
}

// Time to burn without sunscreen for uv index 10:
// 1 -> 10; 2 -> 10-20 (~=15); 3 -> 20; 4 -> 30; 5 -> 60; 6 -> 90
int timesToBurnWithoutSunscreenForUV10[6] = {10, 15, 20, 30, 60, 90};

// Calculate time for different uvIndexes and skintypes (Time is proportional to uv index)
float timeToNextReapplicationForUVAndSkintype(int uvIndex, int skintype) {
  if (uvIndex == 0) return 0;

  return timesToBurnWithoutSunscreenForUV10[skintype - 1] * 10 / uvIndex;
}

// Increase time for reapplication (should be called once the reapplied button was pressed)
void increaseReapplication(int value) {
  // TODO: Potentially add the calculation in here (and more parameters for that)
  nextReapplication += value;




  // timeToBurnWithSunscreen = SPF x timeToBurnWithoutSunscreen
}

// TODO: Create a function that calculates the next time sunscreen needs to be reapplied and let all buzzers go off if that is the case.
