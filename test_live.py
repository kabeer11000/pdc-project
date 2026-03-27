#!/usr/bin/env python3
"""
Live Test - Connects to test server and verifies communication
"""

import asyncio
import json
import sys

try:
    import websockets
except ImportError:
    print("Error: websockets library not installed!")
    sys.exit(1)

# Configuration
SERVER_HOST = "localhost"
SERVER_PORT = 81

async def test_communication():
    """Test communication with the server"""
    uri = f"ws://{SERVER_HOST}:{SERVER_PORT}/ws"
    
    print("=" * 60)
    print("  LIVE COMMUNICATION TEST")
    print("=" * 60)
    print()
    print(f"Connecting to ws://{SERVER_HOST}:{SERVER_PORT}/ws...")
    
    try:
        async with websockets.connect(uri) as websocket:
            print("✓ Connected to test server!")
            print()
            
            # Test 1: Send INSERT
            print("TEST 1: Sending INSERT command...")
            insert_msg = {"type": "INSERT", "value": "live_test_item"}
            await websocket.send(json.dumps(insert_msg))
            print(f"  Sent: {json.dumps(insert_msg)}")
            
            # Wait for response
            try:
                response = await asyncio.wait_for(websocket.recv(), timeout=5.0)
                print(f"  Received: {response}")
                data = json.loads(response)
                if data.get("type") == "RESPONSE" and data.get("result") == True:
                    print("  ✓ PASS: INSERT response received correctly")
                else:
                    print("  ✗ FAIL: Unexpected response")
            except asyncio.TimeoutError:
                print("  ✗ FAIL: Timeout waiting for response")
            
            print()
            
            # Test 2: Send LOOKUP
            print("TEST 2: Sending LOOKUP command...")
            lookup_msg = {"type": "LOOKUP", "value": "live_test_item"}
            await websocket.send(json.dumps(lookup_msg))
            print(f"  Sent: {json.dumps(lookup_msg)}")
            
            # Wait for response
            try:
                response = await asyncio.wait_for(websocket.recv(), timeout=5.0)
                print(f"  Received: {response}")
                data = json.loads(response)
                if data.get("type") == "RESPONSE":
                    print(f"  ✓ PASS: LOOKUP response received (result={data.get('result')})")
                else:
                    print("  ✗ FAIL: Unexpected response")
            except asyncio.TimeoutError:
                print("  ✗ FAIL: Timeout waiting for response")
            
            print()
            
            # Test 3: Send DELETE
            print("TEST 3: Sending DELETE command...")
            delete_msg = {"type": "DELETE", "value": "live_test_item"}
            await websocket.send(json.dumps(delete_msg))
            print(f"  Sent: {json.dumps(delete_msg)}")
            
            # Wait for response
            try:
                response = await asyncio.wait_for(websocket.recv(), timeout=5.0)
                print(f"  Received: {response}")
                data = json.loads(response)
                if data.get("type") == "RESPONSE":
                    print(f"  ✓ PASS: DELETE response received (result={data.get('result')})")
                else:
                    print("  ✗ FAIL: Unexpected response")
            except asyncio.TimeoutError:
                print("  ✗ FAIL: Timeout waiting for response")
            
            print()
            print("=" * 60)
            print("  LIVE TEST COMPLETE")
            print("=" * 60)
            
    except ConnectionRefusedError:
        print("✗ Connection refused!")
        print()
        print("Make sure:")
        print("  1. test_system.py is running")
        print("  2. Port 81 is not blocked by firewall")
        print()
        print("To start test server:")
        print("  python test_system.py")
    except Exception as e:
        print(f"✗ Error: {e}")

if __name__ == "__main__":
    asyncio.run(test_communication())
