<template>
  <div class="min-h-screen bg-gray-50 py-12">
    <div class="max-w-4xl mx-auto px-4">
      <div class="flex items-center justify-between mb-8">
        <div>
          <h1 class="text-3xl font-bold text-gray-900">Transaction History</h1>
          <p class="mt-1 text-gray-600">View your past locker usage</p>
        </div>
        <router-link 
          to="/scan"
          class="inline-flex items-center px-4 py-2 bg-blue-600 hover:bg-blue-700 text-white font-medium rounded-lg transition-colors"
        >
          <svg class="w-5 h-5 mr-2" fill="none" stroke="currentColor" viewBox="0 0 24 24">
            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M12 4v16m8-8H4" />
          </svg>
          New Booking
        </router-link>
      </div>

      <!-- Filters -->
      <div class="bg-white rounded-xl shadow-lg p-4 mb-6">
        <div class="flex flex-wrap gap-4">
          <select 
            v-model="filters.status"
            class="px-4 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent"
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
            type="date"
            class="px-4 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent"
            placeholder="Start Date"
          />
          <input
            v-model="filters.endDate"
            type="date"
            class="px-4 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent"
            placeholder="End Date"
          />
          <button
            @click="fetchData"
            class="px-4 py-2 bg-gray-100 hover:bg-gray-200 text-gray-700 font-medium rounded-lg transition-colors"
          >
            Apply Filters
          </button>
        </div>
      </div>

      <!-- Loading State -->
      <div v-if="loading" class="bg-white rounded-xl shadow-lg p-8 text-center">
        <div class="animate-spin w-12 h-12 border-4 border-blue-600 border-t-transparent rounded-full mx-auto"></div>
        <p class="mt-4 text-gray-600">Loading transactions...</p>
      </div>

      <!-- Empty State -->
      <div v-else-if="transactions.length === 0" class="bg-white rounded-xl shadow-lg p-12 text-center">
        <div class="w-20 h-20 mx-auto bg-gray-100 rounded-full flex items-center justify-center mb-4">
          <svg class="w-10 h-10 text-gray-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
              d="M9 5H7a2 2 0 00-2 2v12a2 2 0 002 2h10a2 2 0 002-2V7a2 2 0 00-2-2h-2M9 5a2 2 0 002 2h2a2 2 0 002-2M9 5a2 2 0 012-2h2a2 2 0 012 2" />
          </svg>
        </div>
        <h3 class="text-xl font-semibold text-gray-900">No Transactions Yet</h3>
        <p class="mt-2 text-gray-600">Your locker usage history will appear here</p>
        <router-link 
          to="/scan"
          class="mt-6 inline-block px-6 py-3 bg-blue-600 hover:bg-blue-700 text-white font-medium rounded-lg transition-colors"
        >
          Book Your First Locker
        </router-link>
      </div>

      <!-- Transactions List -->
      <div v-else class="space-y-4">
        <div 
          v-for="transaction in transactions" 
          :key="transaction._id"
          class="bg-white rounded-xl shadow-lg overflow-hidden hover:shadow-xl transition-shadow"
        >
          <div class="p-6">
            <div class="flex items-center justify-between">
              <div class="flex items-center space-x-4">
                <div :class="['w-12 h-12 rounded-full flex items-center justify-center', statusBgColor(transaction.status)]">
                  <component :is="statusIcon(transaction.status)" :class="['w-6 h-6', statusIconColor(transaction.status)]" />
                </div>
                <div>
                  <h3 class="font-semibold text-gray-900">
                    {{ transaction.rack?.name || 'Unknown Locker' }}
                  </h3>
                  <p class="text-sm text-gray-500">
                    {{ transaction.rack?.location || 'Unknown Location' }}
                  </p>
                </div>
              </div>
              <div class="text-right">
                <p class="text-lg font-bold text-gray-900">₹{{ transaction.amount }}</p>
                <p :class="['text-sm font-medium capitalize', statusTextColor(transaction.status)]">
                  {{ formatStatus(transaction.status) }}
                </p>
              </div>
            </div>

            <div class="mt-4 pt-4 border-t border-gray-100 flex items-center justify-between">
              <div class="flex items-center space-x-4 text-sm text-gray-500">
                <span class="flex items-center">
                  <svg class="w-4 h-4 mr-1" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
                      d="M8 7V3m8 4V3m-9 8h10M5 21h14a2 2 0 002-2V7a2 2 0 00-2-2H5a2 2 0 00-2 2v12a2 2 0 002 2z" />
                  </svg>
                  {{ formatDate(transaction.createdAt) }}
                </span>
                <span class="flex items-center">
                  <svg class="w-4 h-4 mr-1" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
                      d="M12 8v4l3 3m6-3a9 9 0 11-18 0 9 9 0 0118 0z" />
                  </svg>
                  {{ formatTime(transaction.createdAt) }}
                </span>
              </div>
              <button
                v-if="transaction.status === 'locked'"
                @click="downloadReceipt(transaction._id)"
                class="flex items-center space-x-1 text-blue-600 hover:text-blue-700 text-sm font-medium"
              >
                <svg class="w-4 h-4" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                  <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
                    d="M4 16v1a3 3 0 003 3h10a3 3 0 003-3v-1m-4-4l-4 4m0 0l-4-4m4 4V4" />
                </svg>
                <span>Receipt</span>
              </button>
            </div>
          </div>
        </div>

        <!-- Pagination -->
        <div v-if="pagination.pages > 1" class="flex justify-center mt-8">
          <nav class="flex items-center space-x-2">
            <button
              @click="changePage(pagination.page - 1)"
              :disabled="pagination.page === 1"
              class="px-3 py-2 rounded-lg border border-gray-300 disabled:opacity-50 disabled:cursor-not-allowed hover:bg-gray-50"
            >
              Previous
            </button>
            <span class="px-4 py-2 text-gray-600">
              Page {{ pagination.page }} of {{ pagination.pages }}
            </span>
            <button
              @click="changePage(pagination.page + 1)"
              :disabled="pagination.page === pagination.pages"
              class="px-3 py-2 rounded-lg border border-gray-300 disabled:opacity-50 disabled:cursor-not-allowed hover:bg-gray-50"
            >
              Next
            </button>
          </nav>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, computed, onMounted, h } from 'vue'
