<template>
  <div>
    <h1 class="text-xl sm:text-2xl font-bold text-gray-900 mb-6">All Transactions</h1>

    <!-- Filters -->
    <div class="bg-white rounded-xl shadow-lg p-4 mb-6">
      <div class="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-4 gap-3">
        <select 
          v-model="filters.status"
          @change="fetchTransactions"
          class="w-full px-4 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent"
        >
          <option value="">All Status</option>
          <option value="initiated">Initiated</option>
          <option value="paid">Paid</option>
          <option value="unlocked">Unlocked</option>
          <option value="locked">Completed</option>
          <option value="expired">Expired</option>
        </select>
        <input
          v-model="filters.startDate"
          @change="fetchTransactions"
          type="date"
          class="w-full px-4 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent"
        />
        <input
          v-model="filters.endDate"
          @change="fetchTransactions"
          type="date"
          class="w-full px-4 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent"
        />
        <button
          @click="exportTransactions"
          class="w-full px-4 py-2 bg-green-600 hover:bg-green-700 text-white font-medium rounded-lg transition-colors inline-flex items-center justify-center"
        >
          <svg class="w-5 h-5 mr-2" fill="none" stroke="currentColor" viewBox="0 0 24 24">
            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
              d="M4 16v1a3 3 0 003 3h10a3 3 0 003-3v-1m-4-4l-4 4m0 0l-4-4m4 4V4" />
          </svg>
          Export
        </button>
      </div>
    </div>

    <!-- Stats Summary -->
    <div class="grid grid-cols-2 lg:grid-cols-4 gap-3 sm:gap-4 mb-6">
      <div class="bg-white rounded-lg shadow p-3 sm:p-4">
        <p class="text-xs sm:text-sm text-gray-500">Total Transactions</p>
        <p class="text-xl sm:text-2xl font-bold text-gray-900">{{ pagination.total }}</p>
      </div>
      <div class="bg-white rounded-lg shadow p-3 sm:p-4">
        <p class="text-xs sm:text-sm text-gray-500">Total Amount</p>
        <p class="text-xl sm:text-2xl font-bold text-green-600">₹{{ totalAmount.toLocaleString() }}</p>
      </div>
      <div class="bg-white rounded-lg shadow p-3 sm:p-4">
        <p class="text-xs sm:text-sm text-gray-500">Completed</p>
        <p class="text-xl sm:text-2xl font-bold text-blue-600">{{ completedCount }}</p>
      </div>
      <div class="bg-white rounded-lg shadow p-3 sm:p-4">
        <p class="text-xs sm:text-sm text-gray-500">Active</p>
        <p class="text-xl sm:text-2xl font-bold text-orange-600">{{ activeCount }}</p>
      </div>
    </div>

    <!-- Loading State -->
    <div v-if="loading" class="bg-white rounded-xl shadow-lg p-8 text-center">
      <div class="animate-spin w-12 h-12 border-4 border-blue-600 border-t-transparent rounded-full mx-auto"></div>
      <p class="mt-4 text-gray-600">Loading transactions...</p>
    </div>

    <!-- Mobile Card View -->
    <div v-else class="block lg:hidden space-y-4">
      <div v-if="transactions.length === 0" class="bg-white rounded-xl shadow-lg p-8 text-center text-gray-500">
        No transactions found
      </div>
      <div 
        v-for="transaction in transactions" 
        :key="transaction._id + '-mobile'"
        class="bg-white rounded-xl shadow-lg p-4"
      >
        <div class="flex items-start justify-between mb-3">
          <div>
            <p class="font-mono text-xs text-gray-500">{{ transaction._id.slice(-8) }}</p>
            <p class="font-medium text-gray-900">{{ transaction.user?.name || 'Unknown' }}</p>
          </div>
          <span :class="['px-2 py-1 text-xs font-medium rounded-full', statusBadge(transaction.status)]">
            {{ formatStatus(transaction.status) }}
          </span>
        </div>
        
        <div class="grid grid-cols-2 gap-3 text-sm mb-3">
          <div>
            <span class="text-gray-500">Rack:</span>
            <span class="font-medium text-gray-900 ml-1">{{ transaction.rack?.name || 'N/A' }}</span>
          </div>
          <div>
            <span class="text-gray-500">Amount:</span>
            <span class="font-bold text-gray-900 ml-1">₹{{ transaction.amount }}</span>
          </div>
        </div>
        
        <div class="flex items-center justify-between pt-3 border-t border-gray-100">
          <span class="text-xs text-gray-500">{{ formatDate(transaction.createdAt) }} {{ formatTime(transaction.createdAt) }}</span>
          <button
            @click="viewDetails(transaction)"
            class="p-2 text-blue-600 hover:bg-blue-50 rounded-lg"
          >
            <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M15 12a3 3 0 11-6 0 3 3 0 016 0z" />
              <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M2.458 12C3.732 7.943 7.523 5 12 5c4.478 0 8.268 2.943 9.542 7-1.274 4.057-5.064 7-9.542 7-4.477 0-8.268-2.943-9.542-7z" />
            </svg>
          </button>
        </div>
      </div>
    </div>

    <!-- Desktop Table View -->
    <div v-if="!loading" class="hidden lg:block bg-white rounded-xl shadow-lg overflow-hidden">
      <div class="overflow-x-auto">
        <table class="min-w-full divide-y divide-gray-200">
          <thead class="bg-gray-50">
            <tr>
              <th class="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase tracking-wider">ID</th>
              <th class="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase tracking-wider">User</th>
              <th class="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase tracking-wider">Rack</th>
              <th class="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase tracking-wider">Amount</th>
              <th class="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase tracking-wider">Status</th>
              <th class="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase tracking-wider">Date</th>
              <th class="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase tracking-wider">Actions</th>
            </tr>
          </thead>
          <tbody class="bg-white divide-y divide-gray-200">
            <tr v-for="transaction in transactions" :key="transaction._id" class="hover:bg-gray-50">
              <td class="px-6 py-4 whitespace-nowrap">
                <span class="font-mono text-sm text-gray-600">{{ transaction._id.slice(-8) }}</span>
              </td>
              <td class="px-6 py-4 whitespace-nowrap">
                <div class="font-medium text-gray-900">{{ transaction.user?.name || 'Unknown' }}</div>
                <div class="text-sm text-gray-500">{{ transaction.user?.email || '-' }}</div>
              </td>
              <td class="px-6 py-4 whitespace-nowrap">
                <div class="font-medium text-gray-900">{{ transaction.rack?.name || 'Unknown' }}</div>
                <div class="text-sm text-gray-500">{{ transaction.rack?.location || '-' }}</div>
              </td>
              <td class="px-6 py-4 whitespace-nowrap font-medium text-gray-900">
                ₹{{ transaction.amount }}
              </td>
              <td class="px-6 py-4 whitespace-nowrap">
                <span :class="['px-2 py-1 text-xs font-medium rounded-full', statusBadge(transaction.status)]">
                  {{ formatStatus(transaction.status) }}
                </span>
              </td>
              <td class="px-6 py-4 whitespace-nowrap">
                <div class="text-sm text-gray-900">{{ formatDate(transaction.createdAt) }}</div>
                <div class="text-xs text-gray-500">{{ formatTime(transaction.createdAt) }}</div>
              </td>
              <td class="px-6 py-4 whitespace-nowrap">
                <button
                  @click="viewDetails(transaction)"
                  class="text-blue-600 hover:text-blue-700"
                  title="View Details"
                >
                  <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
                      d="M15 12a3 3 0 11-6 0 3 3 0 016 0z" />
                    <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
                      d="M2.458 12C3.732 7.943 7.523 5 12 5c4.478 0 8.268 2.943 9.542 7-1.274 4.057-5.064 7-9.542 7-4.477 0-8.268-2.943-9.542-7z" />
                  </svg>
                </button>
              </td>
            </tr>
            <tr v-if="transactions.length === 0">
              <td colspan="7" class="px-6 py-12 text-center text-gray-500">
                No transactions found
              </td>
            </tr>
          </tbody>
        </table>
      </div>

      <!-- Pagination -->
      <div v-if="pagination.pages > 1" class="px-6 py-4 border-t border-gray-200 flex items-center justify-between">
        <p class="text-sm text-gray-600">
          Showing page {{ pagination.page }} of {{ pagination.pages }}
        </p>
        <div class="flex space-x-2">
          <button
            @click="changePage(pagination.page - 1)"
            :disabled="pagination.page === 1"
            class="px-3 py-1 rounded border border-gray-300 disabled:opacity-50 disabled:cursor-not-allowed hover:bg-gray-50"
          >
            Previous
          </button>
          <button
            @click="changePage(pagination.page + 1)"
            :disabled="pagination.page === pagination.pages"
            class="px-3 py-1 rounded border border-gray-300 disabled:opacity-50 disabled:cursor-not-allowed hover:bg-gray-50"
          >
            Next
          </button>
        </div>
      </div>
    </div>

    <!-- Details Modal -->
    <div v-if="showDetailsModal" class="fixed inset-0 z-50 overflow-y-auto">
      <div class="flex items-center justify-center min-h-screen px-4">
        <div class="fixed inset-0 bg-black bg-opacity-50" @click="showDetailsModal = false"></div>
        <div class="relative bg-white rounded-xl shadow-xl max-w-lg w-full p-6">
          <h2 class="text-xl font-bold text-gray-900 mb-4">Transaction Details</h2>
          
          <div v-if="selectedTransaction" class="space-y-4">
            <div class="grid grid-cols-2 gap-4">
              <div>
                <p class="text-sm text-gray-500">Transaction ID</p>
                <p class="font-mono text-sm">{{ selectedTransaction._id }}</p>
              </div>
              <div>
                <p class="text-sm text-gray-500">Status</p>
                <span :class="['px-2 py-1 text-xs font-medium rounded-full', statusBadge(selectedTransaction.status)]">
                  {{ formatStatus(selectedTransaction.status) }}
                </span>
              </div>
              <div>
                <p class="text-sm text-gray-500">User</p>
                <p class="font-medium">{{ selectedTransaction.user?.name }}</p>
                <p class="text-sm text-gray-600">{{ selectedTransaction.user?.email }}</p>
              </div>
              <div>
                <p class="text-sm text-gray-500">Rack</p>
                <p class="font-medium">{{ selectedTransaction.rack?.name }}</p>
                <p class="text-sm text-gray-600">{{ selectedTransaction.rack?.location }}</p>
              </div>
              <div>
                <p class="text-sm text-gray-500">Amount</p>
                <p class="text-xl font-bold text-green-600">₹{{ selectedTransaction.amount }}</p>
              </div>
              <div>
                <p class="text-sm text-gray-500">Created At</p>
                <p class="font-medium">{{ formatDate(selectedTransaction.createdAt) }}</p>
                <p class="text-sm text-gray-600">{{ formatTime(selectedTransaction.createdAt) }}</p>
              </div>
            </div>

            <div v-if="selectedTransaction.payment" class="pt-4 border-t">
              <h3 class="font-semibold text-gray-900 mb-2">Payment Details</h3>
              <div class="grid grid-cols-2 gap-4">
                <div>
                  <p class="text-sm text-gray-500">Razorpay Order ID</p>
                  <p class="font-mono text-xs">{{ selectedTransaction.payment?.razorpayOrderId || '-' }}</p>
                </div>
                <div>
                  <p class="text-sm text-gray-500">Payment ID</p>
                  <p class="font-mono text-xs">{{ selectedTransaction.payment?.razorpayPaymentId || '-' }}</p>
                </div>
              </div>
            </div>

            <div class="pt-4 border-t">
              <h3 class="font-semibold text-gray-900 mb-2">Timeline</h3>
              <div class="space-y-2 text-sm">
                <div v-if="selectedTransaction.unlockedAt" class="flex justify-between">
                  <span class="text-gray-500">Unlocked</span>
                  <span>{{ formatDateTime(selectedTransaction.unlockedAt) }}</span>
                </div>
                <div v-if="selectedTransaction.lockedAt" class="flex justify-between">
                  <span class="text-gray-500">Locked</span>
                  <span>{{ formatDateTime(selectedTransaction.lockedAt) }}</span>
                </div>
              </div>
            </div>
          </div>

          <button
            @click="showDetailsModal = false"
            class="w-full mt-6 py-2 bg-gray-100 hover:bg-gray-200 text-gray-700 font-medium rounded-lg transition-colors"
          >
            Close
          </button>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, computed, onMounted } from 'vue'
