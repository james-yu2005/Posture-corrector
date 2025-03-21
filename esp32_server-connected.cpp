#include <Wire.h>
#include <math.h>
#include <WiFi.h>
#include <WebServer.h>

// WiFi credentials
const char* ssid = "Rogers116-TP";       // Replace with your WiFi SSID
const char* password = "6478988917"; // Replace with your WiFi password

// Create web server on port 80
WebServer server(80);

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

// Flags for different alert sources
bool back_posture_alert = false;  // MPU6050 tilt detection
bool neck_posture_alert = false;  // ML model web alerts

// System mode control
bool back_detection_enabled = true;
bool neck_detection_enabled = true;

void setup() {
  // Start serial communication
  Serial.begin(115200);
  
  // Connect to WiFi
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
  server.on("/", handleRoot);
  server.on("/buzzer/on", handleBuzzerOn);
  server.on("/buzzer/off", handleBuzzerOff);
  server.on("/neck/alert", handleNeckAlert);
  server.on("/neck/clear", handleNeckClear);
  server.on("/back/enable", handleBackEnable);
  server.on("/back/disable", handleBackDisable);
  server.on("/neck/enable", handleNeckEnable);
  server.on("/neck/disable", handleNeckDisable);
  server.on("/status", handleStatus);
  server.onNotFound(handleNotFound);
  
  // Start server
  server.begin();
  Serial.println("HTTP server started");
  
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
  // Handle client requests
  server.handleClient();
  
  // Process MPU6050 data for back posture if enabled
  if (back_detection_enabled) {
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
  
    // Determine if back posture alert should be active based on tilt
    back_posture_alert = (fabs(roll - initial_roll) > angle_threshold || 
                          fabs(pitch - initial_pitch) > angle_threshold);
  }
  
  // Decide if buzzer should be on based on both alert sources
  bool should_buzz = (back_posture_alert && back_detection_enabled) || 
                      (neck_posture_alert && neck_detection_enabled);
                      
  // Control the buzzer
  digitalWrite(BUZZER_PIN, should_buzz ? HIGH : LOW);
  
  // Small delay to prevent CPU hogging
  delay(50);
}

