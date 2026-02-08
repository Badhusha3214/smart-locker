/*
 * Smart QR-Controlled Locker System - ESP32 SHELF Controller
 * 
 * ═══════════════════════════════════════════════════════════════════════
 * MULTI-DOOR VERSION - 1 ESP32 controls 9-12 doors on a shelf
 * ═══════════════════════════════════════════════════════════════════════
 * 
 * Hardware Setup:
 * - Uses relay modules or shift registers to control multiple solenoids
 * - Each door has its own relay channel
 * - IR sensors for each locker to detect items
 * 
 * Pin Configuration (using direct GPIO - 12 doors max):
 * - Door 1:  GPIO 13 (Relay), GPIO 14 (IR Sensor)
 * - Door 2:  GPIO 27 (Relay), GPIO 26 (IR Sensor)
 * - Door 3:  GPIO 25 (Relay), GPIO 33 (IR Sensor)
 * - Door 4:  GPIO 32 (Relay), GPIO 35 (IR Sensor)
 * - Door 5:  GPIO 23 (Relay), GPIO 34 (IR Sensor)
 * - Door 6:  GPIO 22 (Relay), GPIO 39 (IR Sensor)
 * - Door 7:  GPIO 21 (Relay), GPIO 36 (IR Sensor)
 * - Door 8:  GPIO 19 (Relay)
 * - Door 9:  GPIO 18 (Relay)
 * - Door 10: GPIO 5  (Relay)
 * - Door 11: GPIO 4  (Relay)
 * - Door 12: GPIO 16 (Relay)
 * 
 * - Status LED: GPIO 2 (built-in)
 * - Buzzer: GPIO 15
 */

#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>

// ============== CONFIGURATION ==============
const char* WIFI_SSID = "Robot";
const char* WIFI_PASSWORD = "Robot123";
const char* SERVER_HOST = "35.225.93.34";  //10.229.233.58
const int SERVER_PORT = 5000;
const bool USE_SSL = false;

// Shelf Configuration
const char* SHELF_ID = "SHELF_001";     // Unique shelf identifier
const int NUM_DOORS = 9;                 // Number of doors on this shelf (9 or 12)

// Rack IDs for each door - MUST match database rackNumber!
const char* RACK_IDS[12] = {
  "RACK-001",  // Door 0
  "RACK-002",  // Door 1
  "RACK-003",  // Door 2
  "RACK-004",  // Door 3
  "RACK-005",  // Door 4
  "RACK-006",  // Door 5
  "RACK-007",  // Door 6
  "RACK-008",  // Door 7
  "RACK-009",  // Door 8
  "RACK-010",  // Door 9
  "RACK-011",  // Door 10
  "RACK-012"   // Door 11
};

// Relay pins for each door
const int RELAY_PINS[12] = {
  13,  // Door 0
  27,  // Door 1
  25,  // Door 2
  32,  // Door 3
  23,  // Door 4
  22,  // Door 5
  21,  // Door 6
  19,  // Door 7
  18,  // Door 8
  5,   // Door 9
  4,   // Door 10
  16   // Door 11
};

// ============== IR SENSOR CONFIGURATION ==============
// Set to false to disable all IR sensors (for future use)
#define IR_SENSORS_ENABLED false

// IR Sensor pins (if using sensors - set to -1 if not connected)
// Uncomment IR_SENSORS_ENABLED = true above when ready to use
const int IR_SENSOR_PINS[12] = {
  14,  // Door 0
  26,  // Door 1
  33,  // Door 2
  35,  // Door 3
  34,  // Door 4
  39,  // Door 5
  36,  // Door 6
  -1,  // Door 7 (no sensor)
  -1,  // Door 8 (no sensor)
  -1,  // Door 9 (no sensor)
  -1,  // Door 10 (no sensor)
  -1   // Door 11 (no sensor)
};

// Common pins
#define LED_PIN 2
#define BUZZER_PIN 15

// Solenoid States
#define UNLOCK_STATE HIGH
#define LOCK_STATE LOW

