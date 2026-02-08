<template>
  <div class="min-h-screen bg-gray-50 py-12">
    <div class="max-w-lg mx-auto px-4">
      <!-- Loading State -->
      <div v-if="loading" class="bg-white rounded-2xl shadow-lg p-8 text-center">
        <div class="animate-spin w-12 h-12 border-4 border-blue-600 border-t-transparent rounded-full mx-auto"></div>
        <p class="mt-4 text-gray-600">Loading access controls...</p>
      </div>

      <!-- Access Control Panel -->
      <div v-else-if="rack && transaction" class="space-y-6">
        <div class="bg-white rounded-2xl shadow-lg overflow-hidden">
          <!-- Header -->
          <div :class="['px-6 py-6 text-white', isUnlocked ? 'bg-green-600' : 'bg-blue-600']">
            <div class="flex items-center justify-between">
              <div>
                <h1 class="text-2xl font-bold">{{ rack.name }}</h1>
                <p class="mt-1 opacity-90">{{ rack.location }}</p>
              </div>
              <LEDStatusIndicator :status="ledStatus" :showText="false" />
            </div>
          </div>

          <!-- Status Display -->
          <div class="p-6">
            <div class="text-center">
              <!-- Lock Status Icon -->
              <div 
                :class="[
                  'w-24 h-24 mx-auto rounded-full flex items-center justify-center transition-all duration-500',
                  isUnlocked ? 'bg-green-100' : 'bg-gray-100'
                ]"
              >
                <svg 
                  v-if="isUnlocked"
                  class="w-12 h-12 text-green-600" 
                  fill="none" 
                  stroke="currentColor" 
                  viewBox="0 0 24 24"
                >
                  <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
                    d="M8 11V7a4 4 0 118 0m-4 8v2m-6 4h12a2 2 0 002-2v-6a2 2 0 00-2-2H6a2 2 0 00-2 2v6a2 2 0 002 2z" />
                </svg>
                <svg 
                  v-else
                  class="w-12 h-12 text-gray-400" 
                  fill="none" 
                  stroke="currentColor" 
                  viewBox="0 0 24 24"
                >
                  <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
                    d="M12 15v2m-6 4h12a2 2 0 002-2v-6a2 2 0 00-2-2H6a2 2 0 00-2 2v6a2 2 0 002 2zm10-10V7a4 4 0 00-8 0v4h8z" />
                </svg>
              </div>

              <h2 class="mt-4 text-2xl font-bold text-gray-900">
                {{ isUnlocked ? 'Locker Unlocked' : 'Locker Locked' }}
              </h2>
              <p class="mt-2 text-gray-600">
                {{ isUnlocked ? 'You can now access your locker' : 'Click unlock to open the locker' }}
              </p>
            </div>

            <!-- Countdown Timer (when unlocked) -->
            <div v-if="isUnlocked" class="mt-8">
              <CountdownTimer
                ref="timerRef"
                :duration="30"
                :autoStart="true"
                size="lg"
                @complete="handleTimerComplete"
                @warning="handleTimerWarning"
              />
              <p class="mt-4 text-center text-gray-600">
                Locker will auto-lock when timer ends
              </p>
            </div>

            <!-- Action Buttons -->
            <div class="mt-8 space-y-3">
              <button
                v-if="!isUnlocked"
                @click="handleUnlock"
                :disabled="unlocking"
                class="w-full py-4 bg-green-600 hover:bg-green-700 disabled:bg-gray-400 text-white font-medium rounded-lg transition-colors flex items-center justify-center space-x-2"
              >
                <svg v-if="unlocking" class="animate-spin w-5 h-5" fill="none" viewBox="0 0 24 24">
                  <circle class="opacity-25" cx="12" cy="12" r="10" stroke="currentColor" stroke-width="4"></circle>
                  <path class="opacity-75" fill="currentColor" d="M4 12a8 8 0 018-8V0C5.373 0 0 5.373 0 12h4zm2 5.291A7.962 7.962 0 014 12H0c0 3.042 1.135 5.824 3 7.938l3-2.647z"></path>
                </svg>
                <svg v-else class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                  <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
                    d="M8 11V7a4 4 0 118 0m-4 8v2m-6 4h12a2 2 0 002-2v-6a2 2 0 00-2-2H6a2 2 0 00-2 2v6a2 2 0 002 2z" />
                </svg>
                <span>{{ unlocking ? 'Unlocking...' : 'Unlock Locker' }}</span>
              </button>

              <button
                v-else
                @click="handleLock"
                :disabled="locking"
                class="w-full py-4 bg-red-600 hover:bg-red-700 disabled:bg-gray-400 text-white font-medium rounded-lg transition-colors flex items-center justify-center space-x-2"
              >
                <svg v-if="locking" class="animate-spin w-5 h-5" fill="none" viewBox="0 0 24 24">
                  <circle class="opacity-25" cx="12" cy="12" r="10" stroke="currentColor" stroke-width="4"></circle>
                  <path class="opacity-75" fill="currentColor" d="M4 12a8 8 0 018-8V0C5.373 0 0 5.373 0 12h4zm2 5.291A7.962 7.962 0 014 12H0c0 3.042 1.135 5.824 3 7.938l3-2.647z"></path>
                </svg>
                <svg v-else class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                  <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
                    d="M12 15v2m-6 4h12a2 2 0 002-2v-6a2 2 0 00-2-2H6a2 2 0 00-2 2v6a2 2 0 002 2zm10-10V7a4 4 0 00-8 0v4h8z" />
                </svg>
                <span>{{ locking ? 'Locking...' : 'Lock Now (Done Using)' }}</span>
              </button>
            </div>

            <!-- Error Message -->
            <div v-if="error" class="mt-4 p-4 bg-red-50 border border-red-200 rounded-lg">
              <p class="text-red-700 text-center">{{ error }}</p>
            </div>
          </div>
        </div>

        <!-- Transaction Info -->
        <div class="bg-white rounded-xl shadow-lg p-6">
          <h3 class="font-semibold text-gray-900 mb-4">Transaction Details</h3>
          <div class="space-y-2 text-sm">
            <div class="flex justify-between">
              <span class="text-gray-500">Transaction ID</span>
              <span class="font-mono text-gray-900">{{ transaction._id?.slice(-8) }}</span>
            </div>
            <div class="flex justify-between">
              <span class="text-gray-500">Amount Paid</span>
              <span class="text-gray-900">₹{{ transaction.amount }}</span>
            </div>
            <div class="flex justify-between">
              <span class="text-gray-500">Status</span>
              <span :class="['capitalize font-medium', statusColor]">{{ transaction.status }}</span>
            </div>
          </div>
        </div>

        <!-- Help Section -->
        <div class="bg-yellow-50 border border-yellow-200 rounded-xl p-4">
          <div class="flex items-start space-x-3">
            <svg class="w-5 h-5 text-yellow-600 flex-shrink-0 mt-0.5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
                d="M13 16h-1v-4h-1m1-4h.01M21 12a9 9 0 11-18 0 9 9 0 0118 0z" />
            </svg>
            <div>
              <p class="font-medium text-yellow-800">Need Help?</p>
              <p class="text-sm text-yellow-700 mt-1">
                If the locker doesn't respond, please contact support or try again in a few seconds.
              </p>
            </div>
          </div>
        </div>
      </div>

      <!-- Error State -->
      <div v-else class="bg-white rounded-2xl shadow-lg p-8 text-center">
        <div class="w-16 h-16 mx-auto bg-red-100 rounded-full flex items-center justify-center mb-4">
          <svg class="w-8 h-8 text-red-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M6 18L18 6M6 6l12 12" />
          </svg>
        </div>
        <h2 class="text-xl font-bold text-gray-900">Access Denied</h2>
        <p class="mt-2 text-gray-600">{{ error || 'Invalid access request' }}</p>
        <router-link 
          to="/scan" 
          class="mt-6 inline-block px-6 py-2 bg-blue-600 hover:bg-blue-700 text-white font-medium rounded-lg transition-colors"
        >
          Scan New Locker
        </router-link>
      </div>
    </div>

    <!-- Toast Notifications -->
    <NotificationToast
      v-if="showToast"
      v-model="showToast"
      :type="toastType"
      :message="toastMessage"
    />
  </div>
