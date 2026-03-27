# Complete Development Guide - Distributed Cuckoo Filter

**For:** Kabeer and Team  
**Last Updated:** March 27, 2026  
**Status:** ✅ Ready for Development

---

## 🚀 Quick Start (Choose Your Method)

### Method 1: WokWi Simulator (Easiest - No Hardware Needed)

**Time:** 5 minutes

```bash
# 1. Install Python test server dependencies
pip install websockets asyncio

# 2. Open VS Code with the project

# 3. Install these VS Code extensions:
#    - PlatformIO IDE
#    - Wokwi Simulator

# 4. Start the test server (simulates Master node)
python test_system.py

# 5. In VS Code, press F1 and type:
#    "Wokwi: Start Simulator"

# 6. Watch both terminals for communication!
```

**Expected Output:**
```
TEST SERVER:
[18:30:00] SLAVE 1 connected from 192.168.1.50
[18:30:05] TEST 1: Master sends INSERT command
[18:30:05] Sent: {"type":"INSERT","value":"test_item_1"}
[18:30:06] PASS: Received valid RESPONSE (result=true) ✓

SLAVE (WokWi):
[WebSocket] Connected
[Cuckoo] INSERT test_item_1 -> OK
[WebSocket] Sent RESPONSE: OK -> test_item_1 (result=1)
```

---

### Method 2: Two ESP32 Hardware Boards

**Time:** 15 minutes

**What You Need:**
- 2x ESP32 DevKit v1 boards
- 2x USB cables
- WiFi network (2.4GHz)

**Steps:**

```bash
# 1. Update WiFi credentials in both files:

# src/main.cpp (Slave):
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

# src/master/main_master.cpp (Master):
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

# 2. Build and upload SLAVE to first ESP32:
python -m platformio run -e esp32dev --target upload

# 3. Build and upload MASTER to second ESP32:
python -m platformio run -e esp32dev_master --target upload

# 4. Open two serial monitors:
python -m platformio device monitor --baud 115200
# (Find correct COM ports for each board)
```

---

### Method 3: One ESP32 + Python Test Server

**Time:** 10 minutes

**Best for:** Testing slave node only

```bash
# 1. Find your PC's IP address:
ipconfig  # Windows (look for IPv4 Address)
ifconfig  # Linux/Mac

# 2. Update src/main.cpp line 11:
const char* WS_HOST = "192.168.1.100";  # Your PC's IP

# 3. Run Python test server:
python test_system.py

# 4. Build and upload slave:
python -m platformio run -e esp32dev --target upload

# 5. Open serial monitor:
python -m platformio device monitor --baud 115200
```

---

## 📁 Project Structure

```
pdc-project/
├── src/
│   ├── main.cpp                    # SLAVE node (connects to master)
│   ├── master/
│   │   └── main_master.cpp         # MASTER node (coordinates slaves)
│   ├── cuckoo_filter/
│   │   ├── cuckoo_filter.h         # Filter header
│   │   └── cuckoo_filter.cpp       # Hamza's filter implementation
│   └── websocket/
│       ├── websocket_client.h/cpp  # WebSocket client (for slaves)
│       └── websocket_server.h/cpp  # WebSocket server (for master)
├── tests/
│   └── test_cuckoo_filter.cpp      # Unit tests for filter
├── docs/
│   ├── README_SETUP.md             # Setup guide
│   ├── FINAL_VERIFICATION.md       # Communication verification
│   └── TECHNICAL_BREAKDOWN.md      # Architecture docs
├── test_system.py                  # Automated test server
├── test_websocket_server.py        # Simple test server
└── platformio.ini                  # Build configuration
```

---

## 🔨 Build Commands

### Build Everything
```bash
python -m platformio run
```

### Build Specific Nodes
```bash
# Slave node
python -m platformio run -e esp32dev

# Master node
python -m platformio run -e esp32dev_master
```

