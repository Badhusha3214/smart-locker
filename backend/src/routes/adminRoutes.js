const express = require('express');
const router = express.Router();
const adminController = require('../controllers/adminController');
const auth = require('../middleware/auth');
const adminAuth = require('../middleware/adminAuth');
const { createRackValidation, updateRackValidation, idParamValidation, validate } = require('../middleware/validator');

// All admin routes require authentication and admin role
router.use(auth, adminAuth);

// Statistics
router.get('/stats', adminController.getStats);
router.get('/recent-transactions', adminController.getRecentTransactions);

// Rack management
router.get('/racks', adminController.getAllRacks);
router.post('/racks', createRackValidation, validate, adminController.createRack);
router.put('/racks/:id', updateRackValidation, validate, adminController.updateRack);
router.delete('/racks/:id', idParamValidation, validate, adminController.deleteRack);
router.post('/racks/:id/manual-unlock', idParamValidation, validate, adminController.manualUnlock);
router.post('/racks/:id/manual-lock', idParamValidation, validate, adminController.manualLock);

// Transaction management
router.get('/transactions', adminController.getAllTransactions);

// User management
router.get('/users', adminController.getAllUsers);

module.exports = router;
