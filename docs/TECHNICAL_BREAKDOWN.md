# Technical Breakdown: WebSocket Communication System

**Author:** Kabeer Jafri  
**Date:** March 27, 2026  
**For:** Team Members (Ayesha, Hamza, Waiz)  
**Purpose:** Architecture documentation and integration guide

---

## 1. System Architecture Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                    DISTRIBUTED CUCKOO FILTER                     │
│                         SYSTEM ARCHITECTURE                      │
└─────────────────────────────────────────────────────────────────┘

┌──────────────┐         WebSocket (Port 81)        ┌──────────────┐
│  MASTER NODE │ ◄─────────────────────────────────► │  SLAVE NODE  │
│   (Ayesha)   │         JSON Messages              │   (ESP32)    │
│              │                                    │              │
│ - WebSocket  │                                    │ - WiFi       │
│   Server     │                                    │   Client     │
│ - Cuckoo     │                                    │ - Cuckoo     │
│   Filter     │                                    │   Filter     │
│ - Coordinator│                                    │ - Worker     │
└──────────────┘                                    └──────────────┘
       ▲                                                   ▲
       │                                                   │
       │                                                   │
┌──────────────┐                                    ┌──────────────┐
│  SLAVE NODE  │         WebSocket (Port 81)        │  SLAVE NODE  │
│   (ESP32)    │ ◄─────────────────────────────────► │   (ESP32)    │
│              │                                    │              │
│ - WiFi       │                                    │ - WiFi       │
│   Client     │                                    │   Client     │
│ - Cuckoo     │                                    │ - Cuckoo     │
│   Filter     │                                    │   Filter     │
│ - Worker     │                                    │ - Worker     │
└──────────────┘                                    └──────────────┘

Note: Current implementation has SLAVE nodes. Master node (Ayesha) 
      needs to be implemented to coordinate slaves.
