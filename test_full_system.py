#!/usr/bin/env python3
"""
Full System Test - Runs master simulator and slave simulator in one process
This lets us see all communication in one console
"""

import asyncio
import json
import time
from datetime import datetime

try:
    import websockets
except ImportError:
    print("Error: websockets library not installed!")
    exit(1)

# Configuration
HOST = "localhost"
PORT = 81

# State
connected_slaves = {}
test_results = {"passed": 0, "failed": 0}


def log(source, message):
    timestamp = datetime.now().strftime("%H:%M:%S.%f")[:-3]
    print(f"[{timestamp}] [{source}] {message}")


# ============== MASTER SIMULATOR ==============
async def handle_slave(websocket):
    """Handle slave connections (simulates master)"""
    slave_id = len(connected_slaves) + 1
    connected_slaves[slave_id] = websocket
    
    log("MASTER", f"✓ Slave {slave_id} connected!")
    
    try:
        async for message in websocket:
            log("SLAVE→MASTER", message)
            
            data = json.loads(message)
            msg_type = data.get("type", "UNKNOWN")
            value = data.get("value", "")
            result = data.get("result", None)
            
            if msg_type == "RESPONSE":
                log("MASTER", f"Received RESPONSE: {value} = {result}")
            else:
                log("MASTER", f"Received {msg_type} for {value} (result={result})")
                # Send acknowledgment
                response = {"type": "RESPONSE", "value": value, "result": True}
                await websocket.send(json.dumps(response))
                log("MASTER→SLAVE", json.dumps(response))
                
    except websockets.exceptions.ConnectionClosed:
        log("MASTER", f"Slave {slave_id} disconnected")
        del connected_slaves[slave_id]


async def run_master_server():
    """Run master server"""
    server = await websockets.serve(handle_slave, HOST, PORT)
    log("MASTER", f"Server started on ws://{HOST}:{PORT}")
    return server


# ============== SLAVE SIMULATOR ==============
async def run_slave_client():
    """Run slave client"""
    uri = f"ws://{HOST}:{PORT}/ws"
    
    await asyncio.sleep(1)  # Wait for server to start
    
    log("SLAVE", f"Connecting to {uri}...")
    
    try:
        async with websockets.connect(uri) as websocket:
            log("SLAVE", "✓ Connected to master!")
            
            # Simulated cuckoo filter
            cuckoo_filter = set()
            
            # Test sequence
            test_items = [
                ("INSERT", "test_item_1"),
                ("LOOKUP", "test_item_1"),
                ("INSERT", "test_item_2"),
                ("LOOKUP", "test_item_2"),
                ("LOOKUP", "nonexistent"),
                ("DELETE", "test_item_1"),
                ("LOOKUP", "test_item_1"),
            ]
            
            for msg_type, value in test_items:
                # Send command
                msg = {"type": msg_type, "value": value}
                await websocket.send(json.dumps(msg))
                log("SLAVE→MASTER", json.dumps(msg))
                
                # Process locally (simulate cuckoo filter)
                if msg_type == "INSERT":
                    cuckoo_filter.add(value)
                    result = True
                    log("CUCKOO", f"INSERT {value} -> OK")
                elif msg_type == "LOOKUP":
                    result = value in cuckoo_filter
                    log("CUCKOO", f"LOOKUP {value} -> {'FOUND' if result else 'NOT FOUND'}")
                elif msg_type == "DELETE":
                    if value in cuckoo_filter:
                        cuckoo_filter.remove(value)
                        result = True
                        log("CUCKOO", f"DELETE {value} -> OK")
                    else:
                        result = False
                        log("CUCKOO", f"DELETE {value} -> NOT FOUND")
                
                # Wait for response
                try:
                    response = await asyncio.wait_for(websocket.recv(), timeout=2.0)
                    log("MASTER→SLAVE", response)
                    
                    resp_data = json.loads(response)
                    if resp_data.get("type") == "RESPONSE":
                        log("TEST", f"✓ PASS: Got response for {msg_type} {value}")
                        test_results["passed"] += 1
                    else:
                        log("TEST", f"✗ FAIL: Unexpected response type")
                        test_results["failed"] += 1
                        
                except asyncio.TimeoutError:
                    log("TEST", f"✗ FAIL: Timeout waiting for response")
                    test_results["failed"] += 1
                
                await asyncio.sleep(0.5)
            
            # Final summary
            print()
            log("TEST", "=" * 50)
            log("TEST", f"RESULTS: {test_results['passed']} passed, {test_results['failed']} failed")
            log("TEST", "=" * 50)
            
    except ConnectionRefusedError:
        log("SLAVE", "✗ Connection refused - is master running?")
    except Exception as e:
        log("SLAVE", f"✗ Error: {e}")


# ============== MAIN ==============
async def main():
    print()
    print("=" * 60)
    print("  FULL SYSTEM TEST - Master + Slave Communication")
    print("=" * 60)
    print()
    
    # Start master server
    await run_master_server()
    
    # Run slave client
    await run_slave_client()
    
    # Keep server running for a bit
    await asyncio.sleep(2)
    
    print()
    log("SYSTEM", "Test complete!")


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\n\nTest stopped by user")
    except Exception as e:
        print(f"\n\nError: {e}")
        import traceback
        traceback.print_exc()
