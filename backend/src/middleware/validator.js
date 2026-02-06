const { body, param, query, validationResult } = require('express-validator');

// Middleware to check validation results
exports.validate = (req, res, next) => {
  const errors = validationResult(req);
  if (!errors.isEmpty()) {
    return res.status(400).json({ errors: errors.array() });
  }
  next();
};

// Auth validations
exports.registerValidation = [
  body('name')
    .trim()
    .notEmpty().withMessage('Name is required')
    .isLength({ min: 2, max: 50 }).withMessage('Name must be between 2 and 50 characters'),
  body('email')
    .trim()
    .notEmpty().withMessage('Email is required')
    .isEmail().withMessage('Please provide a valid email')
    .normalizeEmail(),
  body('phone')
    .trim()
    .notEmpty().withMessage('Phone number is required')
    .matches(/^[0-9]{10}$/).withMessage('Please provide a valid 10-digit phone number'),
  body('password')
    .notEmpty().withMessage('Password is required')
    .isLength({ min: 8 }).withMessage('Password must be at least 8 characters')
    .matches(/\d/).withMessage('Password must contain at least one number')
];

exports.loginValidation = [
  body('email')
    .trim()
    .notEmpty().withMessage('Email is required')
    .isEmail().withMessage('Please provide a valid email'),
  body('password')
    .notEmpty().withMessage('Password is required')
];

// Rack validations
exports.createRackValidation = [
  body('rackNumber')
    .trim()
    .notEmpty().withMessage('Rack number is required'),
  body('location')
    .trim()
    .optional(),
  body('pricePerUse')
    .notEmpty().withMessage('Price per use is required')
    .isFloat({ min: 0 }).withMessage('Price must be a positive number'),
  body('esp32DeviceId')
    .trim()
    .optional()
];

exports.updateRackValidation = [
  param('id')
    .isMongoId().withMessage('Invalid rack ID'),
  body('rackNumber')
    .trim()
    .optional(),
  body('location')
    .trim()
    .optional(),
  body('pricePerUse')
    .optional()
    .isFloat({ min: 0 }).withMessage('Price must be a positive number'),
  body('status')
    .optional()
    .isIn(['available', 'occupied', 'maintenance']).withMessage('Invalid status')
];

// Payment validations
exports.createOrderValidation = [
  body('rackId')
    .notEmpty().withMessage('Rack ID is required')
    .isMongoId().withMessage('Invalid rack ID')
];

exports.verifyPaymentValidation = [
  body('razorpayOrderId')
    .notEmpty().withMessage('Razorpay Order ID is required'),
  body('razorpayPaymentId')
    .notEmpty().withMessage('Razorpay Payment ID is required'),
  body('razorpaySignature')
    .notEmpty().withMessage('Razorpay Signature is required'),
  body('transactionId')
    .notEmpty().withMessage('Transaction ID is required')
    .isMongoId().withMessage('Invalid transaction ID')
];

// Transaction validations
exports.transactionQueryValidation = [
  query('page')
    .optional()
    .isInt({ min: 1 }).withMessage('Page must be a positive integer'),
  query('limit')
    .optional()
    .isInt({ min: 1, max: 100 }).withMessage('Limit must be between 1 and 100'),
  query('startDate')
    .optional()
    .isISO8601().withMessage('Invalid start date format'),
  query('endDate')
    .optional()
    .isISO8601().withMessage('Invalid end date format')
];

// ID param validation
exports.idParamValidation = [
  param('id')
    .isMongoId().withMessage('Invalid ID format')
];