### Upload to Hardware
```bash
# Slave
python -m platformio run -e esp32dev --target upload

# Master
python -m platformio run -e esp32dev_master --target upload
```

### Serial Monitor
```bash
python -m platformio device monitor --baud 115200
```

### Clean Build
```bash
python -m platformio run --target clean
python -m platformio run
```

---

## 🧪 Testing

### Run Automated Tests
```bash
# Start test server
python test_system.py

# The test server will:
# 1. Wait for slave to connect
# 2. Send INSERT commands
# 3. Send LOOKUP commands
# 4. Send DELETE commands
# 5. Validate responses
# 6. Show pass/fail results
```

### Expected Test Output
```
[TEST] ==================================================
[TEST] WAITING FOR SLAVES TO CONNECT...
[TEST] ✓ Slave connected!
[TEST] ==================================================
[TEST] STARTING TEST SEQUENCE
[TEST] --------------------------------------------------
[TEST] TEST 1: Master sends INSERT command
[TEST] Sent: {"type":"INSERT","value":"test_item_1"}
[TEST] Parsed: type=RESPONSE, value=test_item_1, result=True
[TEST] PASS: Received valid RESPONSE (result=True)
[TEST] --------------------------------------------------
[TEST] TEST 2: Master sends LOOKUP command
[TEST] PASS: Received valid RESPONSE (result=True)
[TEST] --------------------------------------------------
[TEST] TEST SUMMARY
[TEST] Passed: 7
[TEST] Failed: 0
[TEST] ✓ ALL TESTS PASSED!
```

---

## 💡 How It Works

### System Architecture

```
┌─────────────────────────────────────────────────────────┐
│                  COMPLETE SYSTEM                        │
└─────────────────────────────────────────────────────────┘

┌──────────────┐         WebSocket:81        ┌──────────────┐
│   MASTER     │ ◄─────────────────────────► │   SLAVE 1    │
│   (ESP32)    │        JSON Messages        │   (ESP32)    │
│              │                             │              │
│ - WS Server  │                             │ - WS Client  │
│ - Coordinator│                             │ - Cuckoo     │
│ - Filter     │                             │   Filter     │
└──────────────┘                             └──────────────┘
       ▲                                            ▲
       │                                            │
       │  Can add more slaves...                    │
       │                                            │
┌──────────────┐                             ┌──────────────┐
│   SLAVE 2    │                             │   SLAVE 3    │
│   (ESP32)    │                             │   (ESP32)    │
└──────────────┘                             └──────────────┘
```

### Message Flow

```
MASTER                              SLAVE
  │                                   │
  │──INSERT {"type":"INSERT", ───────►│
  │     "value":"test",               │
  │     "nodeId":0}                   │
  │                                   │ Receives & parses
  │                                   │ Calls filterInsert()
  │                                   │
  │◄──RESPONSE {"type":"RESPONSE", ──│
  │     "value":"test",               │
  │     "result":true}                │ Sends with result
  │                                   │
  │ Receives & returns true           │
  │                                   │
```

---

## 🛠️ How to Develop Further

### Feature Ideas

#### 1. Add Multiple Slave Support ⭐⭐⭐

**Current:** Master can connect to multiple slaves but doesn't properly distribute load.

**To Implement:**
```cpp
// In master node, improve getSlaveForItem():
uint8_t getSlaveForItem(const char* item) {
    // Better distribution algorithm
    // Option 1: Hash-based (current)
    // Option 2: Round-robin
    // Option 3: Load-based (check slave capacity)
}
```

**Difficulty:** Medium  
**Impact:** High

---

#### 2. Add Slave Health Monitoring ⭐⭐⭐

**Current:** No way to detect if slave disconnects.

**To Implement:**
```cpp
// Add heartbeat mechanism
void checkSlaveHealth() {
    for (each slave) {
        if (no_message_for_30_seconds) {
            mark_slave_as_offline();
            redistribute_items();
        }
    }
}
```

**Difficulty:** Medium  
**Impact:** High

