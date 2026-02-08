<template>
  <div class="min-h-screen bg-gray-50 py-12">
    <div class="max-w-lg mx-auto px-4">
      <!-- Loading State -->
      <div v-if="loading && !rack" class="bg-white rounded-2xl shadow-lg p-8 text-center">
        <div class="animate-spin w-12 h-12 border-4 border-blue-600 border-t-transparent rounded-full mx-auto"></div>
        <p class="mt-4 text-gray-600">Loading payment details...</p>
      </div>

      <!-- Error State -->
      <div v-else-if="error && !rack" class="bg-white rounded-2xl shadow-lg p-8 text-center">
        <div class="w-16 h-16 mx-auto bg-red-100 rounded-full flex items-center justify-center mb-4">
          <svg class="w-8 h-8 text-red-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M6 18L18 6M6 6l12 12" />
          </svg>
        </div>
        <h2 class="text-xl font-bold text-gray-900">Error</h2>
        <p class="mt-2 text-gray-600">{{ error }}</p>
        <router-link 
          to="/scan" 
          class="mt-6 inline-block px-6 py-2 bg-blue-600 hover:bg-blue-700 text-white font-medium rounded-lg transition-colors"
        >
          Go Back
        </router-link>
      </div>

      <!-- Mock Payment Modal -->
      <div v-else-if="showMockPayment" class="space-y-6">
        <div class="bg-white rounded-2xl shadow-lg overflow-hidden">
          <!-- Header -->
          <div class="bg-gradient-to-r from-blue-600 to-indigo-600 px-6 py-6 text-white">
            <div class="flex items-center justify-between">
              <h1 class="text-2xl font-bold">Mock Payment</h1>
              <span class="px-3 py-1 bg-white/20 rounded-full text-sm">Prototype</span>
            </div>
            <p class="mt-1 text-blue-100">Simulated payment for testing</p>
          </div>

          <!-- Payment Details -->
          <div class="p-6">
            <div class="bg-gray-50 rounded-lg p-4 mb-6">
              <div class="flex justify-between items-center mb-2">
                <span class="text-gray-600">Locker</span>
                <span class="font-medium text-gray-900">{{ rack.name }}</span>
              </div>
              <div class="flex justify-between items-center mb-2">
                <span class="text-gray-600">Location</span>
                <span class="font-medium text-gray-900">{{ rack.location }}</span>
              </div>
              <div class="border-t border-gray-200 pt-3 mt-3">
                <div class="flex justify-between items-center">
                  <span class="text-lg font-semibold text-gray-900">Total</span>
                  <span class="text-2xl font-bold text-green-600">₹{{ rack.price }}</span>
                </div>
              </div>
            </div>

            <!-- Mock Payment Animation -->
            <div v-if="processingPayment" class="text-center py-8">
              <div class="relative w-20 h-20 mx-auto mb-4">
                <div class="absolute inset-0 border-4 border-blue-200 rounded-full"></div>
                <div class="absolute inset-0 border-4 border-blue-600 border-t-transparent rounded-full animate-spin"></div>
                <div class="absolute inset-2 bg-blue-50 rounded-full flex items-center justify-center">
                  <svg class="w-8 h-8 text-blue-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
                      d="M3 10h18M7 15h1m4 0h1m-7 4h12a3 3 0 003-3V8a3 3 0 00-3-3H6a3 3 0 00-3 3v8a3 3 0 003 3z" />
                  </svg>
                </div>
              </div>
              <p class="text-gray-600 font-medium">Processing payment...</p>
              <p class="text-sm text-gray-500 mt-1">Please wait</p>
            </div>

            <!-- Simulated Card Selection -->
            <div v-else class="space-y-4">
              <h3 class="text-sm font-medium text-gray-500 uppercase tracking-wide">Select Payment Method</h3>
              
              <div class="space-y-2">
                <label 
                  v-for="method in paymentMethods" 
                  :key="method.id"
                  class="flex items-center p-4 border-2 rounded-lg cursor-pointer transition-all"
                  :class="selectedMethod === method.id ? 'border-blue-500 bg-blue-50' : 'border-gray-200 hover:border-gray-300'"
                >
                  <input 
                    type="radio" 
                    :value="method.id" 
                    v-model="selectedMethod"
                    class="sr-only"
                  />
                  <span class="text-2xl mr-3">{{ method.icon }}</span>
                  <div class="flex-1">
                    <span class="font-medium text-gray-900">{{ method.name }}</span>
                    <p class="text-sm text-gray-500">{{ method.desc }}</p>
                  </div>
                  <div 
                    class="w-5 h-5 rounded-full border-2 flex items-center justify-center"
                    :class="selectedMethod === method.id ? 'border-blue-500 bg-blue-500' : 'border-gray-300'"
                  >
                    <svg v-if="selectedMethod === method.id" class="w-3 h-3 text-white" fill="currentColor" viewBox="0 0 20 20">
                      <path fill-rule="evenodd" d="M16.707 5.293a1 1 0 010 1.414l-8 8a1 1 0 01-1.414 0l-4-4a1 1 0 011.414-1.414L8 12.586l7.293-7.293a1 1 0 011.414 0z" clip-rule="evenodd" />
                    </svg>
                  </div>
                </label>
              </div>

              <!-- Info Banner -->
              <div class="mt-4 p-3 bg-amber-50 border border-amber-200 rounded-lg">
                <div class="flex items-center space-x-2">
                  <svg class="w-5 h-5 text-amber-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
                      d="M13 16h-1v-4h-1m1-4h.01M21 12a9 9 0 11-18 0 9 9 0 0118 0z" />
                  </svg>
                  <p class="text-sm text-amber-800">
                    <strong>Prototype Mode:</strong> No real payment will be processed
                  </p>
                </div>
              </div>
            </div>

            <!-- Error Message -->
            <div v-if="paymentError" class="mt-6 p-4 bg-red-50 border border-red-200 rounded-lg">
              <div class="flex items-center space-x-2">
                <svg class="w-5 h-5 text-red-500" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                  <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
                    d="M12 8v4m0 4h.01M21 12a9 9 0 11-18 0 9 9 0 0118 0z" />
                </svg>
                <p class="text-red-700">{{ paymentError }}</p>
              </div>
            </div>
          </div>

          <!-- Actions -->
          <div v-if="!processingPayment" class="px-6 pb-6 space-y-3">
            <button
              @click="processPayment"
              :disabled="!selectedMethod"
              class="w-full py-4 bg-gradient-to-r from-green-600 to-emerald-600 hover:from-green-700 hover:to-emerald-700 disabled:from-gray-400 disabled:to-gray-500 text-white font-bold rounded-xl transition-all transform hover:scale-[1.02] active:scale-[0.98] shadow-lg"
            >
              Pay ₹{{ rack.price }}
            </button>
            
            <button
              @click="cancelPayment"
              class="block w-full py-3 bg-gray-100 hover:bg-gray-200 text-gray-700 font-medium rounded-lg transition-colors text-center"
            >
              Cancel
            </button>
          </div>
        </div>
      </div>

      <!-- Payment Form (Initial) -->
      <div v-else-if="rack" class="space-y-6">
        <div class="bg-white rounded-2xl shadow-lg overflow-hidden">
          <!-- Header -->
          <div class="bg-gradient-to-r from-green-600 to-emerald-600 px-6 py-6 text-white">
            <h1 class="text-2xl font-bold">Complete Payment</h1>
            <p class="mt-1 text-green-100">Mock Payment System (Prototype)</p>
          </div>

          <!-- Order Summary -->
          <div class="p-6">
            <h3 class="text-sm font-medium text-gray-500 uppercase tracking-wide mb-4">Order Summary</h3>
            
            <div class="bg-gray-50 rounded-lg p-4 space-y-3">
              <div class="flex justify-between items-center">
                <span class="text-gray-600">Locker</span>
                <span class="font-medium text-gray-900">{{ rack.name }}</span>
              </div>
              <div class="flex justify-between items-center">
                <span class="text-gray-600">Location</span>
                <span class="font-medium text-gray-900">{{ rack.location }}</span>
              </div>
              <div class="flex justify-between items-center">
                <span class="text-gray-600">Size</span>
                <span class="font-medium text-gray-900 capitalize">{{ rack.size }}</span>
              </div>
              <div class="flex justify-between items-center">
                <span class="text-gray-600">Access Duration</span>
                <span class="font-medium text-gray-900">{{ accessTime }}</span>
              </div>
              <div class="border-t border-gray-200 pt-3 mt-3">
                <div class="flex justify-between items-center">
                  <span class="text-lg font-semibold text-gray-900">Total</span>
                  <span class="text-2xl font-bold text-green-600">₹{{ rack.price }}</span>
                </div>
              </div>
            </div>

            <!-- Prototype Mode Info -->
            <div class="mt-6 p-4 bg-blue-50 border border-blue-200 rounded-lg">
              <div class="flex items-start space-x-3">
                <svg class="w-6 h-6 text-blue-600 flex-shrink-0" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                  <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
                    d="M9.663 17h4.673M12 3v1m6.364 1.636l-.707.707M21 12h-1M4 12H3m3.343-5.657l-.707-.707m2.828 9.9a5 5 0 117.072 0l-.548.547A3.374 3.374 0 0014 18.469V19a2 2 0 11-4 0v-.531c0-.895-.356-1.754-.988-2.386l-.548-.547z" />
                </svg>
                <div>
                  <h4 class="font-semibold text-blue-900">Prototype Mode</h4>
                  <p class="text-sm text-blue-700 mt-1">
                    This is a mock payment system for demonstration purposes. No real transactions will be processed.
                  </p>
                </div>
              </div>
            </div>

            <!-- Error Message -->
            <div v-if="paymentError" class="mt-6 p-4 bg-red-50 border border-red-200 rounded-lg">
              <div class="flex items-center space-x-2">
                <svg class="w-5 h-5 text-red-500" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                  <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
                    d="M12 8v4m0 4h.01M21 12a9 9 0 11-18 0 9 9 0 0118 0z" />
                </svg>
                <p class="text-red-700">{{ paymentError }}</p>
              </div>
            </div>
          </div>

          <!-- Actions -->
          <div class="px-6 pb-6 space-y-3">
            <PaymentButton
              :amount="rack.price"
              :loading="processing"
              size="lg"
              @pay="initiatePayment"
              class="w-full"
            />
            
            <router-link
              :to="{ name: 'rack-details', params: { id: rack._id } }"
              class="block w-full py-3 bg-gray-100 hover:bg-gray-200 text-gray-700 font-medium rounded-lg transition-colors text-center"
            >
              Cancel
            </router-link>
          </div>
        </div>

        <!-- Security Badge -->
        <div class="flex items-center justify-center space-x-2 text-gray-500 text-sm">
          <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
              d="M9 12l2 2 4-4m5.618-4.016A11.955 11.955 0 0112 2.944a11.955 11.955 0 01-8.618 3.04A12.02 12.02 0 003 9c0 5.591 3.824 10.29 9 11.622 5.176-1.332 9-6.03 9-11.622 0-1.042-.133-2.052-.382-3.016z" />
          </svg>
          <span>Prototype Payment System</span>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, computed, onMounted } from 'vue'
