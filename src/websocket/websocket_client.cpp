#include "websocket_client.h"

WebSocketClient::WebSocketClient() : connected(false), callback(nullptr) {}

void WebSocketClient::webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
    // This is a static wrapper to call the instance method
    static WebSocketClient* instance = nullptr;
    if (!instance) {
        return;
    }
    instance->handleEvent(type, payload, length);
}

void WebSocketClient::handleEvent(WStype_t type, uint8_t * payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            Serial.println("[WebSocket] Disconnected");
            connected = false;
            break;

        case WStype_CONNECTED:
            Serial.println("[WebSocket] Connected");
            connected = true;
            break;

        case WStype_TEXT: {
            Serial.printf("[WebSocket] Received: %s\n", payload);
            
            // Parse JSON message
            StaticJsonDocument<256> doc;
            DeserializationError error = deserializeJson(doc, payload, length);
            
            if (!error && callback) {
                const char* type = doc["type"];
                const char* value = doc["value"];
                bool result = doc["result"] | false;
                
                MessageType msgType;
                if (strcmp(type, "INSERT") == 0) msgType = MSG_INSERT;
                else if (strcmp(type, "DELETE") == 0) msgType = MSG_DELETE;
                else if (strcmp(type, "LOOKUP") == 0) msgType = MSG_LOOKUP;
                else if (strcmp(type, "RESPONSE") == 0) msgType = MSG_RESPONSE;
                else return;
                
                callback(msgType, value, result);
            }
            break;
        }

        case WStype_PING:
            Serial.println("[WebSocket] PING received");
            break;

        case WStype_PONG:
            Serial.println("[WebSocket] PONG received");
            break;
            
        default:
            break;
    }
}

void WebSocketClient::begin(const char* host, uint16_t port, const char* url) {
    webSocket.begin(host, port, url);
    webSocket.onEvent([this](WStype_t type, uint8_t * payload, size_t length) {
        this->handleEvent(type, payload, length);
    });
    webSocket.setReconnectInterval(5000);
    
    Serial.printf("[WebSocket] Connecting to ws://%s:%d%s\n", host, port, url);
}

void WebSocketClient::loop() {
    webSocket.loop();
}

bool WebSocketClient::isConnected() {
    return connected;
}

bool WebSocketClient::sendInsert(const char* value) {
    String msg = createMessage(MSG_INSERT, value);
    bool sent = webSocket.sendTXT(msg);
    Serial.printf("[WebSocket] Sent INSERT: %s -> %s\n", sent ? "OK" : "FAILED", value);
    return sent;
}

bool WebSocketClient::sendDelete(const char* value) {
    String msg = createMessage(MSG_DELETE, value);
    bool sent = webSocket.sendTXT(msg);
    Serial.printf("[WebSocket] Sent DELETE: %s -> %s\n", sent ? "OK" : "FAILED", value);
    return sent;
}

bool WebSocketClient::sendLookup(const char* value) {
    String msg = createMessage(MSG_LOOKUP, value);
    bool sent = webSocket.sendTXT(msg);
    Serial.printf("[WebSocket] Sent LOOKUP: %s -> %s\n", sent ? "OK" : "FAILED", value);
    return sent;
}

bool WebSocketClient::sendResponse(const char* value, bool result) {
    String msg = createMessage(MSG_RESPONSE, value, result);
    bool sent = webSocket.sendTXT(msg);
    Serial.printf("[WebSocket] Sent RESPONSE: %s -> %s (result=%d)\n", sent ? "OK" : "FAILED", value, result);
    return sent;
}

void WebSocketClient::onMessage(MessageCallback cb) {
    callback = cb;
}

String createMessage(MessageType type, const char* value, int nodeId) {
    StaticJsonDocument<128> doc;
    
    const char* typeStr;
    switch (type) {
        case MSG_INSERT: typeStr = "INSERT"; break;
        case MSG_DELETE: typeStr = "DELETE"; break;
        case MSG_LOOKUP: typeStr = "LOOKUP"; break;
        case MSG_RESPONSE: typeStr = "RESPONSE"; break;
        default: typeStr = "UNKNOWN";
    }
    
    doc["type"] = typeStr;
    doc["value"] = value;
    if (nodeId > 0) {
        doc["nodeId"] = nodeId;
    }
    
    String output;
    serializeJson(doc, output);
    return output;
}
