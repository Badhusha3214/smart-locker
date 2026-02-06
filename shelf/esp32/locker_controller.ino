/*
 * Smart QR-Controlled Locker System - ESP32 Controller
 * 
 * This code controls the physical locker hardware and communicates
 * with the Node.js backend via WebSocket (Socket.IO)
 * 
 * Hardware Connections:
 * - Servo Motor: GPIO 13
 * - IR Sensor (item detection): GPIO 14
 * - Status LED: GPIO 2 (built-in)
 * - Buzzer: GPIO 15 (optional)
 */

#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>

// ============== CONFIGURATION ==============
// WiFi Credentials
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// Backend Server (your computer's IP on local network)
const char* SERVER_HOST = "192.168.1.100";  // Change to your server IP
const int SERVER_PORT = 5000;

// Rack Configuration - Each ESP32 controls one rack
const char* RACK_ID = "RACK_001";  // Must match MongoDB rack ID

// Hardware Pins
#define SERVO_PIN 13
#define IR_SENSOR_PIN 14
#define LED_PIN 2
#define BUZZER_PIN 15

// Lock Positions (adjust based on your servo)
#define LOCK_POSITION 0      // Servo angle when locked
#define UNLOCK_POSITION 90   // Servo angle when unlocked

// Timing
#define UNLOCK_DURATION 30000     // Keep unlocked for 30 seconds
#define SENSOR_CHECK_INTERVAL 1000 // Check sensor every 1 second
#define RECONNECT_INTERVAL 5000    // Retry connection every 5 seconds

// ============== GLOBAL VARIABLES ==============
WebSocketsClient webSocket;
Servo lockServo;

bool isConnected = false;
bool isLocked = true;
bool itemPresent = false;
bool lastItemState = false;

unsigned long unlockTime = 0;
unsigned long lastSensorCheck = 0;
unsigned long lastReconnectAttempt = 0;

// ============== SETUP ==============
void setup() {
  Serial.begin(115200);
  Serial.println("\n\n=================================");
  Serial.println("Smart Locker System - ESP32");
  Serial.println("=================================\n");

  // Initialize pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(IR_SENSOR_PIN, INPUT);
  
  // Initialize servo
  ESP32PWM::allocateTimer(0);
  lockServo.setPeriodHertz(50);
  lockServo.attach(SERVO_PIN, 500, 2400);
  
  // Start in locked position
  lockServo.write(LOCK_POSITION);
  Serial.println("✓ Servo initialized - LOCKED");
  
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
  
  // Blink LED when connected
  if (isConnected) {
    digitalWrite(LED_PIN, (millis() / 1000) % 2);
  }
}

// ============== WIFI CONNECTION ==============
void connectWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✓ WiFi Connected!");
    Serial.print("  IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n✗ WiFi Connection Failed!");
  }
}

// ============== WEBSOCKET CONNECTION ==============
void connectWebSocket() {
  Serial.print("Connecting to WebSocket: ");
  Serial.print(SERVER_HOST);
  Serial.print(":");
  Serial.println(SERVER_PORT);
  
  // Socket.IO uses /socket.io/ path with specific query params
  webSocket.beginSocketIO(SERVER_HOST, SERVER_PORT, "/socket.io/?EIO=4");
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
      
      // Register this ESP32 with the server
      registerWithServer();
      break;
      
    case WStype_TEXT:
      handleMessage((char*)payload);
      break;
      
    case WStype_ERROR:
      Serial.println("✗ WebSocket Error");
      break;
      
    default:
      break;
  }
}

// ============== REGISTER WITH SERVER ==============
void registerWithServer() {
  // Socket.IO message format: 42["event", data]
  StaticJsonDocument<256> doc;
  doc["rackId"] = RACK_ID;
  doc["type"] = "esp32";
  doc["status"] = isLocked ? "locked" : "unlocked";
  doc["itemPresent"] = itemPresent;
  
  String jsonStr;
  serializeJson(doc, jsonStr);
  
  String message = "42[\"esp:register\"," + jsonStr + "]";
  webSocket.sendTXT(message);
  
  Serial.println("→ Registered with server");
  Serial.print("  Rack ID: ");
  Serial.println(RACK_ID);
}

// ============== HANDLE INCOMING MESSAGES ==============
void handleMessage(char* payload) {
  String msg = String(payload);
  Serial.print("← Received: ");
  Serial.println(msg);
  
  // Socket.IO messages start with "42" for event messages
  if (!msg.startsWith("42")) {
    // Handle Socket.IO protocol messages (ping/pong)
    if (msg == "2") {
      webSocket.sendTXT("3"); // Pong
    }
    return;
  }
  
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
    // Parse the data to get rack ID
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

// ============== UNLOCK LOCKER ==============
void unlockLocker() {
  if (!isLocked) {
    Serial.println("Already unlocked");
    return;
  }
  
  Serial.println("🔓 Unlocking...");
  
  // Move servo to unlock position
  lockServo.write(UNLOCK_POSITION);
  isLocked = false;
  unlockTime = millis();
  
  // Feedback
  digitalWrite(LED_PIN, HIGH);
  beep(1, 200);
  
  // Notify server
  sendStatusUpdate();
  
  Serial.println("✓ Unlocked!");
}

// ============== LOCK LOCKER ==============
void lockLocker() {
  if (isLocked) {
    Serial.println("Already locked");
    return;
  }
  
  Serial.println("🔒 Locking...");
  
  // Move servo to lock position
  lockServo.write(LOCK_POSITION);
  isLocked = true;
  
  // Feedback
  beep(2, 100);
  
  // Notify server
  sendStatusUpdate();
  
  Serial.println("✓ Locked!");
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

// ============== UTILITY: GET STATUS JSON ==============
String getStatusJson() {
  StaticJsonDocument<256> doc;
  doc["rackId"] = RACK_ID;
  doc["isLocked"] = isLocked;
  doc["itemPresent"] = itemPresent;
  doc["wifiConnected"] = WiFi.status() == WL_CONNECTED;
  doc["serverConnected"] = isConnected;
  doc["uptime"] = millis() / 1000;
  
  String output;
  serializeJson(doc, output);
  return output;
}
