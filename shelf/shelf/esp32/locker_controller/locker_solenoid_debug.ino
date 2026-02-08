/*
 * Smart QR-Controlled Locker System - ESP32 Controller (SOLENOID VERSION)
 * 
 * ═══════════════════════════════════════════════════════════════════════
 * DEBUG VERSION - Enhanced logging for troubleshooting
 * ═══════════════════════════════════════════════════════════════════════
 * 
 * Hardware Connections:
 * - Relay Module IN: GPIO 13 (controls solenoid)
 * - IR Sensor (item detection): GPIO 14
 * - Status LED: GPIO 2 (built-in)
 * - Buzzer: GPIO 15 (optional)
 * 
 * Serial Commands for Testing:
 * - 'u' or 'U' : Unlock
 * - 'l' or 'L' : Lock
 * - 's' or 'S' : Print status
 * - 'r' or 'R' : Reconnect WebSocket
 * - 'd' or 'D' : Toggle debug mode
 * - 'h' or 'H' : Show help
 * - 't' or 'T' : Test all hardware
 */

#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>

// ============== DEBUG CONFIGURATION ==============
#define DEBUG_ENABLED true       // Master debug switch
#define DEBUG_WIFI true          // WiFi connection logs
#define DEBUG_WEBSOCKET true     // WebSocket message logs
#define DEBUG_SENSOR true        // Sensor reading logs
#define DEBUG_RELAY true         // Relay/lock state logs
#define DEBUG_HEARTBEAT false    // Heartbeat logs (can be noisy)
#define DEBUG_TIMING true        // Timing/performance logs

// Debug log levels
#define LOG_ERROR   0
#define LOG_WARN    1
#define LOG_INFO    2
#define LOG_DEBUG   3
#define LOG_VERBOSE 4

int currentLogLevel = LOG_DEBUG;  // Change to control verbosity

// ============== CONFIGURATION ==============
const char* WIFI_SSID = "KERALA_VISION_UCV_12";
const char* WIFI_PASSWORD = "Shajee@1423";
const char* SERVER_HOST = "192.168.1.6";
const int SERVER_PORT = 5000;
const bool USE_SSL = false;
const char* RACK_ID = "RACK-001";  // Must match rackNumber in database!

// Hardware Pins
#define RELAY_PIN 13
// #define IR_SENSOR_PIN 14  // Commented out - IR sensor not currently used
#define LED_PIN 2
#define BUZZER_PIN 15

// Solenoid States
#define UNLOCK_STATE HIGH
#define LOCK_STATE LOW

// Timing
#define UNLOCK_DURATION 30000     // Keep unlocked for 30 seconds
// #define SENSOR_CHECK_INTERVAL 1000  // Commented out - IR sensor not currently used
#define RECONNECT_INTERVAL 5000
#define HEARTBEAT_INTERVAL 30000
#define STATUS_PRINT_INTERVAL 10000  // Print status every 10s in debug

// ============== GLOBAL VARIABLES ==============
WebSocketsClient webSocket;

bool isConnected = false;
bool socketIOConnected = false;  // Socket.IO namespace connected
bool isLocked = true;
// IR Sensor variables - commented out, not currently used
// bool itemPresent = false;
// bool lastItemState = false;
bool itemPresent = false;  // Keep for API compatibility, always false
bool debugMode = DEBUG_ENABLED;

unsigned long unlockTime = 0;
// unsigned long lastSensorCheck = 0;  // Commented out - IR sensor not currently used
unsigned long lastHeartbeat = 0;
unsigned long lastStatusPrint = 0;
unsigned long bootTime = 0;
// IR Sensor debounce - commented out, not currently used
// unsigned long lastSensorChange = 0;  // For debouncing
// #define SENSOR_DEBOUNCE_TIME 500  // 500ms debounce
// bool pendingSensorState = false;
// bool hasPendingChange = false;

// Statistics for debugging
unsigned long wsMessagesReceived = 0;
unsigned long wsMessagesSent = 0;
unsigned long wifiReconnects = 0;
unsigned long wsReconnects = 0;
unsigned long lockUnlockCycles = 0;
unsigned long sensorChanges = 0;

// ============== DEBUG LOGGING FUNCTIONS ==============
void logPrint(int level, const char* tag, String message) {
  if (!debugMode || level > currentLogLevel) return;
  
  String prefix = "";
  switch(level) {
    case LOG_ERROR:   prefix = "[ERROR]"; break;
    case LOG_WARN:    prefix = "[WARN] "; break;
    case LOG_INFO:    prefix = "[INFO] "; break;
    case LOG_DEBUG:   prefix = "[DEBUG]"; break;
    case LOG_VERBOSE: prefix = "[VERB] "; break;
  }
  
  unsigned long uptime = millis() / 1000;
  Serial.printf("%s [%lus] [%s] %s\n", prefix.c_str(), uptime, tag, message.c_str());
}

