#pragma once
#include <Arduino.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

#define WEBSOCKET_SERVER_PORT 81

// Message types
enum MessageType {
    MSG_INSERT,
    MSG_DELETE,
    MSG_LOOKUP,
    MSG_RESPONSE
};

// Callback function type for received messages
typedef void (*MessageCallback)(uint8_t num, MessageType type, const char* value);

class WebSocketServer {
private:
    WebSocketsServer webSocket;
    MessageCallback callback;
    
    void handleEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
    
public:
    WebSocketServer(uint16_t port = WEBSOCKET_SERVER_PORT);
    
    void begin();
    void loop();
    
    // Send to specific client
    bool sendResponse(uint8_t num, MessageType type, const char* value, bool result);
    bool broadcast(const char* message);
    
    // Set callback for received messages
    void onMessage(MessageCallback cb);
    
    // Get connected client count
    uint8_t getConnectedCount();
};

// Helper function to create JSON message
String createMessage(MessageType type, const char* value, bool result = false);
