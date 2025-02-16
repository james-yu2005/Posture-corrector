#include <Wire.h>

const int MPU = 0x68;  // MPU6050 I2C address
float AccX, AccY, AccZ;  // Accelerometer readings
float GyroX, GyroY, GyroZ;  // Gyroscope readings

float kalmanAngleX, kalmanAngleY;  // Filtered angles
float biasX = 0, biasY = 0;  // Gyro bias
float P[2][2] = {{1, 0}, {0, 1}};  // Error covariance matrix
float dt = 0.01;  // Time step (10ms)

// Function to initialize MPU6050
void setup() {
  Serial.begin(19200);
  Wire.begin();

  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission(true);
}

// Function to read MPU6050 data
void read_MPU6050_data() {
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);  
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true);

  AccX = (Wire.read() << 8 | Wire.read()) / 4096.0;
  AccY = (Wire.read() << 8 | Wire.read()) / 4096.0;
  AccZ = (Wire.read() << 8 | Wire.read()) / 4096.0;

  Wire.beginTransmission(MPU);
  Wire.write(0x43);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true);

  GyroX = (Wire.read() << 8 | Wire.read()) / 32.8;
  GyroY = (Wire.read() << 8 | Wire.read()) / 32.8;
  GyroZ = (Wire.read() << 8 | Wire.read()) / 32.8;
}

void loop() {
  read_MPU6050_data();

  // Calculate accelerometer angles
  float accelAngleX = atan(AccY / sqrt(AccX * AccX + AccZ * AccZ)) * 180 / PI;
  float accelAngleY = atan(-AccX / sqrt(AccY * AccY + AccZ * AccZ)) * 180 / PI;

  // Kalman Prediction Step
  float rateX = GyroX - biasX;
  float rateY = GyroY - biasY;

  kalmanAngleX += rateX * dt;
  kalmanAngleY += rateY * dt;

  P[0][0] += dt;
  P[1][1] += dt;

  // Kalman Correction Step
  float K[2] = {P[0][0] / (P[0][0] + 0.1), P[1][1] / (P[1][1] + 0.1)};

  kalmanAngleX += K[0] * (accelAngleX - kalmanAngleX);
  kalmanAngleY += K[1] * (accelAngleY - kalmanAngleY);

  P[0][0] -= K[0] * P[0][0];
  P[1][1] -= K[1] * P[1][1];

  // Print filtered angles
  Serial.print("Roll: ");
  Serial.print(kalmanAngleX);
  Serial.print(" | Pitch: ");
  Serial.println(kalmanAngleY);

  delay(10);
}
