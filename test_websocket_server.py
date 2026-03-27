#!/usr/bin/env python3
"""
Test WebSocket Server for ESP32 Distributed Cuckoo Filter
Run this on your PC to test WebSocket communication with the ESP32 slave node.

Usage:
    pip install websockets asyncio
    python test_websocket_server.py
"""

import asyncio
import json
from datetime import datetime

try:
    import websockets
except ImportError:
    print("Error: websockets library not installed!")
    print("Install with: pip install websockets")
    exit(1)

# WebSocket server configuration
HOST = "0.0.0.0"  # Listen on all interfaces
PORT = 81

# Connected clients
connected_clients = set()


def log_message(message):
    """Print timestamped log message"""
    timestamp = datetime.now().strftime("%H:%M:%S")
    print(f"[{timestamp}] {message}")


async def handle_client(websocket, path):
    """Handle a WebSocket client connection"""
    connected_clients.add(websocket)
    client_ip = websocket.remote_address[0]
    log_message(f"Client connected: {client_ip}")
    
    try:
        async for message in websocket:
            log_message(f"Received: {message}")
            
            try:
                # Parse JSON message
                data = json.loads(message)
                msg_type = data.get("type", "UNKNOWN")
                value = data.get("value", "")
                node_id = data.get("nodeId", 0)
                
                log_message(f"  Type: {msg_type}, Value: {value}, NodeId: {node_id}")
                
                # Process message based on type
                if msg_type == "INSERT":
                    log_message(f"  -> Processing INSERT for '{value}'")
                    # Simulate processing (in real code, this would call cuckoo filter)
                    response = {
                        "type": "RESPONSE",
                        "value": value,
                        "result": True
                    }
                    await websocket.send(json.dumps(response))
                    log_message(f"  -> Sent RESPONSE: success")
                    
                elif msg_type == "LOOKUP":
                    log_message(f"  -> Processing LOOKUP for '{value}'")
                    # Simulate lookup (random for testing)
                    import random
                    found = random.choice([True, False])
                    response = {
                        "type": "RESPONSE",
                        "value": value,
                        "result": found
                    }
                    await websocket.send(json.dumps(response))
                    log_message(f"  -> Sent RESPONSE: {'FOUND' if found else 'NOT FOUND'}")
                    
                elif msg_type == "DELETE":
                    log_message(f"  -> Processing DELETE for '{value}'")
                    response = {
                        "type": "RESPONSE",
                        "value": value,
                        "result": True
                    }
                    await websocket.send(json.dumps(response))
                    log_message(f"  -> Sent RESPONSE: deleted")
                    
                elif msg_type == "RESPONSE":
                    result = data.get("result", False)
                    log_message(f"  -> Received RESPONSE: {value} = {result}")
                    
                else:
                    log_message(f"  -> Unknown message type: {msg_type}")
                    
            except json.JSONDecodeError:
                log_message(f"  -> Invalid JSON received")
            except Exception as e:
                log_message(f"  -> Error processing message: {e}")
                
    except websockets.exceptions.ConnectionClosed:
        log_message(f"Client connection closed")
    finally:
        connected_clients.remove(websocket)
        log_message(f"Client disconnected: {client_ip}")


async def main():
    """Main server function"""
    log_message("=" * 50)
    log_message("WebSocket Test Server for ESP32 Cuckoo Filter")
    log_message("=" * 50)
    log_message(f"Listening on ws://{HOST}:{PORT}")
    log_message(f"Local IPs: Check with 'ipconfig' (Windows) or 'ifconfig' (Linux/Mac)")
    log_message("")
    log_message("Waiting for ESP32 connection...")
    log_message("")
    
    # Start WebSocket server
    async with websockets.serve(handle_client, HOST, PORT):
        await asyncio.Future()  # Run forever


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        log_message("\nServer stopped by user")
    except OSError as e:
        log_message(f"Error: Could not start server on port {PORT}")
        log_message(f"Make sure no other program is using port {PORT}")
        log_message(f"Details: {e}")
