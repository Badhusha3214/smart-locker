# ESP32 Smart Locker Controller

This folder contains the ESP32 firmware for controlling the physical locker hardware.

## 🔧 Hardware Options

### Option A: Solenoid Lock (Recommended)
Best for security - electromagnet-based lock mechanism.

### Option B: Servo Motor Lock
Good for prototyping - mechanical rotation-based lock.

---

## Hardware Requirements

### For Solenoid Lock Setup:

| Component | Quantity | Description |
|-----------|----------|-------------|
| ESP32 DevKit | 1 | Main microcontroller |
| Solenoid Lock (12V) | 1 | Electric lock mechanism |
| 5V Relay Module | 1 | Controls solenoid power |
| 12V Power Supply | 1 | Powers the solenoid |
| IR Sensor (FC-51) | 1 | Item detection |
| Flyback Diode (1N4007) | 1 | Protects relay from solenoid kickback |
| Buzzer (optional) | 1 | Audio feedback |

### For Servo Motor Setup:

| Component | Quantity | Description |
|-----------|----------|-------------|
| ESP32 DevKit | 1 | Main microcontroller |
| SG90 Servo Motor | 1 | Lock mechanism |
| IR Sensor (FC-51) | 1 | Item detection |
| 5V Power Supply | 1 | Powers ESP32 and servo |
| Buzzer (optional) | 1 | Audio feedback |

---

## 📐 Wiring Diagrams

### Solenoid Lock Wiring (Recommended):

```
                         12V Power Supply
                              │
                              │ (+)
                              ▼
    ┌─────────────────────────┴─────────────────────┐
    │                                               │
    │    ┌──────────────────────┐                   │
    │    │   5V Relay Module    │                   │
    │    │                      │                   │
    │    │  COM ────────────────┼───► Solenoid (+)  │
    │    │  NO  ◄───────────────┼─── 12V (+)        │
    │    │  NC  (not used)      │                   │
    │    │                      │                   │
    │    │  VCC ◄─── ESP32 VIN  │                   │
    │    │  GND ◄─── ESP32 GND ─┼─── 12V GND ───────┘
    │    │  IN  ◄─── ESP32 GPIO13                   
    │    └──────────────────────┘                   
    │                                               
    │    ┌──────────────────────┐                   
    │    │   Solenoid Lock      │                   
    │    │   ┌────────────┐     │     1N4007 Diode
    │    │   │ (+)    (-) │     │    ──┬──►|──┬──
    │    │   └──┬─────┬───┘     │      │      │
    │    │      │     │         │      │      │
    │    │      │     └─────────┼──────┴──────┘
    │    │      │               │     (Flyback protection)
    │    │      └───────────────┼─► From Relay COM
    │    └──────────────────────┘
    │
    │         ESP32 DevKit
    │    ┌─────────────────────┐
    │    │                     │
    │    │ GPIO 13 ────────────│──► Relay IN
    │    │ GPIO 14 ────────────│──► IR Sensor OUT
    │    │ GPIO 15 ────────────│──► Buzzer (+)
    │    │ GPIO 2  ────────────│──► Built-in LED
    │    │                     │
    │    │ VIN (5V) ───────────│──► Relay VCC
    │    │ GND     ────────────│──► Common Ground
    │    │ 3.3V    ────────────│──► IR Sensor VCC
    │    └─────────────────────┘
    │
    │    IR Sensor (FC-51)
    │    ┌──────────────┐
    │    │ OUT ─────────│──► GPIO 14
    │    │ VCC ─────────│──► 3.3V
    │    │ GND ─────────│──► GND
    │    └──────────────┘
    │
    └─────────────────────────────────────────────────
```

### Servo Motor Wiring:

```
                    ESP32 DevKit V1
                    ┌─────────────────┐
                    │                 │
    Servo Signal ───│ GPIO 13         │
    IR Sensor OUT ──│ GPIO 14         │
    Buzzer (+) ─────│ GPIO 15         │
    Built-in LED ───│ GPIO 2          │
                    │                 │
                    │ 3.3V ───────────│──── IR Sensor VCC
                    │ GND  ───────────│──── IR Sensor GND
                    │                 │     Servo GND
                    │ VIN (5V) ───────│──── Servo VCC (Red)
                    │                 │
                    └─────────────────┘

    Servo Motor (SG90)
    ┌─────────────┐
    │ Orange ─────│ Signal → GPIO 13
    │ Red    ─────│ VCC → 5V (VIN)
    │ Brown  ─────│ GND → GND
    └─────────────┘
```

---

## ⚡ Solenoid Lock Types

### Fail-Secure (Normally Locked) - RECOMMENDED
- **Default State**: LOCKED (no power)
- **Powered State**: UNLOCKED
- **Advantage**: If power fails, locker stays locked (secure)
- **Use Case**: Security lockers, storage

### Fail-Safe (Normally Unlocked)
- **Default State**: UNLOCKED (no power)  
- **Powered State**: LOCKED
- **Advantage**: If power fails, door can be opened (safety)
- **Use Case**: Emergency exits

**Our code uses Fail-Secure by default.**

## Software Setup

