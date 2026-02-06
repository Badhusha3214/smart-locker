const Rack = require('../models/Rack');
const Transaction = require('../models/Transaction');
const ESP32Service = require('../services/esp32Service');
const lockService = require('../services/lockService');

// Get all racks
exports.getAllRacks = async (req, res) => {
  try {
    const { status, location, page = 1, limit = 20 } = req.query;
    
    const query = { isActive: true };
    if (status) query.status = status;
    if (location) query.location = new RegExp(location, 'i');

    const racks = await Rack.find(query)
      .limit(limit * 1)
      .skip((page - 1) * limit)
      .sort({ rackNumber: 1 });

    const count = await Rack.countDocuments(query);

    res.json({
      success: true,
      racks,
      pagination: {
        total: count,
        page: parseInt(page),
        pages: Math.ceil(count / limit)
      }
    });
  } catch (error) {
    console.error('Get racks error:', error);
    res.status(500).json({ error: 'Server error' });
  }
};

// Get rack by ID
exports.getRackById = async (req, res) => {
  try {
    const rack = await Rack.findById(req.params.id)
      .populate('currentTransactionId');

    if (!rack) {
      return res.status(404).json({ error: 'Rack not found' });
    }

    res.json({
      success: true,
      rack
    });
  } catch (error) {
    console.error('Get rack error:', error);
    res.status(500).json({ error: 'Server error' });
  }
};

// Get rack by QR code
exports.getRackByQR = async (req, res) => {
  try {
    const { qrData } = req.body;
    
    // Parse QR data
    let parsedData;
    try {
      parsedData = JSON.parse(qrData);
    } catch {
      return res.status(400).json({ error: 'Invalid QR code format' });
    }

    const rack = await Rack.findById(parsedData.rackId);

    if (!rack || !rack.isActive) {
      return res.status(404).json({ error: 'Rack not found or inactive' });
    }

    res.json({
      success: true,
      rack
    });
  } catch (error) {
    console.error('Get rack by QR error:', error);
    res.status(400).json({ error: 'Invalid QR code' });
  }
};

// Unlock rack
exports.unlockRack = async (req, res) => {
  try {
    const { id } = req.params;
    const { transactionId } = req.body;

    const rack = await Rack.findById(id);
    if (!rack) {
      return res.status(404).json({ error: 'Rack not found' });
    }

    // Check if rack is available
    if (rack.status !== 'available' && rack.status !== 'unlocking') {
      return res.status(400).json({ 
        error: 'Rack is currently occupied or under maintenance' 
      });
    }

    // Verify transaction exists and is paid
    const transaction = await Transaction.findById(transactionId);
    if (!transaction || transaction.status !== 'paid') {
      return res.status(400).json({ 
        error: 'Invalid or unpaid transaction' 
      });
    }

    // Update rack status to unlocking
    rack.status = 'unlocking';
    rack.ledStatus = 'blue';
    rack.currentTransactionId = transactionId;
    await rack.save();

    // Get IO instance for ESP32 communication
    const io = req.app.get('io');
    
    // Send unlock command to ESP32 via WebSocket
    const esp32Service = new ESP32Service(io);
    const unlockResult = await esp32Service.unlockRack(id, rack.rackNumber);

    if (unlockResult.success) {
      // Update rack and transaction
      rack.status = 'occupied';
      rack.ledStatus = 'red';
      rack.isLocked = false;  // Mark as unlocked
      rack.lastUnlocked = new Date();
      await rack.save();

      transaction.status = 'unlocked';
      transaction.unlockedAt = new Date();
      await transaction.save();

      // Emit WebSocket event
      const io = req.app.get('io');
      if (io) {
        io.to(`rack-${id}`).emit('lock-status', { 
          status: 'unlocked',
          rackId: id 
        });
        io.to(`rack-${id}`).emit('led-status', { 
          color: 'red',
          rackId: id 
        });
        io.to('admin').emit('rack-status', {
          rackId: id,
          status: 'occupied'
        });
      }

      // Schedule auto-lock (30 seconds default)
      const autoLockDuration = parseInt(process.env.AUTO_LOCK_DURATION) || 30;
      await lockService.scheduleAutoLock(transactionId, id, autoLockDuration, io);

      res.json({
        success: true,
        message: 'Rack unlocked successfully',
        rack,
        transaction,
        autoLockIn: autoLockDuration
      });
    } else {
      rack.status = 'maintenance';
      rack.ledStatus = 'off';
      await rack.save();

      res.status(500).json({ 
        error: 'Failed to unlock rack. Please contact support.' 
      });
    }
  } catch (error) {
    console.error('Unlock rack error:', error);
    res.status(500).json({ error: 'Server error' });
  }
};