void logError(const char* tag, String msg) { logPrint(LOG_ERROR, tag, msg); }
void logWarn(const char* tag, String msg) { logPrint(LOG_WARN, tag, msg); }
void logInfo(const char* tag, String msg) { logPrint(LOG_INFO, tag, msg); }
void logDebug(const char* tag, String msg) { logPrint(LOG_DEBUG, tag, msg); }
void logVerbose(const char* tag, String msg) { logPrint(LOG_VERBOSE, tag, msg); }

// ============== SETUP ==============
void setup() {
  Serial.begin(115200);
  delay(1000);
  bootTime = millis();
  
  printBanner();
  printSystemInfo();
  
  // Initialize pins with debug output
  initializeHardware();
  
  // Connect to WiFi
  connectWiFi();
  
  // Connect to WebSocket server
  connectWebSocket();
  
  // Initial sensor read - commented out, IR sensor not currently used
  // itemPresent = digitalRead(IR_SENSOR_PIN) == LOW;
  // lastItemState = itemPresent;
  // logInfo("SENSOR", "Initial state: " + String(itemPresent ? "ITEM PRESENT" : "EMPTY"));
  itemPresent = false;  // Default to false when IR sensor not used
  
  // Startup complete
  beep(2, 100);
  printStartupComplete();
}

void printBanner() {
  Serial.println("\n");
  Serial.println("╔════════════════════════════════════════════════════════════╗");
  Serial.println("║     SMART LOCKER SYSTEM - ESP32 CONTROLLER                 ║");
  Serial.println("║     SOLENOID LOCK VERSION - DEBUG MODE                     ║");
  Serial.println("╠════════════════════════════════════════════════════════════╣");
  Serial.println("║  Serial Commands:                                          ║");
  Serial.println("║    u=Unlock  l=Lock  s=Status  r=Reconnect                 ║");
  Serial.println("║    d=Debug toggle  h=Help  t=Test hardware                 ║");
  Serial.println("╚════════════════════════════════════════════════════════════╝");
  Serial.println();
}

void printSystemInfo() {
  Serial.println("┌─────────────────── SYSTEM INFO ───────────────────┐");
  Serial.printf("│ Chip Model:     %s                          │\n", ESP.getChipModel());
  Serial.printf("│ Chip Revision:  %d                                  │\n", ESP.getChipRevision());
  Serial.printf("│ CPU Frequency:  %d MHz                             │\n", ESP.getCpuFreqMHz());
  Serial.printf("│ Flash Size:     %d MB                               │\n", ESP.getFlashChipSize() / 1024 / 1024);
  Serial.printf("│ Free Heap:      %d bytes                       │\n", ESP.getFreeHeap());
  Serial.printf("│ SDK Version:    %s                          │\n", ESP.getSdkVersion());
  Serial.println("├───────────────────────────────────────────────────┤");
  Serial.printf("│ Rack ID:        %s                            │\n", RACK_ID);
  Serial.printf("│ Server:         %s:%d                    │\n", SERVER_HOST, SERVER_PORT);
  Serial.printf("│ SSL Enabled:    %s                                │\n", USE_SSL ? "Yes" : "No");
  Serial.println("└───────────────────────────────────────────────────┘\n");
}

void initializeHardware() {
  Serial.println("┌─────────────── HARDWARE INITIALIZATION ───────────────┐");
  
  // Relay Pin
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOCK_STATE);
  isLocked = true;
  Serial.printf("│ ✓ RELAY_PIN (GPIO %d) - OUTPUT - Set to LOCK_STATE    │\n", RELAY_PIN);
  
  // LED Pin
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  Serial.printf("│ ✓ LED_PIN (GPIO %d) - OUTPUT - Set to LOW             │\n", LED_PIN);
  
  // Buzzer Pin
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  Serial.printf("│ ✓ BUZZER_PIN (GPIO %d) - OUTPUT - Set to LOW          │\n", BUZZER_PIN);
  
  // IR Sensor Pin - commented out, not currently used
  // pinMode(IR_SENSOR_PIN, INPUT);
  // int sensorReading = digitalRead(IR_SENSOR_PIN);
  // Serial.printf("│ ✓ IR_SENSOR_PIN (GPIO %d) - INPUT - Reading: %d        │\n", IR_SENSOR_PIN, sensorReading);
  Serial.println("│ ✗ IR_SENSOR - DISABLED (not currently used)           │");
  
  Serial.println("└────────────────────────────────────────────────────────┘\n");
}

