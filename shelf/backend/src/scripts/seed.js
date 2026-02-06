require('dotenv').config();
const mongoose = require('mongoose');
const bcrypt = require('bcryptjs');
const { v4: uuidv4 } = require('uuid');

// Import models
const User = require('../models/User');
const Rack = require('../models/Rack');

const seedDatabase = async () => {
  try {
    // Connect to MongoDB
    await mongoose.connect(process.env.MONGODB_URI || 'mongodb://localhost:27017/smart-locker');
    console.log('Connected to MongoDB');

    // Clear existing data
    await User.deleteMany({});
    await Rack.deleteMany({});
    
    // Drop the racks collection indexes to avoid qrCode unique constraint issues
    try {
      await mongoose.connection.collection('racks').dropIndexes();
      console.log('Dropped rack indexes');
    } catch (e) {
      // Ignore if collection doesn't exist
    }
    
    console.log('Cleared existing data');

    // Create admin user (password will be hashed by the model's pre-save hook)
    const admin = await User.create({
      name: 'Admin User',
      email: 'admin@smartlocker.com',
      phone: '9876543210',
      password: 'admin123',
      role: 'admin'
    });
    console.log('Created admin user:', admin.email);

    // Create test user
    const user = await User.create({
      name: 'Test User',
      email: 'user@test.com',
      phone: '9876543211',
      password: 'user1234',
      role: 'user'
    });
    console.log('Created test user:', user.email);

    // Create sample racks with unique QR codes
    const locations = ['Floor 1, Zone A', 'Floor 1, Zone B', 'Floor 2, Zone A', 'Floor 2, Zone B'];
    const sizes = ['small', 'medium', 'large'];
    const prices = { small: 10, medium: 20, large: 30 };

    // Insert racks one by one to trigger pre-save hook for QR generation
    for (let i = 1; i <= 12; i++) {
      const size = sizes[(i - 1) % 3];
      const location = locations[(i - 1) % 4];
      
      // Use RACK_XXX format to match ESP32 configuration
      const rackNumber = `RACK_${String(i).padStart(3, '0')}`;
      
      const rack = new Rack({
        rackNumber: rackNumber,
        location: location,
        pricePerUse: prices[size],
        status: i <= 10 ? 'available' : (i === 11 ? 'occupied' : 'maintenance'),
        esp32DeviceId: `ESP32_${String(i).padStart(3, '0')}`,
        ledStatus: 'green',
        isActive: true,
        needsRefill: false
      });
      await rack.save(); // This will trigger pre-save hook
      console.log(`  Created rack: ${rackNumber}`);
    }
    console.log('Created 12 racks with QR codes');

    console.log('\n=== Seed Complete ===');
    console.log('Admin login: admin@smartlocker.com / admin123');
    console.log('User login: user@test.com / user1234');
    console.log('======================\n');

    process.exit(0);
  } catch (error) {
    console.error('Seed error:', error);
    process.exit(1);
  }
};

seedDatabase();
