/**
 * Master Node - Distributed Cuckoo Filter System
 * 
 * This is the coordinator node that:
 * - Runs a WebSocket server on port 81
 * - Accepts connections from slave nodes
 * - Distributes items across slaves based on hash
 * - Aggregates responses from slaves
 * 
 * For: Ayesha (Team Lead)
 * Author: Kabeer Jafri
 * Date: March 27, 2026
 */

#include <Arduino.h>
#include <WiFi.h>
#include "websocket/websocket_server.h"
#include "cuckoo_filter/cuckoo_filter.h"

// WiFi Configuration
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// WebSocket Server on port 81
WebSocketServer wsServer(81);

// Local Cuckoo Filter (for master's own storage)
CuckooFilter masterFilter;

// Track connected slaves
#define MAX_SLAVES 10
uint8_t connectedSlaves[MAX_SLAVES];
uint8_t slaveCount = 0;

// Node ID for master
const uint8_t MASTER_NODE_ID = 0;

// Function prototypes
void onSlaveMessage(uint8_t num, MessageType type, const char* value);
void broadcastToSlaves(const char* message);
uint8_t getSlaveForItem(const char* item);
void printConnectedSlaves();

/**
 * Callback when a slave node sends a message
 */
void onSlaveMessage(uint8_t num, MessageType type, const char* value) {
    Serial.printf("[Master] Slave %d sent: type=%d, value=%s\n", num, type, value);
    
    switch (type) {
        case MSG_INSERT: {
            // Slave is notifying us of an insert (for sync)
            // Add to master's filter too
            bool success = filterInsert(&masterFilter, value);
            Serial.printf("[Master] Sync INSERT %s -> %s\n", value, success ? "OK" : "FAILED");
            
            // Acknowledge
            wsServer.sendResponse(num, MSG_RESPONSE, value, success);
            break;
        }
        
        case MSG_LOOKUP: {
            // Slave is checking with master
            bool found = filterLookup(&masterFilter, value);
            Serial.printf("[Master] Sync LOOKUP %s -> %s\n", value, found ? "FOUND" : "NOT FOUND");
            
            wsServer.sendResponse(num, MSG_RESPONSE, value, found);
            break;
        }
        
        case MSG_DELETE: {
            // Slave is notifying of delete
            bool deleted = filterDelete(&masterFilter, value);
            Serial.printf("[Master] Sync DELETE %s -> %s\n", value, deleted ? "OK" : "NOT FOUND");
            
            wsServer.sendResponse(num, MSG_RESPONSE, value, deleted);
            break;
        }
        
        case MSG_RESPONSE:
            // Slave responded to our command (handled elsewhere)
            Serial.printf("[Master] Received RESPONSE from slave %d\n", num);
            break;
    }
}

/**
 * Broadcast a message to all connected slaves
 */
void broadcastToSlaves(const char* message) {
    wsServer.broadcast(message);
    Serial.printf("[Master] Broadcast: %s\n", message);
}

/**
 * Determine which slave should handle an item
 * Uses simple hash-based distribution
 */
uint8_t getSlaveForItem(const char* item) {
    if (slaveCount == 0) {
        return 255;  // No slaves connected
    }
    
    // Simple hash-based distribution
    uint32_t hash = 0;
    const char* ptr = item;
    while (*ptr) {
        hash = hash * 31 + (uint8_t)(*ptr++);
    }
    
    return hash % slaveCount;
}

/**
 * Print list of connected slaves
 */
void printConnectedSlaves() {
    Serial.print("[Master] Connected slaves: ");
    if (slaveCount == 0) {
        Serial.println("None");
        return;
    }
    
    for (uint8_t i = 0; i < slaveCount; i++) {
        Serial.printf("%d ", connectedSlaves[i]);
    }
    Serial.println();
}

/**
 * Send command to specific slave
 */
bool sendToSlave(uint8_t slaveIndex, MessageType type, const char* value) {
    if (slaveIndex >= slaveCount) {
        Serial.printf("[Master] Invalid slave index: %d\n", slaveIndex);
        return false;
    }
    
    // Create message
    StaticJsonDocument<128> doc;
    
    const char* typeStr;
    switch (type) {
        case MSG_INSERT: typeStr = "INSERT"; break;
        case MSG_DELETE: typeStr = "DELETE"; break;
        case MSG_LOOKUP: typeStr = "LOOKUP"; break;
        default: typeStr = "UNKNOWN";
    }
    
    doc["type"] = typeStr;
    doc["value"] = value;
    doc["nodeId"] = MASTER_NODE_ID;
    
    String msg;
    serializeJson(doc, msg);
    
    // Send to specific slave (by connection number)
    // Note: In this simple implementation, slaveIndex == connection number
    bool sent = wsServer.sendTXT(slaveIndex, msg);
    Serial.printf("[Master] Sent to slave %d: %s -> %s\n", slaveIndex, value, sent ? "OK" : "FAILED");
    
    return sent;
}

