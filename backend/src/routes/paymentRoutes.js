const express = require('express');
const router = express.Router();
const paymentController = require('../controllers/paymentController');
const auth = require('../middleware/auth');
const { paymentLimiter } = require('../middleware/rateLimiter');
const { createOrderValidation, validate } = require('../middleware/validator');

// Protected routes
router.post('/create', auth, paymentLimiter, createOrderValidation, validate, paymentController.createOrder);
router.post('/verify', auth, paymentController.verifyPayment);
router.post('/simulate', auth, paymentController.simulatePayment); // Mock payment processing
router.get('/:id', auth, paymentController.getPaymentDetails);

// Webhook (no auth - for future production use)
router.post('/webhook', paymentController.handleWebhook);

module.exports = router;
