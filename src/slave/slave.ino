#include <WiFi.h>

// Wokwi's virtual WiFi network
const char* ssid = "Wokwi-GUEST";
const char* password = "";

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  delay(10); // Short delay to let Serial initialize

  Serial.println();
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  // Start connecting to the virtual WiFi
  WiFi.begin(ssid, password);

  // Wait until the connection is established
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Print success message and IP address
  Serial.println("\n--- WiFi Connected! ---");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Slave node is ready and waiting for WebSocket setup.");
}

void loop() {
  // The main loop does nothing for now.
  // Kabeer will add the WebSocket listening logic here later.
  delay(10); 
}