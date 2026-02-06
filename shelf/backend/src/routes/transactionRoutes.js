const express = require('express');
const router = express.Router();
const transactionController = require('../controllers/transactionController');
const auth = require('../middleware/auth');
const { transactionQueryValidation, idParamValidation, validate } = require('../middleware/validator');

// All routes are protected
router.use(auth);

router.get('/', transactionQueryValidation, validate, transactionController.getUserTransactions);
router.get('/:id', idParamValidation, validate, transactionController.getTransactionById);
router.get('/:id/receipt', idParamValidation, validate, transactionController.generateReceipt);

module.exports = router;