</template>

<script setup>
import { ref, computed, onMounted, onUnmounted } from 'vue'
import { useRoute, useRouter } from 'vue-router'
import { useRackStore } from '@/stores/rack'
import { useTransactionStore } from '@/stores/transaction'
import LEDStatusIndicator from '@/components/LEDStatusIndicator.vue'
import CountdownTimer from '@/components/CountdownTimer.vue'
import NotificationToast from '@/components/NotificationToast.vue'

const props = defineProps({
  rackId: {
    type: String,
    required: true
  }
})

const route = useRoute()
const router = useRouter()
const rackStore = useRackStore()
const transactionStore = useTransactionStore()

const loading = ref(true)
const unlocking = ref(false)
const locking = ref(false)
const error = ref(null)
const isUnlocked = ref(false)
const timerRef = ref(null)

// Toast
const showToast = ref(false)
const toastType = ref('info')
const toastMessage = ref('')

const rack = computed(() => rackStore.currentRack)
const transaction = computed(() => transactionStore.currentTransaction)

const ledStatus = computed(() => {
  if (isUnlocked.value) return 'green'
  return 'blue'
})

const statusColor = computed(() => {
  if (!transaction.value) return 'text-gray-600'
  switch (transaction.value.status) {
    case 'paid': return 'text-green-600'
    case 'unlocked': return 'text-blue-600'
    case 'locked': return 'text-gray-600'
    default: return 'text-yellow-600'
  }
})

