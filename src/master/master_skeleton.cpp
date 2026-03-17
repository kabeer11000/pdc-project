// Master node skeleton for Milestone 1. Sends dummy messages to slave nodes.
 
#include <iostream>
#include <string>

// Placeholder for WebSocket library
// #include <WebSocketsClient.h>

using namespace std;

// Initialize master node (placeholder)
void setup() {
    cout << "Master node setup complete." << endl;
    // Initialize WebSocket here when ready
}

// Function to send a dummy request
void sendRequest(const string &message) {
    cout << "Sending request to slave: " << message << endl;
    // Replace with actual WebSocket send
}

// Function to receive a dummy response
string receiveResponse() {
    string dummyResponse = "DUMMY_RESPONSE_FROM_SLAVE";
    cout << "Received response: " << dummyResponse << endl;
    return dummyResponse;
}

int main() {
    setup();

    // Example (dummy flow)
    string message = "TEST_INSERT";
    sendRequest(message);

    string response = receiveResponse();

    cout << "Master node finished dummy communication." << endl;

    return 0;
}