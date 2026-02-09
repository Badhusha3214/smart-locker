/*
 * Smart QR-Controlled Locker System - ESP32 SHELF Controller
 * HTTP Polling Mode (More reliable than WebSocket for ESP32)
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// ============== CONFIGURATION ==============
const char* WIFI_SSID = "Robot";
const char* WIFI_PASSWORD = "Robot123";
const char* SERVER_HOST = "35.225.93.34";
const int SERVER_PORT = 5000;

// Shelf Configuration
const char* SHELF_ID = "SHELF_001";
const int NUM_DOORS = 9;

// Rack IDs for each door
const char* RACK_IDS[12] = {
  "RACK-001", "RACK-002", "RACK-003", "RACK-004",
  "RACK-005", "RACK-006", "RACK-007", "RACK-008",
  "RACK-009", "RACK-010", "RACK-011", "RACK-012"
};

// Relay pins
const int RELAY_PINS[12] = {
  13, 27, 25, 32, 23, 22, 21, 19, 18, 5, 4, 16
};

// IR Sensor Configuration
#define IR_SENSORS_ENABLED false
const int IR_SENSOR_PINS[12] = {
  14, 26, 33, 35, 34, 39, 36, -1, -1, -1, -1, -1
};

// Common pins
#define LED_PIN 2
#define BUZZER_PIN 15

// States
#define UNLOCK_STATE HIGH
#define LOCK_STATE LOW

// Timing
#define UNLOCK_DURATION 30000
#define POLL_INTERVAL 100
#define PING_INTERVAL 20000
#define HEARTBEAT_INTERVAL 30000
#define SENSOR_CHECK_INTERVAL 1000
#define SENSOR_DEBOUNCE_TIME 500

// ============== GLOBALS ==============
String sessionId = "";
bool socketIOConnected = false;

// Door states
bool doorLocked[12];
bool itemPresent[12];
bool lastItemState[12];
unsigned long unlockTime[12];
unsigned long lastSensorChange[12];
bool hasPendingChange[12];
bool pendingSensorState[12];

// Timing
unsigned long lastPoll = 0;
unsigned long lastPing = 0;
unsigned long lastHeartbeat = 0;
unsigned long lastSensorCheck = 0;

// ============== FUNCTION PROTOTYPES ==============
void connectWiFi();
void connectSocketIO();
void pollMessages();
void handleMessage(String msg);
void sendMessage(String event, String data);
void sendPing();
String getPollingURL();
void registerWithServer();
void sendHeartbeat();
int findDoorByRackId(const char* rackId);
void unlockDoor(int doorIndex);
void lockDoor(int doorIndex);
void sendDoorStatus(int doorIndex);
void beep(int times, int duration);
void handleSerialCommands();
void printStatus();
void printHelp();
void testAllDoors();
#if IR_SENSORS_ENABLED
void checkAllSensors();
void sendItemStatus(int doorIndex);
#endif

// ============== SETUP ==============
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  printBanner();
  initializeHardware();
  connectWiFi();
  connectSocketIO();
  
  // Initial sensor read
  for (int i = 0; i < NUM_DOORS; i++) {
    if (IR_SENSOR_PINS[i] >= 0) {
      itemPresent[i] = digitalRead(IR_SENSOR_PINS[i]) == LOW;
      lastItemState[i] = itemPresent[i];
    }
  }
  
  beep(2, 100);
  Serial.println("\n>>> Shelf controller ready! <<<\n");
}

void printBanner() {
  Serial.println("\n╔════════════════════════════════════════════════════════════╗");
  Serial.println("║     SMART LOCKER SYSTEM - SHELF CONTROLLER                 ║");
  Serial.println("║     HTTP POLLING MODE                                      ║");
  Serial.println("╠════════════════════════════════════════════════════════════╣");
  Serial.printf("║  Shelf ID:    %s                                     ║\n", SHELF_ID);
  Serial.printf("║  Doors:       %d                                            ║\n", NUM_DOORS);
  Serial.println("╠════════════════════════════════════════════════════════════╣");
  Serial.println("║  Commands: u<n>=Unlock door n, l<n>=Lock, s=Status         ║");
  Serial.println("╚════════════════════════════════════════════════════════════╝\n");
}

void initializeHardware() {
  Serial.println("Initializing hardware...");
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  
  for (int i = 0; i < NUM_DOORS; i++) {
    pinMode(RELAY_PINS[i], OUTPUT);
    digitalWrite(RELAY_PINS[i], LOCK_STATE);
    doorLocked[i] = true;
    unlockTime[i] = 0;
    hasPendingChange[i] = false;
    
    Serial.printf("  Door %d: Relay GPIO %d", i, RELAY_PINS[i]);
    
    #if IR_SENSORS_ENABLED
    if (IR_SENSOR_PINS[i] >= 0) {
      pinMode(IR_SENSOR_PINS[i], INPUT);
      Serial.printf(", Sensor GPIO %d", IR_SENSOR_PINS[i]);
    }
    #endif
    
    Serial.printf(" → %s\n", RACK_IDS[i]);
  }
  
  Serial.println("Hardware initialized!\n");
}

// ============== MAIN LOOP ==============
void loop() {
  // Check WiFi
  if (WiFi.status() != WL_CONNECTED) {
    socketIOConnected = false;
    connectWiFi();
    connectSocketIO();
  }
  
  // Poll for messages
  if (sessionId.length() > 0 && millis() - lastPoll > POLL_INTERVAL) {
    lastPoll = millis();
    pollMessages();
  }
  
  // Send ping
  if (socketIOConnected && millis() - lastPing > PING_INTERVAL) {
    lastPing = millis();
    sendPing();
  }
  
  // Send heartbeat
  if (socketIOConnected && millis() - lastHeartbeat > HEARTBEAT_INTERVAL) {
    lastHeartbeat = millis();
    sendHeartbeat();
  }
  
  // Auto-lock doors
  for (int i = 0; i < NUM_DOORS; i++) {
    if (!doorLocked[i] && millis() - unlockTime[i] > UNLOCK_DURATION) {
      Serial.printf("[AUTO-LOCK] Door %d (%s)\n", i, RACK_IDS[i]);
      lockDoor(i);
    }
  }
  
  // Check sensors
  #if IR_SENSORS_ENABLED
  if (millis() - lastSensorCheck > SENSOR_CHECK_INTERVAL) {
    lastSensorCheck = millis();
    checkAllSensors();
  }
  #endif
  
  // LED status
  digitalWrite(LED_PIN, socketIOConnected ? (millis() / 1000) % 2 : (millis() / 200) % 2);
  
  // Serial commands
  handleSerialCommands();
}

// ============== WIFI ==============
void connectWiFi() {
  Serial.printf("[WIFI] Connecting to %s", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\n[WIFI] Connected! IP: %s\n\n", WiFi.localIP().toString().c_str());
    beep(1, 100);
  } else {
    Serial.println("\n[WIFI] Connection failed!\n");
    beep(3, 200);
  }
}

// ============== SOCKET.IO CONNECTION ==============
void connectSocketIO() {
  Serial.println("[SOCKET.IO] Connecting...");
  
  HTTPClient http;
  String url = String("http://") + SERVER_HOST + ":" + String(SERVER_PORT) + "/socket.io/?EIO=4&transport=polling";
  
  http.begin(url);
  int httpCode = http.GET();
  
  if (httpCode == 200) {
    String payload = http.getString();
    Serial.printf("[HTTP] Handshake: %s\n", payload.c_str());
    
    int sidStart = payload.indexOf("\"sid\":\"") + 7;
    int sidEnd = payload.indexOf("\"", sidStart);
    
    if (sidStart > 7 && sidEnd > sidStart) {
      sessionId = payload.substring(sidStart, sidEnd);
      Serial.printf("[HTTP] Session ID: %s\n", sessionId.c_str());
      
      http.end();
      delay(100);
      
      // Connect to namespace
      http.begin(getPollingURL());
      http.addHeader("Content-Type", "text/plain");
      int postCode = http.POST("40");
      Serial.printf("[HTTP] Namespace connect: %d\n", postCode);
      http.end();
      
      socketIOConnected = true;
      Serial.println("[SOCKET.IO] ✓ CONNECTED!\n");
      beep(1, 50);
      
      delay(500);
      registerWithServer();
    }
  } else {
    Serial.printf("[HTTP] Connection failed: %d\n", httpCode);
  }
  http.end();
}

// ============== POLLING ==============
void pollMessages() {
  HTTPClient http;
  http.begin(getPollingURL());
  http.setTimeout(1000);
  
  int httpCode = http.GET();
  
  if (httpCode == 200) {
    String payload = http.getString();
    
    if (payload.length() > 2) {
      handleMessage(payload);
    }
  } else if (httpCode < 0) {
    Serial.println("[POLL] Connection lost, reconnecting...");
    socketIOConnected = false;
    sessionId = "";
    connectSocketIO();
  }
  
  http.end();
}

// ============== MESSAGE HANDLER ==============
void handleMessage(String msg) {
  // Parse Socket.IO event: 42["event_name",{...data...}]
  if (msg.startsWith("42[")) {
    int firstQuote = msg.indexOf('"');
    int secondQuote = msg.indexOf('"', firstQuote + 1);
    
    if (firstQuote != -1 && secondQuote != -1) {
      String eventName = msg.substring(firstQuote + 1, secondQuote);
      
      // Extract data
      int dataStart = msg.indexOf(',', secondQuote);
      int dataEnd = msg.lastIndexOf(']');
      
      if (dataStart != -1 && dataEnd != -1) {
        String dataStr = msg.substring(dataStart + 1, dataEnd);
        
        StaticJsonDocument<512> doc;
        if (deserializeJson(doc, dataStr) == DeserializationError::Ok) {
          
          if (eventName == "locker:unlock") {
            const char* targetRack = doc["rackId"];
            int doorIndex = findDoorByRackId(targetRack);
            
            if (doorIndex >= 0) {
              Serial.printf("\n[CMD] UNLOCK Door %d (%s)\n", doorIndex, targetRack);
              unlockDoor(doorIndex);
            }
          }
          else if (eventName == "locker:lock") {
            const char* targetRack = doc["rackId"];
            int doorIndex = findDoorByRackId(targetRack);
            
            if (doorIndex >= 0) {
              Serial.printf("\n[CMD] LOCK Door %d (%s)\n", doorIndex, targetRack);
              lockDoor(doorIndex);
            }
          }
          else if (eventName == "shelf:registered") {
            Serial.println("[REG] Server acknowledged registration!");
          }
        }
      }
    }
  }
}

// ============== SEND MESSAGE ==============
void sendMessage(String event, String data) {
  if (!socketIOConnected) return;
  
  String message = "42[\"" + event + "\"," + data + "]";
  
  HTTPClient http;
  http.begin(getPollingURL());
  http.addHeader("Content-Type", "text/plain");
  http.POST(message);
  http.end();
}

// ============== SEND PING ==============
void sendPing() {
  HTTPClient http;
  http.begin(getPollingURL());
  http.addHeader("Content-Type", "text/plain");
  http.POST("2");
  http.end();
}

// ============== REGISTER WITH SERVER ==============
void registerWithServer() {
  Serial.println("[REG] Registering shelf with server...");
  
  StaticJsonDocument<1024> doc;
  doc["shelfId"] = SHELF_ID;
  doc["type"] = "shelf-controller";
  doc["numDoors"] = NUM_DOORS;
  doc["ip"] = WiFi.localIP().toString();
  doc["rssi"] = WiFi.RSSI();
  
  JsonArray racks = doc.createNestedArray("racks");
  for (int i = 0; i < NUM_DOORS; i++) {
    JsonObject rack = racks.createNestedObject();
    rack["doorIndex"] = i;
    rack["rackId"] = RACK_IDS[i];
    rack["isLocked"] = doorLocked[i];
    rack["itemPresent"] = itemPresent[i];
  }
  
  String jsonStr;
  serializeJson(doc, jsonStr);
  
  sendMessage("shelf:register", jsonStr);
  Serial.println("[REG] Registration sent");
}

// ============== SEND HEARTBEAT ==============
void sendHeartbeat() {
  StaticJsonDocument<512> doc;
  doc["shelfId"] = SHELF_ID;
  doc["uptime"] = millis() / 1000;
  doc["rssi"] = WiFi.RSSI();
  doc["freeHeap"] = ESP.getFreeHeap();
  
  JsonArray doors = doc.createNestedArray("doors");
  for (int i = 0; i < NUM_DOORS; i++) {
    JsonObject door = doors.createNestedObject();
    door["rackId"] = RACK_IDS[i];
    door["locked"] = doorLocked[i];
    door["item"] = itemPresent[i];
  }
  
  String jsonStr;
  serializeJson(doc, jsonStr);
  sendMessage("shelf:heartbeat", jsonStr);
}

// ============== FIND DOOR BY RACK ID ==============
int findDoorByRackId(const char* rackId) {
  for (int i = 0; i < NUM_DOORS; i++) {
    if (strcmp(RACK_IDS[i], rackId) == 0) {
      return i;
    }
  }
  return -1;
}

// ============== UNLOCK DOOR ==============
void unlockDoor(int doorIndex) {
  if (doorIndex < 0 || doorIndex >= NUM_DOORS) return;
  if (!doorLocked[doorIndex]) return;
  
  Serial.printf("[UNLOCK] Door %d - GPIO %d → HIGH\n", doorIndex, RELAY_PINS[doorIndex]);
  
  digitalWrite(RELAY_PINS[doorIndex], UNLOCK_STATE);
  doorLocked[doorIndex] = false;
  unlockTime[doorIndex] = millis();
  
  beep(1, 200);
  sendDoorStatus(doorIndex);
  
  Serial.printf("[UNLOCK] Door %d unlocked! Auto-lock in %d seconds\n", doorIndex, UNLOCK_DURATION / 1000);
}

// ============== LOCK DOOR ==============
void lockDoor(int doorIndex) {
  if (doorIndex < 0 || doorIndex >= NUM_DOORS) return;
  if (doorLocked[doorIndex]) return;
  
  Serial.printf("[LOCK] Door %d - GPIO %d → LOW\n", doorIndex, RELAY_PINS[doorIndex]);
  
  digitalWrite(RELAY_PINS[doorIndex], LOCK_STATE);
  doorLocked[doorIndex] = true;
  
  beep(2, 100);
  sendDoorStatus(doorIndex);
}

// ============== SEND DOOR STATUS ==============
void sendDoorStatus(int doorIndex) {
  if (!socketIOConnected) return;
  
  StaticJsonDocument<256> doc;
  doc["shelfId"] = SHELF_ID;
  doc["rackId"] = RACK_IDS[doorIndex];
  doc["doorIndex"] = doorIndex;
  doc["isLocked"] = doorLocked[doorIndex];
  doc["itemPresent"] = itemPresent[doorIndex];
  
  String jsonStr;
  serializeJson(doc, jsonStr);
  sendMessage("esp:status", jsonStr);
}

// ============== SENSORS (if enabled) ==============
#if IR_SENSORS_ENABLED
void checkAllSensors() {
  for (int i = 0; i < NUM_DOORS; i++) {
    if (IR_SENSOR_PINS[i] < 0) continue;
    
    bool currentState = digitalRead(IR_SENSOR_PINS[i]) == LOW;
    
    if (currentState != lastItemState[i]) {
      if (!hasPendingChange[i] || pendingSensorState[i] != currentState) {
        hasPendingChange[i] = true;
        pendingSensorState[i] = currentState;
        lastSensorChange[i] = millis();
        continue;
      }
      
      if (millis() - lastSensorChange[i] >= SENSOR_DEBOUNCE_TIME) {
        hasPendingChange[i] = false;
        lastItemState[i] = currentState;
        itemPresent[i] = currentState;
        
        Serial.printf("[SENSOR] Door %d (%s): %s\n", i, RACK_IDS[i], itemPresent[i] ? "ITEM PRESENT" : "EMPTY");
        sendItemStatus(i);
      }
    } else {
      hasPendingChange[i] = false;
    }
  }
}

void sendItemStatus(int doorIndex) {
  if (!socketIOConnected) return;
  
  StaticJsonDocument<256> doc;
  doc["shelfId"] = SHELF_ID;
  doc["rackId"] = RACK_IDS[doorIndex];
  doc["doorIndex"] = doorIndex;
  doc["itemPresent"] = itemPresent[doorIndex];
  doc["needsRefill"] = !itemPresent[doorIndex];
  
  String jsonStr;
  serializeJson(doc, jsonStr);
  sendMessage("esp:itemStatus", jsonStr);
}
#endif

// ============== BUZZER ==============
void beep(int times, int duration) {
  for (int i = 0; i < times; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(duration);
    digitalWrite(BUZZER_PIN, LOW);
    if (i < times - 1) delay(duration);
  }
}

// ============== HELPERS ==============
String getPollingURL() {
  return String("http://") + SERVER_HOST + ":" + String(SERVER_PORT) + "/socket.io/?EIO=4&transport=polling&sid=" + sessionId;
}

// ============== SERIAL COMMANDS (same as before) ==============
void handleSerialCommands() {
  if (!Serial.available()) return;
  
  String cmd = Serial.readStringUntil('\n');
  cmd.trim();
  if (cmd.length() == 0) return;
  
  char action = cmd.charAt(0);
  
  if (action == 's' || action == 'S') {
    printStatus();
  }
  else if (action == 'u' || action == 'U') {
    if (cmd.length() > 1) {
      int doorIndex = cmd.substring(1).toInt();
      Serial.printf("\n>>> MANUAL UNLOCK Door %d <<<\n", doorIndex);
      unlockDoor(doorIndex);
    }
  }
  else if (action == 'l' || action == 'L') {
    if (cmd.length() > 1) {
      int doorIndex = cmd.substring(1).toInt();
      Serial.printf("\n>>> MANUAL LOCK Door %d <<<\n", doorIndex);
      lockDoor(doorIndex);
    }
  }
  else if (action == 't' || action == 'T') {
    testAllDoors();
  }
  else if (action == 'r' || action == 'R') {
    Serial.println("\n>>> RECONNECTING <<<");
    socketIOConnected = false;
    sessionId = "";
    connectSocketIO();
  }
  else if (action == 'h' || action == 'H') {
    printHelp();
  }
  else {
    Serial.println("Unknown command. Type 'h' for help.");
  }
}

void printStatus() {
  Serial.println("\n╔═══════════════════════════════════════════════════════════╗");
  Serial.println("║                    SHELF STATUS                           ║");
  Serial.println("╠═══════════════════════════════════════════════════════════╣");
  Serial.printf("║ Shelf ID:     %s                                    ║\n", SHELF_ID);
  Serial.printf("║ WiFi:         %s (RSSI: %d dBm)                     ║\n", WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected", WiFi.RSSI());
  Serial.printf("║ Socket.IO:    %s                                    ║\n", socketIOConnected ? "Connected" : "Disconnected");
  Serial.printf("║ Uptime:       %lu seconds                           ║\n", millis() / 1000);
  Serial.println("╠═══════════════════════════════════════════════════════════╣");
  Serial.println("║ Door  │ Rack ID   │ Status   │ Item    │ Relay Pin       ║");
  Serial.println("╠═══════════════════════════════════════════════════════════╣");
  
  for (int i = 0; i < NUM_DOORS; i++) {
    Serial.printf("║  %2d   │ %-9s │ %-8s │ %-7s │ GPIO %-2d         ║\n", i, RACK_IDS[i], doorLocked[i] ? "LOCKED" : "UNLOCKED", itemPresent[i] ? "Yes" : "No", RELAY_PINS[i]);
  }
  
  Serial.println("╚═══════════════════════════════════════════════════════════╝\n");
}

void printHelp() {
  Serial.println("\n╔════════════════════════════════════════════════════════╗");
  Serial.println("║                 SERIAL COMMANDS                        ║");
  Serial.println("╠════════════════════════════════════════════════════════╣");
  Serial.println("║  u<n>  - Unlock door n (e.g., u0, u5, u11)             ║");
  Serial.println("║  l<n>  - Lock door n (e.g., l0, l5)                    ║");
  Serial.println("║  s     - Print status of all doors                    ║");
  Serial.println("║  t     - Test all doors (unlock/lock cycle)           ║");
  Serial.println("║  r     - Reconnect Socket.IO                          ║");
  Serial.println("║  h     - Show this help                               ║");
  Serial.println("╚════════════════════════════════════════════════════════╝\n");
}

void testAllDoors() {
  Serial.println("\n>>> TESTING ALL DOORS <<<\n");
  for (int i = 0; i < NUM_DOORS; i++) {
    Serial.printf("Testing Door %d (%s)...\n", i, RACK_IDS[i]);
    digitalWrite(RELAY_PINS[i], UNLOCK_STATE);
    delay(500);
    digitalWrite(RELAY_PINS[i], LOCK_STATE);
    delay(200);
  }
  Serial.println("\n>>> ALL DOORS TESTED <<<\n");
  beep(2, 100);
}