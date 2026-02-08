const crypto = require('crypto');

/**
 * Mock Payment Service
 * Simulates payment gateway for prototype/development
 */
class PaymentService {
  constructor() {
    console.log('[Payment] Mock Payment Service initialized (Prototype Mode)');
  }

  /**
   * Create a mock payment order
   * @param {number} amount - Amount in INR
   * @param {string} currency - Currency code (default: INR)
   * @returns {Promise<Object>} Mock order object
   */
  async createOrder(amount, currency = 'INR') {
    // Simulate network delay
    await new Promise(resolve => setTimeout(resolve, 300));

    const orderId = `mock_order_${Date.now()}_${crypto.randomBytes(4).toString('hex')}`;
    
    const order = {
      id: orderId,
      amount: Math.round(amount * 100), // In paise
      amount_due: Math.round(amount * 100),
      amount_paid: 0,
      currency,
      receipt: `receipt_${Date.now()}`,
      status: 'created',
      created_at: Date.now()
    };

    console.log('[Payment] Mock order created:', orderId);
    return order;
  }

  /**
   * Verify mock payment signature
   * For prototype, we accept any signature that matches our pattern
   * @param {string} orderId - Order ID
   * @param {string} paymentId - Payment ID
   * @param {string} signature - Signature
   * @returns {boolean} Always true for mock
   */
  verifySignature(orderId, paymentId, signature) {
    // For prototype, accept if signature exists and has correct format
    const isValid = signature && signature.startsWith('mock_sig_');
    console.log('[Payment] Mock signature verification:', isValid ? 'SUCCESS' : 'FAILED');
    return isValid;
  }

  /**
   * Generate mock payment ID
   * @returns {string} Mock payment ID
   */
  generatePaymentId() {
    return `mock_pay_${Date.now()}_${crypto.randomBytes(4).toString('hex')}`;
  }

  /**
   * Generate mock signature
   * @param {string} orderId - Order ID
   * @param {string} paymentId - Payment ID
   * @returns {string} Mock signature
   */
  generateSignature(orderId, paymentId) {
    return `mock_sig_${crypto.createHash('sha256').update(`${orderId}|${paymentId}`).digest('hex').substring(0, 16)}`;
  }

  /**
   * Simulate payment capture
   * @param {string} orderId - Order ID
   * @returns {Promise<Object>} Payment result
   */
  async capturePayment(orderId) {
    await new Promise(resolve => setTimeout(resolve, 200));
    
    const paymentId = this.generatePaymentId();
    const signature = this.generateSignature(orderId, paymentId);

    return {
      success: true,
      orderId,
      paymentId,
      signature,
      status: 'captured'
    };
  }

  /**
   * Get mock payment details
   * @param {string} paymentId - Payment ID
   * @returns {Promise<Object>} Payment details
   */
  async getPaymentDetails(paymentId) {
    return {
      id: paymentId,
      amount: 0,
      currency: 'INR',
      status: 'captured',
      method: 'mock',
      created_at: Date.now()
    };
  }

  /**
   * Get mock order details
   * @param {string} orderId - Order ID
   * @returns {Promise<Object>} Order details
   */
  async getOrderDetails(orderId) {
    return {
      id: orderId,
      amount: 0,
      status: 'paid',
      created_at: Date.now()
    };
  }

  /**
   * Process mock refund
   * @param {string} paymentId - Payment ID
   * @param {number} amount - Amount to refund
   * @returns {Promise<Object>} Refund result
   */
  async processRefund(paymentId, amount) {
    await new Promise(resolve => setTimeout(resolve, 300));
    
    return {
      id: `mock_refund_${Date.now()}`,
      payment_id: paymentId,
      amount: Math.round(amount * 100),
      status: 'processed',
      created_at: Date.now()
    };
  }
}

module.exports = PaymentService;
