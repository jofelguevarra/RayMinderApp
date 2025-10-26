// Part of Task 3 RayMinder

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>

Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);

#define SDA_PIN 4
#define SCL_PIN 5

float lastDirection = 0;
float allowedAngleDifference = 2;

void setup(void) 
{
  Serial.begin(9600);
  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.println("HMC5883 Magnetometer will be started...");
  Serial.println("");
  
  /* Initialise the sensor */
  if(!mag.begin())
  {
    /* There was a problem detecting the HMC5883 ... check your connections */
    Serial.println("Ooops, no HMC5883 detected ... Check your wiring!");
    while(1);
  }
}

void loop(void) 
{
  /* Get a new sensor event */ 
  sensors_event_t event; 
  mag.getEvent(&event);
 
  // Calculate heading when the magnetometer is level
  float heading = atan2(event.magnetic.y, event.magnetic.x);
  
  // 'Declination Angle / 'Error' of the magnetic field in our location (Sunshine Coast)
  float declinationAngle = 0.1894;
  heading -= declinationAngle;

  // Correct for when signs are reversed.
  if(heading < 0)
    heading += 2*PI;
    
  // Check for wrap due to addition of declination.
  if(heading > 2*PI)
    heading -= 2*PI;
   
  // Convert radians to degrees for readability.
  float headingDegrees = heading * 180/M_PI; 

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
  }
  
  delay(500);
}