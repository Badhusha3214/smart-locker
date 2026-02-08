/**
 * ESP32 Service
 * Handles communication with ESP32 microcontroller for lock/unlock operations
 * 
 * Communication Methods:
 * 1. WebSocket (Socket.IO) - Primary method for real-time bidirectional communication
 * 2. HTTP REST API - Fallback method for direct ESP32 communication
 * 
 * ESP32 WebSocket Events:
 * - locker:unlock - Unlock the solenoid/servo lock
 * - locker:lock - Lock the solenoid/servo lock
 * - locker:status - Request status update
 */

let ioInstance = null;

class ESP32Service {
  constructor(io = null) {
    this.io = io || ioInstance;
    this.timeout = 5000;
  }

  // Set the Socket.IO instance (called from server.js)
  static setIO(io) {
    ioInstance = io;
  }

  /**
   * Send unlock command to ESP32 via WebSocket
   * ESP32 will:
   * 1. Activate servo to unlock position
   * 2. Update LED status
   * 3. Send confirmation back
   */
  async unlockRack(rackId, rackNumber) {
    try {
      console.log(`[ESP32] Sending unlock command for rack ${rackId} (${rackNumber})`);

      // Use WebSocket if available
      if (this.io && this.io.unlockLocker) {
        // Check if ESP32 is connected
        if (this.io.isEspConnected && this.io.isEspConnected(rackNumber)) {
          this.io.unlockLocker(rackNumber);
          console.log(`[ESP32] WebSocket unlock command sent to ${rackNumber}`);
          return { success: true, data: { message: 'Unlock command sent via WebSocket' } };
        } else {
          console.log(`[ESP32] ESP32 not connected for ${rackNumber}, simulating...`);
        }
      }

      // In development/testing, simulate successful unlock
      if (process.env.NODE_ENV === 'development') {
        console.log(`[ESP32] Simulated unlock for rack ${rackId}`);
        return { success: true, data: { message: 'Simulated unlock (no ESP32 connected)' } };
      }

      return { success: false, error: 'ESP32 not connected' };
    } catch (error) {
      console.error(`[ESP32] Unlock error for rack ${rackId}:`, error.message);
      
      // In development, still return success for testing
      if (process.env.NODE_ENV === 'development') {
        return { success: true, data: { message: 'Simulated unlock (ESP32 unavailable)' } };
      }
      
      return { success: false, error: error.message };
    }
  }

  /**
   * Send lock command to ESP32 via WebSocket
   * ESP32 will:
   * 1. Move servo to lock position
   * 2. Update LED to green
   * 3. Send confirmation back
   */
  async lockRack(rackId, rackNumber) {
    try {
      console.log(`[ESP32] Sending lock command for rack ${rackId} (${rackNumber})`);

      // Use WebSocket if available
      if (this.io && this.io.lockLocker) {
        if (this.io.isEspConnected && this.io.isEspConnected(rackNumber)) {
          this.io.lockLocker(rackNumber);
          console.log(`[ESP32] WebSocket lock command sent to ${rackNumber}`);
          return { success: true, data: { message: 'Lock command sent via WebSocket' } };
        } else {
          console.log(`[ESP32] ESP32 not connected for ${rackNumber}, simulating...`);
        }
      }

      // In development/testing, simulate successful lock
      if (process.env.NODE_ENV === 'development') {
        console.log(`[ESP32] Simulated lock for rack ${rackId}`);
        return { success: true, data: { message: 'Simulated lock (no ESP32 connected)' } };
      }

      return { success: false, error: 'ESP32 not connected' };
    } catch (error) {
      console.error(`[ESP32] Lock error for rack ${rackId}:`, error.message);
      
      // In development, still return success for testing
      if (process.env.NODE_ENV === 'development') {
        return { success: true, data: { message: 'Simulated lock (ESP32 unavailable)' } };
      }
      
      return { success: false, error: error.message };
    }
  }

  /**
   * Update LED status on ESP32
   * Colors: green (available), red (occupied), blue (unlocking)
   */
  async updateLEDStatus(rackId, color, blinking = false) {
    try {
      if (process.env.NODE_ENV === 'development') {
        console.log(`[ESP32] LED update for rack ${rackId}: ${color}, blinking: ${blinking}`);
        return { success: true };
      }

      const response = await axios.post(
        `${this.baseURL}/led`,
        { rackId, color, blinking },
        { timeout: this.timeout }
      );
      
      return { success: true, data: response.data };
    } catch (error) {
      console.error(`[ESP32] LED error for rack ${rackId}:`, error.message);
      return { success: false, error: error.message };
    }
  }

  /**
   * Get current status from ESP32
   */
  async getRackStatus(esp32DeviceId) {
    try {
      if (process.env.NODE_ENV === 'development') {
        return { 
          success: true, 
          data: { locked: true, ledColor: 'green' } 
        };
      }

      const response = await axios.get(
        `${this.baseURL}/status/${esp32DeviceId}`,
        { timeout: this.timeout }
      );
      
      return { success: true, data: response.data };
    } catch (error) {
      console.error('[ESP32] Status error:', error.message);
      return { success: false, error: error.message };
    }
  }

  /**
   * Health check for ESP32 connection
   */
  async healthCheck() {
    try {
      const response = await axios.get(
        `${this.baseURL}/health`,
        { timeout: 3000 }
      );
      return { success: true, data: response.data };
    } catch (error) {
      return { success: false, error: error.message };
    }
  }
}

module.exports = ESP32Service;
