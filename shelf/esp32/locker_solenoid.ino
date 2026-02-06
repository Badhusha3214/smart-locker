/*
 * Smart QR-Controlled Locker System - ESP32 Controller (SOLENOID VERSION)
 * 
 * This code controls a SOLENOID LOCK via relay module and communicates
 * with the Node.js backend via WebSocket (Socket.IO)
 * 
 * Hardware Connections:
 * - Relay Module IN: GPIO 13 (controls solenoid)
 * - IR Sensor (item detection): GPIO 14
 * - Status LED: GPIO 2 (built-in)
 * - Buzzer: GPIO 15 (optional)
 * 
 * Solenoid Lock Types:
 * - Fail-Secure (Normally Locked): Power ON = Unlock, Power OFF = Lock
 * - Fail-Safe (Normally Unlocked): Power ON = Lock, Power OFF = Unlock
 * 
 * This code assumes FAIL-SECURE (Normally Locked) solenoid
 */

#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>

// ============== CONFIGURATION ==============
// WiFi Credentials - UPDATE THESE!
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// Backend Server - UPDATE WITH YOUR SERVER IP/DOMAIN
// For local testing: Use your computer's local IP (e.g., 192.168.1.100)
// For production: Use your server domain (e.g., api.yourlocker.com)
const char* SERVER_HOST = "192.168.1.100";  // Change this!
const int SERVER_PORT = 5000;
const bool USE_SSL = false;  // Set true for production with HTTPS

// Rack Configuration - Each ESP32 controls one rack
const char* RACK_ID = "RACK_001";  // Must match MongoDB rack rackNumber

// Hardware Pins
#define RELAY_PIN 13        // Relay module IN pin (controls solenoid)
#define IR_SENSOR_PIN 14    // Item detection sensor
#define LED_PIN 2           // Built-in LED
#define BUZZER_PIN 15       // Buzzer for audio feedback

// Solenoid Configuration
// For FAIL-SECURE solenoid (normally locked):
//   - HIGH = Unlock (power to solenoid)
//   - LOW = Lock (no power to solenoid)
// For FAIL-SAFE solenoid (normally unlocked), swap these values
#define UNLOCK_STATE HIGH
#define LOCK_STATE LOW

// Timing Configuration
#define UNLOCK_DURATION 30000      // Keep unlocked for 30 seconds
#define SENSOR_CHECK_INTERVAL 1000 // Check sensor every 1 second
#define RECONNECT_INTERVAL 5000    // Retry connection every 5 seconds
#define HEARTBEAT_INTERVAL 30000   // Send heartbeat every 30 seconds

// ============== GLOBAL VARIABLES ==============
WebSocketsClient webSocket;

bool isConnected = false;
bool isLocked = true;
bool itemPresent = false;
bool lastItemState = false;

unsigned long unlockTime = 0;
unsigned long lastSensorCheck = 0;
unsigned long lastHeartbeat = 0;
unsigned long lastReconnectAttempt = 0;

// ============== SETUP ==============
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n╔════════════════════════════════════════╗");
  Serial.println("║  Smart Locker System - ESP32           ║");
  Serial.println("║  SOLENOID LOCK VERSION                 ║");
  Serial.println("╚════════════════════════════════════════╝\n");

  // Initialize pins
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(IR_SENSOR_PIN, INPUT);
  
  // Start in LOCKED state (relay OFF for fail-secure solenoid)
  digitalWrite(RELAY_PIN, LOCK_STATE);
  isLocked = true;
  Serial.println("✓ Relay initialized - LOCKED");
  
  // Connect to WiFi
  connectWiFi();
  
  // Connect to WebSocket server
  connectWebSocket();
  
  // Initial sensor read
  itemPresent = digitalRead(IR_SENSOR_PIN) == LOW;
  lastItemState = itemPresent;
  
  // Startup beep
  beep(2, 100);
  Serial.println("\n✓ System Ready!\n");
  Serial.println("Rack ID: " + String(RACK_ID));
  Serial.println("Server: " + String(SERVER_HOST) + ":" + String(SERVER_PORT));
}

// ============== MAIN LOOP ==============
void loop() {
  // Handle WebSocket events
  webSocket.loop();
  
  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    isConnected = false;
    digitalWrite(LED_PIN, LOW);
    connectWiFi();
    connectWebSocket();
  }
  
  // Auto-lock after timeout
  if (!isLocked && millis() - unlockTime > UNLOCK_DURATION) {
    lockLocker();
    Serial.println("⏰ Auto-locked after timeout");
  }
  
  // Check item sensor periodically
  if (millis() - lastSensorCheck > SENSOR_CHECK_INTERVAL) {
    lastSensorCheck = millis();
    checkItemSensor();
  }
  
  // Send heartbeat to server
  if (isConnected && millis() - lastHeartbeat > HEARTBEAT_INTERVAL) {
    lastHeartbeat = millis();
    sendHeartbeat();
  }
  
  // Blink LED when connected
  if (isConnected) {
    digitalWrite(LED_PIN, (millis() / 1000) % 2);
  } else {
    // Fast blink when disconnected
    digitalWrite(LED_PIN, (millis() / 200) % 2);
  }
}

