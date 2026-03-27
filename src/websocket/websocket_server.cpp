#include "websocket_server.h"

WebSocketServer::WebSocketServer(uint16_t port) 
    : webSocket(port), callback(nullptr) {}

void WebSocketServer::begin() {
    webSocket.begin();
    webSocket.onEvent([this](uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
        this->handleEvent(num, type, payload, length);
    });
    Serial.printf("[WebSocket Server] Started on port %d\n", WEBSOCKET_SERVER_PORT);
}

void WebSocketServer::loop() {
    webSocket.loop();
}

void WebSocketServer::handleEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            Serial.printf("[WebSocket] Client %u disconnected\n", num);
            break;
            
        case WStype_CONNECTED:
            Serial.printf("[WebSocket] Client %u connected\n", num);
            break;
            
        case WStype_TEXT: {
            Serial.printf("[WebSocket] Client %u sent: %s\n", num, payload);
            
            // Parse JSON message
            StaticJsonDocument<256> doc;
            DeserializationError error = deserializeJson(doc, payload, length);
            
            if (!error && callback) {
                const char* type = doc["type"];
                const char* value = doc["value"];
                
                MessageType msgType;
                if (strcmp(type, "INSERT") == 0) msgType = MSG_INSERT;
                else if (strcmp(type, "DELETE") == 0) msgType = MSG_DELETE;
                else if (strcmp(type, "LOOKUP") == 0) msgType = MSG_LOOKUP;
                else if (strcmp(type, "RESPONSE") == 0) msgType = MSG_RESPONSE;
                else return;
                
                callback(num, msgType, value);
            }
            break;
        }
            
        case WStype_PING:
            Serial.printf("[WebSocket] PING from client %u\n", num);
            break;
            
        case WStype_PONG:
            Serial.printf("[WebSocket] PONG from client %u\n", num);
            break;
            
        default:
            break;
    }
}

bool WebSocketServer::sendResponse(uint8_t num, MessageType type, const char* value, bool result) {
    String msg = createMessage(type, value, result);
    bool sent = webSocket.sendTXT(num, msg);
    Serial.printf("[WebSocket] Sent to %u: %s\n", num, sent ? "OK" : "FAILED");
    return sent;
}

bool WebSocketServer::sendTXT(uint8_t num, String& txt) {
    bool sent = webSocket.sendTXT(num, txt);
    Serial.printf("[WebSocket] Sent TXT to %u: %s\n", num, sent ? "OK" : "FAILED");
    return sent;
}

bool WebSocketServer::broadcast(const char* message) {
    bool sent = webSocket.broadcastTXT(message);
    Serial.printf("[WebSocket] Broadcast: %s\n", sent ? "OK" : "FAILED");
    return sent;
}

void WebSocketServer::onMessage(MessageCallback cb) {
    callback = cb;
}

uint8_t WebSocketServer::getConnectedCount() {
    return webSocket.connectedClients();
}

String createMessage(MessageType type, const char* value, bool result) {
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
    doc["result"] = result;
    
    String output;
    serializeJson(doc, output);
    return output;
}
