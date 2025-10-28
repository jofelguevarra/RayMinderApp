// Little test program for calculation of reapplication when different spfs where applied and if the uv index changed

#include <stdio.h>
#include <time.h>

int uvValue = 0;
int skintype = 2;
int spf = 0;

time_t timeLastApplication;
time_t timeNextApplication;
float timeAllowedTotal = 0; // in seconds
float timeElapsedBeforeChange = 0; 


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

  // Set example values
  uvValue = 5; 
  skintype = 2;
  spf = 0;


  timeNextApplication = time(NULL);
  delay(200);
  timeLastApplication = time(NULL);
  // updateAllowedTime();
}

// the loop function runs over and over again forever
void loop() {
  time_t now = time(NULL);

  if (now >= timeNextApplication && uvValue > 0) {
    Serial.println("Time to reapply sunscreen reached");
  }

  if (Serial.available()) readMessages();
  delay(2000);
  Serial.print("Next application in ");
  Serial.println(difftime(timeNextApplication, now));
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


void setTimeFromMessage() {
  int time = (((int)message[1]) - 48) * 100;
  time += (((int)message[2]) - 48) * 10;
  time += (((int)message[3]) - 48);

  // timeNextApplication = time(NULL);

  // nextReapplication = time;
  Serial.print("Set next reaplication time to ");
  Serial.println(timeNextApplication);
}

// Reapply Sunscreen from the message
void reapplySunscreenFromMessage() {
  int spf = (((int)message[1]) - 48) * 10;
  spf += ((int)message[2]) - 48;
  skintype = ((int)message[3]) - 48;


  Serial.print("Reapply sunscreen with SPF");
  Serial.print(spf);
  Serial.print(", for skintype ");
  Serial.println(skintype);

  float timeToAdd = timeToNextReapplicationForUVAndSkintype();
  Serial.print("Time to be added: ");
  Serial.println(timeToAdd);

  // TODO: Increase the next time for reapplying
  updateAllowedTime();
}

void setUVvalueFromMessage() {
  int newUV = ((int)message[1]) - 48;

  time_t now = time(NULL);

  // UV unchanged
  if (newUV == uvValue) return;
  uvValue = newUV;
  if (uvValue == 0) return;

  if (timeAllowedTotal == 0) {
    updateAllowedTime(); // berechne initialen Wert
  }

  // Calculate what fraction of the time since last application has elapsed
  float elapsed = difftime(now, timeLastApplication);
  float fractionElapsed = elapsed / timeAllowedTotal;
  if (fractionElapsed > 1) fractionElapsed = 1;

  float oldAllowed = timeAllowedTotal;

  // Recalculate with new uv
  updateAllowedTime(); // recalculates with new UV

  // Recalculate with fraction
  timeNextApplication = now + (1 - fractionElapsed) * timeAllowedTotal;

  Serial.print("UV changed to ");
  Serial.print(uvValue);
  Serial.print(", new next Application time: ");
  Serial.println(timeNextApplication);
}


// Time (in mins) to burn without sunscreen for uv index 10:
// 1 -> 10; 2 -> 10-20 (~=15); 3 -> 20; 4 -> 30; 5 -> 60; 6 -> 90
int timesToBurnWithoutSunscreenForUV10[6] = {10, 15, 20, 30, 60, 90};

// Calculate time for different uvIndexes and skintypes (Time is proportional to uv index)
float timeToNextReapplicationForUVAndSkintype() {
  if (uvValue == 0) return 0;

  return timesToBurnWithoutSunscreenForUV10[skintype - 1] * 10 / (float)uvValue;
}


// Recalculate time based on uv, skintype and spf
void updateAllowedTime() {
  // Get time that skintype would be allowed in sun without SPF
  float baseTime = timeToNextReapplicationForUVAndSkintype() * 60;
  float spfFactor = (spf > 0) ? spf : 1;

  // Extra Time allowed in sun with SPF
  timeAllowedTotal = baseTime * spfFactor;

  // Recalculate time of next application
  timeNextApplication = timeLastApplication + timeAllowedTotal;
}
