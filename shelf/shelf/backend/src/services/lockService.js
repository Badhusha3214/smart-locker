const Transaction = require('../models/Transaction');
const Rack = require('../models/Rack');
const ESP32Service = require('./esp32Service');

/**
 * Lock Service
 * Handles auto-lock scheduling and lock state management
 */
class LockService {
  constructor() {
    this.scheduledLocks = new Map();
  }

  /**
   * Schedule auto-lock for a rack after specified delay
   * @param {string} transactionId - Transaction ID
   * @param {string} rackId - Rack ID
   * @param {number} delaySeconds - Delay in seconds before auto-lock
   * @param {Object} io - Socket.io instance for real-time updates
   */
  async scheduleAutoLock(transactionId, rackId, delaySeconds = 30, io = null) {
    // Clear any existing scheduled lock for this rack
    if (this.scheduledLocks.has(rackId)) {
      clearTimeout(this.scheduledLocks.get(rackId));
    }

    console.log(`[LockService] Scheduling auto-lock for rack ${rackId} in ${delaySeconds} seconds`);

    const timeoutId = setTimeout(async () => {
      await this.autoLockRack(transactionId, rackId, io);
    }, delaySeconds * 1000);

    this.scheduledLocks.set(rackId, timeoutId);

    // Mark transaction as scheduled
    try {
      await Transaction.findByIdAndUpdate(transactionId, {
        autoLockScheduled: true
      });
    } catch (error) {
      console.error('[LockService] Failed to update transaction:', error);
    }
  }

  /**
   * Cancel scheduled auto-lock
   * @param {string} rackId - Rack ID
   */
  cancelAutoLock(rackId) {
    if (this.scheduledLocks.has(rackId)) {
      clearTimeout(this.scheduledLocks.get(rackId));
      this.scheduledLocks.delete(rackId);
      console.log(`[LockService] Cancelled auto-lock for rack ${rackId}`);
    }
  }

  /**
   * Execute auto-lock for a rack
   * @param {string} transactionId - Transaction ID
   * @param {string} rackId - Rack ID
   * @param {Object} io - Socket.io instance
   */
  async autoLockRack(transactionId, rackId, io = null) {
    try {
      const transaction = await Transaction.findById(transactionId);
      const rack = await Rack.findById(rackId);

      if (!transaction || !rack) {
        console.error(`[LockService] Transaction or rack not found for auto-lock`);
        return;
      }

      // Only auto-lock if still in unlocked status
      if (transaction.status === 'unlocked' && rack.status === 'occupied') {
        console.log(`[LockService] Auto-locking rack ${rack.rackNumber}`);

        const esp32Service = new ESP32Service();
        const lockResult = await esp32Service.lockRack(rackId, rack.esp32DeviceId);

        if (lockResult.success) {
          // Update rack
          rack.status = 'available';
          rack.ledStatus = 'green';
          rack.currentTransactionId = null;
          await rack.save();

          // Update transaction
          transaction.status = 'locked';
          transaction.lockedAt = new Date();
          await transaction.save();

          console.log(`[LockService] Auto-locked rack ${rack.rackNumber} successfully`);

          // Emit WebSocket events
          if (io) {
            io.to(`rack-${rackId}`).emit('lock-status', { 
              status: 'locked', 
              rackId,
              autoLocked: true 
            });
            io.to(`rack-${rackId}`).emit('led-status', { 
              color: 'green', 
              rackId 
            });
            io.to('admin').emit('rack-status', { 
              rackId, 
              status: 'available',
              autoLocked: true 
            });
          }
        } else {
          console.error(`[LockService] Auto-lock failed for rack ${rack.rackNumber}`);
          
          // Notify admin of failed auto-lock
          if (io) {
            io.to('admin').emit('auto-lock-failed', {
              rackId,
              rackNumber: rack.rackNumber,
              transactionId
            });
          }
        }
      } else {
        console.log(`[LockService] Skipping auto-lock - status already changed`);
      }

      // Remove from scheduled locks
      this.scheduledLocks.delete(rackId);
    } catch (error) {
      console.error('[LockService] Auto-lock error:', error);
    }
  }

  /**
   * Manual lock with cancel of auto-lock
   * @param {string} rackId - Rack ID
   * @param {Object} io - Socket.io instance
   */
  async manualLock(rackId, io = null) {
    // Cancel any scheduled auto-lock
    this.cancelAutoLock(rackId);

    const rack = await Rack.findById(rackId);
    if (!rack) {
      throw new Error('Rack not found');
    }

    const esp32Service = new ESP32Service();
    const result = await esp32Service.lockRack(rackId, rack.esp32DeviceId);

    if (result.success) {
      const currentTransactionId = rack.currentTransactionId;
      
      rack.status = 'available';
      rack.ledStatus = 'green';
      rack.currentTransactionId = null;
      await rack.save();

      // Update transaction if exists
      if (currentTransactionId) {
        await Transaction.findByIdAndUpdate(currentTransactionId, {
          status: 'locked',
          lockedAt: new Date()
        });
      }

      // Emit events
      if (io) {
        io.to(`rack-${rackId}`).emit('lock-status', { 
          status: 'locked', 
          rackId,
          manualLock: true 
        });
        io.to(`rack-${rackId}`).emit('led-status', { 
          color: 'green', 
          rackId 
        });
        io.to('admin').emit('rack-status', { 
          rackId, 
          status: 'available' 
        });
      }

      return { success: true, rack };
    }

    return { success: false, error: 'Failed to lock rack' };
  }

  /**
   * Get remaining time until auto-lock
   * @param {string} rackId - Rack ID
   * @returns {number|null} Remaining seconds or null if not scheduled
   */
  getRemainingTime(rackId) {
    // This would need additional tracking of start times
    // For now, return null - client tracks countdown locally
    return null;
  }
}

// Export singleton instance
module.exports = new LockService();