import { useRouter } from 'vue-router'
import { useRackStore } from '@/stores/rack'
import { useUserStore } from '@/stores/user'
import PaymentButton from '@/components/PaymentButton.vue'
import paymentService from '@/services/payment'

const props = defineProps({
  rackId: {
    type: String,
    required: true
  }
})

const router = useRouter()
const rackStore = useRackStore()
const userStore = useUserStore()

const loading = ref(true)
const processing = ref(false)
const processingPayment = ref(false)
const error = ref(null)
const paymentError = ref(null)
const transactionId = ref(null)
const orderId = ref(null)
const showMockPayment = ref(false)
const selectedMethod = ref('upi')

const paymentMethods = [
  { id: 'upi', name: 'UPI', desc: 'Pay using UPI apps', icon: '📱' },
  { id: 'card', name: 'Credit/Debit Card', desc: 'Visa, Mastercard, RuPay', icon: '💳' },
  { id: 'netbanking', name: 'Net Banking', desc: 'All major banks', icon: '🏦' },
  { id: 'wallet', name: 'Wallet', desc: 'Paytm, PhonePe, etc.', icon: '👛' }
]

const rack = computed(() => rackStore.currentRack)

const accessTime = computed(() => {
  if (!rack.value) return '-'
  const times = { small: '30 min', medium: '1 hour', large: '2 hours' }
  return times[rack.value.size] || '30 min'
})

