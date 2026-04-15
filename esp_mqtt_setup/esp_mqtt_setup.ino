#include <WiFi.h>
#include <WiFiClientSecure.h>   // 🔥 REQUIRED
#include <PubSubClient.h>

const char* ssid = "AM";
const char* password = "1234@1234";

const char* mqtt_server = "654dcb2e2b224e16af4fe695597c1b42.s1.eu.hivemq.cloud";

// 🔐 Your MQTT credentials
const char* mqtt_user = "esp32_user";
const char* mqtt_pass = "Esp32_user";

WiFiClientSecure espClient;
PubSubClient client(espClient);

// ---------------- WIFI ----------------
void setup_wifi() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// ---------------- MQTT RECONNECT ----------------
void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");

    // 🔥 IMPORTANT: Unique client ID
    if (client.connect("esp32_lock_1", mqtt_user, mqtt_pass)) {
      Serial.println("connected");

      // Subscribe to a test topic
      client.subscribe("test/topic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 2 sec...");
      delay(2000);
    }
  }
}

// ---------------- MESSAGE CALLBACK ----------------
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived: ");

  String msg = "";
  for (int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }

  Serial.println(msg);
}

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);

  setup_wifi();

  espClient.setInsecure(); // 🔥 required for HiveMQ TLS

  client.setServer(mqtt_server, 8883);
  client.setCallback(callback);
}

// ---------------- LOOP ----------------
void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  // 🔥 TEST: publish message every 3 seconds
  static unsigned long lastMsg = 0;
  if (millis() - lastMsg > 3000) {
    lastMsg = millis();

    client.publish("test/topic", "hello from esp32");
    Serial.println("Message sent");
  }
}