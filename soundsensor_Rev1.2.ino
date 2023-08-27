#define SOUND_SENSOR 35
const int sampleWindow = 50;
int soundThreshold = 100; //adjustable
unsigned int sample;

const unsigned long soundTimer = 500;  // 5 seconds in milliseconds

unsigned long startTime = 0;
bool aboveThreshold = false;

void setup() {
  // put your setup code here, to run once:
  pinMode(SOUND_SENSOR, INPUT); // Set the signal pin as input
  Serial.begin(115200);
}

void loop() {
  // SOUNDSENSOR CODE
  unsigned long startMillis = millis();                  // Start of sample window
  float peakToPeak = 0;                                  // peak-to-peak level

  //collect data for 50 mS
  while (millis() - startMillis < sampleWindow) {
    sample = analogRead(SOUND_SENSOR);
    Serial.println(sample);
    if (sample < soundThreshold){
            aboveThreshold = false;  // Reset if the value drops below the threshold
        }
    if (sample > soundThreshold) {
            if (!aboveThreshold) {
                aboveThreshold = true;
                startTime = millis();  // Record the time when the value goes above 100
            }
            
            if (aboveThreshold && (millis() - startTime >= soundTimer)) {
                // Value has stayed above 100 for the required duration
                Serial.println("Input stayed above 100 for 10 seconds.");
                // Reset the state for future checks
                delay(5000); //initiate breathing sequence
            }
        }
    
  delay(200);
  }
}
