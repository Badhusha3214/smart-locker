<template>
  <div class="px-2 sm:px-0">
    <h1 class="text-xl sm:text-2xl font-bold text-gray-900 mb-4 sm:mb-6">Dashboard</h1>

    <!-- Stats Cards -->
    <div v-if="loading" class="grid grid-cols-2 lg:grid-cols-4 gap-3 sm:gap-6 mb-6 sm:mb-8">
      <div v-for="i in 4" :key="i" class="bg-white rounded-xl shadow-lg p-4 sm:p-6 animate-pulse">
        <div class="h-3 sm:h-4 bg-gray-200 rounded w-1/2 mb-3 sm:mb-4"></div>
        <div class="h-6 sm:h-8 bg-gray-200 rounded w-3/4"></div>
      </div>
    </div>

    <div v-else class="grid grid-cols-2 lg:grid-cols-4 gap-3 sm:gap-6 mb-6 sm:mb-8">
      <div class="bg-white rounded-xl shadow-lg p-4 sm:p-6">
        <div class="flex items-center justify-between">
          <div>
            <p class="text-gray-500 text-xs sm:text-sm">Total Racks</p>
            <p class="text-xl sm:text-3xl font-bold text-gray-900 mt-1">{{ stats.totalRacks }}</p>
          </div>
          <div class="w-10 h-10 sm:w-12 sm:h-12 bg-blue-100 rounded-full flex items-center justify-center">
            <svg class="w-5 h-5 sm:w-6 sm:h-6 text-blue-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
                d="M19 11H5m14 0a2 2 0 012 2v6a2 2 0 01-2 2H5a2 2 0 01-2-2v-6a2 2 0 012-2m14 0V9a2 2 0 00-2-2M5 11V9a2 2 0 012-2m0 0V5a2 2 0 012-2h6a2 2 0 012 2v2M7 7h10" />
            </svg>
          </div>
        </div>
        <div class="mt-3 sm:mt-4 flex items-center text-xs sm:text-sm">
          <span class="text-green-600 font-medium">{{ stats.availableRacks }}</span>
          <span class="text-gray-500 ml-1">available</span>
        </div>
      </div>

      <div class="bg-white rounded-xl shadow-lg p-4 sm:p-6">
        <div class="flex items-center justify-between">
          <div>
            <p class="text-gray-500 text-xs sm:text-sm">Total Users</p>
            <p class="text-xl sm:text-3xl font-bold text-gray-900 mt-1">{{ stats.totalUsers }}</p>
          </div>
          <div class="w-10 h-10 sm:w-12 sm:h-12 bg-green-100 rounded-full flex items-center justify-center">
            <svg class="w-5 h-5 sm:w-6 sm:h-6 text-green-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
                d="M12 4.354a4 4 0 110 5.292M15 21H3v-1a6 6 0 0112 0v1zm0 0h6v-1a6 6 0 00-9-5.197M13 7a4 4 0 11-8 0 4 4 0 018 0z" />
            </svg>
          </div>
        </div>
      </div>

      <div class="bg-white rounded-xl shadow-lg p-4 sm:p-6">
        <div class="flex items-center justify-between">
          <div>
            <p class="text-gray-500 text-xs sm:text-sm">Revenue</p>
            <p class="text-lg sm:text-3xl font-bold text-gray-900 mt-1">₹{{ stats.totalRevenue?.toLocaleString() }}</p>
          </div>
          <div class="w-10 h-10 sm:w-12 sm:h-12 bg-purple-100 rounded-full flex items-center justify-center">
            <svg class="w-5 h-5 sm:w-6 sm:h-6 text-purple-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
                d="M12 8c-1.657 0-3 .895-3 2s1.343 2 3 2 3 .895 3 2-1.343 2-3 2m0-8c1.11 0 2.08.402 2.599 1M12 8V7m0 1v8m0 0v1m0-1c-1.11 0-2.08-.402-2.599-1M21 12a9 9 0 11-18 0 9 9 0 0118 0z" />
            </svg>
          </div>
        </div>
        <div class="mt-3 sm:mt-4 flex items-center text-xs sm:text-sm">
          <span class="text-green-600 font-medium">{{ stats.todayRevenue }}</span>
          <span class="text-gray-500 ml-1">today</span>
        </div>
      </div>

      <div class="bg-white rounded-xl shadow-lg p-4 sm:p-6">
        <div class="flex items-center justify-between">
          <div>
            <p class="text-gray-500 text-xs sm:text-sm">Transactions</p>
            <p class="text-xl sm:text-3xl font-bold text-gray-900 mt-1">{{ stats.totalTransactions }}</p>
          </div>
          <div class="w-10 h-10 sm:w-12 sm:h-12 bg-yellow-100 rounded-full flex items-center justify-center">
            <svg class="w-5 h-5 sm:w-6 sm:h-6 text-yellow-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
                d="M9 5H7a2 2 0 00-2 2v12a2 2 0 002 2h10a2 2 0 002-2V7a2 2 0 00-2-2h-2M9 5a2 2 0 002 2h2a2 2 0 002-2M9 5a2 2 0 012-2h2a2 2 0 012 2" />
            </svg>
          </div>
        </div>
        <div class="mt-3 sm:mt-4 flex items-center text-xs sm:text-sm">
          <span class="text-blue-600 font-medium">{{ stats.activeTransactions }}</span>
          <span class="text-gray-500 ml-1">active</span>
        </div>
      </div>
    </div>

    <!-- Rack Status Overview -->
    <div class="grid grid-cols-1 lg:grid-cols-2 gap-4 sm:gap-6 mb-6 sm:mb-8">
      <div class="bg-white rounded-xl shadow-lg p-4 sm:p-6">
        <h2 class="text-base sm:text-lg font-semibold text-gray-900 mb-4">Rack Status</h2>
        <div class="space-y-4">
          <div class="flex items-center justify-between">
            <div class="flex items-center space-x-3">
              <div class="w-3 h-3 bg-green-500 rounded-full"></div>
              <span class="text-gray-600 text-sm sm:text-base">Available</span>
            </div>
            <span class="font-semibold text-gray-900">{{ stats.availableRacks }}</span>
          </div>
          <div class="w-full bg-gray-200 rounded-full h-2">
            <div 
              class="bg-green-500 h-2 rounded-full" 
              :style="{ width: `${(stats.availableRacks / stats.totalRacks) * 100}%` }"
            ></div>
          </div>
          
          <div class="flex items-center justify-between mt-4">
            <div class="flex items-center space-x-3">
              <div class="w-3 h-3 bg-red-500 rounded-full"></div>
              <span class="text-gray-600">Occupied</span>
            </div>
            <span class="font-semibold text-gray-900">{{ stats.occupiedRacks }}</span>
          </div>
          <div class="w-full bg-gray-200 rounded-full h-2">
            <div 
              class="bg-red-500 h-2 rounded-full" 
              :style="{ width: `${(stats.occupiedRacks / stats.totalRacks) * 100}%` }"
            ></div>
          </div>

          <div class="flex items-center justify-between mt-4">
            <div class="flex items-center space-x-3">
              <div class="w-3 h-3 bg-yellow-500 rounded-full"></div>
              <span class="text-gray-600 text-sm sm:text-base">Maintenance</span>
            </div>
            <span class="font-semibold text-gray-900 text-sm sm:text-base">{{ stats.maintenanceRacks || 0 }}</span>
          </div>
        </div>
      </div>

      <div class="bg-white rounded-xl shadow-lg p-4 sm:p-6">
        <h2 class="text-base sm:text-lg font-semibold text-gray-900 mb-4">Recent Transactions</h2>
        <div v-if="recentTransactions.length === 0" class="text-center text-gray-500 py-6 sm:py-8 text-sm sm:text-base">
          No recent transactions
        </div>
        <div v-else class="space-y-2 sm:space-y-3">
          <div 
            v-for="transaction in recentTransactions" 
            :key="transaction._id"
            class="flex items-center justify-between p-2 sm:p-3 bg-gray-50 rounded-lg"
          >
            <div>
              <p class="font-medium text-gray-900 text-sm sm:text-base">{{ transaction.rack?.name || 'Unknown' }}</p>
              <p class="text-xs sm:text-sm text-gray-500">{{ formatDate(transaction.createdAt) }}</p>
            </div>
            <div class="text-right">
              <p class="font-semibold text-gray-900 text-sm sm:text-base">₹{{ transaction.amount }}</p>
              <p :class="['text-xs sm:text-sm capitalize', statusColor(transaction.status)]">
                {{ transaction.status }}
              </p>
            </div>
          </div>
        </div>
      </div>
    </div>

    <!-- Quick Actions -->
    <div class="bg-white rounded-xl shadow-lg p-4 sm:p-6">
      <h2 class="text-base sm:text-lg font-semibold text-gray-900 mb-4">Quick Actions</h2>
      <div class="grid grid-cols-2 md:grid-cols-4 gap-3 sm:gap-4">
        <router-link 
          to="/admin/racks"
          class="flex flex-col items-center p-3 sm:p-4 bg-blue-50 rounded-lg hover:bg-blue-100 transition-colors"
        >
          <svg class="w-6 h-6 sm:w-8 sm:h-8 text-blue-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M12 6v6m0 0v6m0-6h6m-6 0H6" />
          </svg>
          <span class="mt-2 text-xs sm:text-sm font-medium text-blue-600">Add Rack</span>
        </router-link>
        
        <router-link 
          to="/admin/transactions"
          class="flex flex-col items-center p-3 sm:p-4 bg-green-50 rounded-lg hover:bg-green-100 transition-colors"
        >
          <svg class="w-6 h-6 sm:w-8 sm:h-8 text-green-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
              d="M9 5H7a2 2 0 00-2 2v12a2 2 0 002 2h10a2 2 0 002-2V7a2 2 0 00-2-2h-2M9 5a2 2 0 002 2h2a2 2 0 002-2M9 5a2 2 0 012-2h2a2 2 0 012 2" />
          </svg>
          <span class="mt-2 text-xs sm:text-sm font-medium text-green-600">Transactions</span>
        </router-link>
        
        <button 
          @click="refreshData"
          class="flex flex-col items-center p-3 sm:p-4 bg-purple-50 rounded-lg hover:bg-purple-100 transition-colors"
        >
          <svg class="w-6 h-6 sm:w-8 sm:h-8 text-purple-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
              d="M4 4v5h.582m15.356 2A8.001 8.001 0 004.582 9m0 0H9m11 11v-5h-.581m0 0a8.003 8.003 0 01-15.357-2m15.357 2H15" />
          </svg>
          <span class="mt-2 text-xs sm:text-sm font-medium text-purple-600">Refresh</span>
        </button>
        
        <router-link 
          to="/"
          class="flex flex-col items-center p-3 sm:p-4 bg-gray-50 rounded-lg hover:bg-gray-100 transition-colors"
        >
          <svg class="w-6 h-6 sm:w-8 sm:h-8 text-gray-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M10 19l-7-7m0 0l7-7m-7 7h18" />
          </svg>
          <span class="mt-2 text-xs sm:text-sm font-medium text-gray-600">Back</span>
        </router-link>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import api from '@/services/api'

const loading = ref(true)
const stats = ref({
  totalRacks: 0,
  availableRacks: 0,
  occupiedRacks: 0,
  maintenanceRacks: 0,
  totalUsers: 0,
  totalRevenue: 0,
  todayRevenue: 0,
  totalTransactions: 0,
  activeTransactions: 0
})
const recentTransactions = ref([])

function statusColor(status) {
  const colors = {
    initiated: 'text-yellow-600',
    paid: 'text-blue-600',
    unlocked: 'text-green-600',
    locked: 'text-gray-600',
    expired: 'text-red-600'
  }
  return colors[status] || 'text-gray-600'
}

function formatDate(dateString) {
  return new Date(dateString).toLocaleString('en-IN', {
    day: 'numeric',
    month: 'short',
    hour: '2-digit',
    minute: '2-digit'
  })
}

async function fetchStats() {
  loading.value = true
  try {
    const response = await api.get('/admin/stats')
    stats.value = response.data.stats
    recentTransactions.value = response.data.recentTransactions || []
  } catch (err) {
    console.error('Failed to fetch stats:', err)
  } finally {
    loading.value = false
  }
}

async function refreshData() {
  await fetchStats()
}

onMounted(() => {
  fetchStats()
})
</script>