/**
 * Insert item into distributed filter
 */
bool distributedInsert(const char* item) {
    uint8_t slaveIdx = getSlaveForItem(item);
    
    if (slaveIdx == 255) {
        // No slaves, insert locally
        bool success = filterInsert(&masterFilter, item);
        Serial.printf("[Master] No slaves, local INSERT %s -> %s\n", item, success ? "OK" : "FAILED");
        return success;
    }
    
    // Send to appropriate slave
    return sendToSlave(slaveIdx, MSG_INSERT, item);
}

/**
 * Lookup item in distributed filter
 */
bool distributedLookup(const char* item) {
    uint8_t slaveIdx = getSlaveForItem(item);
    
    if (slaveIdx == 255) {
        // No slaves, check locally
        bool found = filterLookup(&masterFilter, item);
        Serial.printf("[Master] No slaves, local LOOKUP %s -> %s\n", item, found ? "FOUND" : "NOT FOUND");
        return found;
    }
    
    // Send to appropriate slave
    return sendToSlave(slaveIdx, MSG_LOOKUP, item);
}

/**
 * Delete item from distributed filter
 */
bool distributedDelete(const char* item) {
    uint8_t slaveIdx = getSlaveForItem(item);
    
    if (slaveIdx == 255) {
        // No slaves, delete locally
        bool deleted = filterDelete(&masterFilter, item);
        Serial.printf("[Master] No slaves, local DELETE %s -> %s\n", item, deleted ? "OK" : "NOT FOUND");
        return deleted;
    }
    
    // Send to appropriate slave
    return sendToSlave(slaveIdx, MSG_DELETE, item);
}

void setup() {
    // Initialize Serial
    Serial.begin(115200);
    delay(10);
    
    Serial.println();
    Serial.println("=============================================");
    Serial.println("  Distributed Cuckoo Filter - MASTER NODE");
    Serial.println("=============================================");
    
    // Initialize Cuckoo Filter
    filterInit(&masterFilter);
    Serial.println("[Master] Cuckoo filter initialized");
    
    // Connect to WiFi
    Serial.print("[Master] Connecting to WiFi: ");
    Serial.println(ssid);
    
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println("\n[Master] WiFi Connected!");
    Serial.print("[Master] IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("[Master] Start WebSocket server on port 81");
    
    // Start WebSocket Server
    wsServer.begin();
    wsServer.onMessage(onSlaveMessage);
    
    Serial.println("=============================================");
    Serial.println("[Master] READY - Waiting for slave nodes...");
    Serial.println("=============================================");
    Serial.println();
    
    // Initialize slave tracking
    slaveCount = 0;
    memset(connectedSlaves, 0, sizeof(connectedSlaves));
}

// Counter for demo operations
int demoCounter = 0;
unsigned long lastDemoTime = 0;
const unsigned long DEMO_INTERVAL = 5000;  // Demo every 5 seconds

void loop() {
    // Process WebSocket connections
    wsServer.loop();
    
    // Check for new slave connections
    uint8_t currentCount = wsServer.getConnectedCount();
    if (currentCount != slaveCount) {
        Serial.printf("[Master] Slave count changed: %d -> %d\n", slaveCount, currentCount);
        slaveCount = currentCount;
        printConnectedSlaves();
    }
    
    // Run demo operations periodically
    unsigned long now = millis();
    if (now - lastDemoTime > DEMO_INTERVAL) {
        lastDemoTime = now;
        demoCounter++;
        
        Serial.println();
        Serial.println("========== DEMO OPERATION ==========");
        
        char item[32];
        snprintf(item, sizeof(item), "demo_item_%d", demoCounter);
        
        // Demo: Insert
        Serial.printf("[Demo] INSERT %s\n", item);
        distributedInsert(item);
        
        // Demo: Lookup (should find it)
        delay(100);
        Serial.printf("[Demo] LOOKUP %s\n", item);
        distributedLookup(item);
        
        // Demo: Lookup random (might not exist)
        char randomItem[32];
        snprintf(randomItem, sizeof(randomItem), "random_%d", random() % 1000);
        Serial.printf("[Demo] LOOKUP %s\n", randomItem);
        distributedLookup(randomItem);
        
        Serial.println("====================================");
        Serial.println();
    }
    
    delay(10);
}
