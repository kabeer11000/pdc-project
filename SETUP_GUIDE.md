# ESP32 WebSocket Setup Guide

## Setup Completed ✓

The ESP32 toolchain and WebSocket implementation have been set up successfully.

---

## What Was Installed

1. **PlatformIO** - ESP32 development framework
2. **ESP32 Platform** (v5.4.0) - Espressif32 platform
3. **Libraries**:
   - WebSockets v2.7.3 (links2004)
   - ArduinoJson v6.21.6 (bblanchon)

---

## Project Structure

```
src/
├── main.cpp                          # Main slave node code
├── cuckoo_filter/
│   ├── cuckoo_filter.h               # Cuckoo filter header
│   └── cuckoo_filter.cpp             # Cuckoo filter implementation
├── websocket/
│   ├── websocket_client.h            # WebSocket client header
│   ├── websocket_client.cpp          # WebSocket client implementation
│   ├── websocket_server.h            # WebSocket server header (for master)
│   └── websocket_server.cpp          # WebSocket server implementation
└── master/
    └── master_skeleton.cpp           # Master node skeleton (not built)
```

---

## How to Build

```bash
# Build the project
python -m platformio run

# Build and upload to ESP32
python -m platformio run --target upload

# Open serial monitor
python -m platformio device monitor
```

---

## How to Run in Wokwi Simulator

1. Install the **Wokwi Simulator** VS Code extension
2. Press `F1` → "Wokwi: Start Simulator"
3. The ESP32 will boot and connect to "Wokwi-GUEST" WiFi

---

## WebSocket Configuration

### For Wokwi Simulation

The slave node connects to a WebSocket server. You need to:

1. **Run a WebSocket server** on your PC (see `test_websocket_server.py`)
2. **Find your PC's IP address**:
   - Windows: `ipconfig` in cmd
   - Look for IPv4 Address (e.g., `192.168.1.100`)
3. **Update `src/main.cpp`**:
   ```cpp
   const char* WS_HOST = "YOUR_PC_IP_HERE";  // e.g., "192.168.1.100"
   ```

### Message Format

All messages use JSON:

```json
// INSERT message
{"type": "INSERT", "value": "test_item"}

// LOOKUP message
{"type": "LOOKUP", "value": "test_item"}

// DELETE message
{"type": "DELETE", "value": "test_item"}

// RESPONSE message
{"type": "RESPONSE", "value": "test_item", "result": true}
```

---

## Testing with Python WebSocket Server

Run the test server:

```bash
pip install websockets asyncio
python test_websocket_server.py
```

Then build and run the ESP32 code. You should see:
- ESP32 connects to WiFi
- ESP32 connects to WebSocket server
- Messages appear in both serial monitor and Python server console

---

## Your Role (Kabeer - WebSocket Communication)

### Completed:
- ✓ WebSocket client library integrated
- ✓ WebSocket server library available for master node
- ✓ JSON message format implemented
- ✓ Message types: INSERT, DELETE, LOOKUP, RESPONSE

### Next Steps:
1. **Test WebSocket connectivity** using the Python test server
2. **Implement master node** WebSocket server using `websocket_server.h/cpp`
3. **Add response sending** to WebSocket client (currently receives but doesn't send responses back)
4. **Integrate with cuckoo filter** operations

---

## Serial Monitor Output (Expected)

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

## Troubleshooting

### Build Errors
- Run `python -m platformio run --target clean` then rebuild

### WiFi Connection Fails
- Ensure Wokwi is using the correct WiFi network name
- Check `ssid` in `main.cpp`

### WebSocket Connection Fails
- Verify PC IP address is correct
- Ensure firewall allows incoming connections on port 81
- Make sure WebSocket server is running before ESP32 tries to connect

---

## Files Created for You

1. `src/websocket/websocket_client.h` - Client header
2. `src/websocket/websocket_client.cpp` - Client implementation
3. `src/websocket/websocket_server.h` - Server header (for master)
4. `src/websocket/websocket_server.cpp` - Server implementation (for master)
5. `src/main.cpp` - Updated with WebSocket client + Cuckoo filter
6. `test_websocket_server.py` - Python test server (run on PC)

---

Good luck with your WebSocket implementation! 🚀
