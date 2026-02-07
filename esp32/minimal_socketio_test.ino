/*
 * Minimal Socket.IO Test Sketch for ESP32
 * Connects to Socket.IO backend and logs connection events.
 *
 * Update WIFI_SSID, WIFI_PASSWORD, and SERVER_HOST as needed.
 */

#include <WiFi.h>
#include <SocketIOclient.h>

const char* WIFI_SSID = "Robot";
const char* WIFI_PASSWORD = "Robot123";
const char* SERVER_HOST = "smart-locker-c3vr.onrender.com";
const int SERVER_PORT = 443;

SocketIOclient socketIO;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n[TEST] Minimal Socket.IO Test Sketch\n");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\nWiFi connected! IP: %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("\nWiFi connection failed!");
    return;
  }

  // Connect to Socket.IO backend (SSL)
  socketIO.beginSSL(SERVER_HOST, SERVER_PORT, "/socket.io/?transport=websocket");
  socketIO.onEvent(socketIOEvent);
}

void loop() {
  socketIO.loop();
}

void socketIOEvent(socketIOmessageType_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case sIOtype_DISCONNECT:
      Serial.println("[SOCKETIO] Disconnected");
      break;
    case sIOtype_CONNECT:
      Serial.println("[SOCKETIO] Connected!");
      break;
    case sIOtype_EVENT:
      Serial.printf("[SOCKETIO] Event: %.*s\n", length, payload);
      break;
    case sIOtype_ACK:
      Serial.println("[SOCKETIO] ACK received");
      break;
    case sIOtype_ERROR:
      Serial.printf("[SOCKETIO] Error: %s\n", payload);
      break;
    default:
      break;
  }
}
