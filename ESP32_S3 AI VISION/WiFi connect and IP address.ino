/*
  ESP32-S3 WiFi Connection and IP Address Example
*/

#include <WiFi.h>

// Enter your WiFi credentials
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("Connecting to WiFi...");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("================================");
  Serial.println("WiFi Connected Successfully!");
  Serial.print("ESP32-S3 IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println("================================");
}

void loop() {
  // Nothing to do here
}
