/*
  RayMinder

  This is going to be the script for the RayMinders. Right now, it's still under construction.
*/

// Define pins of the 4 buzzers
#define PIN_BUZZER0 6
#define PIN_BUZZER1 7
#define PIN_BUZZER2 8
#define PIN_BUZZER3 9

// Define pin of the uv sensor
#define PIN_UVSENSOR 7

// Define global variable for time of next reapplication (in seconds)
int nextReapplication = 100;

// Declare later used functions
void changeBuzzer(int buzzerNr, int status);
void readAppMessages();
void decreaseReaplication(int uvValue);

void setup() {
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

  // Get UV sensor data
  int uvValue = analogRead(PIN_UVSENSOR);
  Serial.println("UV value: " + uvValue); // TODO: Does this print work?

  // TODO: Possibly filter received uv values

  // Calculate if sunscreen needs to be reapplied
  // TODO: Add calculation function
  decreaseReaplication(uvValue);


  if (nextReapplication <= 0) { 
    // Turn on all buzzers for 400ms
    for (int i = 0; i <= 3; i++) changeBuzzer(i, 1);
    delay(400);
    for (int i = 0; i <= 3; i++) changeBuzzer(i, 0);
  }

  // Receive messages from app that triggers buzzer in direction of friend OR that sunscreen was reapplied
  readAppMessages();
}

// Change buzzer status (off or on) by number
// buzzerNr: 0 -> back left, 1 -> front left, 2 -> front right, 3 -> back right
// status: 0 -> off, 1 -> on
void changeBuzzer(int buzzerNr, int status) {
  switch (buzzerNr) {
    case 0:
      // TODO: Must it be analog wirte?
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

// Read Messages of app (0-3 -> turn on buzzer, 4 -> add time to NextReapplication)
void readAppMessages() {
  // Read messages; if there is a message, turn on the accoring buzzer
  if (Serial.available() > 0) {
    int message = Serial.parseInt();
    Serial.println("Received message: " + message);

    // Turn on the buzzer from the message
    if (message < 4) {
      changeBuzzer(message, 1);
    } else {
      // TODO: Add time to reapplication; Possibly change message type to add SPF and skin type
    }
    
    // Let it on for a short time
    delay(100);

    // Turn it off again
    changeBuzzer(messsage, 0);
  }
}

// Decrease Reapplication accoring to the uvValue
void decreaseReaplication(int uvValue) {
  int decrease = uvValue; // TODO: Change to a value that fits to the time
  nextApplication -= decrease;
}

// Increase time for reapplication (should be called once the reapplied button was pressed)
void increaseReapplication(int value) {
  // TODO: Potentially add the calculation in here (and more parameters for that)
  nextReapplication += value;
}


// TODO: Add Compass to this and send messages to phone with the direction the cap is facing

// TODO: Create a function that calculates the next time sunscreen needs to be reapplied and let all buzzers go off if that is the case.
