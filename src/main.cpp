#include <Arduino.h>
#include <WiFi.h>
#include "websocket/websocket_client.h"
#include "cuckoo_filter/cuckoo_filter.h"

// WokWi's virtual WiFi network
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// WebSocket configuration - CHANGE THIS to your master node IP
const char* WS_HOST = "192.168.1.100";  // Master/PC IP address
const uint16_t WS_PORT = 81;

// Local Cuckoo Filter instance
CuckooFilter filter;

// WebSocket client instance
WebSocketClient wsClient;

// Node ID for this slave
uint8_t nodeId = 1;

// Callback for received WebSocket messages
void onWebSocketMessage(MessageType type, const char* value, bool result) {
    Serial.printf("[Callback] Type: %d, Value: %s, Result: %d\n", type, value, result);

    switch (type) {
        case MSG_INSERT: {
            bool success = filterInsert(&filter, value);
            Serial.printf("[Cuckoo] INSERT %s -> %s\n", value, success ? "OK" : "FAILED");
            // Send response back to master with actual result
            wsClient.sendResponse(value, success);
            break;
        }

        case MSG_DELETE: {
            bool success = filterDelete(&filter, value);
            Serial.printf("[Cuckoo] DELETE %s -> %s\n", value, success ? "OK" : "NOT FOUND");
            // Send response back to master with actual result
            wsClient.sendResponse(value, success);
            break;
        }

        case MSG_LOOKUP: {
            bool found = filterLookup(&filter, value);
            Serial.printf("[Cuckoo] LOOKUP %s -> %s\n", value, found ? "FOUND" : "NOT FOUND");
            // Send response back to master with actual result
            wsClient.sendResponse(value, found);
            break;
        }

        case MSG_RESPONSE:
            Serial.printf("[Response] %s -> %s\n", value, result ? "SUCCESS" : "FAILED");
            break;
    }
}

void setup() {
    // Initialize Serial Monitor
    Serial.begin(115200);
    delay(10);
    
    Serial.println();
    Serial.println("=== Distributed Cuckoo Filter - Slave Node ===");
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
    
    // Initialize Cuckoo Filter
    filterInit(&filter);
    Serial.println("[Cuckoo] Filter initialized");
    
    // Start connecting to WiFi
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println("\n--- WiFi Connected! ---");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    
    // Initialize WebSocket client
    wsClient.begin(WS_HOST, WS_PORT, "/ws");
    wsClient.onMessage(onWebSocketMessage);
    
    Serial.println("[WebSocket] Client initialized, waiting for connection...");
    Serial.println("Slave node is ready!");
    Serial.println("=============================================");
}

void loop() {
    // Process WebSocket events
    wsClient.loop();
    
    // Reconnect if disconnected
    if (!wsClient.isConnected()) {
        Serial.println("[WebSocket] Not connected, attempting reconnect...");
        delay(1000);
    }
    
    delay(10);
}
