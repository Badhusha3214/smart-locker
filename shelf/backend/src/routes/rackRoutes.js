const express = require('express');
const router = express.Router();
const rackController = require('../controllers/rackController');
const auth = require('../middleware/auth');
const { unlockLimiter } = require('../middleware/rateLimiter');
const { idParamValidation, validate } = require('../middleware/validator');

// Public routes
router.get('/', rackController.getAllRacks);
router.get('/:id', idParamValidation, validate, rackController.getRackById);
router.post('/qr', rackController.getRackByQR);
router.get('/:id/status', idParamValidation, validate, rackController.getRackStatus);

// Protected routes
router.post('/:id/unlock', auth, unlockLimiter, idParamValidation, validate, rackController.unlockRack);
router.post('/:id/lock', auth, idParamValidation, validate, rackController.lockRack);
router.post('/:id/refill', auth, idParamValidation, validate, rackController.refillRack);

// Admin routes - ESP32 device management
router.get('/devices/connected', auth, rackController.getConnectedDevices);

module.exports = router;