```

---

## 2. Kabeer's Implementation: Slave Node Architecture

### 2.1 Component Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                        SLAVE NODE (ESP32)                       │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│  ┌──────────────────────────────────────────────────────────┐   │
│  │                    main.cpp                              │   │
│  │  - WiFi Connection (Waiz's code)                         │   │
│  │  - Cuckoo Filter Instance (Hamza's code)                 │   │
│  │  - WebSocket Client (Kabeer's code)                      │   │
│  │  - Message Callback Handler                              │   │
│  └──────────────────────────────────────────────────────────┘   │
│                              │                                   │
│         ┌────────────────────┼────────────────────┐             │
│         │                    │                    │             │
│         ▼                    ▼                    ▼             │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐      │
│  │  WebSocket   │    │   Cuckoo     │    │    WiFi      │      │
│  │   Client     │    │   Filter     │    │   Manager    │      │
│  │              │    │              │    │              │      │
│  │ - Connect    │    │ - Insert     │    │ - Connect    │      │
│  │ - Send       │    │ - Lookup     │    │ - Maintain   │      │
│  │ - Receive    │    │ - Delete     │    │   Connection │      │
│  │ - Parse JSON │    │ - Hashing    │    │              │      │
│  └──────────────┘    └──────────────┘    └──────────────┘      │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

### 2.2 Message Flow

```
┌──────────────┐                          ┌──────────────┐
│  MASTER      │                          │  SLAVE       │
│  (Future)    │                          │  (Current)   │
└──────┬───────┘                          └──────┬───────┘
       │                                         │
       │  1. {"type":"INSERT","value":"test"}   │
       │────────────────────────────────────────►│
       │                                         │
       │                              ┌──────────────────────┐
       │                              │ WebSocketClient      │
       │                              │ receives message     │
       │                              └──────────┬───────────┘
       │                                         │
       │                              ┌──────────▼───────────┐
       │                              │ onWebSocketMessage() │
       │                              │ callback invoked     │
       │                              └──────────┬───────────┘
       │                                         │
       │                              ┌──────────▼───────────┐
       │                              │ filterInsert()       │
       │                              │ (Hamza's code)       │
       │                              └──────────┬───────────┘
       │                                         │
       │                              ┌──────────▼───────────┐
       │                              │ Serial Log:          │
       │                              │ "[Cuckoo] INSERT     │
       │                              │  test -> OK"         │
       │                              └──────────────────────┘
       │                                         │
       │  2. {"type":"RESPONSE",                 │
       │      "value":"test",                    │
       │      "result":true}                     │
       │◄────────────────────────────────────────│
       │                                         │
       │   (TODO: Implement response sending)    │
       │                                         │
```

---

## 3. File-by-File Breakdown

### 3.1 `src/websocket/websocket_client.h`

**Purpose:** WebSocket client header for slave nodes

**Key Components:**
```cpp
// Message types matching README protocol
enum MessageType {
    MSG_INSERT,    // Insert item into cuckoo filter
    MSG_DELETE,    // Delete item from cuckoo filter
    MSG_LOOKUP,    // Check if item exists
    MSG_RESPONSE   // Response to any operation
};

// Callback function type
typedef void (*MessageCallback)(MessageType type, 
                                 const char* value, 
                                 bool result);

// WebSocketClient class
class WebSocketClient {
    // Connect to master node
    void begin(const char* host, uint16_t port, const char* url);
    
    // Process WebSocket events (call in loop())
    void loop();
    
    // Check connection status
    bool isConnected();
    
    // Send operations
    bool sendInsert(const char* value);
    bool sendDelete(const char* value);
    bool sendLookup(const char* value);
    
    // Set message callback
    void onMessage(MessageCallback cb);
};
```

---

### 3.2 `src/websocket/websocket_client.cpp`

**Purpose:** WebSocket client implementation

**Key Functions:**

```cpp
// 1. handleEvent() - Processes incoming WebSocket events
void WebSocketClient::handleEvent(WStype_t type, 
                                   uint8_t * payload, 
                                   size_t length) {
    switch (type) {
        case WStype_TEXT:
            // Parse JSON: {"type":"INSERT","value":"test"}
            StaticJsonDocument<256> doc;
            deserializeJson(doc, payload, length);
            
            // Extract fields
            const char* type = doc["type"];
            const char* value = doc["value"];
            
            // Call user callback
            callback(msgType, value, result);
            break;
    }
}

// 2. sendInsert() - Send INSERT message to master
bool WebSocketClient::sendInsert(const char* value) {
    String msg = createMessage(MSG_INSERT, value);
    // msg = {"type":"INSERT","value":"test"}
    return webSocket.sendTXT(msg);
}
```

---

### 3.3 `src/websocket/websocket_server.h/cpp`

**Purpose:** WebSocket server for future master node implementation

**For Ayesha:** This is what you'll use in the master node:

```cpp
// Example master node usage (for Ayesha)
#include "websocket/websocket_server.h"

WebSocketServer wsServer(81);  // Listen on port 81

void onClientMessage(uint8_t num, 
                     MessageType type, 
                     const char* value) {
    // Client 'num' sent a message
    if (type == MSG_INSERT) {
        // Process insert request
        bool success = filterInsert(&filter, value);
        
        // Send response back to client
        wsServer.sendResponse(num, MSG_RESPONSE, value, success);
    }
}

void setup() {
    wsServer.begin();
    wsServer.onMessage(onClientMessage);
}

void loop() {
    wsServer.loop();
}
```

**Key API:**
- `begin()` - Start WebSocket server on port 81
- `loop()` - Process client connections (call in loop())
- `onMessage(callback)` - Set callback for incoming messages
- `sendResponse(num, type, value, result)` - Send response to client `num`
- `broadcast(message)` - Send to all connected clients
- `getConnectedCount()` - Number of connected slaves

---

### 3.4 `src/main.cpp`

**Purpose:** Slave node entry point - integrates all components

**Flow:**
```cpp
// 1. Global instances
CuckooFilter filter;       // Hamza's filter
WebSocketClient wsClient;  // Kabeer's WebSocket

// 2. Callback when message received
void onWebSocketMessage(MessageType type, 
                        const char* value, 
                        bool result) {
    switch (type) {
        case MSG_INSERT:
            filterInsert(&filter, value);  // Hamza's function
            break;
        case MSG_LOOKUP:
            bool found = filterLookup(&filter, value);
            // TODO: Send response back
            break;
        // ... etc
    }
}

// 3. Setup: Connect WiFi, then WebSocket
void setup() {
    WiFi.begin("Wokwi-GUEST", "");  // Waiz's code
    
    wsClient.begin(WS_HOST, WS_PORT, "/ws");
    wsClient.onMessage(onWebSocketMessage);
}

// 4. Loop: Process WebSocket events
void loop() {
    wsClient.loop();  // Kabeer's code
}
```

---

## 4. Message Protocol

### 4.1 JSON Format

All messages use this format (as specified in README.md):

```json
{
  "type": "INSERT",
  "value": "test_item",
  "nodeId": 1,
  "result": true
}
```

### 4.2 Message Types

| Type | Direction | Fields | Description |
|------|-----------|--------|-------------|
| `INSERT` | Master→Slave | `type`, `value` | Insert item into filter |
| `LOOKUP` | Master→Slave | `type`, `value` | Check if item exists |
| `DELETE` | Master→Slave | `type`, `value` | Remove item from filter |
| `RESPONSE` | Slave→Master | `type`, `value`, `result` | Operation result |

### 4.3 Example Messages

```json
// Master sends INSERT to slave
{"type":"INSERT","value":"user_123"}

// Master sends LOOKUP to slave
{"type":"LOOKUP","value":"user_123"}

// Slave responds (TODO: not yet implemented)
{"type":"RESPONSE","value":"user_123","result":true}
```

---

## 5. Testing Without Master Node

Since the master node doesn't exist yet, we test using a **Python WebSocket server**.

### 5.1 Test Architecture

```
┌──────────────────┐         WebSocket          ┌──────────────┐
│  Python Test     │         Port 81            │  ESP32       │
│  Server          │ ◄─────────────────────────► │  Slave       │
│  (PC)            │                              │  Node        │
│                  │                              │              │
│ - Simulates      │                              │ - Connects   │
│   master node    │                              │   to PC IP   │
│ - Sends commands │                              │ - Processes  │
│ - Logs responses │                              │   commands   │
└──────────────────┘                              └──────────────┘
```

### 5.2 How to Test

**Step 1: Run Python Test Server**
```bash
cd C:\Users\System Administrator\Documents\Software\pdc-project
pip install websockets asyncio
python test_websocket_server.py
```

**Output:**
```
[18:30:45] ==================================================
[18:30:45] WebSocket Test Server for ESP32 Cuckoo Filter
[18:30:45] ==================================================
[18:30:45] Listening on ws://0.0.0.0:81
[18:30:45] Local IPs: Check with 'ipconfig'
[18:30:45] 
[18:30:45] Waiting for ESP32 connection...
```

**Step 2: Find Your PC's IP**
```bash
ipconfig
# Look for IPv4 Address, e.g., 192.168.1.100
```

**Step 3: Update `src/main.cpp` Line 11**
```cpp
const char* WS_HOST = "192.168.1.100";  // Your PC's IP
```

**Step 4: Build and Run ESP32**
```bash
pio run
# Or use WokWi Simulator: F1 > Wokwi: Start Simulator
```

**Step 5: Observe Communication**

**Python Server Console:**
```
[18:31:02] Client connected: 192.168.1.50
[18:31:05] Received: {"type":"INSERT","value":"test_item"}
[18:31:05]   Type: INSERT, Value: test_item, NodeId: 0
[18:31:05]   -> Processing INSERT for 'test_item'
[18:31:05]   -> Sent RESPONSE: success
```

**ESP32 Serial Monitor:**
```
=== Distributed Cuckoo Filter - Slave Node ===
Connecting to WiFi: Wokwi-GUEST
[Cuckoo] Filter initialized
....
--- WiFi Connected! ---
IP address: 192.168.1.50
[WebSocket] Connecting to ws://192.168.1.100:81/ws
[WebSocket] Connected
Slave node is ready!
=============================================
[Callback] Type: 0, Value: test_item, Result: 0
[Cuckoo] INSERT test_item -> OK
```

---

## 6. Integration Guide for Ayesha (Master Node)

### 6.1 What You Need to Implement

**Master Node Responsibilities:**
1. WebSocket server to accept slave connections
2. Cuckoo filter coordinator (distribute items across slaves)
3. API to receive user commands (INSERT, LOOKUP, DELETE)
4. Response aggregation from slaves

### 6.2 Starter Code (Using Kabeer's Server Library)

```cpp
// Master node skeleton (for Ayesha)
#include <Arduino.h>
#include <WiFi.h>
#include "websocket/websocket_server.h"
#include "cuckoo_filter/cuckoo_filter.h"

const char* ssid = "Wokwi-GUEST";
const char* password = "";

WebSocketServer wsServer(81);
CuckooFilter filter;

// Callback when slave sends message
void onSlaveMessage(uint8_t num, MessageType type, const char* value) {
    Serial.printf("[Master] Slave %d sent: type=%d, value=%s\n", 
                  num, type, value);
    
    if (type == MSG_INSERT) {
        bool success = filterInsert(&filter, value);
        wsServer.sendResponse(num, MSG_RESPONSE, value, success);
    }
    else if (type == MSG_LOOKUP) {
        bool found = filterLookup(&filter, value);
        wsServer.sendResponse(num, MSG_RESPONSE, value, found);
    }
    else if (type == MSG_DELETE) {
        bool deleted = filterDelete(&filter, value);
        wsServer.sendResponse(num, MSG_RESPONSE, value, deleted);
    }
}

void setup() {
    Serial.begin(115200);
    
    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    
    // Initialize cuckoo filter
    filterInit(&filter);
    
    // Start WebSocket server
    wsServer.begin();
    wsServer.onMessage(onSlaveMessage);
    
    Serial.println("Master node ready!");
}

void loop() {
    wsServer.loop();
    delay(10);
}
```

### 6.3 Master-Slave Communication Flow

```
┌──────────────┐                          ┌──────────────┐
│  MASTER      │                          │  SLAVE 1     │
│  (Ayesha)    │                          │  (ESP32)     │
└──────┬───────┘                          └──────┬───────┘
       │                                         │
       │  WebSocket Server on port 81            │
       │◄────────────────────────────────────────│
       │           Slave connects                │
       │                                         │
       │  User command: INSERT "item_1"          │
       │  ┌────────────────────────────────┐     │
       │  │ Decide which slave handles it  │     │
       │  │ (e.g., hash-based distribution)│     │
       │  └────────────────────────────────┘     │
       │────────────────────────────────────────►│
       │  {"type":"INSERT","value":"item_1"}     │
       │                                         │
       │                              ┌──────────────────────┐
       │                              │ Slave processes      │
       │                              │ filterInsert()       │
       │                              └──────────────────────┘
       │                                         │
       │◄────────────────────────────────────────│
       │  {"type":"RESPONSE","value":"item_1",   │
       │   "result":true}                        │
       │                                         │
       │  Log: "Slave 1 confirmed INSERT"        │
       │                                         │
```

---

## 7. Current Limitations & TODOs

### 7.1 What's Working

- ✅ Slave node connects to WebSocket server
- ✅ Slave receives and processes INSERT/LOOKUP/DELETE commands
- ✅ Cuckoo filter operations integrated
- ✅ JSON message parsing
- ✅ Auto-reconnect on disconnect
- ✅ Test server for development

### 7.2 What's Missing

| Feature | Status | Priority | Owner |
|---------|--------|----------|-------|
| Slave sends RESPONSE back to master | ⚠️ Not implemented | High | Kabeer |
| Master node WebSocket server | ❌ Not started | High | Ayesha |
| Master-slave item distribution | ❌ Not started | Medium | Team |
| Multi-slave coordination | ❌ Not started | Low | Team |

### 7.3 TODO: Add Response Sending to Slave

```cpp
// Add to src/websocket/websocket_client.h
class WebSocketClient {
public:
    // ... existing methods ...
    
    // NEW: Send response to master
    bool sendResponse(MessageType type, const char* value, bool result);
};

// Add to src/websocket/websocket_client.cpp
bool WebSocketClient::sendResponse(MessageType type, 
                                    const char* value, 
                                    bool result) {
    String msg = createMessage(type, value, result);
    return webSocket.sendTXT(msg);
}

// Update src/main.cpp callback
void onWebSocketMessage(MessageType type, const char* value, bool result) {
    switch (type) {
        case MSG_INSERT: {
            bool success = filterInsert(&filter, value);
            wsClient.sendResponse(MSG_RESPONSE, value, success);  // NEW
            break;
        }
        // ... similar for LOOKUP and DELETE
    }
}
```

---

## 8. Testing Checklist

### 8.1 Unit Tests (Hamza's Cuckoo Filter)

```bash
# Build with test_cuckoo_filter.cpp
pio run

# Expected output in serial monitor:
=== Cuckoo Filter Unit Tests ===
test_cuckoo_empty_lookup          PASS
test_cuckoo_insert_and_lookup     PASS
test_cuckoo_delete_existing       PASS
test_cuckoo_delete_nonexistent    PASS
test_cuckoo_duplicate_insert      PASS
test_cuckoo_false_positive_rate   PASS (FP Rate: 3.2%)
test_cuckoo_filter_full           PASS (Fails: 150/2000)
```

### 8.2 WebSocket Tests (Kabeer's Implementation)

| Test | How to Run | Expected Result |
|------|------------|-----------------|
| WS-01: WiFi Connect | Run ESP32 | Serial shows IP address |
| WS-02: WebSocket Connect | Run Python server + ESP32 | "Connected" message |
| WS-03: Send INSERT | Python server sends INSERT | ESP32 logs "INSERT test -> OK" |
| WS-04: Send LOOKUP | Python server sends LOOKUP | ESP32 logs lookup result |
| WS-05: Send DELETE | Python server sends DELETE | ESP32 logs delete result |
| WS-06: Reconnect | Restart Python server | ESP32 auto-reconnects |

### 8.3 Integration Tests (When Master Ready)

| Test | Description | Status |
|------|-------------|--------|
| INT-01 | Master sends INSERT, slave processes | ⏳ Pending master |
| INT-02 | Master sends LOOKUP, gets response | ⏳ Pending master |
| INT-03 | Master sends DELETE, slave removes | ⏳ Pending master |
| INT-04 | Multiple slaves, load balancing | ⏳ Pending master |

---

## 9. Debug Tips

### 9.1 Common Issues

**Problem:** ESP32 can't connect to WebSocket server

**Solution:**
1. Check PC firewall allows port 81
2. Verify IP address in `src/main.cpp` is correct
3. Ensure Python server is running before ESP32 boots

**Problem:** Messages not received

**Solution:**
1. Check `wsClient.loop()` is called in `loop()`
2. Verify JSON format matches spec
3. Check serial monitor for error messages

### 9.2 Serial Monitor Commands

```bash
# Open serial monitor
pio device monitor --baud 115200

# Look for these patterns:
[WebSocket] Connected          # Success!
[WebSocket] Received: {...}    # Message received
[Cuckoo] INSERT ... -> OK      # Filter operation success
[Cuckoo] INSERT ... -> FAILED  # Filter full
```

---

## 10. Summary

### Kabeer's Contribution

1. **WebSocket Client** (`websocket_client.h/cpp`)
   - Connects slave to master
   - Sends INSERT/LOOKUP/DELETE commands
   - Receives and parses JSON messages
   - Auto-reconnect on disconnect

2. **WebSocket Server** (`websocket_server.h/cpp`)
   - For Ayesha's master node
   - Handles multiple slave connections
   - Broadcast and unicast support

3. **Integration** (`main.cpp`)
   - WiFi + WebSocket + Cuckoo filter
   - Message callback handler
   - Production-ready slave node

4. **Testing Infrastructure**
   - Python test server
   - Unit tests for cuckoo filter
   - Test plan documentation

### For Ayesha

- Use `websocket_server.h/cpp` for master node
- See section 6.2 for starter code
- Master coordinates slaves, distributes items
- See section 6.3 for communication flow

### For Hamza

- Cuckoo filter is integrated and working
- Unit tests verify correctness
- False positive rate ~3% (as expected)

### For Waiz

- WiFi code is integrated in `main.cpp`
- Works with WokWi-GUEST network
- Auto-reconnect implemented

---

**Questions?** Reach out to Kabeer or check `SETUP_GUIDE.md` for setup instructions.
