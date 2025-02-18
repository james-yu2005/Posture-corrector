#include <Wire.h>

const int MPU = 0x68;  // MPU6050 I2C address
float AccX, AccY, AccZ;  // Accelerometer readings
float GyroX, GyroY, GyroZ;  // Gyroscope readings

float kalmanAngleX, kalmanAngleY;  // Filtered angles
float absAngleX, absAngleY;
float biasX = 0, biasY = 0;  // Gyro bias
float P[2][2] = {{1, 0}, {0, 1}};  // Error covariance matrix
float dt = 0.01;  // Time step (10ms)

float initial_x = 0, initial_y = 0;  // Initial angles
bool initial_read = false;  // Flag to check if initial angles are set

void setup() {
  Serial.begin(19200);
  Wire.begin();

  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission(true);

  delay(1000);  // Allow MPU6050 to stabilize
  read_initial_data();  // Capture initial angles
}

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

void read_initial_data() {
  read_MPU6050_data();
  
  float accelAngleX = atan(AccY / sqrt(AccX * AccX + AccZ * AccZ)) * 180 / PI;
  float accelAngleY = atan(-AccX / sqrt(AccY * AccY + AccZ * AccZ)) * 180 / PI;

  initial_x = accelAngleX;
  initial_y = accelAngleY;
  initial_read = true;
}

void loop() {
  read_MPU6050_data();

  float accelAngleX = atan(AccY / sqrt(AccX * AccX + AccZ * AccZ)) * 180 / PI;
  float accelAngleY = atan(-AccX / sqrt(AccY * AccY + AccZ * AccZ)) * 180 / PI;

  float rateX = GyroX - biasX;
  float rateY = GyroY - biasY;

  kalmanAngleX += rateX * dt;
  kalmanAngleY += rateY * dt;

  P[0][0] += dt;
  P[1][1] += dt;

  float K[2] = {P[0][0] / (P[0][0] + 0.1), P[1][1] / (P[1][1] + 0.1)};

  kalmanAngleX += K[0] * (accelAngleX - kalmanAngleX);
  kalmanAngleY += K[1] * (accelAngleY - kalmanAngleY);

  P[0][0] -= K[0] * P[0][0];
  P[1][1] -= K[1] * P[1][1];

  // Calculate absolute change from initial angles
  absAngleX = kalmanAngleX - initial_x;
  absAngleY = kalmanAngleY - initial_y;

  Serial.print("Roll Change: ");
  Serial.print(absAngleX);
  Serial.print(" | Pitch Change: ");
  Serial.println(absAngleY);

  delay(10);
}