// Timing
#define UNLOCK_DURATION 30000        // Auto-lock after 30 seconds
#define SENSOR_CHECK_INTERVAL 1000   // Check sensors every 1 second (only if IR_SENSORS_ENABLED)
#define RECONNECT_INTERVAL 5000
#define HEARTBEAT_INTERVAL 30000
#define SENSOR_DEBOUNCE_TIME 500

// ============== GLOBAL VARIABLES ==============
WebSocketsClient webSocket;

// Door states
bool doorLocked[12];          // Lock state for each door
bool itemPresent[12];         // Item present in each locker
bool lastItemState[12];       // For debouncing
unsigned long unlockTime[12]; // When each door was unlocked
unsigned long lastSensorChange[12];
bool hasPendingChange[12];
bool pendingSensorState[12];

// Connection states
bool isConnected = false;
bool socketIOConnected = false;

// Timing
unsigned long lastSensorCheck = 0;
unsigned long lastHeartbeat = 0;

// ============== SETUP ==============
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  printBanner();
  
  // Initialize all doors
  initializeHardware();
  
  // Connect to WiFi
  connectWiFi();
  
  // Connect to WebSocket server
  connectWebSocket();
  
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
  Serial.println("\n");
  Serial.println("╔════════════════════════════════════════════════════════════╗");
  Serial.println("║     SMART LOCKER SYSTEM - SHELF CONTROLLER                 ║");
  Serial.println("║     MULTI-DOOR VERSION                                     ║");
  Serial.println("╠════════════════════════════════════════════════════════════╣");
  Serial.printf("║  Shelf ID:    %s                                     ║\n", SHELF_ID);
  Serial.printf("║  Doors:       %d                                            ║\n", NUM_DOORS);
  Serial.println("╠════════════════════════════════════════════════════════════╣");
  Serial.println("║  Commands: u<n>=Unlock door n, l<n>=Lock, s=Status         ║");
  Serial.println("╚════════════════════════════════════════════════════════════╝");
  Serial.println();
}

void initializeHardware() {
  Serial.println("Initializing hardware...");
  
  // Initialize LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // Initialize Buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  
  // Initialize each door
  for (int i = 0; i < NUM_DOORS; i++) {
    // Relay pin
    pinMode(RELAY_PINS[i], OUTPUT);
    digitalWrite(RELAY_PINS[i], LOCK_STATE);
    doorLocked[i] = true;
    unlockTime[i] = 0;
    hasPendingChange[i] = false;
    
    Serial.printf("  Door %d: Relay GPIO %d", i, RELAY_PINS[i]);
    
    // IR Sensor pin (if available and enabled)
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
  webSocket.loop();
  
  // Check WiFi
  if (WiFi.status() != WL_CONNECTED) {
    isConnected = false;
    connectWiFi();
    connectWebSocket();
  }
  
  // Auto-lock doors after timeout
  for (int i = 0; i < NUM_DOORS; i++) {
    if (!doorLocked[i] && millis() - unlockTime[i] > UNLOCK_DURATION) {
      Serial.printf("[AUTO-LOCK] Door %d (%s)\n", i, RACK_IDS[i]);
      lockDoor(i);
    }
  }
  
  // Check sensors (only if IR sensors are enabled)
  #if IR_SENSORS_ENABLED
  if (millis() - lastSensorCheck > SENSOR_CHECK_INTERVAL) {
    lastSensorCheck = millis();
    checkAllSensors();
  }
  #endif
  
  // Heartbeat
  if (isConnected && socketIOConnected && millis() - lastHeartbeat > HEARTBEAT_INTERVAL) {
    lastHeartbeat = millis();
    sendHeartbeat();
  }
  
  // LED status
  if (isConnected) {
    digitalWrite(LED_PIN, (millis() / 1000) % 2);
  } else {
    digitalWrite(LED_PIN, (millis() / 200) % 2);
  }
  
  // Serial commands
  handleSerialCommands();
}

// ============== WIFI CONNECTION ==============
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
    Serial.printf("\n[WIFI] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
    beep(1, 100);
  } else {
    Serial.println("\n[WIFI] Connection failed!");
    beep(3, 200);
  }
}

