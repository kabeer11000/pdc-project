#!/usr/bin/env python3
"""
ESP32 Slave Simulator - Simulates the ESP32 slave node for testing
This lets us test the master/test-server without needing hardware or WokWi
"""

import asyncio
import json
import random

try:
    import websockets
except ImportError:
    print("Error: websockets library not installed!")
    print("Install with: pip install websockets")
    exit(1)

# Configuration
MASTER_HOST = "localhost"
MASTER_PORT = 81

# Simulated Cuckoo Filter (simple set for testing)
cuckoo_filter = set()

def filter_insert(item):
    """Simulate cuckoo filter insert"""
    cuckoo_filter.add(item)
    return True

def filter_lookup(item):
    """Simulate cuckoo filter lookup"""
    return item in cuckoo_filter

def filter_delete(item):
    """Simulate cuckoo filter delete"""
    if item in cuckoo_filter:
        cuckoo_filter.remove(item)
        return True
    return False

async def connect_to_master():
    """Connect to master node and process commands"""
    uri = f"ws://{MASTER_HOST}:{MASTER_PORT}/ws"
    
    print(f"[Slave] Connecting to {uri}...")
    
    try:
        async with websockets.connect(uri) as websocket:
            print(f"[Slave] ✓ Connected to master!")
            print(f"[Slave] Waiting for commands...")
            print()
            
            async for message in websocket:
                print(f"[Slave] Received: {message}")
                
                try:
                    data = json.loads(message)
                    msg_type = data.get("type", "UNKNOWN")
                    value = data.get("value", "")
                    
                    # Process command
                    if msg_type == "INSERT":
                        result = filter_insert(value)
                        print(f"[Slave] [Cuckoo] INSERT {value} -> {'OK' if result else 'FAILED'}")
                        
                    elif msg_type == "LOOKUP":
                        result = filter_lookup(value)
                        print(f"[Slave] [Cuckoo] LOOKUP {value} -> {'FOUND' if result else 'NOT FOUND'}")
                        
                    elif msg_type == "DELETE":
                        result = filter_delete(value)
                        print(f"[Slave] [Cuckoo] DELETE {value} -> {'OK' if result else 'NOT FOUND'}")
                        
                    else:
                        print(f"[Slave] Unknown command: {msg_type}")
                        continue
                    
                    # Send response
                    response = {
                        "type": "RESPONSE",
                        "value": value,
                        "result": result
                    }
                    await websocket.send(json.dumps(response))
                    print(f"[Slave] Sent RESPONSE: {json.dumps(response)}")
                    print()
                    
                except json.JSONDecodeError:
                    print(f"[Slave] Invalid JSON received")
                except Exception as e:
                    print(f"[Slave] Error: {e}")
                    
    except ConnectionRefusedError:
        print(f"[Slave] ✗ Connection refused - is the master server running?")
        print(f"[Slave] Make sure test_system.py is running!")
    except Exception as e:
        print(f"[Slave] ✗ Error: {e}")

async def main():
    print("=" * 60)
    print("  ESP32 SLAVE SIMULATOR")
    print("  Testing WebSocket communication with master")
    print("=" * 60)
    print()
    
    # Try to reconnect if disconnected
    while True:
        await connect_to_master()
        print()
        print("[Slave] Disconnected. Reconnecting in 5 seconds...")
        await asyncio.sleep(5)

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\n\n[Slave] Stopped by user")
