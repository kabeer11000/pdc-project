#pragma once
#include <Arduino.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>

// WebSocket configuration
#define WS_CLIENT_HOST "192.168.1.100"  // Master node IP - change this
#define WS_CLIENT_PORT 81
#define WS_CLIENT_URL "/ws"

// Message types
enum MessageType {
    MSG_INSERT,
    MSG_DELETE,
    MSG_LOOKUP,
    MSG_RESPONSE
};

// Callback function type for received messages
typedef void (*MessageCallback)(MessageType type, const char* value, bool result);

class WebSocketClient {
private:
    WebSocketsClient webSocket;
    MessageCallback callback;
    bool connected;
    
    static void webSocketEvent(WStype_t type, uint8_t * payload, size_t length);
    void handleEvent(WStype_t type, uint8_t * payload, size_t length);
    
public:
    WebSocketClient();
    
    void begin(const char* host = WS_CLIENT_HOST, uint16_t port = WS_CLIENT_PORT, const char* url = WS_CLIENT_URL);
    void loop();
    bool isConnected();
    
    // Send operations
    bool sendInsert(const char* value);
    bool sendDelete(const char* value);
    bool sendLookup(const char* value);
    bool sendResponse(const char* value, bool result);
    
    // Set callback for received messages
    void onMessage(MessageCallback cb);
};

// Helper function to create JSON message
String createMessage(MessageType type, const char* value, int nodeId = 0);
