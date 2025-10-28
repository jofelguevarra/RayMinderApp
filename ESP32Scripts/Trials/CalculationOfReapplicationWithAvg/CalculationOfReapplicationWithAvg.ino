#include <stdio.h>
#include <time.h>

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

float timeToNextReapplicationForUVAndSkintype();


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(200);

  skintype = 1;
  uvCurrentValue = 10;


  timeNextApplication = time(NULL);

  for (int i = 0; i < UV_SAMPLE_SIZE; i++) {
    uvHistory[i] = uvCurrentValue;
  }
}

void loop() {
  now = time(NULL);

  if (Serial.available()) readMessages();

  timeToNextReapplication = (int)difftime(timeNextApplication, now); // in seconds

  if (uvMediumValue > 0 && timeToNextReapplication < 0) {
    Serial.println("Time to reapply!");
  }

  updateUVMedium();

  int timeHours = timeToNextReapplication / 3600;
  int timeMins = (timeToNextReapplication % 3600) / 60;
  int timeSeconds = timeToNextReapplication % 60;
  // Serial.printf("Next reapplication in %dmin, current UV: %d\n", timeToNextReapplication / 60, uvMediumValue);
  Serial.printf("Next reapplication in %d:%02d:%02d, current UV: %d\n", timeHours,timeMins, timeSeconds, uvMediumValue);
  delay(2000);
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


String message;
char c;

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

  if (message[0] == '0') {
    setUVvalueFromMessage(); // Testing purposes
  } else if (message[0] == '1') {
    reapplySunscreenFromMessage();
  }
}


void setUVvalueFromMessage() {
  uvCurrentValue = ((int)message[1]) - 48;
  if (uvCurrentValue > 10) uvCurrentValue = 10;
  Serial.print("UV changed to ");
  Serial.println(uvCurrentValue);
}

void recalculateTimeAfterUVChange(float previousUVMedium) {
  if (uvMediumValue == 0) {
    timeNextApplication = now;
    return;
  }

  if (previousUVMedium < 1) previousUVMedium = 0.1;

  float ratio = previousUVMedium / (float)uvMediumValue;
  Serial.printf("timeToNextReapplication=%d, previousUVMedium=%f, uvMediumValue=%d, ratio=%f\n", timeToNextReapplication,previousUVMedium, uvMediumValue,ratio);
  timeToNextReapplication = (int)((float)timeToNextReapplication * ratio);
  Serial.printf("New next time: %d\n", timeToNextReapplication);
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

// Reapply Sunscreen from the message
void reapplySunscreenFromMessage() {
  spf = (((int)message[1]) - 48) * 10;
  spf += ((int)message[2]) - 48;
  skintype = ((int)message[3]) - 48;

  float spfFactor = spf > 0 ? spf : 0.1;
  float factorForBadApplication = 0.6;

  Serial.printf("Reapply sunscreen with SPF %f, for skintype %d\n", spfFactor, skintype);
  int newTimeToNext = (int)(timeToNextReapplicationForUVAndSkintype() * spfFactor * factorForBadApplication);
  Serial.printf("Added %dsec\n", newTimeToNext);
  timeNextApplication = now + newTimeToNext;
}