void printStartupComplete() {
  unsigned long startupTime = millis() - bootTime;
  Serial.println("\n┌─────────────── STARTUP COMPLETE ───────────────┐");
  Serial.printf("│ Startup Time:    %lu ms                          \n", startupTime);
  Serial.printf("│ WiFi Connected:  %s                             \n", WiFi.status() == WL_CONNECTED ? "Yes" : "No");
  Serial.printf("│ WebSocket:       %s                         \n", isConnected ? "Connected" : "Connecting...");
  Serial.printf("│ Lock State:      %s                          \n", isLocked ? "LOCKED" : "UNLOCKED");
  Serial.printf("│ Item Present:    %s                             \n", itemPresent ? "Yes" : "No");
  Serial.println("└─────────────────────────────────────────────────┘\n");
  Serial.println(">>> System ready. Waiting for commands...\n");
}

// ============== MAIN LOOP ==============
void loop() {
  unsigned long loopStart = millis();
  
  // Handle WebSocket events
  webSocket.loop();
  
  // Check WiFi connection
  checkWiFiConnection();
  
  // Auto-lock after timeout
  checkAutoLock();
  
  // Check item sensor periodically - commented out, IR sensor not currently used
  // if (millis() - lastSensorCheck > SENSOR_CHECK_INTERVAL) {
  //   lastSensorCheck = millis();
  //   checkItemSensor();
  // }
  
  // Send heartbeat to server
  if (isConnected && socketIOConnected && millis() - lastHeartbeat > HEARTBEAT_INTERVAL) {
    lastHeartbeat = millis();
    sendHeartbeat();
  }
  
  // Debug: Print periodic status
  if (debugMode && millis() - lastStatusPrint > STATUS_PRINT_INTERVAL) {
    lastStatusPrint = millis();
    printPeriodicStatus();
  }
  
  // Update LED status
  updateLED();
  
  // Handle serial commands
  handleSerialCommands();
  
  // Debug: Log slow loop iterations
  if (DEBUG_TIMING) {
    unsigned long loopTime = millis() - loopStart;
    if (loopTime > 100) {
      logWarn("LOOP", "Slow loop iteration: " + String(loopTime) + "ms");
    }
  }
}

void checkWiFiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    if (DEBUG_WIFI) {
      logWarn("WIFI", "Connection lost! Reconnecting...");
    }
    isConnected = false;
    wifiReconnects++;
    connectWiFi();
    connectWebSocket();
  }
}

void checkAutoLock() {
  if (!isLocked && millis() - unlockTime > UNLOCK_DURATION) {
    unsigned long unlockedFor = millis() - unlockTime;
    logInfo("LOCK", "Auto-locking after " + String(unlockedFor / 1000) + " seconds");
    lockLocker();
  }
}

void updateLED() {
  if (isConnected) {
    // Slow blink when connected
    digitalWrite(LED_PIN, (millis() / 1000) % 2);
  } else {
    // Fast blink when disconnected
    digitalWrite(LED_PIN, (millis() / 200) % 2);
  }
}

void printPeriodicStatus() {
  unsigned long uptime = millis() / 1000;
  Serial.println("\n┌─────────────── PERIODIC STATUS ───────────────┐");
  Serial.printf("│ Uptime:          %lu seconds                    \n", uptime);
  Serial.printf("│ WiFi RSSI:       %d dBm                         \n", WiFi.RSSI());
  Serial.printf("│ Free Heap:       %d bytes                       \n", ESP.getFreeHeap());
  Serial.printf("│ Lock State:      %s                             \n", isLocked ? "LOCKED" : "UNLOCKED");
  Serial.printf("│ Item Present:    %s                             \n", itemPresent ? "Yes" : "No");
  Serial.printf("│ WS Connected:    %s                             \n", isConnected ? "Yes" : "No");
  Serial.printf("│ Socket.IO:       %s                             \n", socketIOConnected ? "Yes" : "No");
  Serial.println("├─────────────── STATISTICS ─────────────────────┤");
  Serial.printf("│ WS Messages Rx:  %lu                            \n", wsMessagesReceived);
  Serial.printf("│ WS Messages Tx:  %lu                            \n", wsMessagesSent);
  Serial.printf("│ WiFi Reconnects: %lu                            \n", wifiReconnects);
  Serial.printf("│ WS Reconnects:   %lu                            \n", wsReconnects);
  Serial.printf("│ Lock/Unlock:     %lu cycles                     \n", lockUnlockCycles);
  Serial.printf("│ Sensor Changes:  %lu                            \n", sensorChanges);
  Serial.println("└────────────────────────────────────────────────┘\n");
}