// ============== WIFI CONNECTION ==============
void connectWiFi() {
  Serial.print("📶 Connecting to WiFi: ");
  Serial.println(WIFI_SSID);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    attempts++;
    
    // Blink LED while connecting
    digitalWrite(LED_PIN, attempts % 2);
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✓ WiFi Connected!");
    Serial.print("  IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("  Signal Strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    
    digitalWrite(LED_PIN, HIGH);
    beep(1, 100);
  } else {
    Serial.println("\n✗ WiFi Connection Failed!");
    Serial.println("  Check SSID and password");
    
    // Error beep
    beep(3, 200);
  }
}

// ============== WEBSOCKET CONNECTION ==============
void connectWebSocket() {
  Serial.print("🔌 Connecting to WebSocket: ");
  Serial.print(SERVER_HOST);
  Serial.print(":");
  Serial.println(SERVER_PORT);
  
  // Socket.IO uses /socket.io/ path with specific query params
  if (USE_SSL) {
    webSocket.beginSocketIOSSL(SERVER_HOST, 443, "/socket.io/?EIO=4");
  } else {
    webSocket.beginSocketIO(SERVER_HOST, SERVER_PORT, "/socket.io/?EIO=4");
  }
  
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(RECONNECT_INTERVAL);
}

// ============== WEBSOCKET EVENT HANDLER ==============
void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.println("✗ WebSocket Disconnected");
      isConnected = false;
      digitalWrite(LED_PIN, LOW);
      break;
      
    case WStype_CONNECTED:
      Serial.println("✓ WebSocket Connected!");
      isConnected = true;
      digitalWrite(LED_PIN, HIGH);
      beep(1, 50);
      
      // Register this ESP32 with the server
      registerWithServer();
      break;
      
    case WStype_TEXT:
      handleMessage((char*)payload);
      break;
      
    case WStype_ERROR:
      Serial.println("✗ WebSocket Error");
      break;
      
    case WStype_PING:
      Serial.println("← Ping");
      break;
      
    case WStype_PONG:
      Serial.println("→ Pong");
      break;
      
    default:
      break;
  }
}

// ============== REGISTER WITH SERVER ==============
void registerWithServer() {
  // Socket.IO message format: 42["event", data]
  StaticJsonDocument<512> doc;
  doc["rackId"] = RACK_ID;
  doc["type"] = "esp32";
  doc["lockType"] = "solenoid";
  doc["status"] = isLocked ? "locked" : "unlocked";
  doc["itemPresent"] = itemPresent;
  doc["firmwareVersion"] = "1.0.0";
  doc["ip"] = WiFi.localIP().toString();
  doc["rssi"] = WiFi.RSSI();
  
  String jsonStr;
  serializeJson(doc, jsonStr);
  
  String message = "42[\"esp:register\"," + jsonStr + "]";
  webSocket.sendTXT(message);
  
  Serial.println("→ Registered with server");
  Serial.print("  Rack ID: ");
  Serial.println(RACK_ID);
}

// ============== SEND HEARTBEAT ==============
void sendHeartbeat() {
  if (!isConnected) return;
  
  StaticJsonDocument<256> doc;
  doc["rackId"] = RACK_ID;
  doc["uptime"] = millis() / 1000;
  doc["rssi"] = WiFi.RSSI();
  doc["freeHeap"] = ESP.getFreeHeap();
  
  String jsonStr;
  serializeJson(doc, jsonStr);
  
  String message = "42[\"esp:heartbeat\"," + jsonStr + "]";
  webSocket.sendTXT(message);
}

// ============== HANDLE INCOMING MESSAGES ==============
void handleMessage(char* payload) {
  String msg = String(payload);
  
  // Socket.IO messages start with "42" for event messages
  if (!msg.startsWith("42")) {
    // Handle Socket.IO protocol messages (ping/pong)
    if (msg == "2") {
      webSocket.sendTXT("3"); // Pong
    }
    return;
  }
  
  Serial.print("← Received: ");
  Serial.println(msg);
  
  // Parse Socket.IO event: 42["eventName", {data}]
  int firstBracket = msg.indexOf('[');
  int firstQuote = msg.indexOf('"', firstBracket);
  int secondQuote = msg.indexOf('"', firstQuote + 1);
  
  if (firstQuote == -1 || secondQuote == -1) return;
  
  String eventName = msg.substring(firstQuote + 1, secondQuote);
  
  // Find JSON data after event name
  int dataStart = msg.indexOf(',', secondQuote);
  int dataEnd = msg.lastIndexOf(']');
  
  if (eventName == "locker:unlock") {
    if (dataStart != -1 && dataEnd != -1) {
      String dataStr = msg.substring(dataStart + 1, dataEnd);
      
      StaticJsonDocument<256> doc;
      DeserializationError error = deserializeJson(doc, dataStr);
      
      if (!error) {
        const char* targetRack = doc["rackId"];
        
        // Check if this command is for this rack
        if (String(targetRack) == String(RACK_ID)) {
          Serial.println("🔓 UNLOCK COMMAND RECEIVED!");
          unlockLocker();
        }
      }
    }
  }
  else if (eventName == "locker:lock") {
    if (dataStart != -1 && dataEnd != -1) {
      String dataStr = msg.substring(dataStart + 1, dataEnd);
      
      StaticJsonDocument<256> doc;
      DeserializationError error = deserializeJson(doc, dataStr);
      
      if (!error) {
        const char* targetRack = doc["rackId"];
        
        if (String(targetRack) == String(RACK_ID)) {
          Serial.println("🔒 LOCK COMMAND RECEIVED!");
          lockLocker();
        }
      }
    }
  }
  else if (eventName == "locker:status") {
    // Server requesting status update
    sendStatusUpdate();
  }
}

