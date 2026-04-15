#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

// -------- MQTT --------
const char* mqtt_server = "YOUR_HOST";
const char* mqtt_user = "esp32_user";
const char* mqtt_pass = "Esp32_user";

WiFiClientSecure espClient;
PubSubClient client(espClient);

#define STAND_ID "stand_1"

// -------- CALLBACK --------
void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (int i = 0; i < length; i++) msg += (char)payload[i];
  msg.trim();

  Serial.println(msg);

  // OTP from backend
  if (String(topic) == "stand/1/otp") {
    // forward to lock
    client.publish("stand/1/otp", msg.c_str());
  }

  // ride data from lock
  if (String(topic) == "stand/1/data") {
    Serial.print("Ride time: ");
    Serial.println(msg);

    // forward to backend (same topic or different)
    client.publish("backend/ride", msg.c_str());
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
}

// -------- LOOP --------
void loop() {
  if (!client.connected()) {
    client.connect(STAND_ID, mqtt_user, mqtt_pass);

    client.subscribe("stand/1/otp");
    client.subscribe("stand/1/data");
  }

  client.loop();

  // simulate docking
  // replace with real pogo detection later
  client.publish("stand/1/dock", "connected");
  delay(5000);
  client.publish("stand/1/dock", "removed");
  delay(10000);
}