// ============== WIFI CONNECTION ==============
void connectWiFi() {
  logInfo("WIFI", "Connecting to: " + String(WIFI_SSID));
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  int maxAttempts = 40;
  
  Serial.print("[WIFI] Connecting");
  while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
    delay(500);
    Serial.print(".");
    attempts++;
    digitalWrite(LED_PIN, attempts % 2);
    
    // Debug: Print WiFi status codes
    if (DEBUG_WIFI && attempts % 10 == 0) {
      Serial.printf("\n[WIFI] Status: %d (attempt %d/%d)", WiFi.status(), attempts, maxAttempts);
    }
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    logInfo("WIFI", "✓ Connected successfully!");
    logInfo("WIFI", "IP Address: " + WiFi.localIP().toString());
    logInfo("WIFI", "Subnet Mask: " + WiFi.subnetMask().toString());
    logInfo("WIFI", "Gateway: " + WiFi.gatewayIP().toString());
    logInfo("WIFI", "DNS: " + WiFi.dnsIP().toString());
    logInfo("WIFI", "RSSI: " + String(WiFi.RSSI()) + " dBm");
    logInfo("WIFI", "MAC Address: " + WiFi.macAddress());
    
    digitalWrite(LED_PIN, HIGH);
    beep(1, 100);
  } else {
    Serial.println();
    logError("WIFI", "✗ Connection FAILED!");
    logError("WIFI", "Status code: " + String(WiFi.status()));
    logError("WIFI", "Possible causes:");
    logError("WIFI", "  - Wrong SSID or password");
    logError("WIFI", "  - WiFi network out of range");
    logError("WIFI", "  - Router not responding");
    
    beep(3, 200);
  }
}

// ============== WEBSOCKET CONNECTION ==============
void connectWebSocket() {
  logInfo("WS", "Connecting to WebSocket server...");
  logInfo("WS", "Host: " + String(SERVER_HOST));
  logInfo("WS", "Port: " + String(SERVER_PORT));
  logInfo("WS", "SSL: " + String(USE_SSL ? "Enabled" : "Disabled"));
  
  // Use transport=websocket to skip polling upgrade
  String wsPath = "/socket.io/?EIO=4&transport=websocket";
  logDebug("WS", "Path: " + wsPath);
  
  if (USE_SSL) {
    webSocket.beginSSL(SERVER_HOST, 443, wsPath.c_str());
    logInfo("WS", "Using SSL connection on port 443");
  } else {
    webSocket.begin(SERVER_HOST, SERVER_PORT, wsPath.c_str());
  }
  
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(RECONNECT_INTERVAL);
  webSocket.enableHeartbeat(25000, 5000, 2);  // ping every 25s, timeout 5s, 2 retries
  logDebug("WS", "Reconnect interval: " + String(RECONNECT_INTERVAL) + "ms");
}

// ============== WEBSOCKET EVENT HANDLER ==============
void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      logWarn("WS", "✗ DISCONNECTED");
      isConnected = false;
      socketIOConnected = false;
      wsReconnects++;
      digitalWrite(LED_PIN, LOW);
      break;
      
    case WStype_CONNECTED:
      logInfo("WS", "✓ WebSocket CONNECTED to server!");
      logDebug("WS", "Connection URL: " + String((char*)payload));
      isConnected = true;
      socketIOConnected = false;  // Wait for Socket.IO handshake
      digitalWrite(LED_PIN, HIGH);
      // Don't register yet - wait for Socket.IO handshake
      break;
      
    case WStype_TEXT:
      wsMessagesReceived++;
      if (DEBUG_WEBSOCKET) {
        logDebug("WS", "← RX [" + String(length) + " bytes]: " + String((char*)payload));
      }
      handleMessage((char*)payload);
      break;
      
    case WStype_BIN:
      logDebug("WS", "← RX Binary [" + String(length) + " bytes]");
      break;
      
    case WStype_ERROR:
      logError("WS", "✗ ERROR occurred");
      break;
      
    case WStype_FRAGMENT_TEXT_START:
      logDebug("WS", "Fragment text start");
      break;
      
    case WStype_FRAGMENT_BIN_START:
      logDebug("WS", "Fragment binary start");
      break;
      
    case WStype_FRAGMENT:
      logDebug("WS", "Fragment received");
      break;
      
    case WStype_FRAGMENT_FIN:
      logDebug("WS", "Fragment finished");
      break;
      
    case WStype_PING:
      logVerbose("WS", "← PING");
      break;
      
    case WStype_PONG:
      logVerbose("WS", "→ PONG");
      break;
      
    default:
      logWarn("WS", "Unknown event type: " + String(type));
      break;
  }
}

