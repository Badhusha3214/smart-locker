/**
 * Payment Service
 * Uses Mock Payment Service for prototype
 * Can be switched to Razorpay or other gateways in production
 */
const MockPaymentService = require('./mockPaymentService');

// Export the mock service for prototype
// To switch to Razorpay in production, import and export the Razorpay service instead
module.exports = MockPaymentService;
