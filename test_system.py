#!/usr/bin/env python3
"""
Comprehensive Test Script for Distributed Cuckoo Filter System
Tests master-slave communication protocol

Usage:
    python test_system.py
"""

import asyncio
import json
import time
from datetime import datetime

try:
    import websockets
except ImportError:
    print("Error: websockets library not installed!")
    print("Install with: pip install websockets")
    exit(1)

# Configuration
HOST = "0.0.0.0"
PORT = 81

# Test state
connected_clients = {}
test_results = {
    "passed": 0,
    "failed": 0,
    "tests": []
}


def log(message, level="INFO"):
    """Print timestamped log message"""
    timestamp = datetime.now().strftime("%H:%M:%S.%f")[:-3]
    print(f"[{timestamp}] [{level}] {message}")


def log_master(message):
    log(message, "MASTER")


def log_slave(message):
    log(message, "SLAVE")


def log_test(message):
    log(message, "TEST")


async def handle_slave(websocket, path):
    """Handle a slave node connection"""
    client_id = len(connected_clients) + 1
    connected_clients[client_id] = {
        "websocket": websocket,
        "type": "slave",
        "connected_at": time.time()
    }
    
    log_slave(f"Slave {client_id} connected from {websocket.remote_address[0]}")
    log_master(f"Total connected slaves: {len(connected_clients)}")
    
    try:
        async for message in websocket:
            log_slave(f"Slave {client_id} → {message}")
            
            try:
                data = json.loads(message)
                msg_type = data.get("type", "UNKNOWN")
                value = data.get("value", "")
                result = data.get("result", None)
                node_id = data.get("nodeId", 0)
                
                log_test(f"Parsed: type={msg_type}, value={value}, result={result}, nodeId={node_id}")
                
                # Validate message format
                if msg_type not in ["INSERT", "DELETE", "LOOKUP", "RESPONSE"]:
                    log_test(f"FAIL: Unknown message type: {msg_type}", "ERROR")
                    test_results["failed"] += 1
                    continue
                
                # Check required fields
                if not value:
                    log_test(f"FAIL: Missing 'value' field", "ERROR")
                    test_results["failed"] += 1
                    continue
                
                # Process based on type
                if msg_type == "RESPONSE":
                    # Slave is responding to our command
                    if result is None:
                        log_test(f"FAIL: RESPONSE missing 'result' field", "ERROR")
                        test_results["failed"] += 1
                    else:
                        log_test(f"PASS: Received valid RESPONSE (result={result})")
                        test_results["passed"] += 1
                else:
                    # Slave sent a command (unexpected but handle it)
                    log_test(f"INFO: Slave sent {msg_type} command (unexpected)")
                    
                    # Send acknowledgment
                    response = {
                        "type": "RESPONSE",
                        "value": value,
                        "result": True
                    }
                    await websocket.send(json.dumps(response))
                    log_master(f"Sent acknowledgment: {json.dumps(response)}")
                    
            except json.JSONDecodeError as e:
                log_test(f"FAIL: Invalid JSON: {e}", "ERROR")
                test_results["failed"] += 1
            except Exception as e:
                log_test(f"FAIL: Error processing message: {e}", "ERROR")
                test_results["failed"] += 1
                
    except websockets.exceptions.ConnectionClosed:
        log_slave(f"Slave {client_id} disconnected")
    finally:
        del connected_clients[client_id]
        log_master(f"Total connected slaves: {len(connected_clients)}")


