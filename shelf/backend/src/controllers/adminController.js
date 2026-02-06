const Rack = require('../models/Rack');
const User = require('../models/User');
const Transaction = require('../models/Transaction');
const Payment = require('../models/Payment');
const ESP32Service = require('../services/esp32Service');

// Get dashboard statistics
exports.getStats = async (req, res) => {
  try {
    const totalRacks = await Rack.countDocuments({ isActive: true });
    const activeUsers = await User.countDocuments({ isActive: true });
    
    const today = new Date();
    today.setHours(0, 0, 0, 0);
    
    const todayRevenue = await Transaction.aggregate([
      {
        $match: {
          status: { $in: ['paid', 'unlocked', 'locked'] },
          createdAt: { $gte: today }
        }
      },
      {
        $group: {
          _id: null,
          total: { $sum: '$amount' }
        }
      }
    ]);

    const firstDayOfMonth = new Date(today.getFullYear(), today.getMonth(), 1);
    const monthRevenue = await Transaction.aggregate([
      {
        $match: {
          status: { $in: ['paid', 'unlocked', 'locked'] },
          createdAt: { $gte: firstDayOfMonth }
        }
      },
      {
        $group: {
          _id: null,
          total: { $sum: '$amount' }
        }
      }
    ]);

    const activeSessions = await Rack.countDocuments({ 
      status: { $in: ['occupied', 'unlocking'] } 
    });

    const todayTransactions = await Transaction.countDocuments({
      createdAt: { $gte: today }
    });

    // Get rack status distribution
    const rackStatusDist = await Rack.aggregate([
      { $match: { isActive: true } },
      { $group: { _id: '$status', count: { $sum: 1 } } }
    ]);

    res.json({
      success: true,
      stats: {
        totalRacks,
        activeUsers,
        todayRevenue: todayRevenue[0]?.total || 0,
        monthRevenue: monthRevenue[0]?.total || 0,
        activeSessions,
        todayTransactions,
        rackStatusDistribution: rackStatusDist.reduce((acc, item) => {
          acc[item._id] = item.count;
          return acc;
        }, {})
      }
    });
  } catch (error) {
    console.error('Get stats error:', error);
    res.status(500).json({ error: 'Server error' });
  }
};

