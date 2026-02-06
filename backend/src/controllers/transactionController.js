const Transaction = require('../models/Transaction');
const PDFDocument = require('pdfkit');

// Get user transactions
exports.getUserTransactions = async (req, res) => {
  try {
    const userId = req.user.id;
    const { 
      status, 
      startDate, 
      endDate, 
      page = 1, 
      limit = 20 
    } = req.query;

    const query = { userId };
    
    if (status) query.status = status;
    if (startDate || endDate) {
      query.createdAt = {};
      if (startDate) query.createdAt.$gte = new Date(startDate);
      if (endDate) query.createdAt.$lte = new Date(endDate);
    }

    const transactions = await Transaction.find(query)
      .populate('rackId', 'rackNumber location')
      .populate('paymentId')
      .limit(limit * 1)
      .skip((page - 1) * limit)
      .sort({ createdAt: -1 });

    const count = await Transaction.countDocuments(query);

    res.json({
      success: true,
      transactions,
      pagination: {
        total: count,
        page: parseInt(page),
        pages: Math.ceil(count / limit)
      }
    });
  } catch (error) {
    console.error('Get transactions error:', error);
    res.status(500).json({ error: 'Server error' });
  }
};

// Get transaction by ID
exports.getTransactionById = async (req, res) => {
  try {
    const transaction = await Transaction.findById(req.params.id)
      .populate('userId', 'name email phone')
      .populate('rackId', 'rackNumber location')
      .populate('paymentId');

    if (!transaction) {
      return res.status(404).json({ error: 'Transaction not found' });
    }

    // Check if user owns this transaction (or is admin)
    if (transaction.userId._id.toString() !== req.user.id && req.user.role !== 'admin') {
      return res.status(403).json({ error: 'Access denied' });
    }

    res.json({
      success: true,
      transaction
    });
  } catch (error) {
    console.error('Get transaction error:', error);
    res.status(500).json({ error: 'Server error' });
  }
};

// Generate receipt PDF
exports.generateReceipt = async (req, res) => {
  try {
    const transaction = await Transaction.findById(req.params.id)
      .populate('userId', 'name email phone')
      .populate('rackId', 'rackNumber location')
      .populate('paymentId');

    if (!transaction) {
      return res.status(404).json({ error: 'Transaction not found' });
    }

    // Check if user owns this transaction
    if (transaction.userId._id.toString() !== req.user.id && req.user.role !== 'admin') {
      return res.status(403).json({ error: 'Access denied' });
    }

    // Create PDF
    const doc = new PDFDocument({ margin: 50 });
    
    res.setHeader('Content-Type', 'application/pdf');
    res.setHeader('Content-Disposition', `attachment; filename=receipt-${transaction._id}.pdf`);

    doc.pipe(res);

    // Header
    doc.fontSize(24).font('Helvetica-Bold').text('Smart Locker System', { align: 'center' });
    doc.moveDown(0.5);
    doc.fontSize(16).font('Helvetica').text('Payment Receipt', { align: 'center' });
    doc.moveDown();

    // Divider
    doc.strokeColor('#aaaaaa').lineWidth(1).moveTo(50, doc.y).lineTo(550, doc.y).stroke();
    doc.moveDown();

    // Transaction Details
    doc.fontSize(12).font('Helvetica-Bold').text('Transaction Details');
    doc.moveDown(0.5);
    doc.font('Helvetica');
    doc.text(`Transaction ID: ${transaction._id}`);
    doc.text(`Date: ${transaction.createdAt.toLocaleString()}`);
    doc.text(`Status: ${transaction.status.toUpperCase()}`);
    doc.moveDown();

    // Rack Details
    doc.font('Helvetica-Bold').text('Locker Details');
    doc.moveDown(0.5);
    doc.font('Helvetica');
    doc.text(`Rack Number: ${transaction.rackId?.rackNumber || 'N/A'}`);
    doc.text(`Location: ${transaction.rackId?.location || 'N/A'}`);
    if (transaction.duration > 0) {
      const minutes = Math.floor(transaction.duration / 60);
      const seconds = transaction.duration % 60;
      doc.text(`Duration: ${minutes}m ${seconds}s`);
    }
    doc.moveDown();

    // Payment Details
    doc.font('Helvetica-Bold').text('Payment Details');
    doc.moveDown(0.5);
    doc.font('Helvetica');
    doc.text(`Amount: ₹${transaction.amount.toFixed(2)}`);
    if (transaction.paymentId) {
      doc.text(`Payment ID: ${transaction.paymentId.razorpayPaymentId || 'N/A'}`);
      doc.text(`Order ID: ${transaction.paymentId.razorpayOrderId || 'N/A'}`);
    }
    doc.moveDown();

    // Customer Details
    doc.font('Helvetica-Bold').text('Customer Details');
    doc.moveDown(0.5);
    doc.font('Helvetica');
    doc.text(`Name: ${transaction.userId?.name || 'N/A'}`);
    doc.text(`Email: ${transaction.userId?.email || 'N/A'}`);
    doc.text(`Phone: ${transaction.userId?.phone || 'N/A'}`);
    doc.moveDown(2);

    // Footer
    doc.strokeColor('#aaaaaa').lineWidth(1).moveTo(50, doc.y).lineTo(550, doc.y).stroke();
    doc.moveDown();
    doc.fontSize(10).fillColor('#666666').text('Thank you for using Smart Locker System!', { align: 'center' });
    doc.text('For support, contact: support@smartlocker.com', { align: 'center' });

    doc.end();
  } catch (error) {
    console.error('Generate receipt error:', error);
    res.status(500).json({ error: 'Failed to generate receipt' });
  }
};