import { useTransactionStore } from '@/stores/transaction'

const transactionStore = useTransactionStore()

const loading = ref(true)
const filters = ref({
  status: '',
  startDate: '',
  endDate: ''
})

const transactions = computed(() => transactionStore.transactions)
const pagination = computed(() => transactionStore.pagination)

function statusBgColor(status) {
  const colors = {
    initiated: 'bg-yellow-100',
    paid: 'bg-blue-100',
    unlocked: 'bg-green-100',
    locked: 'bg-gray-100',
    expired: 'bg-red-100'
  }
  return colors[status] || 'bg-gray-100'
}

function statusIconColor(status) {
  const colors = {
    initiated: 'text-yellow-600',
    paid: 'text-blue-600',
    unlocked: 'text-green-600',
    locked: 'text-gray-600',
    expired: 'text-red-600'
  }
  return colors[status] || 'text-gray-600'
}

function statusTextColor(status) {
  const colors = {
    initiated: 'text-yellow-600',
    paid: 'text-blue-600',
    unlocked: 'text-green-600',
    locked: 'text-gray-600',
    expired: 'text-red-600'
  }
  return colors[status] || 'text-gray-600'
}

function statusIcon(status) {
  const icons = {
    initiated: {
      render() {
        return h('svg', { fill: 'none', stroke: 'currentColor', viewBox: '0 0 24 24' }, [
          h('path', { 'stroke-linecap': 'round', 'stroke-linejoin': 'round', 'stroke-width': '2', d: 'M12 8v4l3 3m6-3a9 9 0 11-18 0 9 9 0 0118 0z' })
        ])
      }
    },
    paid: {
      render() {
        return h('svg', { fill: 'none', stroke: 'currentColor', viewBox: '0 0 24 24' }, [
          h('path', { 'stroke-linecap': 'round', 'stroke-linejoin': 'round', 'stroke-width': '2', d: 'M9 12l2 2 4-4m6 2a9 9 0 11-18 0 9 9 0 0118 0z' })
        ])
      }
    },
    unlocked: {
      render() {
        return h('svg', { fill: 'none', stroke: 'currentColor', viewBox: '0 0 24 24' }, [
          h('path', { 'stroke-linecap': 'round', 'stroke-linejoin': 'round', 'stroke-width': '2', d: 'M8 11V7a4 4 0 118 0m-4 8v2m-6 4h12a2 2 0 002-2v-6a2 2 0 00-2-2H6a2 2 0 00-2 2v6a2 2 0 002 2z' })
        ])
      }
    },
    locked: {
      render() {
        return h('svg', { fill: 'none', stroke: 'currentColor', viewBox: '0 0 24 24' }, [
          h('path', { 'stroke-linecap': 'round', 'stroke-linejoin': 'round', 'stroke-width': '2', d: 'M5 13l4 4L19 7' })
        ])
      }
    },
    expired: {
      render() {
        return h('svg', { fill: 'none', stroke: 'currentColor', viewBox: '0 0 24 24' }, [
          h('path', { 'stroke-linecap': 'round', 'stroke-linejoin': 'round', 'stroke-width': '2', d: 'M6 18L18 6M6 6l12 12' })
        ])
      }
    }
  }
  return icons[status] || icons.initiated
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

async function fetchData() {
  loading.value = true
  try {
    const params = { ...filters.value }
    if (!params.status) delete params.status
    if (!params.startDate) delete params.startDate
    if (!params.endDate) delete params.endDate
    
    await transactionStore.fetchTransactions(params)
  } finally {
    loading.value = false
  }
}

async function changePage(page) {
  if (page < 1 || page > pagination.value.pages) return
  
  loading.value = true
  try {
    await transactionStore.fetchTransactions({ ...filters.value, page })
  } finally {
    loading.value = false
  }
}

async function downloadReceipt(transactionId) {
  try {
    await transactionStore.downloadReceipt(transactionId)
  } catch (err) {
    console.error('Failed to download receipt:', err)
  }
}

onMounted(() => {
  fetchData()
})
</script>