---

#### 3. Add Data Persistence ⭐⭐

**Current:** Filter data lost on power cycle.

**To Implement:**
```cpp
// Use ESP32's SPIFFS or EEPROM
#include <SPIFFS.h>

void saveFilterToFile() {
    File f = SPIFFS.open("/filter.dat", "w");
    f.write((uint8_t*)&filter, sizeof(filter));
    f.close();
}

void loadFilterFromFile() {
    File f = SPIFFS.open("/filter.dat", "r");
    f.read((uint8_t*)&filter, sizeof(filter));
    f.close();
}
```

**Difficulty:** Easy  
**Impact:** Medium

---

#### 4. Add Web Interface ⭐⭐

**Current:** Only serial monitor for debugging.

**To Implement:**
```cpp
// Add web server to master node
#include <WebServer.h>

WebServer server(80);

void setup() {
    server.on("/insert", HTTP_GET, []( ){
        String item = server.arg("item");
        distributedInsert(item.c_str());
        server.send(200, "text/plain", "OK");
    });
    server.begin();
}
```

**Access via browser:** `http://192.168.1.100/insert?item=test`

**Difficulty:** Easy  
**Impact:** Medium

---

#### 5. Add Authentication ⭐⭐

**Current:** Any client can connect.

**To Implement:**
```cpp
// Add token-based auth in WebSocket handshake
void onWebSocketConnect(uint8_t num) {
    String token = getAuthToken(num);
    if (!validateToken(token)) {
        webSocket.disconnect(num);
    }
}
```

**Difficulty:** Medium  
**Impact:** Medium

---

#### 6. Add Statistics & Monitoring ⭐

**Current:** No metrics.

**To Implement:**
```cpp
struct FilterStats {
    uint32_t total_inserts;
    uint32_t total_lookups;
    uint32_t total_deletes;
    uint32_t false_positives;
    float load_factor;
};

FilterStats stats;

// Send to master periodically
void sendStats() {
    StaticJsonDocument<256> doc;
    doc["inserts"] = stats.total_inserts;
    doc["load"] = stats.load_factor;
    // ... send via WebSocket
}
```

**Difficulty:** Easy  
**Impact:** Low

---

#### 7. Add Sync Mechanism ⭐⭐⭐

**Current:** Slaves operate independently.

**To Implement:**
```cpp
// Periodic sync between slaves
void syncFilters() {
    // Every 60 seconds:
    // 1. Master requests filter state from all slaves
    // 2. Compares for inconsistencies
    // 3. Reconciles differences
}
```

**Difficulty:** Hard  
**Impact:** High

---

#### 8. Add Bloom Filter Alternative ⭐

**Current:** Only Cuckoo filter.

**To Implement:**
```cpp
// Add bloom_filter/ directory
// Compare performance:
// - Cuckoo: Supports delete, slightly more memory
// - Bloom: No delete, less memory, higher FP rate
```

**Difficulty:** Medium  
**Impact:** Low

---

### Code Improvement Ideas

#### 1. Add Error Handling

```cpp
// Current: No error recovery
bool distributedInsert(const char* item) {
    // ...
    if (!responseReceived) {
        return false;  // Just returns false
    }
}

// Improved: Retry logic
bool distributedInsert(const char* item) {
    const int MAX_RETRIES = 3;
    for (int i = 0; i < MAX_RETRIES; i++) {
        if (tryInsert(item)) {
            return true;
        }
        delay(100 * i);  // Exponential backoff
    }
    return false;
}
```

---

#### 2. Add Logging System

```cpp
// Create logger.h
enum LogLevel { DEBUG, INFO, WARN, ERROR };

void log(LogLevel level, const char* message) {
    if (level >= currentLogLevel) {
        Serial.printf("[%s] %s\n", levelToString(level), message);
        // Also send to master for centralized logging
    }
}
```

---

#### 3. Add Configuration System