### 1. Install Arduino IDE
Download from: https://www.arduino.cc/en/software

### 2. Add ESP32 Board Support
1. Open Arduino IDE → File → Preferences
2. Add to "Additional Board Manager URLs":
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Go to Tools → Board → Board Manager
4. Search "ESP32" and install "ESP32 by Espressif Systems"

### 3. Install Required Libraries
Go to Tools → Manage Libraries and install:
- **WebSockets** by Markus Sattler (for Socket.IO)
- **ArduinoJson** by Benoit Blanchon
- **ESP32Servo** by Kevin Harrington

### 4. Configure the Code
Open `locker_controller.ino` and update these values:

```cpp
// WiFi Credentials
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// Backend Server (your computer's IP on local network)
const char* SERVER_HOST = "192.168.1.100";  // Change to your server IP
const int SERVER_PORT = 5000;

// Rack Configuration
const char* RACK_ID = "RACK_001";  // Must match your rack's rackNumber in MongoDB
```

### 5. Find Your Computer's IP
Run in terminal:
- **Windows**: `ipconfig` (look for IPv4 Address)
- **Mac/Linux**: `ifconfig` or `ip addr`

### 6. Upload to ESP32
1. Connect ESP32 via USB
2. Select board: Tools → Board → ESP32 Dev Module
3. Select port: Tools → Port → COM# (your ESP32 port)
4. Click Upload button

## How It Works

```
┌──────────────┐         ┌──────────────┐         ┌──────────────┐
│  Vue.js App  │         │   Backend    │         │    ESP32     │
│   (Phone)    │         │  (Node.js)   │         │  (Hardware)  │
└──────┬───────┘         └──────┬───────┘         └──────┬───────┘
       │                        │                        │
       │ 1. Scan QR Code        │                        │
       │───────────────────────>│                        │
       │                        │                        │
       │ 2. Make Payment        │                        │
       │───────────────────────>│                        │
       │                        │                        │
       │                        │ 3. locker:unlock       │
       │                        │───────────────────────>│
       │                        │      (WebSocket)       │
       │                        │                        │ 4. Servo Unlocks
       │                        │                        │────────┐
       │                        │                        │<───────┘
       │                        │                        │
       │                        │ 5. esp:status          │
       │                        │<───────────────────────│
       │                        │                        │
       │ 6. Unlock Confirmed    │                        │
       │<───────────────────────│                        │
       │                        │                        │
       │                        │         (10 sec)       │
       │                        │                        │ 7. Auto-lock
       │                        │                        │────────┐
       │                        │                        │<───────┘
       │                        │                        │
       │                        │ 8. esp:status (locked) │
       │                        │<───────────────────────│
       │                        │                        │
       │ 9. Rack Locked         │                        │
       │<───────────────────────│                        │
       │                        │                        │
```

## WebSocket Events

### ESP32 → Server
| Event | Data | Description |
|-------|------|-------------|
| `esp:register` | `{ rackId, type, status, itemPresent }` | Register ESP32 with server |
| `esp:status` | `{ rackId, isLocked, itemPresent }` | Lock status update |
| `esp:itemStatus` | `{ rackId, itemPresent, needsRefill }` | Item sensor update |

### Server → ESP32
| Event | Data | Description |
|-------|------|-------------|
| `locker:unlock` | `{ rackId }` | Command to unlock |
| `locker:lock` | `{ rackId }` | Command to lock |
| `locker:status` | `{}` | Request status update |

## Servo Lock Mechanism

The servo motor acts as the lock mechanism:

```
LOCKED (0°)                    UNLOCKED (90°)
┌─────────────┐               ┌─────────────┐
│ ┌─────┐     │               │     ┌─────┐ │
│ │     │ ◄───│ Lock bar      │ ───►│     │ │ Lock bar retracted
│ │     │     │ blocks door   │     │     │ │ door can open
│ └─────┘     │               │     └─────┘ │
│      ◄──────│ Door cannot   │ ───────►    │ Door can open
└─────────────┘ open          └─────────────┘
```

## Item Detection

The IR sensor detects if an item is present in the locker:

```
IR Sensor
    │
    ▼
┌───────────────────┐
│                   │
│   ┌─────────┐     │  Object Detected → itemPresent = true
│   │  Item   │ ◄───│  
│   └─────────┘     │
│                   │
└───────────────────┘

┌───────────────────┐
│                   │
│                   │  No Object → itemPresent = false
│      Empty    ◄───│  → needsRefill = true
│                   │
└───────────────────┘
```

## Testing

1. **Serial Monitor**: Open Arduino Serial Monitor (115200 baud) to see debug logs
2. **Check Connection**: ESP32 should show "WebSocket Connected!" 
3. **Test from Admin Panel**: Go to admin panel and try unlocking a rack
4. **Check Backend Logs**: Backend will show ESP32 connection and events

---

## 🐛 Debug Version

Use `locker_solenoid_debug.ino` for comprehensive debugging. This version includes:

### Serial Commands for Testing