// ============== REGISTER WITH SERVER ==============
void registerWithServer() {
  logInfo("REG", "Registering with server...");
  
  StaticJsonDocument<512> doc;
  doc["rackId"] = RACK_ID;
  doc["type"] = "esp32";
  doc["lockType"] = "solenoid";
  doc["status"] = isLocked ? "locked" : "unlocked";
  doc["itemPresent"] = itemPresent;
  doc["firmwareVersion"] = "1.1.0-debug";
  doc["ip"] = WiFi.localIP().toString();
  doc["rssi"] = WiFi.RSSI();
  doc["freeHeap"] = ESP.getFreeHeap();
  doc["chipModel"] = ESP.getChipModel();
  
  String jsonStr;
  serializeJson(doc, jsonStr);
  
  String message = "42[\"esp:register\"," + jsonStr + "]";
  webSocket.sendTXT(message);
  wsMessagesSent++;
  
  logInfo("REG", "→ Registration sent");
  if (DEBUG_WEBSOCKET) {
    logDebug("REG", "Payload: " + jsonStr);
  }
}

// ============== SEND HEARTBEAT ==============
void sendHeartbeat() {
  if (!isConnected) return;
  
  StaticJsonDocument<256> doc;
  doc["rackId"] = RACK_ID;
  doc["uptime"] = millis() / 1000;
  doc["rssi"] = WiFi.RSSI();
  doc["freeHeap"] = ESP.getFreeHeap();
  doc["isLocked"] = isLocked;
  doc["itemPresent"] = itemPresent;
  
  String jsonStr;
  serializeJson(doc, jsonStr);
  
  String message = "42[\"esp:heartbeat\"," + jsonStr + "]";
  webSocket.sendTXT(message);
  wsMessagesSent++;
  
  if (DEBUG_HEARTBEAT) {
    logDebug("BEAT", "→ Heartbeat sent");
  }
}

// ============== HANDLE INCOMING MESSAGES ==============
void handleMessage(char* payload) {
  String msg = String(payload);
  
  // Handle Socket.IO protocol messages
  if (msg.startsWith("0")) {
    // Engine.IO handshake - extract ping interval
    logInfo("WS", "Engine.IO handshake received");
    logDebug("WS", "Sending Socket.IO connect (40)...");
    webSocket.sendTXT("40");  // Connect to default namespace
    wsMessagesSent++;
    return;
  }
  
  if (msg.startsWith("40")) {
    // Socket.IO connected to namespace - NOW we can register!
    logInfo("WS", "✓ Socket.IO namespace connected!");
    socketIOConnected = true;
    beep(1, 50);
    registerWithServer();
    return;
  }
  
  if (msg == "2") {
    // Engine.IO ping
    webSocket.sendTXT("3");  // Pong
    logVerbose("WS", "← Ping, → Pong");
    return;
  }
  
  if (!msg.startsWith("42")) {
    logDebug("WS", "Non-event message: " + msg.substring(0, 20));
    return;
  }
  
  logInfo("MSG", "Processing Socket.IO event");
  
  // Parse Socket.IO event
  int firstBracket = msg.indexOf('[');
  int firstQuote = msg.indexOf('"', firstBracket);
  int secondQuote = msg.indexOf('"', firstQuote + 1);
  
  if (firstQuote == -1 || secondQuote == -1) {
    logWarn("MSG", "Invalid message format");
    return;
  }
  
  String eventName = msg.substring(firstQuote + 1, secondQuote);
  logInfo("MSG", "Event: " + eventName);
  
  int dataStart = msg.indexOf(',', secondQuote);
  int dataEnd = msg.lastIndexOf(']');
  
  if (eventName == "locker:unlock") {
    logInfo("CMD", "═══════════════════════════════════════");
    logInfo("CMD", "  UNLOCK COMMAND RECEIVED");
    logInfo("CMD", "═══════════════════════════════════════");
    
    if (dataStart != -1 && dataEnd != -1) {
      String dataStr = msg.substring(dataStart + 1, dataEnd);
      logDebug("CMD", "Data: " + dataStr);
      
      StaticJsonDocument<256> doc;
      DeserializationError error = deserializeJson(doc, dataStr);
      
      if (error) {
        logError("CMD", "JSON parse error: " + String(error.c_str()));
        return;
      }
      
      const char* targetRack = doc["rackId"];
      logDebug("CMD", "Target rack: " + String(targetRack));
      logDebug("CMD", "This rack: " + String(RACK_ID));
      
      if (String(targetRack) == String(RACK_ID)) {
        logInfo("CMD", "✓ Command is for this rack - EXECUTING UNLOCK");
        unlockLocker();
      } else {
        logWarn("CMD", "✗ Command is for different rack - IGNORING");
      }
    }
  }
  else if (eventName == "locker:lock") {
    logInfo("CMD", "═══════════════════════════════════════");
    logInfo("CMD", "  LOCK COMMAND RECEIVED");
    logInfo("CMD", "═══════════════════════════════════════");
    
    if (dataStart != -1 && dataEnd != -1) {
      String dataStr = msg.substring(dataStart + 1, dataEnd);
      
      StaticJsonDocument<256> doc;
      DeserializationError error = deserializeJson(doc, dataStr);
      
      if (!error) {
        const char* targetRack = doc["rackId"];
        if (String(targetRack) == String(RACK_ID)) {
          logInfo("CMD", "✓ Executing LOCK");
          lockLocker();
        }
      }
    }
  }
  else if (eventName == "locker:status") {
    logInfo("CMD", "Status request received");
    sendStatusUpdate();
  }
  else if (eventName == "esp:registered") {
    logInfo("REG", "✓ Server acknowledged registration!");
  }
  else {
    logDebug("CMD", "Unknown event: " + eventName);
  }
}

