# Wokwi Simulator & PlatformIO Setup Guide

This project relies on **Wokwi** for testing ESP32 code without physical hardware and **PlatformIO** for compiling the C++ code within VS Code.

## Required VS Code Extensions
Please install the following extensions before starting:
1. **PlatformIO IDE** (by PlatformIO) - Handles the C++ build process. (Wait for it to fully install and prompt you to reload VS Code).
2. **Wokwi Simulator** (by Wokwi) - Simulates the hardware. *Note: You will need to request a free license via the VS Code command palette `Wokwi: Request a New License` if this is your first time using it.*

---

## Daily Workflow: How to Build and Simulate

### Step 1: Compile the Code
Wokwi cannot run loose C++ files; they must be built into a binary (`.bin`) file first.
1. Look at the bottom blue/purple status bar in VS Code.
2. Click the tiny **Checkmark (✓)** icon (PlatformIO: Build).
3. Wait for the terminal to display a green **`SUCCESS`** message.

*(Command line alternative: Open a PlatformIO Terminal and run `pio run`)*

### Step 2: Start the Wokwi Simulator
Once the code is successfully compiled:
1. Press **F1** (or `Ctrl+Shift+P` / `Cmd+Shift+P`) to open the Command Palette.
2. Type **`Wokwi: Start Simulator`** and press Enter.
3. The simulator will open in a split pane. You can see the ESP32 console output (like the WiFi connection status) in the VS Code terminal.

---

## Important Project Configuration Files

- **`platformio.ini`**: Tells PlatformIO how to build the project. 
  - **Important Note for Ayesha/Master Node:** Right now, the `build_src_filter` is set to only compile `src/main.cpp` (which contains Waiz's slave WiFi code) so that it doesn't clash with `master_skeleton.cpp`. To compile the master node instead, you will need to update this filter!
- **`wokwi.toml`**: Tells the Wokwi extension exactly where to find PlatformIO's compiled output (inside the hidden `.pio/build/` folder).
- **`diagram.json`**: This is Wokwi's "hardware layout" file. It currently just generates a standard ESP32 board.
- **`src/main.cpp`**: Contains the baseline ESP32 Slave Node logic (WiFi connection tests for Milestone 1). Kabeer and Hamza will add the WebSocket and Cuckoo filter logic here.