// ============== UNLOCK LOCKER (SOLENOID) ==============
void unlockLocker() {
  if (!isLocked) {
    Serial.println("Already unlocked");
    return;
  }
  
  Serial.println("🔓 Unlocking solenoid...");
  
  // Activate relay to power solenoid (unlock)
  digitalWrite(RELAY_PIN, UNLOCK_STATE);
  isLocked = false;
  unlockTime = millis();
  
  // Visual and audio feedback
  digitalWrite(LED_PIN, HIGH);
  beep(1, 200);
  
  // Notify server
  sendStatusUpdate();
  
  Serial.println("✓ UNLOCKED - Solenoid powered");
}

// ============== LOCK LOCKER (SOLENOID) ==============
void lockLocker() {
  if (isLocked) {
    Serial.println("Already locked");
    return;
  }
  
  Serial.println("🔒 Locking solenoid...");
  
  // Deactivate relay (solenoid returns to locked state)
  digitalWrite(RELAY_PIN, LOCK_STATE);
  isLocked = true;
  
  // Audio feedback
  beep(2, 100);
  
  // Notify server
  sendStatusUpdate();
  
  Serial.println("✓ LOCKED - Solenoid de-powered");
}

// ============== CHECK ITEM SENSOR ==============
void checkItemSensor() {
  // IR sensor: LOW when object detected, HIGH when empty
  bool currentState = digitalRead(IR_SENSOR_PIN) == LOW;
  
  if (currentState != lastItemState) {
    lastItemState = currentState;
    itemPresent = currentState;
    
    Serial.print("📦 Item sensor: ");
    Serial.println(itemPresent ? "ITEM PRESENT" : "EMPTY");
    
    // Notify server of change
    sendItemStatusUpdate();
  }
}

// ============== SEND STATUS UPDATE ==============
void sendStatusUpdate() {
  if (!isConnected) return;
  
  StaticJsonDocument<256> doc;
  doc["rackId"] = RACK_ID;
  doc["isLocked"] = isLocked;
  doc["itemPresent"] = itemPresent;
  doc["timestamp"] = millis();
  
  String jsonStr;
  serializeJson(doc, jsonStr);
  
  String message = "42[\"esp:status\"," + jsonStr + "]";
  webSocket.sendTXT(message);
  
  Serial.println("→ Status update sent");
}

// ============== SEND ITEM STATUS UPDATE ==============
void sendItemStatusUpdate() {
  if (!isConnected) return;
  
  StaticJsonDocument<256> doc;
  doc["rackId"] = RACK_ID;
  doc["itemPresent"] = itemPresent;
  doc["needsRefill"] = !itemPresent; // Empty = needs refill
  
  String jsonStr;
  serializeJson(doc, jsonStr);
  
  String message = "42[\"esp:itemStatus\"," + jsonStr + "]";
  webSocket.sendTXT(message);
  
  Serial.print("→ Item status: ");
  Serial.println(itemPresent ? "present" : "needs refill");
}

// ============== BUZZER BEEP ==============
void beep(int times, int duration) {
  for (int i = 0; i < times; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(duration);
    digitalWrite(BUZZER_PIN, LOW);
    if (i < times - 1) delay(duration);
  }
}

// ============== MANUAL TEST (via Serial) ==============
void serialEvent() {
  while (Serial.available()) {
    char c = Serial.read();
    
    if (c == 'u' || c == 'U') {
      Serial.println("\n[MANUAL] Unlock command");
      unlockLocker();
    }
    else if (c == 'l' || c == 'L') {
      Serial.println("\n[MANUAL] Lock command");
      lockLocker();
    }
    else if (c == 's' || c == 'S') {
      Serial.println("\n[MANUAL] Status:");
      Serial.print("  Locked: ");
      Serial.println(isLocked ? "Yes" : "No");
      Serial.print("  Item Present: ");
      Serial.println(itemPresent ? "Yes" : "No");
      Serial.print("  Connected: ");
      Serial.println(isConnected ? "Yes" : "No");
      Serial.print("  WiFi RSSI: ");
      Serial.println(WiFi.RSSI());
    }
    else if (c == 'r' || c == 'R') {
      Serial.println("\n[MANUAL] Reconnecting...");
      webSocket.disconnect();
      delay(1000);
      connectWebSocket();
    }
  }
}
