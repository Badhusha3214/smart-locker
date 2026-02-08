const Payment = require('../models/Payment');
const Transaction = require('../models/Transaction');
const Rack = require('../models/Rack');
const PaymentService = require('../services/paymentService');

const paymentService = new PaymentService();

// Create payment order
exports.createOrder = async (req, res) => {
  try {
    const { rackId } = req.body;
    const userId = req.user.id;

    // Get rack details
    const rack = await Rack.findById(rackId);
    if (!rack || !rack.isActive) {
      return res.status(404).json({ error: 'Rack not found' });
    }

    if (rack.status !== 'available') {
      return res.status(400).json({ error: 'Rack is not available' });
    }

    // Create transaction
    const transaction = new Transaction({
      userId,
      rackId,
      amount: rack.pricePerUse,
      status: 'initiated'
    });
    await transaction.save();

    // Create mock order
    const mockOrder = await paymentService.createOrder(
      rack.pricePerUse,
      'INR'
    );

    // Create payment record
    const payment = new Payment({
      transactionId: transaction._id,
      razorpayOrderId: mockOrder.id, // Using same field for mock order ID
      amount: rack.pricePerUse,
      currency: 'INR',
      status: 'created'
    });
    await payment.save();

    // Update transaction with payment ID
    transaction.paymentId = payment._id;
    await transaction.save();

    res.json({
      success: true,
      orderId: mockOrder.id,
      amount: rack.pricePerUse,
      currency: 'INR',
      transactionId: transaction._id,
      isMockPayment: true, // Flag for frontend to know this is mock
      rack: {
        id: rack._id,
        rackNumber: rack.rackNumber,
        location: rack.location
      }
    });
  } catch (error) {
    console.error('Create order error:', error);
    res.status(500).json({ error: 'Failed to create payment order' });
  }
};

// Verify payment (Mock version)
exports.verifyPayment = async (req, res) => {
  try {
    const { 
      orderId, 
      paymentId, 
      signature, 
      transactionId 
    } = req.body;

    // Verify mock signature
    const isValid = paymentService.verifySignature(
      orderId,
      paymentId,
      signature
    );

    if (!isValid) {
      return res.status(400).json({ error: 'Invalid payment signature' });
    }

    // Find payment and transaction
    const payment = await Payment.findOne({ razorpayOrderId: orderId });
    const transaction = await Transaction.findById(transactionId);

    if (!payment || !transaction) {
      return res.status(404).json({ error: 'Payment or transaction not found' });
    }

    // Update payment
    payment.razorpayPaymentId = paymentId;
    payment.razorpaySignature = signature;
    payment.status = 'captured';
    payment.paymentMethod = 'mock';
    await payment.save();

    // Update transaction
    transaction.status = 'paid';
    await transaction.save();

    res.json({
      success: true,
      message: 'Payment verified successfully',
      transaction: {
        id: transaction._id,
        status: transaction.status,
        amount: transaction.amount
      }
    });
  } catch (error) {
    console.error('Verify payment error:', error);
    res.status(500).json({ error: 'Payment verification failed' });
  }
};

// Simulate mock payment (for prototype - instant payment processing)
exports.simulatePayment = async (req, res) => {
  try {
    const { orderId, transactionId } = req.body;

    // Simulate payment delay (1-2 seconds)
    await new Promise(resolve => setTimeout(resolve, 1500));

    // Capture mock payment
    const captureResult = await paymentService.capturePayment(orderId);

    // Find payment and transaction
    const payment = await Payment.findOne({ razorpayOrderId: orderId });
    const transaction = await Transaction.findById(transactionId);

    if (!payment || !transaction) {
      return res.status(404).json({ error: 'Payment or transaction not found' });
    }

    // Update payment
    payment.razorpayPaymentId = captureResult.paymentId;
    payment.razorpaySignature = captureResult.signature;
    payment.status = 'captured';
    payment.paymentMethod = 'mock';
    await payment.save();

    // Update transaction
    transaction.status = 'paid';
    await transaction.save();

    res.json({
      success: true,
      message: 'Mock payment processed successfully',
      paymentId: captureResult.paymentId,
      signature: captureResult.signature,
      transaction: {
        id: transaction._id,
        status: 'paid',
        amount: transaction.amount
      }
    });
  } catch (error) {
    console.error('Simulate payment error:', error);
    res.status(500).json({ error: 'Mock payment simulation failed' });
  }
};

// Webhook handler (kept for future production use with real payment gateway)
exports.handleWebhook = async (req, res) => {
  // Mock webhook - just acknowledge
  console.log('[Payment] Mock webhook received:', req.body);
  res.json({ success: true, message: 'Mock webhook acknowledged' });
};

// Get payment details
exports.getPaymentDetails = async (req, res) => {
  try {
    const payment = await Payment.findById(req.params.id)
      .populate('transactionId');

    if (!payment) {
      return res.status(404).json({ error: 'Payment not found' });
    }

    res.json({
      success: true,
      payment
    });
  } catch (error) {
    console.error('Get payment error:', error);
    res.status(500).json({ error: 'Server error' });
  }
};