import { useTransactionStore } from '@/stores/transaction'

const transactionStore = useTransactionStore()

const loading = ref(true)
const showDetailsModal = ref(false)
const selectedTransaction = ref(null)

const filters = ref({
  status: '',
  startDate: '',
  endDate: ''
})

const transactions = computed(() => transactionStore.transactions)
const pagination = computed(() => transactionStore.pagination)

const totalAmount = computed(() => {
  return transactions.value.reduce((sum, t) => sum + (t.amount || 0), 0)
})

const completedCount = computed(() => {
  return transactions.value.filter(t => t.status === 'locked').length
})

const activeCount = computed(() => {
  return transactions.value.filter(t => ['paid', 'unlocked'].includes(t.status)).length
})

function statusBadge(status) {
  const badges = {
    initiated: 'bg-yellow-100 text-yellow-700',
    paid: 'bg-blue-100 text-blue-700',
    unlocked: 'bg-green-100 text-green-700',
    locked: 'bg-gray-100 text-gray-700',
    expired: 'bg-red-100 text-red-700'
  }
  return badges[status] || 'bg-gray-100 text-gray-700'
}

function formatStatus(status) {
  const labels = {
    initiated: 'Pending',
    paid: 'Paid',
    unlocked: 'In Use',
    locked: 'Completed',
    expired: 'Expired'
  }
  return labels[status] || status
}