// ============== UNLOCK LOCKER ==============
void unlockLocker() {
  logInfo("LOCK", "┌─────────────────────────────────────┐");
  logInfo("LOCK", "│         UNLOCKING SOLENOID          │");
  logInfo("LOCK", "└─────────────────────────────────────┘");
  
  if (!isLocked) {
    logWarn("LOCK", "Already unlocked - ignoring command");
    return;
  }
  
  logDebug("RELAY", "Setting GPIO " + String(RELAY_PIN) + " to " + String(UNLOCK_STATE));
  
  // Activate relay
  digitalWrite(RELAY_PIN, UNLOCK_STATE);
  isLocked = false;
  unlockTime = millis();
  lockUnlockCycles++;
  
  // Verify relay state
  int relayState = digitalRead(RELAY_PIN);
  logDebug("RELAY", "Relay pin state after write: " + String(relayState));
  
  // Feedback
  digitalWrite(LED_PIN, HIGH);
  beep(1, 200);
  
  // Notify server
  sendStatusUpdate();
  
  logInfo("LOCK", "✓ UNLOCKED successfully");
  logInfo("LOCK", "Will auto-lock in " + String(UNLOCK_DURATION / 1000) + " seconds");
}

// ============== LOCK LOCKER ==============
void lockLocker() {
  logInfo("LOCK", "┌─────────────────────────────────────┐");
  logInfo("LOCK", "│          LOCKING SOLENOID           │");
  logInfo("LOCK", "└─────────────────────────────────────┘");
  
  if (isLocked) {
    logWarn("LOCK", "Already locked - ignoring command");
    return;
  }
  
  unsigned long unlockedDuration = millis() - unlockTime;
  logDebug("LOCK", "Was unlocked for: " + String(unlockedDuration / 1000) + " seconds");
  
  logDebug("RELAY", "Setting GPIO " + String(RELAY_PIN) + " to " + String(LOCK_STATE));
  
  // Deactivate relay
  digitalWrite(RELAY_PIN, LOCK_STATE);
  isLocked = true;
  
  // Verify relay state
  int relayState = digitalRead(RELAY_PIN);
  logDebug("RELAY", "Relay pin state after write: " + String(relayState));
  
  // Feedback
  beep(2, 100);
  
  // Notify server
  sendStatusUpdate();
  
  logInfo("LOCK", "✓ LOCKED successfully");
}

// ============== CHECK ITEM SENSOR (WITH DEBOUNCE) ==============
// Commented out - IR sensor not currently used. Uncomment for future use.
/*
void checkItemSensor() {
  int rawReading = digitalRead(IR_SENSOR_PIN);
  bool currentState = rawReading == LOW;  // LOW = object detected
  
  if (DEBUG_SENSOR) {
    logVerbose("SENSOR", "Raw reading: " + String(rawReading) + " (" + String(currentState ? "PRESENT" : "EMPTY") + ")");
  }
  
  // Check if state is different from last confirmed state
  if (currentState != lastItemState) {
    // If this is a new potential change, start debounce timer
    if (!hasPendingChange || pendingSensorState != currentState) {
      hasPendingChange = true;
      pendingSensorState = currentState;
      lastSensorChange = millis();
      logDebug("SENSOR", "Potential change detected, waiting for debounce...");
      return;
    }
    
    // Check if debounce time has passed
    if (millis() - lastSensorChange >= SENSOR_DEBOUNCE_TIME) {
      // State is stable - confirm the change
      hasPendingChange = false;
      sensorChanges++;
      lastItemState = currentState;
      itemPresent = currentState;
      
      logInfo("SENSOR", "════════════════════════════════════");
      logInfo("SENSOR", "  STATE CHANGE CONFIRMED (debounced)");
      logInfo("SENSOR", "  New state: " + String(itemPresent ? "ITEM PRESENT" : "EMPTY"));
      logInfo("SENSOR", "════════════════════════════════════");
      
      sendItemStatusUpdate();
    }
  } else {
    // State matches confirmed state - cancel any pending change
    if (hasPendingChange) {
      logDebug("SENSOR", "State reverted before debounce - ignoring bounce");
      hasPendingChange = false;
    }
  }
}
*/

