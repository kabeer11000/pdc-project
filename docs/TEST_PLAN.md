# Test Plan - Distributed Cuckoo Filter System

## Milestone 1 Test Plan

**Date:** March 27, 2026  
**Team:** Ayesha, Hamza, Waiz, Kabeer  
**Status:** Ready for Testing

---

## 1. Test Environment Setup

### Prerequisites
- PlatformIO installed (`pip install platformio`)
- ESP32 platform v5.4.0
- Libraries: WebSockets v2.7.3, ArduinoJson v6.21.6
- Python 3.x with `websockets` package (for WebSocket testing)

### Test Tools
```bash
# Build project
pio run

# Run unit tests on ESP32
pio test -e esp32dev

# Open serial monitor
pio device monitor
```

---

## 2. Component Tests

### 2.1 Cuckoo Filter Tests (Hamza's Implementation)

**File:** `src/cuckoo_filter/cuckoo_filter.cpp`

| Test ID | Test Name | Description | Expected Result |
|---------|-----------|-------------|-----------------|
| CF-01 | Empty Lookup | Lookup on empty filter | Returns `false` |
| CF-02 | Insert + Lookup | Insert item, then lookup | Returns `true` |
| CF-03 | Delete Existing | Delete inserted item | Returns `true`, item not found after |
| CF-04 | Delete Non-existent | Delete never-inserted item | Returns `false` |
| CF-05 | Duplicate Insert | Insert same item twice | Both succeed, lookup returns `true` |
| CF-06 | False Positive Rate | Insert 500, query 500 new items | FP rate < 10% |
| CF-07 | Filter Full | Insert 2000 items | Some inserts fail gracefully |

**Run Command:**
```bash
pio run  # Build with test_cuckoo_filter.cpp
```

**Pass Criteria:**
- All basic operations (CF-01 to CF-05) pass
- False positive rate ≤ 10%
- Filter handles overflow gracefully

---

### 2.2 WebSocket Communication Tests (Kabeer's Implementation)

**Files:** 
- `src/websocket/websocket_client.cpp`
- `src/websocket/websocket_server.cpp`
- `src/main.cpp`

| Test ID | Test Name | Description | Expected Result |
|---------|-----------|-------------|-----------------|
| WS-01 | WiFi Connection | ESP32 connects to WokWi-GUEST | Serial shows IP address |
| WS-02 | WebSocket Connect | ESP32 connects to test server | "Connected" message |
| WS-03 | Send INSERT | Send INSERT message | Server receives JSON |
| WS-04 | Send LOOKUP | Send LOOKUP message | Server receives JSON |
| WS-05 | Send DELETE | Send DELETE message | Server receives JSON |
| WS-06 | Receive Response | Server sends RESPONSE | ESP32 processes callback |
| WS-07 | Reconnect | Disconnect server, restart | ESP32 auto-reconnects |

**Run Command:**
```bash
# Terminal 1: Start test server
python test_websocket_server.py

# Terminal 2: Build and run ESP32
pio run
# Use WokWi simulator or upload to hardware
```

**Pass Criteria:**
- ESP32 connects to WiFi within 10 seconds
- WebSocket connection established
- Messages sent and received correctly
- Auto-reconnect works after disconnect

---

### 2.3 WiFi Connectivity Tests (Waiz's Implementation)

**File:** `src/main.cpp`

| Test ID | Test Name | Description | Expected Result |
|---------|-----------|-------------|-----------------|
| WF-01 | WokWi Connection | Connect to WokWi-GUEST | IP address displayed |
| WF-02 | Connection Retry | WiFi unavailable initially | Retries until connected |

**Pass Criteria:**
- WiFi connects successfully in WokWi simulator
- IP address is valid (not 0.0.0.0)

---

### 2.4 Integration Tests

**Scenario:** Full system communication flow

| Test ID | Test Name | Description | Expected Result |
|---------|-----------|-------------|-----------------|
| INT-01 | Insert Flow | Master→Slave INSERT | Cuckoo filter contains item |
| INT-02 | Lookup Flow | Master→Slave LOOKUP | Correct response returned |
| INT-03 | Delete Flow | Master→Slave DELETE | Item removed from filter |
| INT-04 | Multi-Node | Multiple slaves connected | Each handles own messages |

**Run Command:**
```bash
# Requires master node implementation
# See: src/master/master_skeleton.cpp
```

---

## 3. Test Execution Checklist

### Pre-Test Verification
- [ ] PlatformIO build succeeds without errors
- [ ] No compilation warnings in critical code
- [ ] Git status clean (except build artifacts)

### Test Execution
- [ ] CF-01 to CF-07: Cuckoo filter tests pass
- [ ] WS-01 to WS-07: WebSocket tests pass
- [ ] WF-01 to WF-02: WiFi tests pass
- [ ] INT-01 to INT-04: Integration tests pass (when master ready)

### Post-Test Verification
- [ ] Memory usage < 80% of available RAM
- [ ] Flash usage < 80% of available flash
- [ ] No memory leaks observed
- [ ] Serial output is clean and informative

---

## 4. Known Issues & Limitations

1. **Master Node Not Implemented:** `master_skeleton.cpp` is a placeholder
2. **Response Sending:** WebSocket client receives but doesn't send responses back
3. **WokWi Limitations:** Virtual WiFi may behave differently from real hardware

---

## 5. Test Results Template

```
=== Cuckoo Filter Unit Tests ===

--- Running Cuckoo Filter Tests ---
test_cuckoo_empty_lookup          PASS
test_cuckoo_insert_and_lookup     PASS
test_cuckoo_delete_existing       PASS
test_cuckoo_delete_nonexistent    PASS
test_cuckoo_duplicate_insert      PASS
test_cuckoo_false_positive_rate   PASS (FP Rate: X.XX%)
test_cuckoo_filter_full           PASS (Fails: X/2000)

=== All Tests Complete ===
```

---

## 6. Definition of Done (Milestone 1)

- [x] Cuckoo filter working on single node (Hamza)
- [x] Basic WebSocket communication working (Kabeer)
- [x] ESP32 WiFi connectivity working (Waiz)
- [ ] Master node can send request and receive response (Ayesha - pending)
- [x] Results documented (this file)

---

## 7. How to Run All Tests

### Quick Test (Build Only)
```bash
cd C:\Users\System Administrator\Documents\Software\pdc-project
pio run
```

### Cuckoo Filter Unit Tests
```bash
# Replace main.cpp with test_cuckoo_filter.cpp temporarily
# Or create separate test environment
pio test -e esp32dev
```

### WebSocket Integration Test
```bash
# Terminal 1
python test_websocket_server.py

# Terminal 2 - Use WokWi Simulator
# F1 > Wokwi: Start Simulator
```

---

**Test Plan Approved By:** _________________  
**Date:** _________________
