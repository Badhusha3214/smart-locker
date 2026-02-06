module.exports = (err, req, res, next) => {
  console.error('Error:', err);

  // Mongoose Validation Error
  if (err.name === 'ValidationError') {
    const errors = Object.values(err.errors).map(e => ({
      field: e.path,
      message: e.message
    }));
    return res.status(400).json({
      error: 'Validation error',
      details: errors
    });
  }

  // Mongoose Cast Error (invalid ObjectId)
  if (err.name === 'CastError') {
    return res.status(400).json({
      error: 'Invalid ID format'
    });
  }

  // Mongoose Duplicate Key Error
  if (err.code === 11000) {
    const field = Object.keys(err.keyPattern)[0];
    return res.status(400).json({
      error: `Duplicate value for ${field}`,
      field
    });
  }

  // JWT Errors
  if (err.name === 'JsonWebTokenError') {
    return res.status(401).json({
      error: 'Invalid token'
    });
  }

  if (err.name === 'TokenExpiredError') {
    return res.status(401).json({
      error: 'Token expired'
    });
  }

  // Razorpay Errors
  if (err.error && err.error.code) {
    return res.status(400).json({
      error: 'Payment error',
      details: err.error.description
    });
  }

  // Default Server Error
  res.status(err.status || 500).json({
    error: err.message || 'Internal server error'
  });
};
