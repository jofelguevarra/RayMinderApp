// Little program to test serial input (with buzzer going off when 1 is send, and turned off on 0)
// ! This only works if no \n is send! Otherwise use char instead of int

#define PIN_BUZZER0 5  // PIN 9 / GP5

void setup() {
  Serial.begin(115200);
  delay(500);
  pinMode(PIN_BUZZER0, OUTPUT);
}


int message;
void loop() {
  if (Serial.available()) {
    message = Serial.parseInt();
    Serial.print("Received message: ");
    Serial.println(message);

    if (message == 1) {
      // Turn on buzzer
      digitalWrite(PIN_BUZZER0, HIGH);
    } else if (message == 0) {
      // Turn of buzzer
      digitalWrite(PIN_BUZZER0, LOW);
    }
  }

  delay(200);
}
