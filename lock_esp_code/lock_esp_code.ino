#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <Keypad.h>
#include <ESP32Servo.h>

// -------- MQTT --------
const char* mqtt_server = "YOUR_HOST";
const char* mqtt_user = "esp32_user";
const char* mqtt_pass = "Esp32_user";

WiFiClientSecure espClient;
PubSubClient client(espClient);

// -------- DEVICE ID --------
#define LOCK_ID "lock_1"

// -------- SERVO --------
Servo servo;
#define SERVO_PIN 23
#define LOCK_POS 10
#define UNLOCK_POS 90

// -------- STATE --------
bool isDocked = true;
bool isUnlocked = false;

String currentOTP = "";
String enteredOTP = "";

int wrongAttempts = 0;
const int MAX_ATTEMPTS = 5;

// -------- RIDE --------
unsigned long rideStart = 0;
bool rideActive = false;

// -------- KEYPAD --------
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {13,12,14,27};
byte colPins[COLS] = {26,25,33,32};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// -------- CALLBACK --------
void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (int i = 0; i < length; i++) msg += (char)payload[i];
  msg.trim();

  Serial.println(msg);

  if (String(topic) == "stand/1/dock") {

    if (msg == "connected") {
      isDocked = true;

      if (rideActive) {
        unsigned long duration = millis() - rideStart;
        rideActive = false;

        // send ride data to stand
        client.publish("stand/1/data", String(duration).c_str());
      }

      servo.write(LOCK_POS);
    }

    if (msg == "removed") {
      isDocked = false;
      rideStart = millis();
      rideActive = true;
    }
  }

  // OTP from stand
  if (String(topic) == "stand/1/otp") {
    currentOTP = msg;
    Serial.println("OTP STORED");
  }
}

// -------- KEYPAD --------
void handleKeypad() {
  char key = keypad.getKey();

  if (!key) return;

  if (key == '#') {

    if (enteredOTP == currentOTP) {
      wrongAttempts = 0;

      if (isUnlocked) {
        servo.write(LOCK_POS);
        isUnlocked = false;
      } else {
        servo.write(UNLOCK_POS);
        isUnlocked = true;
      }

    } else {
      wrongAttempts++;
      Serial.println("WRONG");

      if (wrongAttempts >= MAX_ATTEMPTS) {
        Serial.println("LOCKED OUT");
        delay(10000);
        wrongAttempts = 0;
      }
    }

    enteredOTP = "";
  }

  else if (key == '*') {
    enteredOTP = "";
  }

  else {
    enteredOTP += key;
  }
}

// -------- SETUP --------
void setup() {
  Serial.begin(115200);

  WiFi.begin("AM", "1234@1234");
  while (WiFi.status() != WL_CONNECTED) delay(500);

  espClient.setInsecure();
  client.setServer(mqtt_server, 8883);
  client.setCallback(callback);

  servo.attach(SERVO_PIN);
  servo.write(LOCK_POS);
}

// -------- LOOP --------
void loop() {
  if (!client.connected()) {
    client.connect(LOCK_ID, mqtt_user, mqtt_pass);
    client.subscribe("stand/1/dock");
    client.subscribe("stand/1/otp");
  }

  client.loop();
  handleKeypad();
}