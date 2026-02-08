/*
 * Minimal Socket.IO Test Sketch for ESP32
 * Connects to Socket.IO backend and logs connection events.
 *
 * Update WIFI_SSID, WIFI_PASSWORD, and SERVER_HOST as needed.
 */

#include <WiFi.h>
#include <SocketIOclient.h>

#define SOCKETIOCLIENT_DEBUG

const char* WIFI_SSID = "Robot";
const char* WIFI_PASSWORD = "Robot123";
const char* SERVER_HOST = "smart-locker-c3vr.onrender.com";
const int SERVER_PORT = 443;

const char* fingerprint = "C5:C7:2B:24:C6:4C:88:EA:7B:A2:66:59:71:B3:4D:AF:52:53:B4:C6"; // Replace with actual Render cert fingerprint

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

  // Connect to Socket.IO backend (SSL, Render public endpoint)
  socketIO.begin("192.168.1.100", 3000, "/socket.io/?transport=websocket");
  socketIO.onEvent(socketIOEvent);
  socketIO.setReconnectInterval(5000);
}

void loop() {
  socketIO.loop();
  static unsigned long last = 0;
  if (millis() - last > 30000) {
    socketIO.sendEVENT("[\"shelf:heartbeat\",{\"status\":\"alive\"}]");
    last = millis();
  }
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
      Serial.printf("[SIO] Error: %.*s\n", (int)length, payload);
      break;
    case sIOtype_PING:
      Serial.println("[SIO] Ping received");
      break;
    case sIOtype_PONG:
      Serial.println("[SIO] Pong sent");
      break;
    default:
      break;
  }
}
