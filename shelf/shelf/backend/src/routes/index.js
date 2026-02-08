const express = require('express');
const router = express.Router();

const authRoutes = require('./authRoutes');
const rackRoutes = require('./rackRoutes');
const paymentRoutes = require('./paymentRoutes');
const transactionRoutes = require('./transactionRoutes');
const adminRoutes = require('./adminRoutes');

// Mount routes
router.use('/auth', authRoutes);
router.use('/racks', rackRoutes);
router.use('/payments', paymentRoutes);
router.use('/transactions', transactionRoutes);
router.use('/admin', adminRoutes);

// API info
router.get('/', (req, res) => {
  res.json({
    name: 'Smart Locker System API',
    version: '1.0.0',
    endpoints: {
      auth: '/api/auth',
      racks: '/api/racks',
      payments: '/api/payments',
      transactions: '/api/transactions',
      admin: '/api/admin'
    }
  });
});

module.exports = router;