// ============== SEND STATUS UPDATE ==============
void sendStatusUpdate() {
  if (!isConnected) {
    logWarn("STATUS", "Cannot send - not connected");
    return;
  }
  
  StaticJsonDocument<256> doc;
  doc["rackId"] = RACK_ID;
  doc["isLocked"] = isLocked;
  doc["itemPresent"] = itemPresent;
  doc["timestamp"] = millis();
  
  String jsonStr;
  serializeJson(doc, jsonStr);
  
  String message = "42[\"esp:status\"," + jsonStr + "]";
  webSocket.sendTXT(message);
  wsMessagesSent++;
  
  logInfo("STATUS", "→ Status update sent");
  logDebug("STATUS", "Payload: " + jsonStr);
}

// ============== SEND ITEM STATUS UPDATE ==============
// Commented out - IR sensor not currently used. Uncomment for future use.
/*
void sendItemStatusUpdate() {
  if (!isConnected) {
    logWarn("ITEM", "Cannot send - not connected");
    return;
  }
  
  StaticJsonDocument<256> doc;
  doc["rackId"] = RACK_ID;
  doc["itemPresent"] = itemPresent;
  doc["needsRefill"] = !itemPresent;
  
  String jsonStr;
  serializeJson(doc, jsonStr);
  
  String message = "42[\"esp:itemStatus\"," + jsonStr + "]";
  webSocket.sendTXT(message);
  wsMessagesSent++;
  
  logInfo("ITEM", "→ Item status sent: " + String(itemPresent ? "present" : "needs refill"));
}
*/

// ============== BUZZER ==============
void beep(int times, int duration) {
  if (DEBUG_RELAY) {
    logDebug("BUZZER", "Beeping " + String(times) + " times, " + String(duration) + "ms each");
  }
  
  for (int i = 0; i < times; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(duration);
    digitalWrite(BUZZER_PIN, LOW);
    if (i < times - 1) delay(duration);
  }
}

// ============== SERIAL COMMAND HANDLER ==============
void handleSerialCommands() {
  while (Serial.available()) {
    char c = Serial.read();
    
    switch (c) {
      case 'u':
      case 'U':
        Serial.println("\n>>> MANUAL UNLOCK COMMAND <<<");
        unlockLocker();
        break;
        
      case 'l':
      case 'L':
        Serial.println("\n>>> MANUAL LOCK COMMAND <<<");
        lockLocker();
        break;
        
      case 's':
      case 'S':
        printFullStatus();
        break;
        
      case 'r':
      case 'R':
        Serial.println("\n>>> MANUAL RECONNECT <<<");
        webSocket.disconnect();
        delay(1000);
        connectWebSocket();
        break;
        
      case 'd':
      case 'D':
        debugMode = !debugMode;
        Serial.println("\n>>> DEBUG MODE: " + String(debugMode ? "ENABLED" : "DISABLED") + " <<<");
        break;
        
      case 'h':
      case 'H':
        printHelp();
        break;
        
      case 't':
      case 'T':
        testHardware();
        break;
        
      case '+':
        if (currentLogLevel < LOG_VERBOSE) {
          currentLogLevel++;
          Serial.println("\n>>> Log level increased to: " + String(currentLogLevel));
        }
        break;
        
      case '-':
        if (currentLogLevel > LOG_ERROR) {
          currentLogLevel--;
          Serial.println("\n>>> Log level decreased to: " + String(currentLogLevel));
        }
        break;
        
      case '\n':
      case '\r':
        // Ignore newlines
        break;
        
      default:
        Serial.println("\n>>> Unknown command: '" + String(c) + "' - Press 'h' for help");
        break;
    }
  }
}

