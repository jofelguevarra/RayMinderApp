// Test the UV sensor

/*
Cables:
  - -> GND
  OUT -> PIN_UVSENSOR
  + -> 3.3V
*/


// Define pin of the uv sensor
#define PIN_UVSENSOR 1

int uvValue = 0;

void setup() {
  // Setup compass connection
  Serial.begin(115200);
  delay(200);
  Serial.println("Starting...");

  // Configure pin modes
  pinMode(PIN_UVSENSOR, INPUT);
}

// the loop function runs over and over again forever
void loop() {
  uvValue = getUVdata();
  delay(200);
}

int minData = -1;
int maxData = 0;

int minUVIndex = -1;
int maxUVIndex = 0;


int getUVdata() {
  int data = analogRead(PIN_UVSENSOR);
  Serial.print("\t\tUV data: ");
  Serial.println(data);

  // 3.3V voltage, ADC from 0 to 4095
  float voltage = data * 3.3 / 4095.0;
  int uvIndex = (int)(voltage * 10);

  Serial.print("UV index: ");
  Serial.println(uvIndex);

  printMinMax(data, uvIndex);

  // TODO: Possibly filter received data

  return data;
}

void printMinMax(int data, int uvIndex) {
  if (minData == -1) {
    minData = data;
  }

  if (minData > data) {
    minData = data;
  }

  if (maxData < data) {
    maxData = data;
  }

  if (minUVIndex == -1) {
    minUVIndex = uvIndex;
  }

  if (minUVIndex > uvIndex) {
    minUVIndex = uvIndex;
  }

  if (maxUVIndex < uvIndex) {
    maxUVIndex = uvIndex;
  }

  Serial.print("\t\t\t\tData: ");
  Serial.print(minData);
  Serial.print("-");
  Serial.print(maxData);
  Serial.println();

  Serial.print("\t\t\t\tUV Index: ");
  Serial.print(minUVIndex);
  Serial.print("-");
  Serial.print(maxUVIndex);
  Serial.println();
}
