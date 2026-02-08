/**
 * Script to regenerate QR codes for all racks
 * Run this after changing FRONTEND_URL in production
 * 
 * Usage: node src/scripts/regenerateQRCodes.js
 */

require('dotenv').config();
const mongoose = require('mongoose');
const QRCode = require('qrcode');

const MONGODB_URI = process.env.MONGODB_URI || 'mongodb://localhost:27017/smart-locker';
const FRONTEND_URL = process.env.FRONTEND_URL || 'http://localhost:5173';

async function regenerateQRCodes() {
  try {
    console.log('🔗 Connecting to MongoDB...');
    await mongoose.connect(MONGODB_URI);
    console.log('✅ Connected to MongoDB');

    console.log(`🌐 Using FRONTEND_URL: ${FRONTEND_URL}`);

    // Get the Rack model
    const Rack = require('../models/Rack');

    // Find all racks
    const racks = await Rack.find({});
    console.log(`📦 Found ${racks.length} racks to update`);

    let updated = 0;
    let failed = 0;

    for (const rack of racks) {
      try {
        // Generate new QR code with production URL
        const qrUrl = `${FRONTEND_URL}/rack/${rack._id}`;
        
        const qrCode = await QRCode.toDataURL(qrUrl, {
          width: 300,
          margin: 2,
          color: {
            dark: '#000000',
            light: '#ffffff'
          },
          errorCorrectionLevel: 'M'
        });

        // Update rack directly without triggering pre-save hook
        await Rack.updateOne(
          { _id: rack._id },
          { $set: { qrCode: qrCode } }
        );

        console.log(`  ✅ Updated: ${rack.rackNumber} -> ${qrUrl}`);
        updated++;
      } catch (err) {
        console.log(`  ❌ Failed: ${rack.rackNumber} - ${err.message}`);
        failed++;
      }
    }

    console.log('\n📊 Summary:');
    console.log(`  ✅ Updated: ${updated}`);
    console.log(`  ❌ Failed: ${failed}`);
    console.log(`  📦 Total: ${racks.length}`);

    await mongoose.disconnect();
    console.log('\n🔌 Disconnected from MongoDB');
    process.exit(0);
  } catch (error) {
    console.error('❌ Error:', error.message);
    process.exit(1);
  }
}

regenerateQRCodes();
