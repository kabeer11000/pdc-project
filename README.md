# Distributed Cuckoo Filter System

## 📌 Project Overview

This project implements a distributed cuckoo filter system using ESP32 nodes and WebSocket communication. The system consists of a master node coordinating multiple slave nodes, each maintaining a portion of the cuckoo filter.

---

## 🎯 Milestone 1 Goal

Implement a **single-node cuckoo filter** and establish **basic WebSocket communication** between nodes.

---

## 👥 Team Responsibilities

### Ayesha (Team Lead)

* Master node design
* Integration of all components
* Test setup and documentation
* Performance evaluation (false positives, logs)

### Hamza

* Cuckoo Filter implementation in C++
* Insert, delete, lookup operations
* Hashing and kick-out logic

### Waiz

* ESP32 hardware setup
* WiFi connectivity and testing
* Slave node support

### Kabeer

* WebSocket communication setup
* Message sending/receiving
* Protocol/message format design

---

## 🗂️ Repository Structure

```
/cuckoo-filter-project
│
├── README.md
├── docs/
│   └── milestone1.md
│
├── src/
│   ├── cuckoo_filter/
│   ├── websocket/
│   ├── master/
│   └── slave/
│
├── hardware/
│   └── esp32_setup/
│
├── tests/
│   └── test_cases.cpp
│
└── results/
```

---

## 🛠️ Technical Stack

* Language: C++
* Hardware: ESP32
* Communication: WebSockets

---

## 📅 Milestone 1 Timeline (Deadline: 26 March)

### Phase 1 (March 18–19)

* Hamza: Basic cuckoo filter (insert + lookup)
* Kabeer: Simple WebSocket send/receive
* Waiz: ESP32 setup + WiFi connectivity
* Ayesha: Repository setup + master node skeleton

### Phase 2 (March 20–22)

* Integration begins
* Master node sends requests
* Slave nodes respond

### Phase 3 (March 23–24)

* Testing and debugging
* Validate correctness

### Phase 4 (March 25)

* Documentation
* Final cleanup

### Final Submission (March 26)

---

## 📡 Communication Protocol (Initial Plan)

### Message Format (JSON-like)

```
{
  "type": "INSERT",
  "value": 123
}
```

Types:

* INSERT
* LOOKUP
* DELETE
* RESPONSE

---

## 🧪 Testing Plan

* Unit testing for cuckoo filter
* Communication testing (message send/receive)
* Measure false positive rate

---

## 📌 Rules

* All work must be pushed to GitHub daily
* Use clear commit messages
* No last-day submissions

---

## 🚀 Immediate Tasks

### Ayesha

* Set up repository
* Create master node skeleton

### Hamza

* Implement insert() and lookup()

### Kabeer

* Send a test WebSocket message between two nodes

### Waiz

* Connect ESP32 to WiFi and verify connectivity

---

## 📞 Meeting Plan

* Daily 10-minute check-ins
* Progress updates required from each member

---

## ✅ Definition of Done (Milestone 1)

* Cuckoo filter working on single node
* Basic WebSocket communication working
* Master can send request and receive response
* Results documented

---

## 📊 Future Work

* Distributed cuckoo filter
* Partitioning across nodes
* Performance optimization

---

**Team Lead:** Ayesha Kashif