function showNotification(type, message) {
  toastType.value = type
  toastMessage.value = message
  showToast.value = true
}

async function handleUnlock() {
  unlocking.value = true
  error.value = null

  try {
    await rackStore.unlockRack(props.rackId, transaction.value._id)
    isUnlocked.value = true
    showNotification('success', 'Locker unlocked successfully!')
  } catch (err) {
    error.value = err.response?.data?.error || 'Failed to unlock. Please try again.'
    showNotification('error', error.value)
  } finally {
    unlocking.value = false
  }
}

async function handleLock() {
  locking.value = true
  error.value = null

  try {
    await rackStore.lockRack(props.rackId)
    isUnlocked.value = false
    timerRef.value?.stop()
    showNotification('success', 'Locker locked successfully!')
    
    // Redirect to history after a short delay
    setTimeout(() => {
      router.push({ name: 'history' })
    }, 2000)
  } catch (err) {
    error.value = err.response?.data?.error || 'Failed to lock. Please try again.'
    showNotification('error', error.value)
  } finally {
    locking.value = false
  }
}

function handleTimerComplete() {
  handleLock()
}

function handleTimerWarning() {
  showNotification('warning', 'Locker will auto-lock in 10 seconds!')
}

onMounted(async () => {
  const transactionId = route.query.transactionId

  if (!transactionId) {
    error.value = 'Invalid access request'
    loading.value = false
    return
  }

  try {
    // Fetch rack and transaction data
    await Promise.all([
      rackStore.fetchRackById(props.rackId),
      transactionStore.fetchTransactionById(transactionId)
    ])

    // Subscribe to real-time updates
    rackStore.subscribeToRackUpdates(props.rackId, {
      onLockStatus: (data) => {
        isUnlocked.value = data.status === 'unlocked'
        if (data.status === 'locked') {
          showNotification('info', 'Locker has been locked')
        }
      },
      onLEDStatus: (data) => {
        console.log('LED status:', data)
      }
    })
  } catch (err) {
    error.value = err.response?.data?.error || 'Failed to load access details'
  } finally {
    loading.value = false
  }
})

onUnmounted(() => {
  if (props.rackId) {
    rackStore.unsubscribeFromRackUpdates(props.rackId)
  }
})
</script>
