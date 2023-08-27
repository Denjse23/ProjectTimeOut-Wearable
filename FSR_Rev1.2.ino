#define FORCE_SENSOR_PIN 36

void setup() {
  Serial.begin(115200);
}

void loop() {
  int analogReading = analogRead(FORCE_SENSOR_PIN);

  Serial.print("The force sensor value = ");
  Serial.println(analogReading); // print the raw analog reading
  if (analogReading < 600) { // low value, calm state 
    Serial.println("CALM STATE");
  }
  else if (analogReading > 600) {
    Serial.println("SENSORY OVERLOAD");
    delay(5000); //undergo breathing sequence, check again after 5s
  }

  
  delay(200);
}