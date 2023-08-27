//Defining Pins and variables 
////SOUND SENSOR
#define SOUND_SENSOR 35
const int sampleWindow = 50;
int soundThreshold = 300; //adjustable
unsigned int sample;

const unsigned long soundTimer = 1000;  // 5 seconds in milliseconds

unsigned long startTime = 0;
bool aboveSoundThreshold = false;
bool sustainedNoisy = false;

////FORCE SENSOR
#define FORCE_SENSOR_PIN 36
bool aboveHeartRate = false; 
int heartThreshold = 1500; //adjustable

////Breathing Time
int breathTime = 5000; //time before wearable checks in again
bool TimeOut = false; 

///Data Transmission
#include <esp_now.h>
#include <WiFi.h>

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0xEC,0xDA,0x3B,0x96,0xEE,0x24}; // {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  bool TOStatus;
} struct_message;

// Create a struct_message called myData
struct_message myData;

esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(SOUND_SENSOR, INPUT);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  sound_tester();
  HeartRateTester();
  if (sustainedNoisy == false && aboveHeartRate == false) {
    // normal state, no intervention needed
    TimeOut = false;
    Serial.print("TOStatus:");
    Serial.println(TimeOut);
    transmit_data();
  }
  else if (sustainedNoisy == true || aboveHeartRate == true) {
    //initiate breathing sequence
    TimeOut = true;
    Serial.print("TOStatus:");
    Serial.println(TimeOut);
    transmit_data();
    delay(breathTime);
  }
}


void sound_tester(){
// SOUNDSENSOR CODE
  unsigned long startMillis = millis();                  // Start of sample window
  float peakToPeak = 0;                                  // peak-to-peak level

  //collect data for 50 mS
  while (millis() - startMillis < sampleWindow) {
    sample = analogRead(SOUND_SENSOR);
    Serial.print("sound level is:");
    Serial.println(sample);
    if (sample < soundThreshold){
            aboveSoundThreshold = false;  // Reset if the value drops below the threshold
            sustainedNoisy = false;
        }
    if (sample > soundThreshold) {
            if (!aboveSoundThreshold) {
                aboveSoundThreshold = true;
                startTime = millis();  // Record the time when the value goes above 100
            }
            
            if (aboveSoundThreshold && (millis() - startTime >= soundTimer)) {
                // Value has stayed above 100 for the required duration
                Serial.println("Noisy for more than desired.");
                sustainedNoisy = true;
                // delay(5000); //initiate breathing sequence
            }
        }
  delay(200);
  }
}

void HeartRateTester(){
  int analogReading = analogRead(FORCE_SENSOR_PIN);

  Serial.print("The heart sensor value = ");
  Serial.println(analogReading); // print the raw analog reading
  if (analogReading < heartThreshold) { // low value, calm state 
    Serial.println("CALM STATE");
    aboveHeartRate = false;
  }
  else if (analogReading > heartThreshold) {
    Serial.println("SENSORY OVERLOAD");
    aboveHeartRate = true;
    
  }
  delay(300);
}

void transmit_data(){
  // Set values to send
  myData.TOStatus = TimeOut;
  
  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  delay(2000);
}