// ============== WEBSOCKET CONNECTION ==============
void connectWebSocket() {
  Serial.printf("[WS] Connecting to %s:%d\n", SERVER_HOST, SERVER_PORT);
  
String wsPath = "/socket.io/?EIO=4&transport=websocket";

  if (USE_SSL) {
    webSocket.beginSSL(SERVER_HOST, 443, wsPath.c_str());
  } else {
    webSocket.begin(SERVER_HOST, SERVER_PORT, wsPath.c_str());
  }
  
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(RECONNECT_INTERVAL);
  webSocket.enableHeartbeat(25000, 5000, 2);
}

// ============== WEBSOCKET EVENT HANDLER ==============
void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.println("[WS] Disconnected");
      isConnected = false;
      socketIOConnected = false;
      break;
      
    case WStype_CONNECTED:
      Serial.println("[WS] Connected!");
      isConnected = true;
      socketIOConnected = false;
      break;
      
    case WStype_TEXT:
      handleMessage((char*)payload);
      break;
      
    default:
      break;
  }
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
  
  // Add all rack IDs this shelf controls
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
  
  String message = "42[\"shelf:register\"," + jsonStr + "]";
  webSocket.sendTXT(message);
  
  Serial.println("[REG] Registration sent");
}

// ============== SEND HEARTBEAT ==============
void sendHeartbeat() {
  StaticJsonDocument<512> doc;
  doc["shelfId"] = SHELF_ID;
  doc["uptime"] = millis() / 1000;
  doc["rssi"] = WiFi.RSSI();
  doc["freeHeap"] = ESP.getFreeHeap();
  
  // Include all door states
  JsonArray doors = doc.createNestedArray("doors");
  for (int i = 0; i < NUM_DOORS; i++) {
    JsonObject door = doors.createNestedObject();
    door["rackId"] = RACK_IDS[i];
    door["locked"] = doorLocked[i];
    door["item"] = itemPresent[i];
  }
  
  String jsonStr;
  serializeJson(doc, jsonStr);
  
  String message = "42[\"shelf:heartbeat\"," + jsonStr + "]";
  webSocket.sendTXT(message);
}

// ============== HANDLE INCOMING MESSAGES ==============
void handleMessage(char* payload) {
  String msg = String(payload);
  
  // Socket.IO handshake
  if (msg.startsWith("0")) {
    Serial.println("[WS] Handshake received, connecting to namespace...");
    webSocket.sendTXT("40");
    return;
  }
  
  // Namespace connected
  if (msg.startsWith("40")) {
    Serial.println("[WS] Namespace connected!");
    socketIOConnected = true;
    beep(1, 50);
    registerWithServer();
    return;
  }
  
  // Ping/Pong
  if (msg == "2") {
    webSocket.sendTXT("3");
    return;
  }
  
  // Socket.IO events
  if (!msg.startsWith("42")) return;
  
  // Parse event name
  int firstQuote = msg.indexOf('"');
  int secondQuote = msg.indexOf('"', firstQuote + 1);
  if (firstQuote == -1 || secondQuote == -1) return;
  
  String eventName = msg.substring(firstQuote + 1, secondQuote);
  
  // Parse data
  int dataStart = msg.indexOf(',', secondQuote);
  int dataEnd = msg.lastIndexOf(']');
  
  if (eventName == "locker:unlock") {
    if (dataStart != -1 && dataEnd != -1) {
      String dataStr = msg.substring(dataStart + 1, dataEnd);
      
      StaticJsonDocument<256> doc;
      if (deserializeJson(doc, dataStr) == DeserializationError::Ok) {
        const char* targetRack = doc["rackId"];
        
        // Find which door this rack belongs to
        int doorIndex = findDoorByRackId(targetRack);
        
        if (doorIndex >= 0) {
          Serial.printf("\n[CMD] UNLOCK Door %d (%s)\n", doorIndex, targetRack);
          unlockDoor(doorIndex);
        } else {
          Serial.printf("[CMD] Rack %s not on this shelf\n", targetRack);
        }
      }
    }
  }
  else if (eventName == "locker:lock") {
    if (dataStart != -1 && dataEnd != -1) {
      String dataStr = msg.substring(dataStart + 1, dataEnd);
      
      StaticJsonDocument<256> doc;
      if (deserializeJson(doc, dataStr) == DeserializationError::Ok) {
        const char* targetRack = doc["rackId"];
        
        int doorIndex = findDoorByRackId(targetRack);
        
        if (doorIndex >= 0) {
          Serial.printf("\n[CMD] LOCK Door %d (%s)\n", doorIndex, targetRack);
          lockDoor(doorIndex);
        }
      }
    }
  }
  else if (eventName == "shelf:registered") {
    Serial.println("[REG] Server acknowledged registration!");
  }
}

