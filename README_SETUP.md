# Complete Setup & Run Guide - Distributed Cuckoo Filter

**Last Updated:** March 27, 2026  
**Status:** ✅ Fully Functional

---

## Quick Start (2 Minutes)

### What You Need:
1. **VS Code** with PlatformIO extension
2. **Wokwi Simulator** extension (for testing without hardware)
3. **Python 3.x** (for test server)

### Run in WokWi Simulator:

```bash
# 1. Install Python dependencies
pip install websockets asyncio

# 2. Start the test server (simulates master node)
python test_websocket_server.py

# 3. In VS Code:
#    - Press F1
#    - Select "Wokwi: Start Simulator"
#    - ESP32 will boot and connect automatically
```

---

## System Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                    COMPLETE SYSTEM                              │
└─────────────────────────────────────────────────────────────────┘

┌──────────────┐         WebSocket (Port 81)        ┌──────────────┐
│  MASTER NODE │ ◄─────────────────────────────────► │  SLAVE NODE  │
│   (ESP32)    │         JSON Messages              │   (ESP32)    │
│              │                                    │              │
│ - WS Server  │                                    │ - WS Client  │
│ - Coordinator│                                    │ - Cuckoo     │
│ - Cuckoo     │                                    │   Filter     │
│   Filter     │                                    │              │
└──────────────┘                                    └──────────────┘
       ▲                                                   ▲
       │                                                   │
       │  Can connect multiple slaves to one master        │
       │                                                   │
┌──────────────┐                                    ┌──────────────┐
│  SLAVE NODE  │                                    │  SLAVE NODE  │
│   (ESP32)    │                                    │   (ESP32)    │
└──────────────┘                                    └──────────────┘
```

---

## Build Commands

### Build Slave Node (Default)
```bash
python -m platformio run -e esp32dev
```

### Build Master Node
```bash
python -m platformio run -e esp32dev_master
```

### Build Both
```bash
python -m platformio run
```

### Upload to Hardware ESP32
```bash
# Slave
python -m platformio run -e esp32dev --target upload

# Master
python -m platformio run -e esp32dev_master --target upload
```

### Open Serial Monitor
```bash
python -m platformio device monitor --baud 115200
```

---

## Testing Options

### Option 1: WokWi Simulator (Recommended for Development)

**Best for:** Quick testing without hardware

1. **Install WokWi Extension** in VS Code
2. **Run test server:**
   ```bash
   python test_websocket_server.py
   ```
3. **Start simulator:** F1 → "Wokwi: Start Simulator"
4. **Watch serial output** in VS Code terminal

**Expected Output:**
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
```

---

### Option 2: Two ESP32 Hardware Boards

**Best for:** Real-world testing

**What You Need:**
- 2x ESP32 DevKit v1
- USB cables
- Same WiFi network

**Setup:**

1. **Find your PC's IP** (for test server):
   ```bash
   ipconfig  # Windows
   ifconfig  # Linux/Mac
   ```

2. **Update slave's WiFi config** (`src/main.cpp`):
   ```cpp
   const char* ssid = "YOUR_WIFI_SSID";
   const char* password = "YOUR_WIFI_PASSWORD";
   ```

3. **Upload slave to first ESP32:**
   ```bash
   python -m platformio run -e esp32dev --target upload
   ```

4. **Upload master to second ESP32:**
   ```bash
   python -m platformio run -e esp32dev_master --target upload
   ```

5. **Open two serial monitors:**
   ```bash
   # Terminal 1 - Master
   python -m platformio device monitor --baud 115200

   # Terminal 2 - Slave (need to find correct port)
   python -m platformio device monitor --baud 115200 --port COMX
   ```

---

### Option 3: Python Test Server + ESP32

**Best for:** Testing slave node with simulated master

1. **Run Python server:**
   ```bash
   python test_websocket_server.py
   ```

2. **Update slave's WebSocket host** (`src/main.cpp` line 11):
   ```cpp
   const char* WS_HOST = "192.168.1.100";  // Your PC's IP
   ```

3. **Build and run slave** (WokWi or hardware)

4. **Watch both consoles:**
   - Python server shows received messages
   - ESP32 serial shows processed commands

---

## Message Flow Example

```
┌──────────────┐                          ┌──────────────┐
│  MASTER      │                          │  SLAVE       │
│  (or Python) │                          │  (ESP32)     │
└──────┬───────┘                          └──────┬───────┘
       │                                         │
       │  {"type":"INSERT","value":"test_1"}    │
       │────────────────────────────────────────►│
       │                                         │
       │                              Receives message
       │                              Parses JSON
       │                              Calls filterInsert()
       │                              Logs: "INSERT test_1 -> OK"
       │                                         │
       │  {"type":"RESPONSE",                    │
       │   "value":"test_1",                     │
       │   "result":true}                        │
       │◄────────────────────────────────────────│
       │                                         │
       │  Logs: "Slave confirmed INSERT"         │
       │                                         │
       │                                         │
       │  {"type":"LOOKUP","value":"test_1"}     │
       │────────────────────────────────────────►│
       │                                         │
       │                              Calls filterLookup()
       │                              Logs: "LOOKUP test_1 -> FOUND"
       │                                         │
       │  {"type":"RESPONSE",                    │
       │   "value":"test_1",                     │
       │   "result":true}                        │
       │◄────────────────────────────────────────│
       │                                         │
```

