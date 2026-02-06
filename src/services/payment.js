import api from './api'

/**
 * Mock Payment Service
 * Simulates payment flow for prototype/development
 */
class PaymentService {
  constructor() {
    this.isMockMode = true
  }

  /**
   * Create payment order
   */
  async createOrder(rackId) {
    const response = await api.post('/payments/create', { rackId })
    return response.data
  }

  /**
   * Verify payment signature
   */
  async verifyPayment(paymentData) {
    const response = await api.post('/payments/verify', paymentData)
    return response.data
  }

  /**
   * Simulate payment processing (for mock mode)
   */
  async simulatePayment(orderId, transactionId) {
    const response = await api.post('/payments/simulate', { 
      orderId, 
      transactionId 
    })
    return response.data
  }

  /**
   * Initialize mock payment
   * Shows a simulated payment UI instead of Razorpay
   */
  async initializePayment(orderData, userInfo, onSuccess, onFailure) {
    // For mock mode, we'll return the order data and let the component handle the UI
    return {
      isMock: true,
      orderId: orderData.orderId,
      transactionId: orderData.transactionId,
      amount: orderData.amount,
      currency: orderData.currency || 'INR',
      rack: orderData.rack,
      userInfo
    }
  }

  /**
   * Process mock payment
   * Called when user confirms in the mock payment UI
   */
  async processMockPayment(orderId, transactionId, onSuccess, onFailure) {
    try {
      // Call backend to simulate the payment
      const result = await this.simulatePayment(orderId, transactionId)
      
      if (result.success) {
        onSuccess(result)
      } else {
        onFailure(new Error(result.message || 'Payment failed'))
      }
    } catch (error) {
      onFailure(error)
    }
  }

  /**
   * Simulate payment failure (for testing)
   */
  async simulateFailure(orderId, transactionId, onFailure) {
    onFailure(new Error('Simulated payment failure for testing'))
  }
}

export default new PaymentService()