void printFullStatus() {
  unsigned long uptime = millis() / 1000;
  
  Serial.println("\n╔════════════════════════════════════════════════════════╗");
  Serial.println("║              FULL SYSTEM STATUS                        ║");
  Serial.println("╠════════════════════════════════════════════════════════╣");
  Serial.printf("║ Uptime:            %lu seconds                          \n", uptime);
  Serial.printf("║ Free Heap:         %d bytes                             \n", ESP.getFreeHeap());
  Serial.printf("║ Min Free Heap:     %d bytes                             \n", ESP.getMinFreeHeap());
  Serial.println("╠═══════════════════ NETWORK ═════════════════════════════╣");
  Serial.printf("║ WiFi Status:       %s                                   \n", WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");
  Serial.printf("║ IP Address:        %s                                   \n", WiFi.localIP().toString().c_str());
  Serial.printf("║ RSSI:              %d dBm                               \n", WiFi.RSSI());
  Serial.printf("║ WebSocket:         %s                                   \n", isConnected ? "Connected" : "Disconnected");
  Serial.println("╠═══════════════════ HARDWARE ════════════════════════════╣");
  Serial.printf("║ Lock State:        %s                                   \n", isLocked ? "LOCKED" : "UNLOCKED");
  Serial.printf("║ Relay Pin:         GPIO %d = %d                          \n", RELAY_PIN, digitalRead(RELAY_PIN));
  Serial.printf("║ IR Sensor:         GPIO %d = %d                          \n", IR_SENSOR_PIN, digitalRead(IR_SENSOR_PIN));
  Serial.printf("║ Item Present:      %s                                   \n", itemPresent ? "Yes" : "No");
  Serial.println("╠═══════════════════ STATISTICS ══════════════════════════╣");
  Serial.printf("║ WS Messages Rx:    %lu                                  \n", wsMessagesReceived);
  Serial.printf("║ WS Messages Tx:    %lu                                  \n", wsMessagesSent);
  Serial.printf("║ WiFi Reconnects:   %lu                                  \n", wifiReconnects);
  Serial.printf("║ WS Reconnects:     %lu                                  \n", wsReconnects);
  Serial.printf("║ Lock/Unlock:       %lu cycles                           \n", lockUnlockCycles);
  Serial.printf("║ Sensor Changes:    %lu                                  \n", sensorChanges);
  Serial.println("╚════════════════════════════════════════════════════════╝\n");
}

void printHelp() {
  Serial.println("\n╔════════════════════════════════════════════════════════╗");
  Serial.println("║              SERIAL COMMANDS HELP                      ║");
  Serial.println("╠════════════════════════════════════════════════════════╣");
  Serial.println("║  u/U  - Unlock solenoid                                ║");
  Serial.println("║  l/L  - Lock solenoid                                  ║");
  Serial.println("║  s/S  - Print full status                              ║");
  Serial.println("║  r/R  - Reconnect WebSocket                            ║");
  Serial.println("║  d/D  - Toggle debug mode                              ║");
  Serial.println("║  t/T  - Test all hardware                              ║");
  Serial.println("║  h/H  - Show this help                                 ║");
  Serial.println("║  +    - Increase log verbosity                         ║");
  Serial.println("║  -    - Decrease log verbosity                         ║");
  Serial.println("╚════════════════════════════════════════════════════════╝\n");
}

void testHardware() {
  Serial.println("\n╔════════════════════════════════════════════════════════╗");
  Serial.println("║              HARDWARE TEST SEQUENCE                    ║");
  Serial.println("╚════════════════════════════════════════════════════════╝\n");
  
  // Test LED
  Serial.println("1. Testing LED...");
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }
  Serial.println("   ✓ LED test complete\n");
  
  // Test Buzzer
  Serial.println("2. Testing Buzzer...");
  beep(3, 150);
  Serial.println("   ✓ Buzzer test complete\n");
  
  // Test Relay (unlock/lock cycle)
  Serial.println("3. Testing Relay/Solenoid...");
  Serial.println("   - Unlocking for 2 seconds...");
  digitalWrite(RELAY_PIN, UNLOCK_STATE);
  delay(2000);
  Serial.println("   - Locking...");
  digitalWrite(RELAY_PIN, LOCK_STATE);
  Serial.println("   ✓ Relay test complete\n");
  
  // Test IR Sensor
  Serial.println("4. Testing IR Sensor...");
  Serial.printf("   Current reading: %d\n", digitalRead(IR_SENSOR_PIN));
  Serial.printf("   Interpreted as: %s\n", digitalRead(IR_SENSOR_PIN) == LOW ? "ITEM PRESENT" : "EMPTY");
  Serial.println("   ✓ Sensor test complete\n");
  
  Serial.println("═══════════════════════════════════════════════════════════");
  Serial.println("  ALL HARDWARE TESTS COMPLETE");
  Serial.println("═══════════════════════════════════════════════════════════\n");
}