// ============== FIND DOOR BY RACK ID ==============
int findDoorByRackId(const char* rackId) {
  for (int i = 0; i < NUM_DOORS; i++) {
    if (strcmp(RACK_IDS[i], rackId) == 0) {
      return i;
    }
  }
  return -1;  // Not found
}

// ============== UNLOCK DOOR ==============
void unlockDoor(int doorIndex) {
  if (doorIndex < 0 || doorIndex >= NUM_DOORS) {
    Serial.printf("[ERROR] Invalid door index: %d\n", doorIndex);
    return;
  }
  
  if (!doorLocked[doorIndex]) {
    Serial.printf("[WARN] Door %d already unlocked\n", doorIndex);
    return;
  }
  
  Serial.printf("[UNLOCK] Door %d - GPIO %d → HIGH\n", doorIndex, RELAY_PINS[doorIndex]);
  
  digitalWrite(RELAY_PINS[doorIndex], UNLOCK_STATE);
  doorLocked[doorIndex] = false;
  unlockTime[doorIndex] = millis();
  
  beep(1, 200);
  
  // Notify server
  sendDoorStatus(doorIndex);
  
  Serial.printf("[UNLOCK] Door %d unlocked! Auto-lock in %d seconds\n", doorIndex, UNLOCK_DURATION / 1000);
}

// ============== LOCK DOOR ==============
void lockDoor(int doorIndex) {
  if (doorIndex < 0 || doorIndex >= NUM_DOORS) {
    Serial.printf("[ERROR] Invalid door index: %d\n", doorIndex);
    return;
  }
  
  if (doorLocked[doorIndex]) {
    Serial.printf("[WARN] Door %d already locked\n", doorIndex);
    return;
  }
  
  Serial.printf("[LOCK] Door %d - GPIO %d → LOW\n", doorIndex, RELAY_PINS[doorIndex]);
  
  digitalWrite(RELAY_PINS[doorIndex], LOCK_STATE);
  doorLocked[doorIndex] = true;
  
  beep(2, 100);
  
  // Notify server
  sendDoorStatus(doorIndex);
  
  Serial.printf("[LOCK] Door %d locked!\n", doorIndex);
}

// ============== SEND DOOR STATUS ==============
void sendDoorStatus(int doorIndex) {
  if (!isConnected || !socketIOConnected) return;
  
  StaticJsonDocument<256> doc;
  doc["shelfId"] = SHELF_ID;
  doc["rackId"] = RACK_IDS[doorIndex];
  doc["doorIndex"] = doorIndex;
  doc["isLocked"] = doorLocked[doorIndex];
  doc["itemPresent"] = itemPresent[doorIndex];
  
  String jsonStr;
  serializeJson(doc, jsonStr);
  
  String message = "42[\"esp:status\"," + jsonStr + "]";
  webSocket.sendTXT(message);
  
  Serial.printf("[STATUS] Sent status for door %d\n", doorIndex);
}

// ============== CHECK ALL SENSORS ==============
// Only compiled if IR_SENSORS_ENABLED is true
#if IR_SENSORS_ENABLED
void checkAllSensors() {
  for (int i = 0; i < NUM_DOORS; i++) {
    if (IR_SENSOR_PINS[i] < 0) continue;  // Skip if no sensor
    
    bool currentState = digitalRead(IR_SENSOR_PINS[i]) == LOW;
    
    // Debounce logic
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
        
        Serial.printf("[SENSOR] Door %d (%s): %s\n", 
          i, RACK_IDS[i], 
          itemPresent[i] ? "ITEM PRESENT" : "EMPTY");
        
        sendItemStatus(i);
      }
    } else {
      hasPendingChange[i] = false;
    }
  }
}
#endif