| Command | Action |
|---------|--------|
| `u` | Unlock solenoid |
| `l` | Lock solenoid |
| `s` | Print full status |
| `r` | Reconnect WebSocket |
| `d` | Toggle debug mode |
| `t` | Test all hardware |
| `h` | Show help |
| `+` | Increase log verbosity |
| `-` | Decrease log verbosity |

### Debug Output Example

```
╔════════════════════════════════════════════════════════════╗
║     SMART LOCKER SYSTEM - ESP32 CONTROLLER                 ║
║     SOLENOID LOCK VERSION - DEBUG MODE                     ║
╚════════════════════════════════════════════════════════════╝

┌─────────────────── SYSTEM INFO ───────────────────┐
│ Chip Model:     ESP32-D0WDQ6                      │
│ CPU Frequency:  240 MHz                           │
│ Free Heap:      285432 bytes                      │
│ Rack ID:        RACK_001                          │
│ Server:         192.168.1.100:5000                │
└───────────────────────────────────────────────────┘

[INFO] [42s] [WIFI] ✓ Connected successfully!
[INFO] [42s] [WIFI] IP Address: 192.168.1.105
[INFO] [42s] [WIFI] RSSI: -45 dBm
[INFO] [43s] [WS] ✓ CONNECTED to server!
[INFO] [43s] [REG] → Registration sent

[INFO] [58s] [CMD] ═══════════════════════════════════════
[INFO] [58s] [CMD]   UNLOCK COMMAND RECEIVED
[INFO] [58s] [CMD] ═══════════════════════════════════════
[DEBUG] [58s] [RELAY] Setting GPIO 13 to 1
[INFO] [58s] [LOCK] ✓ UNLOCKED successfully
```

### Log Levels

| Level | Description |
|-------|-------------|
| `LOG_ERROR` (0) | Errors only |
| `LOG_WARN` (1) | Warnings and errors |
| `LOG_INFO` (2) | General information |
| `LOG_DEBUG` (3) | Detailed debugging (default) |
| `LOG_VERBOSE` (4) | Everything including sensor readings |

### Debug Configuration

```cpp
// In locker_solenoid_debug.ino
#define DEBUG_ENABLED true       // Master debug switch
#define DEBUG_WIFI true          // WiFi connection logs
#define DEBUG_WEBSOCKET true     // WebSocket message logs
#define DEBUG_SENSOR true        // Sensor reading logs
#define DEBUG_RELAY true         // Relay/lock state logs
#define DEBUG_HEARTBEAT false    // Heartbeat logs (can be noisy)
#define DEBUG_TIMING true        // Timing/performance logs

int currentLogLevel = LOG_DEBUG;  // Default verbosity
```

### Hardware Test Mode

Press `t` in Serial Monitor to run hardware test:

```
╔════════════════════════════════════════════════════════╗
║              HARDWARE TEST SEQUENCE                    ║
╚════════════════════════════════════════════════════════╝

1. Testing LED...
   ✓ LED test complete

2. Testing Buzzer...
   ✓ Buzzer test complete

3. Testing Relay/Solenoid...
   - Unlocking for 2 seconds...
   - Locking...
   ✓ Relay test complete

4. Testing IR Sensor...
   Current reading: 1
   Interpreted as: EMPTY
   ✓ Sensor test complete

═══════════════════════════════════════════════════════════
  ALL HARDWARE TESTS COMPLETE
═══════════════════════════════════════════════════════════
```

### Statistics Tracking

The debug version tracks:
- WebSocket messages sent/received
- WiFi reconnection count
- WebSocket reconnection count
- Lock/unlock cycles
- Sensor state changes

Press `s` to see all statistics.

---

## Troubleshooting

| Issue | Solution |
|-------|----------|
| WiFi won't connect | Check SSID/password, ensure 2.4GHz network |
| WebSocket won't connect | Check server IP, ensure backend is running |
| Servo doesn't move | Check wiring, ensure 5V power supply |
| Solenoid doesn't unlock | Check 12V power, relay wiring, flyback diode |
| IR sensor always HIGH/LOW | Adjust sensitivity potentiometer on sensor |
| ESP32 keeps resetting | Check power supply (needs 500mA+) |
| WebSocket disconnects | Check WiFi signal strength (RSSI > -70 dBm) |

### Common WiFi Status Codes

| Code | Meaning |
|------|---------|
| 0 | `WL_IDLE_STATUS` - Idle |
| 1 | `WL_NO_SSID_AVAIL` - SSID not found |
| 2 | `WL_SCAN_COMPLETED` - Scan done |
| 3 | `WL_CONNECTED` - Connected! |
| 4 | `WL_CONNECT_FAILED` - Connection failed |
| 5 | `WL_CONNECTION_LOST` - Connection lost |
| 6 | `WL_DISCONNECTED` - Disconnected |

---

## Multiple Racks Setup

For multiple racks, you need one ESP32 per rack. Each ESP32 should have a unique `RACK_ID`:

```cpp
// ESP32 #1
const char* RACK_ID = "RACK_001";

// ESP32 #2
const char* RACK_ID = "RACK_002";

// ESP32 #3
const char* RACK_ID = "RACK_003";
```

Make sure the `RACK_ID` matches the `rackNumber` in your MongoDB database.