function formatDate(dateString) {
  return new Date(dateString).toLocaleDateString('en-IN', {
    day: 'numeric',
    month: 'short',
    year: 'numeric'
  })
}

function formatTime(dateString) {
  return new Date(dateString).toLocaleTimeString('en-IN', {
    hour: '2-digit',
    minute: '2-digit'
  })
}

function formatDateTime(dateString) {
  return new Date(dateString).toLocaleString('en-IN', {
    day: 'numeric',
    month: 'short',
    hour: '2-digit',
    minute: '2-digit'
  })
}

function viewDetails(transaction) {
  selectedTransaction.value = transaction
  showDetailsModal.value = true
}

async function fetchTransactions() {
  loading.value = true
  try {
    const params = { ...filters.value }
    if (!params.status) delete params.status
    if (!params.startDate) delete params.startDate
    if (!params.endDate) delete params.endDate
    
    await transactionStore.fetchAllTransactions(params)
  } finally {
    loading.value = false
  }
}

async function changePage(page) {
  if (page < 1 || page > pagination.value.pages) return
  
  loading.value = true
  try {
    await transactionStore.fetchAllTransactions({ ...filters.value, page })
  } finally {
    loading.value = false
  }
}

function exportTransactions() {
  // Create CSV content
  const headers = ['ID', 'User', 'Email', 'Rack', 'Amount', 'Status', 'Date']
  const rows = transactions.value.map(t => [
    t._id,
    t.user?.name || '',
    t.user?.email || '',
    t.rack?.name || '',
    t.amount,
    t.status,
    new Date(t.createdAt).toISOString()
  ])
  
  const csv = [headers, ...rows].map(row => row.join(',')).join('\n')
  const blob = new Blob([csv], { type: 'text/csv' })
  const url = window.URL.createObjectURL(blob)
  const link = document.createElement('a')
  link.href = url
  link.download = `transactions-${new Date().toISOString().split('T')[0]}.csv`
  link.click()
  window.URL.revokeObjectURL(url)
}

onMounted(() => {
  fetchTransactions()
})
</script>
