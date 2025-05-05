#include <Wire.h>

const int MPU = 0x68;  // MPU6050 I2C address
const int buttonPin = 2; 
float AccX, AccY, AccZ;  // Accelerometer readings
float GyroX, GyroY, GyroZ;  // Gyroscope readings

int buttonState = 0; 

float kalmanAngleX, kalmanAngleY;  // Filtered angles
float AngleX, AngleY;
float absAngleX, absAngleY;
float biasX = 0, biasY = 0;  // Gyro bias
float P[2][2] = {{1, 0}, {0, 1}};  // Error covariance matrix
float dt = 0.01;  

float initial_x = 0, initial_y = 0; 
bool initial_read = false;  // Flag to check if initial angles are set

const int buzzerPin = 7; 

void setup() {
  pinMode(buttonPin, INPUT);

  Serial.begin(19200);
  Wire.begin();

  // Initialize MPU6050
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission(true);

  delay(1000);  
  read_initial_data();  

  pinMode(buzzerPin, OUTPUT);  
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
  AngleX = kalmanAngleX - initial_x;
  AngleY = kalmanAngleY - initial_y;
  absAngleX = abs(AngleX);
  absAngleY = abs(AngleY);

  Serial.print("Roll Change: ");
  Serial.print(absAngleX);
  Serial.print(" | Pitch Change: ");
  Serial.println(absAngleY);

  // If the absolute angle is above 20 degrees, activate the buzzer
  if (absAngleX > 20 || absAngleY > 20) {
    digitalWrite(buzzerPin, HIGH); 
  } else {
    digitalWrite(buzzerPin, LOW);  
  }

  buttonState = digitalRead(buttonPin);
  // check if the pushbutton is pressed. If it is, then reset all angles, which turns of buzzer
  if (buttonState == HIGH) {
    read_initial_data();
    kalmanAngleX = 0;
    kalmanAngleY = 0;
    Serial.println("Angles reset!");
  }
  delay(10);
}