```cpp
// Create config.h
struct Config {
    char wifi_ssid[32];
    char wifi_password[32];
    char master_ip[16];
    uint16_t master_port = 81;
    uint8_t node_id;
};

// Load from EEPROM/SPIFFS
Config loadConfig();
void saveConfig(Config& cfg);
```

---

## 📊 Performance Optimization

### Current Performance
```
Slave:  RAM 14.0%, Flash 67.6%
Master: RAM 14.1%, Flash 58.5%
```

### Optimization Ideas

1. **Reduce JSON overhead:**
   - Use MessagePack instead of JSON
   - Or use binary protocol

2. **Optimize Cuckoo Filter:**
   - Adjust BUCKET_COUNT for memory/speed tradeoff
   - Use SIMD instructions if available

3. **Reduce WiFi power:**
   ```cpp
   WiFi.setSleepMode(WIFI_NONE_SLEEP);  // Faster but more power
   WiFi.setSleepMode(WIFI_MODEM_SLEEP); // Slower but less power
   ```

---

## 🐛 Debugging Tips

### Enable Debug Output
```cpp
// Add to top of main.cpp
#define DEBUG true

#if DEBUG
    #define DEBUG_PRINT(x) Serial.println(x)
#else
    #define DEBUG_PRINT(x)
#endif
```

### Common Issues

| Problem | Solution |
|---------|----------|
| WiFi won't connect | Check SSID/password, ensure 2.4GHz network |
| WebSocket timeout | Check firewall, verify IP address |
| Build fails | Run `pio run --target clean` |
| Serial shows garbage | Check baud rate (115200) |

---

## 📚 Learning Resources

### For Understanding the Code
1. `docs/FINAL_VERIFICATION.md` - Complete communication flow
2. `docs/TECHNICAL_BREAKDOWN.md` - Architecture details
3. `docs/COMMUNICATION_TEST_REPORT.md` - Test results

### For ESP32 Development
- [ESP32 Official Docs](https://docs.espressif.com/projects/esp-idf/en/latest/)
- [PlatformIO ESP32 Guide](https://docs.platformio.org/en/latest/platforms/espressif32.html)

### For WebSocket
- [RFC 6455 - WebSocket Protocol](https://tools.ietf.org/html/rfc6455)
- [WebSockets Library Docs](https://github.com/Links2004/arduinoWebSockets)

### For Cuckoo Filters
- [Original Cuckoo Filter Paper](https://www.cs.cmu.edu/~dga/papers/cuckoo-conext2014.pdf)

---

## 🎯 Next Milestones

### Milestone 2 Suggestions

1. **Multi-slave load balancing** (Distribute items evenly)
2. **Slave failure recovery** (Detect and handle offline slaves)
3. **Web dashboard** (Monitor and control system)
4. **Performance benchmarks** (Measure throughput, latency)
5. **Data persistence** (Save filter to flash)

---

## 📞 Team Collaboration

### Git Workflow
```bash
# Before starting work
git pull origin main

# Create feature branch
git checkout -b feature/web-interface

# Make changes, then:
git add .
git commit -m "Add web interface for master node"
git push origin feature/web-interface

# Create pull request on GitHub
```

### Code Review Checklist
- [ ] Does it compile without warnings?
- [ ] Are there unit tests?
- [ ] Is documentation updated?
- [ ] Does it follow existing code style?
- [ ] Are there any security issues?

---

## ✅ Quick Reference

### Most Common Commands
```bash
# Build and upload
pio run -e esp32dev --target upload

# Open serial monitor
pio device monitor

# Run tests
python test_system.py

# Check git status
git status

# Pull latest changes
git pull origin main
```

### Pin Reference (ESP32)
```
GPIO 1: TX0 (Serial)
GPIO 3: RX0 (Serial)
GPIO 2: Built-in LED
GPIO 12-15: HSPI
GPIO 18-23: VSPI
```

---

**Happy Coding! 🚀**

For questions, check `docs/` folder or ask the team.
