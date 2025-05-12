#include <ESP8266WiFi.h>
#include <Servo.h>

// WiFi credentials
const char* ssid = "Moto";
const char* password = "anas@1234";

// Servo setup
Servo baseServo;
Servo panelServo;
int baseServoPosition = 90;  // Initial position of the base servo (middle of 0-180)
int panelServoPosition = 90;  // Initial position of the panel servo (middle of 0-180)

// WiFi server setup
WiFiServer server(80);

void setup() {
  Serial.begin(115200); // Start Serial monitor

  baseServo.attach(D4);  // Attach base servo to GPIO pin D4
  panelServo.attach(D8); // Attach panel servo to GPIO pin D8

  baseServo.write(baseServoPosition);  // Set initial base servo position
  panelServo.write(panelServoPosition); // Set initial panel servo position

  // WiFi setup
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); 
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  server.begin();  // Start WiFi server
}

void loop() {
  WiFiClient client = server.available();
  if (!client) return;  // If no client connected, return

  while (!client.available()) delay(1);  // Wait for the client to send a request
  String request = client.readStringUntil('\r');
  client.flush();  // Clear the buffer

  // Handle the servo control for the base
  if (request.indexOf("/baseServo?angle=") != -1) {
    int angle = request.substring(request.indexOf('=') + 1).toInt();
    angle = constrain(angle, 0, 180);  // Ensure the angle is within the valid range for the base servo
    baseServo.write(angle);  // Set the base servo position
    baseServoPosition = angle;  // Store the last position
  }

  // Handle the servo control for the panel
  if (request.indexOf("/panelServo?angle=") != -1) {
    int angle = request.substring(request.indexOf('=') + 1).toInt();
    angle = constrain(angle, 0, 180);  // Ensure the angle is within the valid range for the panel servo
    panelServo.write(angle);  // Set the panel servo position
    panelServoPosition = angle;  // Store the last position
  }

  // Handle the base stepper-like movement (simulate with servo)
  if (request.indexOf("/startBaseRotation?dir=1") != -1) {
    baseServoPosition += 15;  // Rotate base clockwise
    if (baseServoPosition > 180) baseServoPosition = 180;  // Ensure the angle stays within the range
    baseServo.write(baseServoPosition);
  }

  if (request.indexOf("/startBaseRotation?dir=-1") != -1) {
    baseServoPosition -= 15;  // Rotate base counter-clockwise
    if (baseServoPosition < 0) baseServoPosition = 0;  // Ensure the angle stays within the range
    baseServo.write(baseServoPosition);
  }

  if (request.indexOf("/stopBaseRotation") != -1) {
    // Stop the rotation (halt any further movement)
    baseServo.write(baseServoPosition);  // Keep it at the last position
  }

  // Respond to the client with a simple OK message
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  client.println("OK");

  delay(15);  // Small delay for the servos to reach their position
}