// Lock rack
exports.lockRack = async (req, res) => {
  try {
    const { id } = req.params;

    const rack = await Rack.findById(id);
    if (!rack) {
      return res.status(404).json({ error: 'Rack not found' });
    }

    // Send lock command to ESP32
    const esp32Service = new ESP32Service();
    const lockResult = await esp32Service.lockRack(id, rack.esp32DeviceId);

    if (lockResult.success) {
      // Update rack status
      const currentTransactionId = rack.currentTransactionId;
      rack.status = 'available';
      rack.ledStatus = 'green';
      rack.isLocked = true;  // Mark as locked
      rack.currentTransactionId = null;
      rack.needsRefill = true; // Mark as empty/needs refill after item retrieved
      await rack.save();

      // Update transaction if exists
      if (currentTransactionId) {
        const transaction = await Transaction.findById(currentTransactionId);
        if (transaction && transaction.status === 'unlocked') {
          transaction.status = 'locked';
          transaction.lockedAt = new Date();
          await transaction.save();
        }
      }

      // Emit WebSocket event
      const io = req.app.get('io');
      if (io) {
        io.to(`rack-${id}`).emit('lock-status', { 
          status: 'locked',
          rackId: id 
        });
        io.to(`rack-${id}`).emit('led-status', { 
          color: 'green',
          rackId: id 
        });
        io.to('admin').emit('rack-status', {
          rackId: id,
          status: 'available'
        });
      }

      res.json({
        success: true,
        message: 'Rack locked successfully',
        rack
      });
    } else {
      res.status(500).json({ 
        error: 'Failed to lock rack. Please try again.' 
      });
    }
  } catch (error) {
    console.error('Lock rack error:', error);
    res.status(500).json({ error: 'Server error' });
  }
};

// Get rack status (for ESP32 polling)
exports.getRackStatus = async (req, res) => {
  try {
    const rack = await Rack.findById(req.params.id);
    if (!rack) {
      return res.status(404).json({ error: 'Rack not found' });
    }

    res.json({
      success: true,
      status: rack.status,
      ledStatus: rack.ledStatus,
      needsRefill: rack.needsRefill
    });
  } catch (error) {
    console.error('Get rack status error:', error);
    res.status(500).json({ error: 'Server error' });
  }
};

// Mark rack as refilled (admin only)
exports.refillRack = async (req, res) => {
  try {
    const { id } = req.params;

    const rack = await Rack.findById(id);
    if (!rack) {
      return res.status(404).json({ error: 'Rack not found' });
    }

    rack.needsRefill = false;
    rack.isLocked = true;  // Refilling means it's now stocked and locked
    rack.lastRefilledAt = new Date();
    rack.lastRefilledBy = req.user._id;
    await rack.save();

    // Emit WebSocket event to notify admin dashboard
    const io = req.app.get('io');
    if (io) {
      io.to('admin').emit('rack-refilled', {
        rackId: id,
        needsRefill: false
      });
    }

    res.json({
      success: true,
      message: 'Rack marked as refilled',
      rack
    });
  } catch (error) {
    console.error('Refill rack error:', error);
    res.status(500).json({ error: 'Server error' });
  }
};

// Get connected ESP32 devices (admin only)
exports.getConnectedDevices = async (req, res) => {
  try {
    const io = req.app.get('io');
    
    if (!io || !io.getConnectedDevices) {
      return res.json({
        success: true,
        devices: [],
        shelves: [],
        message: 'No devices connected'
      });
    }

    const devices = io.getConnectedDevices();
    const shelves = io.getConnectedShelves ? io.getConnectedShelves() : [];
    
    res.json({
      success: true,
      devices,
      shelves,
      deviceCount: devices.length,
      shelfCount: shelves.length
    });
  } catch (error) {
    console.error('Get connected devices error:', error);
    res.status(500).json({ error: 'Server error' });
  }
};
