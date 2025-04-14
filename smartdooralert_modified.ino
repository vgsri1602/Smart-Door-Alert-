#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>

// Wi-Fi credentials
#define WIFI_SSID "WIFI NAME"
#define WIFI_PASSWORD "WIFI PASSWORD"

// Firebase credentials
#define FIREBASE_HOST "HOST LINK"
#define FIREBASE_AUTH "SECRET KEY"

// Firebase objects
FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;

// Pin definitions
const int trigPin = D1;
const int echoPin = D2;
const int irSensorPin = D6;
const int buttonPin = D7;
const int buzzerPin = D5;

long duration;
int distance;
bool alertSentIR = false;
bool alertSentUltrasonic = false;

void setup() {
  Serial.begin(115200);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(irSensorPin, INPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);

  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected!");

  // Firebase setup
  config.host = FIREBASE_HOST;
  auth.user.email = "";
  auth.user.password = "";
  config.signer.tokens.legacy_token = FIREBASE_AUTH;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  // Check for "Cancel" command from Firebase
  if (Firebase.getString(firebaseData, "/status")) {
    String status = firebaseData.stringData();
    if (status == "Cancel") {
      digitalWrite(buzzerPin, LOW);
      alertSentIR = false;
      alertSentUltrasonic = false;
      Serial.println("Alert cancelled via App");
    }
  }

  // Read button state
  int buttonStatus = digitalRead(buttonPin);
  if (buttonStatus == LOW) {
    digitalWrite(buzzerPin, LOW);
    alertSentIR = false;
    alertSentUltrasonic = false;
    Firebase.setString(firebaseData, "/status", "Alert Cancelled");
    Serial.println("Alert cancelled manually via button.");
    delay(500);
  }

  // Check IR sensor
  int irStatus = digitalRead(irSensorPin);
  if (irStatus == LOW && !alertSentIR) {
    Firebase.setString(firebaseData, "/status", "IR sensor: Person Detected");
    digitalWrite(buzzerPin, HIGH);
    alertSentIR = true;
    Serial.println("IR: Person detected. Beep!");
  }

  // Ultrasonic distance check
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  if (distance <= 50 && !alertSentUltrasonic) {
    Firebase.setString(firebaseData, "/status", "Ultrasonic: Person Detected");
    digitalWrite(buzzerPin, HIGH);
    alertSentUltrasonic = true;
    Serial.println("Ultrasonic: Person detected. Beep!");
  }

  delay(500);
}