---

## Project Structure

```
pdc-project/
├── src/
│   ├── main.cpp                        # Slave node entry point
│   ├── master/
│   │   └── main_master.cpp             # Master node entry point
│   ├── cuckoo_filter/
│   │   ├── cuckoo_filter.h             # Filter header
│   │   └── cuckoo_filter.cpp           # Filter implementation
│   └── websocket/
│       ├── websocket_client.h          # WebSocket client (slave)
│       ├── websocket_client.cpp
│       ├── websocket_server.h          # WebSocket server (master)
│       └── websocket_server.cpp
├── tests/
│   └── test_cuckoo_filter.cpp          # Unit tests
├── docs/
│   ├── TEST_PLAN.md                    # Test plan
│   └── TECHNICAL_BREAKDOWN.md          # Architecture docs
├── platformio.ini                      # Build configuration
├── test_websocket_server.py            # Python test server
└── SETUP_GUIDE.md                      # This file
```

---

## Configuration

### platformio.ini Environments

| Environment | Purpose | Build Command |
|-------------|---------|---------------|
| `esp32dev` | Slave node (default) | `pio run -e esp32dev` |
| `esp32dev_master` | Master node | `pio run -e esp32dev_master` |

### Memory Usage

| Node | Flash | RAM |
|------|-------|-----|
| Slave | 67.6% (886 KB) | 14.0% (46 KB) |
| Master | 58.4% (765 KB) | 14.1% (46 KB) |

---

## Troubleshooting

### Build Fails

```bash
# Clean and rebuild
python -m platformio run --target clean
python -m platformio run
```

### WiFi Connection Fails

1. Check SSID and password in `src/main.cpp`
2. For WokWi, use `"Wokwi-GUEST"` with empty password
3. Ensure WiFi router is on 2.4GHz (ESP32 doesn't support 5GHz)

### WebSocket Connection Fails

1. **Check IP address** in slave's `src/main.cpp`:
   ```cpp
   const char* WS_HOST = "192.168.1.100";  // Must be master/PC IP
   ```

2. **Check firewall** - allow port 81:
   ```bash
   # Windows (run as admin)
   netsh advfirewall firewall add rule name="WebSocket" dir=in action=allow protocol=TCP localport=81
   ```

3. **Verify server is running** before ESP32 boots

### Serial Monitor Shows Garbage

1. Check baud rate: must be `115200`
2. Check correct COM port
3. Try resetting ESP32

---

## Testing Checklist

### Slave Node Tests

- [ ] WiFi connects successfully
- [ ] IP address is displayed
- [ ] WebSocket connects to server
- [ ] Receives INSERT command
- [ ] Processes INSERT (cuckoo filter)
- [ ] Sends RESPONSE back
- [ ] Receives LOOKUP command
- [ ] Sends correct lookup result
- [ ] Receives DELETE command
- [ ] Auto-reconnects after disconnect

### Master Node Tests

- [ ] WiFi connects successfully
- [ ] WebSocket server starts on port 81
- [ ] Accepts slave connections
- [ ] Receives responses from slaves
- [ ] Demo operations run every 5 seconds

### Integration Tests

- [ ] Master sends INSERT → Slave processes → Master receives response
- [ ] Master sends LOOKUP → Slave checks → Master receives result
- [ ] Multiple slaves can connect simultaneously
- [ ] System handles slave disconnect gracefully

---

## Next Steps

### For Testing:
1. Run Python test server
2. Start WokWi simulator
3. Watch communication in both consoles

### For Hardware:
1. Upload master to one ESP32
2. Upload slave to another ESP32
3. Open serial monitors for both
4. Watch them communicate

### For Development:
1. Read `docs/TECHNICAL_BREAKDOWN.md` for architecture
2. Read `docs/TEST_PLAN.md` for test cases
3. Modify `src/main.cpp` for custom behavior

---

## Quick Reference

### Build All
```bash
python -m platformio run
```

### Run Tests
```bash
# Cuckoo filter unit tests
pio test -e esp32dev
```

### Serial Monitor
```bash
python -m platformio device monitor --baud 115200
```

### Test Server
```bash
python test_websocket_server.py
```

---

**Questions?** Check `docs/TECHNICAL_BREAKDOWN.md` for detailed architecture.
