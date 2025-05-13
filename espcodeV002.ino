#include <ESP8266WiFi.h>
#include <Servo.h>

// WiFi credentials
const char* ssid = "Moto";
const char* password = "anas@1234";

// Servo setup
Servo baseServo;
Servo panelServo;
int baseServoPosition = 90;
int panelServoPosition = 90;

// WiFi server setup
WiFiServer server(80);

// LED & Buzzer pins
#define WHITE_LED D1     // Panel movement
#define YELLOW_LED D2    // Base movement
#define GREEN_LED D5     // Wi-Fi connected
#define RED_LED D6       // Wi-Fi error
#define BUZZER D7        // Beeper

void setup() {
  Serial.begin(115200);

  // Attach servos
  baseServo.attach(D4);   // Base servo
  panelServo.attach(D8);  // Panel servo
  baseServo.write(baseServoPosition);
  panelServo.write(panelServoPosition);

  // LED & Buzzer pin setup
  pinMode(WHITE_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  // Start with all indicators off
  digitalWrite(WHITE_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(BUZZER, LOW);

  // Attempt Wi-Fi connection
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to WiFi");
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);
    digitalWrite(BUZZER, LOW);
  } else {
    Serial.println("\nWiFi connection failed");
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, HIGH);
    digitalWrite(BUZZER, HIGH);
    delay(1000);
    digitalWrite(BUZZER, LOW);
  }

  server.begin();
}

void loop() {
  // Monitor Wi-Fi status during loop
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);
    digitalWrite(BUZZER, LOW);
  } else {
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, HIGH);
    digitalWrite(BUZZER, HIGH);
  }

  WiFiClient client = server.available();
  if (!client) return;

  while (!client.available()) delay(1);
  String request = client.readStringUntil('\r');
  client.flush();

  // Handle base servo control
  if (request.indexOf("/baseServo?angle=") != -1) {
    int angle = request.substring(request.indexOf('=') + 1).toInt();
    angle = constrain(angle, 0, 180);
    baseServo.write(angle);
    baseServoPosition = angle;
    digitalWrite(YELLOW_LED, HIGH);
    delay(300);
    digitalWrite(YELLOW_LED, LOW);
  }

  // Handle panel servo control
  if (request.indexOf("/panelServo?angle=") != -1) {
    int angle = request.substring(request.indexOf('=') + 1).toInt();
    angle = constrain(angle, 0, 180);
    panelServo.write(angle);
    panelServoPosition = angle;
    digitalWrite(WHITE_LED, HIGH);
    delay(300);
    digitalWrite(WHITE_LED, LOW);
  }

  // Simulate stepper movement with servo
  if (request.indexOf("/startBaseRotation?dir=1") != -1) {
    baseServoPosition += 15;
    if (baseServoPosition > 180) baseServoPosition = 180;
    baseServo.write(baseServoPosition);
    digitalWrite(YELLOW_LED, HIGH);
    delay(300);
    digitalWrite(YELLOW_LED, LOW);
  }

  if (request.indexOf("/startBaseRotation?dir=-1") != -1) {
    baseServoPosition -= 15;
    if (baseServoPosition < 0) baseServoPosition = 0;
    baseServo.write(baseServoPosition);
    digitalWrite(YELLOW_LED, HIGH);
    delay(300);
    digitalWrite(YELLOW_LED, LOW);
  }

  if (request.indexOf("/stopBaseRotation") != -1) {
    baseServo.write(baseServoPosition);
    digitalWrite(YELLOW_LED, LOW);
  }

  // Send response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  client.println("OK");

  delay(15);
}
