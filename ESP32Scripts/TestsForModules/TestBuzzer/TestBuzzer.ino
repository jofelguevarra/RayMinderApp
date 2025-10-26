// Little program to test buzzer

#define PIN_BUZZER0 5 // PIN 9 / GP5

void setup() {
  pinMode(PIN_BUZZER0, OUTPUT);
}

void loop() {
  // Turn on buzzer
  digitalWrite(PIN_BUZZER0, HIGH);
  delay(1000);

  // Turn of buzzer (after 1sek)
  digitalWrite(PIN_BUZZER0, LOW);
  delay(1000);
}
