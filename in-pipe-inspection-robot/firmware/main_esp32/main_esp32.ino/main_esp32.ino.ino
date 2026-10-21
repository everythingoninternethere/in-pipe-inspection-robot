
#include <WiFi.h>
#include <WebServer.h>

// =====================================================
//  IN PIPE INSPECTION ROBOT
//  Main ESP32: Motors + HC-SR04 + IR + LED
//  ESP32-CAM runs separate firmware
// =====================================================

// Wi-Fi hotspot details
const char* ssid     = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// ---------------- L298N MOTOR DRIVER ----------------

// Left motor pair
#define ENA 25
#define IN1 26
#define IN2 27

// Right motor pair
#define ENB 14
#define IN3 32
#define IN4 33

// PWM duty: 0 to 255
const int MOTOR_SPEED = 128;

// ---------------- HC-SR04 SENSOR --------------------

#define TRIG_PIN 18
#define ECHO_PIN 19

// ---------------- IR SENSOR -------------------------

#define IR_PIN 23
#define IR_DETECTED_LEVEL LOW

// ---------------- LED MOSFET ------------------------

#define LED_PIN 13

// ---------------- WEB SERVER ------------------------

WebServer server(80);

bool ledState = false;
String vehicleState = "Stopped";

// =====================================================
// MOTOR FUNCTIONS
// =====================================================

void setMotorSpeed(int speedValue) {
  speedValue = constrain(speedValue, 0, 255);

  analogWrite(ENA, speedValue);
  analogWrite(ENB, speedValue);
}

void stopMotors() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  vehicleState = "Stopped";
}

void moveForward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  setMotorSpeed(MOTOR_SPEED);
  vehicleState = "Moving Forward";
}

void moveBackward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  setMotorSpeed(MOTOR_SPEED);
  vehicleState = "Moving Backward";
}

// =====================================================
// ULTRASONIC DISTANCE
// =====================================================

float getDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(3);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration =
      pulseIn(ECHO_PIN, HIGH, 30000UL);

  if (duration == 0) {
    return -1.0;
  }

  return duration * 0.0343 / 2.0;
}

void handleDistance() {
  float distance = getDistanceCM();

  if (distance < 0) {
    server.send(200, "text/plain", "NO ECHO");
  } else {
    server.send(200, "text/plain",
                String(distance, 1));
  }
}

// =====================================================
// IR PROXIMITY SENSOR
// =====================================================

void handleIR() {
  bool detected =
      (digitalRead(IR_PIN) == IR_DETECTED_LEVEL);

  if (detected) {
    server.send(200, "text/plain", "NEAR");
  } else {
    server.send(200, "text/plain", "CLEAR");
  }
}

// =====================================================
// LED CONTROL
// =====================================================

void handleLedOn() {
  digitalWrite(LED_PIN, HIGH);
  ledState = true;

  server.send(200, "text/plain", "LED ON");
}

void handleLedOff() {
  digitalWrite(LED_PIN, LOW);
  ledState = false;

  server.send(200, "text/plain", "LED OFF");
}

void handleLedStatus() {
  server.send(
      200,
      "text/plain",
      ledState ? "ON" : "OFF"
  );
}

// =====================================================
// VEHICLE STATUS
// =====================================================

void handleStatus() {
  String response = "Vehicle: " + vehicleState;

  response += "\nLED: ";
  response += ledState ? "ON" : "OFF";

  response += "\nIR: ";
  response +=
      (digitalRead(IR_PIN) == IR_DETECTED_LEVEL)
      ? "NEAR"
      : "CLEAR";

  server.send(200, "text/plain", response);
}

// =====================================================
// HOME PAGE / CONNECTION TEST
// =====================================================

void handleRoot() {
  String page =
      "<html><head>"
      "<meta name='viewport' content='width=device-width,"
      "initial-scale=1'>"
      "</head><body>"
      "<h2>IN Pipe Inspection Robot</h2>"
      "<p>ESP32 control server is running.</p>"
      "<p>Use the app to control movement, sensors and LED.</p>"
      "</body></html>";

  server.send(200, "text/html", page);
}

// =====================================================
// SETUP
// =====================================================

void setup() {
  Serial.begin(115200);

  // Motor pins
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Sensors
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(IR_PIN, INPUT);

  // LED MOSFET control
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  digitalWrite(TRIG_PIN, LOW);
  stopMotors();

  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to Wi-Fi");

  unsigned long startTime = millis();

  while (WiFi.status() != WL_CONNECTED &&
         millis() - startTime < 20000UL) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Wi-Fi connected");
    Serial.print("ESP32 IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Wi-Fi connection failed.");
    Serial.println("Check hotspot and credentials.");
  }

  // Register HTTP endpoints
  server.on("/", HTTP_GET, handleRoot);

  server.on("/forward", HTTP_GET, []() {
    moveForward();
    server.send(200, "text/plain", "Moving Forward");
  });

  server.on("/backward", HTTP_GET, []() {
    moveBackward();
    server.send(200, "text/plain", "Moving Backward");
  });

  server.on("/stop", HTTP_GET, []() {
    stopMotors();
    server.send(200, "text/plain", "Stopped");
  });

  server.on("/distance", HTTP_GET, handleDistance);
  server.on("/ir", HTTP_GET, handleIR);
  server.on("/status", HTTP_GET, handleStatus);

  server.on("/led/on", HTTP_GET, handleLedOn);
  server.on("/led/off", HTTP_GET, handleLedOff);
  server.on("/led/status", HTTP_GET, handleLedStatus);

  server.begin();
  Serial.println("HTTP server started");
}

// =====================================================
// MAIN LOOP
// =====================================================

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    // Stop the motors if Wi-Fi is lost.
    // This is a basic safety behaviour.
    stopMotors();
  } else {
    server.handleClient();
  }

  delay(2);
}
