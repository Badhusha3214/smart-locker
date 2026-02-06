const mongoose = require('mongoose');

const transactionSchema = new mongoose.Schema({
  userId: {
    type: mongoose.Schema.Types.ObjectId,
    ref: 'User',
    required: [true, 'User ID is required']
  },
  rackId: {
    type: mongoose.Schema.Types.ObjectId,
    ref: 'Rack',
    required: [true, 'Rack ID is required']
  },
  paymentId: {
    type: mongoose.Schema.Types.ObjectId,
    ref: 'Payment'
  },
  amount: {
    type: Number,
    required: [true, 'Amount is required'],
    min: [0, 'Amount cannot be negative']
  },
  status: {
    type: String,
    enum: ['initiated', 'paid', 'unlocked', 'locked', 'failed', 'timeout'],
    default: 'initiated'
  },
  unlockedAt: {
    type: Date
  },
  lockedAt: {
    type: Date
  },
  duration: {
    type: Number, // in seconds
    default: 0
  },
  autoLockScheduled: {
    type: Boolean,
    default: false
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

// Calculate duration when locked
transactionSchema.pre('save', function(next) {
  if (this.unlockedAt && this.lockedAt) {
    this.duration = Math.floor((this.lockedAt - this.unlockedAt) / 1000);
  }
  next();
});

// Index for faster queries
transactionSchema.index({ userId: 1, createdAt: -1 });
transactionSchema.index({ rackId: 1, status: 1 });
transactionSchema.index({ status: 1 });

module.exports = mongoose.model('Transaction', transactionSchema);
