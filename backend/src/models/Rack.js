const mongoose = require('mongoose');
const QRCode = require('qrcode');

const rackSchema = new mongoose.Schema({
  rackNumber: {
    type: String,
    required: [true, 'Rack number is required'],
    unique: true,
    trim: true
  },
  location: {
    type: String,
    trim: true
  },
  pricePerUse: {
    type: Number,
    required: [true, 'Price per use is required'],
    min: [0, 'Price cannot be negative']
  },
  status: {
    type: String,
    enum: ['available', 'occupied', 'unlocking', 'maintenance'],
    default: 'available'
  },
  needsRefill: {
    type: Boolean,
    default: false
  },
  lastRefilledAt: {
    type: Date
  },
  lastRefilledBy: {
    type: mongoose.Schema.Types.ObjectId,
    ref: 'User'
  },
  esp32DeviceId: {
    type: String,
    unique: true,
    sparse: true
  },
  qrCode: {
    type: String,
    unique: true
  },
  ledStatus: {
    type: String,
    enum: ['green', 'red', 'blue', 'off'],
    default: 'green'
  },
  lastUnlocked: {
    type: Date
  },
  currentTransactionId: {
    type: mongoose.Schema.Types.ObjectId,
    ref: 'Transaction'
  },
  isActive: {
    type: Boolean,
    default: true
  },
  isLocked: {
    type: Boolean,
    default: true  // Default to locked
  },
  createdAt: {
    type: Date,
    default: Date.now
  },
  updatedAt: {
    type: Date,
    default: Date.now
  }
}, {
  timestamps: true
});

// Generate QR code before saving - creates a universal URL-based QR code
rackSchema.pre('save', async function(next) {
  if (!this.isNew) return next();
  
  // Generate a universal URL that works with any QR scanner
  // Users can scan with any phone camera app and it will open the locker page
  const baseUrl = process.env.FRONTEND_URL || 'http://localhost:5173';
  const qrUrl = `${baseUrl}/rack/${this._id}`;
  
  // Generate QR code with the URL
  this.qrCode = await QRCode.toDataURL(qrUrl, {
    width: 300,
    margin: 2,
    color: {
      dark: '#000000',
      light: '#ffffff'
    },
    errorCorrectionLevel: 'M'
  });
  
  next();
});

// Index for faster queries
rackSchema.index({ status: 1, isActive: 1 });
rackSchema.index({ rackNumber: 1 });

module.exports = mongoose.model('Rack', rackSchema);
