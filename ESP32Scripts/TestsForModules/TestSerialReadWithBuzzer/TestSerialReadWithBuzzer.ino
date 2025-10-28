// Little program to test serial input (with buzzer going off when 1 is send, and turned off on 0)
// ! This only works if no \n is send! Otherwise use char instead of int

#define PIN_BUZZER0 9

void setup() {
  Serial.begin(115200);
  delay(500);
  pinMode(PIN_BUZZER0, OUTPUT);
}


char c;
String message;
int messageAsInt[2];

void loop() {
  if (Serial.available()) {
    readMessage();

    if (messageAsInt[0] == 0 && messageAsInt[1] == 0) {
      digitalWrite(PIN_BUZZER0, HIGH);
      delay(200);
      digitalWrite(PIN_BUZZER0, LOW);
    }
  }

  delay(200);
}

void readMessage() {
  message = "";
  while (Serial.available()) {
    c = Serial.read();

    if (c == '\n' || c == '\0') {
      break;
    }

    Serial.print("Received message: ");
    Serial.println(c);
    message += c;
  }

  messageAsInt[0] = ((int)message[0]) - 48;
  messageAsInt[1] = ((int)message[1]) - 48;

  Serial.print("Whole message: ");
  Serial.print(messageAsInt[0]);
  Serial.println(messageAsInt[1]);
}
