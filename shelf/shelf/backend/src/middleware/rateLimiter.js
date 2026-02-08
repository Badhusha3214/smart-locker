const rateLimit = require('express-rate-limit');

// General API rate limiter
exports.apiLimiter = rateLimit({
  windowMs: 1 * 60 * 1000, // 1 minute
  max: 100,
  message: {
    error: 'Too many requests from this IP, please try again later'
  },
  standardHeaders: true,
  legacyHeaders: false
});

// Payment rate limiter - stricter
exports.paymentLimiter = rateLimit({
  windowMs: 15 * 60 * 1000, // 15 minutes
  max: 5,
  message: {
    error: 'Too many payment requests from this IP, please try again later'
  },
  standardHeaders: true,
  legacyHeaders: false
});

// Auth rate limiter - to prevent brute force
exports.authLimiter = rateLimit({
  windowMs: 1 * 60 * 1000, // 1 minute (reduced for development)
  max: 50, // 50 attempts per minute for development
  message: {
    error: 'Too many authentication attempts, please try again later'
  },
  standardHeaders: true,
  legacyHeaders: false,
  skipSuccessfulRequests: true // Don't count successful logins
});

// Unlock rate limiter
exports.unlockLimiter = rateLimit({
  windowMs: 1 * 60 * 1000, // 1 minute
  max: 3,
  message: {
    error: 'Too many unlock attempts, please try again later'
  },
  standardHeaders: true,
  legacyHeaders: false
});
