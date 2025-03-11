#include <Wire.h>
#include <math.h>

// MPU6050 I2C address
#define MPU6050_ADDR 0x68

// Registers
#define PWR_MGMT_1 0x6B
#define ACCEL_XOUT_H 0x3B
#define GYRO_XOUT_H 0x43

// Buzzer pin
#define BUZZER_PIN 18  // Change to the pin connected to your buzzer

// Variables to store sensor data
int16_t ax, ay, az;

// Angles (in degrees)
float roll = 0.0;
float pitch = 0.0;
float initial_roll = 0.0;
float initial_pitch = 0.0;

// Threshold for change in angle
float angle_threshold = 20.0;

void setup() {
  // Start serial communication
  Serial.begin(115200);
  
  // Initialize I2C communication
  Wire.begin();
  
  // Wake up the MPU6050
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(PWR_MGMT_1); // Wake up MPU6050
  Wire.write(0); // Set to 0 to wake up the device
  Wire.endTransmission(true);
  
  // Initialize the buzzer pin
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Wait for a brief moment to ensure the MPU6050 is ready
  delay(100);

  // Read the initial accelerometer data to calculate the initial angles
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(ACCEL_XOUT_H);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU6050_ADDR, 6, true); // Request 6 bytes of data
  
  ax = (Wire.read() << 8) | Wire.read();
  ay = (Wire.read() << 8) | Wire.read();
  az = (Wire.read() << 8) | Wire.read();
  
  // Convert accelerometer data to 'g' (assuming the sensor is set to ±2g)
  float ax_g = ax / 16384.0;
  float ay_g = ay / 16384.0;
  float az_g = az / 16384.0;

  // Calculate the initial roll and pitch angles (starting angles)
  initial_roll = atan2(ay_g, az_g) * 180.0 / M_PI;
  initial_pitch = atan2(-ax_g, sqrt(ay_g * ay_g + az_g * az_g)) * 180.0 / M_PI;

  // Print the initial angles to the serial monitor
  Serial.print("Initial Roll: ");
  Serial.print(initial_roll);
  Serial.print("\tInitial Pitch: ");
  Serial.println(initial_pitch);
}

void loop() {
  // Read accelerometer data
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(ACCEL_XOUT_H);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU6050_ADDR, 6, true); // Request 6 bytes of data
  
  ax = (Wire.read() << 8) | Wire.read();
  ay = (Wire.read() << 8) | Wire.read();
  az = (Wire.read() << 8) | Wire.read();

  // Convert accelerometer data to 'g' (assuming the sensor is set to ±2g)
  float ax_g = ax / 16384.0;
  float ay_g = ay / 16384.0;
  float az_g = az / 16384.0;

  // Calculate the current roll and pitch angles
  roll = atan2(ay_g, az_g) * 180.0 / M_PI;
  pitch = atan2(-ax_g, sqrt(ay_g * ay_g + az_g * az_g)) * 180.0 / M_PI;

  // Check for change in roll or pitch exceeding the threshold from the initial values
  if (fabs(roll - initial_roll) > angle_threshold || fabs(pitch - initial_pitch) > angle_threshold) {
    digitalWrite(BUZZER_PIN, HIGH);  // Turn on the buzzer
  } else {
    digitalWrite(BUZZER_PIN, LOW);   // Turn off the buzzer
  }

  // Print the angles
  Serial.print("Abs Roll: ");
  Serial.print(roll - initial_roll);
  Serial.print("\tAbs Pitch: ");
  Serial.println(pitch - initial_pitch);

  // Add a small delay to make the output more readable
  delay(100);
}
