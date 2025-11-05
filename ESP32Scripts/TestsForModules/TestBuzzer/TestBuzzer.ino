// Little program to test buzzer

/*
Cables:
  Blue -> GND
  Red -> PIN_BUZZER0
*/


#define PIN_BUZZER0 10 // PIN 9 / GP5
#define PIN_BUZZER1 3 // PIN 9 / GP5
#define PIN_BUZZER2 4 // PIN 9 / GP5
#define PIN_BUZZER3 5 // PIN 9 / GP5


void setup() {
  pinMode(PIN_BUZZER0, OUTPUT);
  pinMode(PIN_BUZZER1, OUTPUT);
  pinMode(PIN_BUZZER2, OUTPUT);
  pinMode(PIN_BUZZER3, OUTPUT);
  delay(200);

  digitalWrite(PIN_BUZZER0, HIGH);
  // digitalWrite(PIN_BUZZER1, HIGH);
  // digitalWrite(PIN_BUZZER2, HIGH);
  digitalWrite(PIN_BUZZER3, HIGH);
}

void loop() {
  // Turn on buzzer
  // digitalWrite(PIN_BUZZER0, HIGH);
  // delay(1000);

  // // Turn of buzzer (after 1sek)
  // digitalWrite(PIN_BUZZER0, LOW);
  // delay(1000);
}