async def run_test_sequence():
    """Run automated test sequence after slaves connect"""
    log_test("=" * 60)
    log_test("WAITING FOR SLAVES TO CONNECT...")
    log_test("=" * 60)
    
    # Wait for at least one slave
    while len(connected_clients) == 0:
        await asyncio.sleep(0.5)
    
    log_test(f"✓ Slave connected! Waiting 2 seconds for stability...")
    await asyncio.sleep(2)
    
    log_test("=" * 60)
    log_test("STARTING TEST SEQUENCE")
    log_test("=" * 60)
    
    # Get first slave
    slave_id = list(connected_clients.keys())[0]
    slave_ws = connected_clients[slave_id]["websocket"]
    
    # Test 1: Send INSERT command
    log_test("-" * 40)
    log_test("TEST 1: Master sends INSERT command")
    insert_cmd = {"type": "INSERT", "value": "test_item_1", "nodeId": 0}
    await slave_ws.send(json.dumps(insert_cmd))
    log_master(f"Sent: {json.dumps(insert_cmd)}")
    await asyncio.sleep(1)
    
    # Test 2: Send LOOKUP command
    log_test("-" * 40)
    log_test("TEST 2: Master sends LOOKUP command")
    lookup_cmd = {"type": "LOOKUP", "value": "test_item_1", "nodeId": 0}
    await slave_ws.send(json.dumps(lookup_cmd))
    log_master(f"Sent: {json.dumps(lookup_cmd)}")
    await asyncio.sleep(1)
    
    # Test 3: Send DELETE command
    log_test("-" * 40)
    log_test("TEST 3: Master sends DELETE command")
    delete_cmd = {"type": "DELETE", "value": "test_item_1", "nodeId": 0}
    await slave_ws.send(json.dumps(delete_cmd))
    log_master(f"Sent: {json.dumps(delete_cmd)}")
    await asyncio.sleep(1)
    
    # Test 4: Send LOOKUP for deleted item
    log_test("-" * 40)
    log_test("TEST 4: Master sends LOOKUP for deleted item")
    lookup_cmd2 = {"type": "LOOKUP", "value": "test_item_1", "nodeId": 0}
    await slave_ws.send(json.dumps(lookup_cmd2))
    log_master(f"Sent: {json.dumps(lookup_cmd2)}")
    await asyncio.sleep(1)
    
    # Test 5: Multiple inserts
    log_test("-" * 40)
    log_test("TEST 5: Multiple INSERT commands")
    for i in range(5):
        cmd = {"type": "INSERT", "value": f"item_{i}", "nodeId": 0}
        await slave_ws.send(json.dumps(cmd))
        log_master(f"Sent: {json.dumps(cmd)}")
        await asyncio.sleep(0.5)
    
    await asyncio.sleep(2)
    
    # Test 6: Invalid JSON (should be handled gracefully)
    log_test("-" * 40)
    log_test("TEST 6: Invalid JSON handling")
    await slave_ws.send("not valid json {{{")
    log_master("Sent: invalid JSON")
    await asyncio.sleep(1)
    
    # Test 7: Missing fields
    log_test("-" * 40)
    log_test("TEST 7: Missing 'value' field")
    await slave_ws.send(json.dumps({"type": "INSERT"}))
    log_master("Sent: {\"type\": \"INSERT\"} (missing value)")
    await asyncio.sleep(1)
    
    # Summary
    log_test("=" * 60)
    log_test("TEST SUMMARY")
    log_test("=" * 60)
    log_test(f"Passed: {test_results['passed']}")
    log_test(f"Failed: {test_results['failed']}")
    log_test(f"Total:  {test_results['passed'] + test_results['failed']}")
    
    if test_results['failed'] == 0:
        log_test("✓ ALL TESTS PASSED!", "SUCCESS")
    else:
        log_test(f"✗ {test_results['failed']} TESTS FAILED", "ERROR")
    
    log_test("=" * 60)


async def main():
    """Main server function"""
    print()
    log_master("=" * 60)
    log_master("DISTRIBUTED CUCKOO FILTER - TEST SERVER")
    log_master("=" * 60)
    log_master(f"Listening on ws://{HOST}:{PORT}")
    log_master("")
    log_master("PROTOCOL SPECIFICATION:")
    log_master("  Message Format: JSON")
    log_master("  Required Fields: type, value")
    log_master("  Optional Fields: result, nodeId")
    log_master("  Types: INSERT, LOOKUP, DELETE, RESPONSE")
    log_master("")
    log_master("EXPECTED MESSAGE FLOW:")
    log_master("  1. Slave connects to server")
    log_master("  2. Master sends: {\"type\":\"INSERT\",\"value\":\"test\"}")
    log_master("  3. Slave processes and responds:")
    log_master("     {\"type\":\"RESPONSE\",\"value\":\"test\",\"result\":true}")
    log_master("=" * 60)
    log_master("")
    log_master("Waiting for ESP32 slave nodes...")
    log_master("(Press Ctrl+C to stop)")
    log_master("")
    
    # Start WebSocket server
    server = await websockets.serve(handle_slave, HOST, PORT)
    
    # Schedule test sequence
    asyncio.create_task(run_test_sequence())
    
    # Run forever
    await asyncio.Future()


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print()
        log_master("Server stopped by user")
        
        # Print final summary
        print()
        log_master("=" * 60)
        log_master("FINAL SUMMARY")
        log_master("=" * 60)
        log_master(f"Tests Passed: {test_results['passed']}")
        log_master(f"Tests Failed: {test_results['failed']}")
        log_master("=" * 60)
    except OSError as e:
        log_master(f"Error: Could not start server on port {PORT}", "ERROR")
        log_master(f"Make sure no other program is using port {PORT}", "ERROR")
        log_master(f"Details: {e}", "ERROR")
