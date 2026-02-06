import { defineStore } from 'pinia'
import { ref, computed } from 'vue'
import api from '@/services/api'

export const useTransactionStore = defineStore('transaction', () => {
  const transactions = ref([])
  const currentTransaction = ref(null)
  const loading = ref(false)
  const error = ref(null)
  const pagination = ref({
    total: 0,
    page: 1,
    pages: 1
  })

  const completedTransactions = computed(() =>
    transactions.value.filter(t => t.status === 'locked')
  )

  const pendingTransactions = computed(() =>
    transactions.value.filter(t => ['initiated', 'paid', 'unlocked'].includes(t.status))
  )

  async function fetchTransactions(filters = {}) {
    loading.value = true
    error.value = null
    try {
      const response = await api.get('/transactions', { params: filters })
      transactions.value = response.data.transactions
      pagination.value = response.data.pagination
      return response.data
    } catch (err) {
      error.value = err.response?.data?.error || 'Failed to fetch transactions'
      throw err
    } finally {
      loading.value = false
    }
  }

  async function fetchTransactionById(id) {
    loading.value = true
    error.value = null
    try {
      const response = await api.get(`/transactions/${id}`)
      currentTransaction.value = response.data.transaction
      return response.data.transaction
    } catch (err) {
      error.value = err.response?.data?.error || 'Failed to fetch transaction'
      throw err
    } finally {
      loading.value = false
    }
  }

  async function downloadReceipt(id) {
    try {
      const response = await api.get(`/transactions/${id}/receipt`, {
        responseType: 'blob'
      })
      
      const url = window.URL.createObjectURL(new Blob([response.data]))
      const link = document.createElement('a')
      link.href = url
      link.setAttribute('download', `receipt-${id}.pdf`)
      document.body.appendChild(link)
      link.click()
      link.remove()
      window.URL.revokeObjectURL(url)
      
      return { success: true }
    } catch (err) {
      error.value = 'Failed to download receipt'
      throw err
    }
  }

  function setCurrentTransaction(transaction) {
    currentTransaction.value = transaction
  }

  function clearCurrentTransaction() {
    currentTransaction.value = null
  }

  // Admin functions
  async function fetchAllTransactions(filters = {}) {
    loading.value = true
    try {
      const response = await api.get('/admin/transactions', { params: filters })
      transactions.value = response.data.transactions
      pagination.value = response.data.pagination
      return response.data
    } catch (err) {
      error.value = err.response?.data?.error || 'Failed to fetch transactions'
      throw err
    } finally {
      loading.value = false
    }
  }

  return {
    transactions,
    currentTransaction,
    loading,
    error,
    pagination,
    completedTransactions,
    pendingTransactions,
    fetchTransactions,
    fetchTransactionById,
    downloadReceipt,
    setCurrentTransaction,
    clearCurrentTransaction,
    fetchAllTransactions
  }
})