async function initiatePayment() {
  processing.value = true
  paymentError.value = null

  try {
    // Create order via API
    const order = await paymentService.createOrder(props.rackId)
    transactionId.value = order.transactionId
    orderId.value = order.orderId
    
    // Show mock payment modal
    showMockPayment.value = true
    processing.value = false
  } catch (err) {
    paymentError.value = err.response?.data?.error || 'Failed to initiate payment'
    processing.value = false
  }
}

async function processPayment() {
  processingPayment.value = true
  paymentError.value = null

  try {
    // Process mock payment via backend
    await paymentService.processMockPayment(
      orderId.value,
      transactionId.value,
      handlePaymentSuccess,
      handlePaymentError
    )
  } catch (err) {
    handlePaymentError(err)
  }
}

function handlePaymentSuccess(result) {
  processingPayment.value = false
  
  // Redirect to access page
  router.push({ 
    name: 'access', 
    params: { rackId: props.rackId },
    query: { transactionId: transactionId.value }
  })
}

function handlePaymentError(error) {
  paymentError.value = error.message || 'Payment failed. Please try again.'
  processingPayment.value = false
}

function cancelPayment() {
  showMockPayment.value = false
  orderId.value = null
  transactionId.value = null
}

onMounted(async () => {
  try {
    if (!rack.value || rack.value._id !== props.rackId) {
      await rackStore.fetchRackById(props.rackId)
    }
    
    if (rack.value?.status !== 'available') {
      error.value = 'This locker is no longer available'
    }
  } catch (err) {
    error.value = err.response?.data?.error || 'Failed to load payment details'
  } finally {
    loading.value = false
  }
})
</script>