// Get all racks (admin)
exports.getAllRacks = async (req, res) => {
  try {
    const { status, location, page = 1, limit = 50 } = req.query;
    
    const query = {};
    if (status) query.status = status;
    if (location) query.location = new RegExp(location, 'i');

    const racks = await Rack.find(query)
      .populate('currentTransactionId')
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

// Create new rack
exports.createRack = async (req, res) => {
  try {
    const { rackNumber, location, pricePerUse, esp32DeviceId } = req.body;

    const existingRack = await Rack.findOne({ rackNumber });
    if (existingRack) {
      return res.status(400).json({ error: 'Rack number already exists' });
    }

    const rack = new Rack({
      rackNumber,
      location,
      pricePerUse,
      esp32DeviceId
    });

    await rack.save();

    res.status(201).json({
      success: true,
      message: 'Rack created successfully',
      rack
    });
  } catch (error) {
    console.error('Create rack error:', error);
    res.status(500).json({ error: 'Server error' });
  }
};

// Update rack
exports.updateRack = async (req, res) => {
  try {
    const { id } = req.params;
    const updates = req.body;

    // Don't allow updating certain fields directly
    delete updates.qrCode;
    delete updates.currentTransactionId;

    const rack = await Rack.findByIdAndUpdate(
      id,
      { ...updates, updatedAt: Date.now() },
      { new: true, runValidators: true }
    );

    if (!rack) {
      return res.status(404).json({ error: 'Rack not found' });
    }

    res.json({
      success: true,
      message: 'Rack updated successfully',
      rack
    });
  } catch (error) {
    console.error('Update rack error:', error);
    res.status(500).json({ error: 'Server error' });
  }
};

// Delete rack (soft delete)
exports.deleteRack = async (req, res) => {
  try {
    const { id } = req.params;

    const rack = await Rack.findById(id);
    if (!rack) {
      return res.status(404).json({ error: 'Rack not found' });
    }

    // Check if rack is in use
    if (rack.status === 'occupied' || rack.status === 'unlocking') {
      return res.status(400).json({ error: 'Cannot delete rack while in use' });
    }

    rack.isActive = false;
    rack.updatedAt = Date.now();
    await rack.save();

    res.json({
      success: true,
      message: 'Rack deleted successfully'
    });
  } catch (error) {
    console.error('Delete rack error:', error);
    res.status(500).json({ error: 'Server error' });
  }
};

// Manual unlock
exports.manualUnlock = async (req, res) => {
  try {
    const { id } = req.params;

    const rack = await Rack.findById(id);
    if (!rack) {
      return res.status(404).json({ error: 'Rack not found' });
    }

    const esp32Service = new ESP32Service();
    const result = await esp32Service.unlockRack(id, rack.esp32DeviceId);

    if (result.success) {
      rack.status = 'occupied';
      rack.ledStatus = 'red';
      rack.lastUnlocked = new Date();
      await rack.save();

      const io = req.app.get('io');
      if (io) {
        io.to(`rack-${id}`).emit('lock-status', { status: 'unlocked', rackId: id });
        io.to('admin').emit('rack-status', { rackId: id, status: 'occupied' });
      }

      res.json({
        success: true,
        message: 'Rack unlocked manually',
        rack
      });
    } else {
      res.status(500).json({ error: 'Failed to unlock rack' });
    }
  } catch (error) {
    console.error('Manual unlock error:', error);
    res.status(500).json({ error: 'Server error' });
  }
};

// Manual lock
exports.manualLock = async (req, res) => {
  try {
    const { id } = req.params;

    const rack = await Rack.findById(id);
    if (!rack) {
      return res.status(404).json({ error: 'Rack not found' });
    }

    const esp32Service = new ESP32Service();
    const result = await esp32Service.lockRack(id, rack.esp32DeviceId);

    if (result.success) {
      rack.status = 'available';
      rack.ledStatus = 'green';
      rack.currentTransactionId = null;
      await rack.save();

      const io = req.app.get('io');
      if (io) {
        io.to(`rack-${id}`).emit('lock-status', { status: 'locked', rackId: id });
        io.to('admin').emit('rack-status', { rackId: id, status: 'available' });
      }

      res.json({
        success: true,
        message: 'Rack locked manually',
        rack
      });
    } else {
      res.status(500).json({ error: 'Failed to lock rack' });
    }
  } catch (error) {
    console.error('Manual lock error:', error);
    res.status(500).json({ error: 'Server error' });
  }
};

// Get all transactions (admin)
exports.getAllTransactions = async (req, res) => {
  try {
    const { 
      userId, 
      rackId, 
      status, 
      startDate, 
      endDate, 
      page = 1, 
      limit = 50 
    } = req.query;

    const query = {};
    
    if (userId) query.userId = userId;
    if (rackId) query.rackId = rackId;
    if (status) query.status = status;
    if (startDate || endDate) {
      query.createdAt = {};
      if (startDate) query.createdAt.$gte = new Date(startDate);
      if (endDate) query.createdAt.$lte = new Date(endDate);
    }

    const transactions = await Transaction.find(query)
      .populate('userId', 'name email phone')
      .populate('rackId', 'rackNumber location')
      .populate('paymentId')
      .limit(limit * 1)
      .skip((page - 1) * limit)
      .sort({ createdAt: -1 });

    const count = await Transaction.countDocuments(query);

    res.json({
      success: true,
      transactions,
      pagination: {
        total: count,
        page: parseInt(page),
        pages: Math.ceil(count / limit)
      }
    });
  } catch (error) {
    console.error('Get all transactions error:', error);
    res.status(500).json({ error: 'Server error' });
  }
};

// Get all users (admin)
exports.getAllUsers = async (req, res) => {
  try {
    const { search, role, page = 1, limit = 50 } = req.query;

    const query = {};
    
    if (role) query.role = role;
    if (search) {
      query.$or = [
        { name: new RegExp(search, 'i') },
        { email: new RegExp(search, 'i') },
        { phone: new RegExp(search, 'i') }
      ];
    }

    const users = await User.find(query)
      .select('-password')
      .limit(limit * 1)
      .skip((page - 1) * limit)
      .sort({ createdAt: -1 });

    const count = await User.countDocuments(query);

    res.json({
      success: true,
      users,
      pagination: {
        total: count,
        page: parseInt(page),
        pages: Math.ceil(count / limit)
      }
    });
  } catch (error) {
    console.error('Get all users error:', error);
    res.status(500).json({ error: 'Server error' });
  }
};

// Recent transactions for dashboard
exports.getRecentTransactions = async (req, res) => {
  try {
    const transactions = await Transaction.find()
      .populate('userId', 'name email')
      .populate('rackId', 'rackNumber location')
      .sort({ createdAt: -1 })
      .limit(10);

    res.json({
      success: true,
      transactions
    });
  } catch (error) {
    console.error('Get recent transactions error:', error);
    res.status(500).json({ error: 'Server error' });
  }
};