// ============== SEND ITEM STATUS ==============
// Only compiled if IR_SENSORS_ENABLED is true
#if IR_SENSORS_ENABLED
void sendItemStatus(int doorIndex) {
  if (!isConnected || !socketIOConnected) return;
  
  StaticJsonDocument<256> doc;
  doc["shelfId"] = SHELF_ID;
  doc["rackId"] = RACK_IDS[doorIndex];
  doc["doorIndex"] = doorIndex;
  doc["itemPresent"] = itemPresent[doorIndex];
  doc["needsRefill"] = !itemPresent[doorIndex];
  
  String jsonStr;
  serializeJson(doc, jsonStr);
  
  String message = "42[\"esp:itemStatus\"," + jsonStr + "]";
  webSocket.sendTXT(message);
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

// ============== SERIAL COMMANDS ==============
void handleSerialCommands() {
  if (!Serial.available()) return;
  
  String cmd = Serial.readStringUntil('\n');
  cmd.trim();
  
  if (cmd.length() == 0) return;
  
  char action = cmd.charAt(0);
  
  if (action == 's' || action == 'S') {
    // Print status
    printStatus();
  }
  else if (action == 'u' || action == 'U') {
    // Unlock: u0, u1, u2, ... or U0, U1, ...
    if (cmd.length() > 1) {
      int doorIndex = cmd.substring(1).toInt();
      Serial.printf("\n>>> MANUAL UNLOCK Door %d <<<\n", doorIndex);
      unlockDoor(doorIndex);
    } else {
      Serial.println("Usage: u<door_number> (e.g., u0, u5)");
    }
  }
  else if (action == 'l' || action == 'L') {
    // Lock: l0, l1, l2, ...
    if (cmd.length() > 1) {
      int doorIndex = cmd.substring(1).toInt();
      Serial.printf("\n>>> MANUAL LOCK Door %d <<<\n", doorIndex);
      lockDoor(doorIndex);
    } else {
      Serial.println("Usage: l<door_number> (e.g., l0, l5)");
    }
  }
  else if (action == 't' || action == 'T') {
    // Test all doors
    testAllDoors();
  }
  else if (action == 'r' || action == 'R') {
    // Reconnect
    Serial.println("\n>>> RECONNECTING <<<");
    webSocket.disconnect();
    delay(1000);
    connectWebSocket();
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
  Serial.printf("║ WiFi:         %s (RSSI: %d dBm)                     ║\n", 
    WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected", WiFi.RSSI());
  Serial.printf("║ WebSocket:    %s                                    ║\n", 
    socketIOConnected ? "Connected" : "Disconnected");
  Serial.printf("║ Uptime:       %lu seconds                           ║\n", millis() / 1000);
  Serial.println("╠═══════════════════════════════════════════════════════════╣");
  Serial.println("║ Door  │ Rack ID   │ Status   │ Item    │ Relay Pin       ║");
  Serial.println("╠═══════════════════════════════════════════════════════════╣");
  
  for (int i = 0; i < NUM_DOORS; i++) {
    Serial.printf("║  %2d   │ %-9s │ %-8s │ %-7s │ GPIO %-2d         ║\n",
      i,
      RACK_IDS[i],
      doorLocked[i] ? "LOCKED" : "UNLOCKED",
      itemPresent[i] ? "Yes" : "No",
      RELAY_PINS[i]
    );
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
  Serial.println("║  r     - Reconnect WebSocket                          ║");
  Serial.println("║  h     - Show this help                               ║");
  Serial.println("╚════════════════════════════════════════════════════════╝\n");
}

void testAllDoors() {
  Serial.println("\n>>> TESTING ALL DOORS <<<\n");
  
  for (int i = 0; i < NUM_DOORS; i++) {
    Serial.printf("Testing Door %d (%s)...\n", i, RACK_IDS[i]);
    
    // Unlock
    digitalWrite(RELAY_PINS[i], UNLOCK_STATE);
    delay(500);
    
    // Lock
    digitalWrite(RELAY_PINS[i], LOCK_STATE);
    delay(200);
  }
  
  Serial.println("\n>>> ALL DOORS TESTED <<<\n");
  beep(2, 100);
}