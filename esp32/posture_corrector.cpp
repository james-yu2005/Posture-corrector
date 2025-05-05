#include <Wire.h>
#include <math.h>
#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "wifi";       // Replace with your WiFi SSID
const char* password = "pswd";   // Replace with your WiFi password

WebServer server(80);

// MPU6050 I2C address
#define MPU6050_ADDR 0x68

// Registers
#define PWR_MGMT_1 0x6B
#define ACCEL_XOUT_H 0x3B

#define BUZZER_PIN 18 

// Variables to store sensor data
int16_t ax, ay, az;

float roll = 0.0;
float pitch = 0.0;
float initial_roll = 0.0;
float initial_pitch = 0.0;

// Threshold for change in angle
float angle_threshold = 20.0;

// Flag for manual buzzer control
bool manual_buzzer_override = false;
bool manual_buzzer_state = false;

void setup() {
  Serial.begin(115200);
  
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  // Initialize web server routes
  server.on("/buzzer-on", handleBuzzerOn);
  server.on("/buzzer-off", handleBuzzerOff);
  
  server.begin();
  Serial.println("HTTP server started");
  
  Wire.begin();
  
  // Wake up the MPU6050
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(PWR_MGMT_1); 
  Wire.write(0); 
  Wire.endTransmission(true);
  
  pinMode(BUZZER_PIN, OUTPUT);
  
  delay(100);

  // Read the initial accelerometer data to calculate the initial angles
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(ACCEL_XOUT_H);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU6050_ADDR, 6, true); // Request 6 bytes of data
  
  ax = (Wire.read() << 8) | Wire.read();
  ay = (Wire.read() << 8) | Wire.read();
  az = (Wire.read() << 8) | Wire.read();
  
  // Convert accelerometer data to 'g' (sensor is set to ±2g)
  float ax_g = ax / 16384.0;
  float ay_g = ay / 16384.0;
  float az_g = az / 16384.0;

  // Calculate the initial roll and pitch angles (starting angles)
  initial_roll = atan2(ay_g, az_g) * 180.0 / M_PI;
  initial_pitch = atan2(-ax_g, sqrt(ay_g * ay_g + az_g * az_g)) * 180.0 / M_PI;

  Serial.print("Initial Roll: ");
  Serial.print(initial_roll);
  Serial.print("\tInitial Pitch: ");
  Serial.println(initial_pitch);
}

void loop() {
  // Handle client requests
  server.handleClient();
  
  // If manual buzzer control is active, use that state
  if (manual_buzzer_override) {
    digitalWrite(BUZZER_PIN, manual_buzzer_state ? HIGH : LOW);
  } 
  else {
    // Read accelerometer data
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(ACCEL_XOUT_H);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU6050_ADDR, 6, true); // Request 6 bytes of data
    
    ax = (Wire.read() << 8) | Wire.read();
    ay = (Wire.read() << 8) | Wire.read();
    az = (Wire.read() << 8) | Wire.read();
  
    // Convert accelerometer data to 'g' (sensor is set to ±2g)
    float ax_g = ax / 16384.0;
    float ay_g = ay / 16384.0;
    float az_g = az / 16384.0;
  
    // Calculate the current roll and pitch angles
    roll = atan2(ay_g, az_g) * 180.0 / M_PI;
    pitch = atan2(-ax_g, sqrt(ay_g * ay_g + az_g * az_g)) * 180.0 / M_PI;
    
    Serial.print("Roll: ");
    Serial.print(roll);
    Serial.print("\tPitch: ");
    Serial.println(pitch);
  
    // Determine if back posture alert should be active based on tilt
    bool bad_posture = (fabs(roll - initial_roll) > angle_threshold || 
                         fabs(pitch - initial_pitch) > angle_threshold);
                         
    digitalWrite(BUZZER_PIN, bad_posture ? HIGH : LOW);
  }
  
  // Small delay to prevent CPU hogging
  delay(100);
}

void handleBuzzerOn() {
  manual_buzzer_override = true;
  manual_buzzer_state = true;
  server.send(200, "text/plain", "Buzzer turned ON");
  Serial.println("Manual buzzer ON");
}

void handleBuzzerOff() {
  manual_buzzer_override = true;
  manual_buzzer_state = false;
  server.send(200, "text/plain", "Buzzer turned OFF");
  Serial.println("Manual buzzer OFF");
}