// Handle root page with control UI
void handleRoot() {
  String html = "<html><head>";
  html += "<title>Posture Monitor System</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; text-align: center; margin: 20px; }";
  html += "button { background-color: #4CAF50; color: white; padding: 10px 20px; margin: 10px; border: none; border-radius: 4px; cursor: pointer; }";
  html += "button.red { background-color: #f44336; }";
  html += "button.blue { background-color: #2196F3; }";
  html += "button.disabled { background-color: #cccccc; }";
  html += ".section { margin: 20px 0; padding: 15px; border: 1px solid #ddd; border-radius: 8px; }";
  html += "table { margin: 0 auto; }";
  html += "td { padding: 8px; }";
  html += ".alert { color: red; font-weight: bold; }";
  html += ".normal { color: green; font-weight: bold; }";
  html += "</style>";
  html += "<script>";
  html += "function updateStatus() {";
  html += "  fetch('/status')";
  html += "    .then(response => response.json())";
  html += "    .then(data => {";
  html += "      document.getElementById('back_status').className = data.back_alert ? 'alert' : 'normal';";
  html += "      document.getElementById('back_status').innerText = data.back_alert ? 'Poor Posture' : 'Good Posture';";
  html += "      document.getElementById('neck_status').className = data.neck_alert ? 'alert' : 'normal';";
  html += "      document.getElementById('neck_status').innerText = data.neck_alert ? 'Poor Posture' : 'Good Posture';";
  html += "      document.getElementById('buzzer_status').innerText = data.buzzer_on ? 'ON' : 'OFF';";
  html += "      document.getElementById('back_enabled').innerText = data.back_enabled ? 'Enabled' : 'Disabled';";
  html += "      document.getElementById('neck_enabled').innerText = data.neck_enabled ? 'Enabled' : 'Disabled';";
  html += "    });";
  html += "}";
  html += "setInterval(updateStatus, 1000);";
  html += "updateStatus();";
  html += "</script>";
  html += "</head><body>";
  html += "<h1>Posture Monitor System</h1>";
  
  html += "<div class='section'>";
  html += "<h2>System Status</h2>";
  html += "<table>";
  html += "<tr><td>Back Posture:</td><td id='back_status'>Loading...</td></tr>";
  html += "<tr><td>Neck Posture:</td><td id='neck_status'>Loading...</td></tr>";
  html += "<tr><td>Buzzer:</td><td id='buzzer_status'>Loading...</td></tr>";
  html += "</table>";
  html += "</div>";
  
  html += "<div class='section'>";
  html += "<h2>Buzzer Control</h2>";
  html += "<button class='red' onclick='location.href=\"/buzzer/on\"'>Turn Buzzer ON</button>";
  html += "<button class='blue' onclick='location.href=\"/buzzer/off\"'>Turn Buzzer OFF</button>";
  html += "</div>";
  
  html += "<div class='section'>";
  html += "<h2>Detection Systems</h2>";
  html += "<table>";
  html += "<tr>";
  html += "<td>Back Sensor (MPU6050):</td>";
  html += "<td id='back_enabled'>Loading...</td>";
  html += "<td><button onclick='location.href=\"/back/enable\"'>Enable</button></td>";
  html += "<td><button onclick='location.href=\"/back/disable\"'>Disable</button></td>";
  html += "</tr>";
  html += "<tr>";
  html += "<td>Neck Detection (ML):</td>";
  html += "<td id='neck_enabled'>Loading...</td>";
  html += "<td><button onclick='location.href=\"/neck/enable\"'>Enable</button></td>";
  html += "<td><button onclick='location.href=\"/neck/disable\"'>Disable</button></td>";
  html += "</tr>";
  html += "</table>";
  html += "</div>";
  
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

// Turn the buzzer on manually
void handleBuzzerOn() {
  back_posture_alert = true;
  neck_posture_alert = true;
  server.sendHeader("Location", "/");
  server.send(303);
}

// Turn the buzzer off manually
void handleBuzzerOff() {
  back_posture_alert = false;
  neck_posture_alert = false;
  server.sendHeader("Location", "/");
  server.send(303);
}

// Receive neck alert from ML model
void handleNeckAlert() {
  neck_posture_alert = true;
  server.send(200, "text/plain", "Neck alert activated");
}

// Clear neck alert from ML model
void handleNeckClear() {
  neck_posture_alert = false;
  server.send(200, "text/plain", "Neck alert cleared");
}

// Enable back posture detection
void handleBackEnable() {
  back_detection_enabled = true;
  server.sendHeader("Location", "/");
  server.send(303);
}

// Disable back posture detection
void handleBackDisable() {
  back_detection_enabled = false;
  back_posture_alert = false;
  server.sendHeader("Location", "/");
  server.send(303);
}

// Enable neck posture detection
void handleNeckEnable() {
  neck_detection_enabled = true;
  server.sendHeader("Location", "/");
  server.send(303);
}

// Disable neck posture detection
void handleNeckDisable() {
  neck_detection_enabled = false;
  neck_posture_alert = false;
  server.sendHeader("Location", "/");
  server.send(303);
}

// Handle status request (JSON response for AJAX updates)
void handleStatus() {
  String json = "{";
  json += "\"back_alert\":" + String(back_posture_alert ? "true" : "false") + ",";
  json += "\"neck_alert\":" + String(neck_posture_alert ? "true" : "false") + ",";
  json += "\"buzzer_on\":" + String(digitalRead(BUZZER_PIN) == HIGH ? "true" : "false") + ",";
  json += "\"back_enabled\":" + String(back_detection_enabled ? "true" : "false") + ",";
  json += "\"neck_enabled\":" + String(neck_detection_enabled ? "true" : "false");
  json += "}";
  
  server.send(200, "application/json", json);
}

// Handle 404 Not Found errors
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  
  server.send(404, "text/plain", message);
}
