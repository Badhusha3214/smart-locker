<template>
  <div class="min-h-screen bg-gray-50 py-12">
    <div class="max-w-lg mx-auto px-4">
      <!-- Loading State -->
      <div v-if="loading" class="bg-white rounded-2xl shadow-lg p-8 text-center">
        <div class="animate-spin w-12 h-12 border-4 border-blue-600 border-t-transparent rounded-full mx-auto"></div>
        <p class="mt-4 text-gray-600">Loading rack details...</p>
      </div>

      <!-- Error State -->
      <div v-else-if="error" class="bg-white rounded-2xl shadow-lg p-8 text-center">
        <div class="w-16 h-16 mx-auto bg-red-100 rounded-full flex items-center justify-center mb-4">
          <svg class="w-8 h-8 text-red-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M6 18L18 6M6 6l12 12" />
          </svg>
        </div>
        <h2 class="text-xl font-bold text-gray-900">Rack Not Found</h2>
        <p class="mt-2 text-gray-600">{{ error }}</p>
        <router-link 
          to="/scan" 
          class="mt-6 inline-block px-6 py-2 bg-blue-600 hover:bg-blue-700 text-white font-medium rounded-lg transition-colors"
        >
          Scan Again
        </router-link>
      </div>

      <!-- Rack Details -->
      <div v-else-if="rack" class="space-y-6">
        <div class="bg-white rounded-2xl shadow-lg overflow-hidden">
          <!-- Header -->
          <div class="bg-gradient-to-r from-blue-600 to-indigo-600 px-6 py-8 text-white">
            <div class="flex items-center justify-between">
              <div>
                <h1 class="text-2xl font-bold">{{ rack.rackNumber }}</h1>
                <p class="mt-1 text-blue-100">{{ rack.location || 'Location not specified' }}</p>
              </div>
              <LEDStatusIndicator :status="ledStatus" />
            </div>
          </div>

          <!-- Details -->
          <div class="p-6">
            <div class="grid grid-cols-2 gap-4">
              <div class="bg-gray-50 rounded-lg p-4">
                <p class="text-sm text-gray-500">Status</p>
                <p :class="['text-lg font-semibold capitalize', statusColor]">
                  {{ rack.status }}
                </p>
              </div>
              <div class="bg-gray-50 rounded-lg p-4">
                <p class="text-sm text-gray-500">Price</p>
                <p class="text-lg font-semibold text-gray-900">₹{{ rack.pricePerUse }}</p>
              </div>
              <div class="bg-gray-50 rounded-lg p-4">
                <p class="text-sm text-gray-500">LED Status</p>
                <p class="text-lg font-semibold text-gray-900 capitalize">{{ rack.ledStatus || 'off' }}</p>
              </div>
              <div class="bg-gray-50 rounded-lg p-4">
                <p class="text-sm text-gray-500">Active</p>
                <p class="text-lg font-semibold" :class="rack.isActive ? 'text-green-600' : 'text-red-600'">
                  {{ rack.isActive ? 'Yes' : 'No' }}
                </p>
              </div>
            </div>

            <!-- Description -->
            <div v-if="rack.description" class="mt-6">
              <h3 class="text-sm font-medium text-gray-500 mb-2">Description</h3>
              <p class="text-gray-700">{{ rack.description }}</p>
            </div>

            <!-- Features -->
            <div class="mt-6">
              <h3 class="text-sm font-medium text-gray-500 mb-3">Features</h3>
              <div class="flex flex-wrap gap-2">
                <span class="inline-flex items-center px-3 py-1 rounded-full text-sm bg-green-100 text-green-700">
                  <svg class="w-4 h-4 mr-1" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M5 13l4 4L19 7" />
                  </svg>
                  Secure Lock
                </span>
                <span class="inline-flex items-center px-3 py-1 rounded-full text-sm bg-blue-100 text-blue-700">
                  <svg class="w-4 h-4 mr-1" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M13 10V3L4 14h7v7l9-11h-7z" />
                  </svg>
                  Real-time Status
                </span>
                <span class="inline-flex items-center px-3 py-1 rounded-full text-sm bg-purple-100 text-purple-700">
                  <svg class="w-4 h-4 mr-1" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M12 8c-1.657 0-3 .895-3 2s1.343 2 3 2 3 .895 3 2-1.343 2-3 2m0-8c1.11 0 2.08.402 2.599 1M12 8V7m0 1v8m0 0v1m0-1c-1.11 0-2.08-.402-2.599-1M21 12a9 9 0 11-18 0 9 9 0 0118 0z" />
                  </svg>
                  Digital Payment
                </span>
              </div>
            </div>
          </div>

          <!-- Actions -->
          <div class="px-6 pb-6">
            <div v-if="rack.status === 'available' && rack.isActive" class="space-y-3">
              <button
                v-if="!userStore.isAuthenticated"
                @click="$router.push({ name: 'login', query: { redirect: $route.fullPath } })"
                class="w-full py-3 bg-blue-600 hover:bg-blue-700 text-white font-medium rounded-lg transition-colors"
              >
                Login to Book
              </button>
              <router-link
                v-else
                :to="{ name: 'payment', params: { rackId: rack._id } }"
                class="block w-full py-3 bg-blue-600 hover:bg-blue-700 text-white font-medium rounded-lg transition-colors text-center"
              >
                Proceed to Payment - ₹{{ rack.pricePerUse }}
              </router-link>
            </div>

            <div v-else-if="!rack.isActive" class="bg-red-50 border border-red-200 rounded-lg p-4 text-center">
              <p class="text-red-700 font-medium">This locker is currently inactive</p>
              <p class="text-red-600 text-sm mt-1">Please try another locker</p>
            </div>

            <div v-else class="bg-yellow-50 border border-yellow-200 rounded-lg p-4 text-center">
              <p class="text-yellow-700 font-medium">This locker is currently {{ rack.status }}</p>
              <p class="text-yellow-600 text-sm mt-1">Please try another locker</p>
            </div>
          </div>
        </div>

        <!-- Back Button -->
        <router-link 
          to="/scan" 
          class="flex items-center justify-center space-x-2 text-gray-600 hover:text-blue-600 transition-colors"
        >
          <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M10 19l-7-7m0 0l7-7m-7 7h18" />
          </svg>
          <span>Scan Another Locker</span>
        </router-link>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, computed, onMounted, onUnmounted } from 'vue'
import { useRoute } from 'vue-router'
import { useRackStore } from '@/stores/rack'
import { useUserStore } from '@/stores/user'
import LEDStatusIndicator from '@/components/LEDStatusIndicator.vue'

const props = defineProps({
  id: {
    type: String,
    required: true
  }
})

const route = useRoute()
const rackStore = useRackStore()
const userStore = useUserStore()

const loading = ref(true)
const error = ref(null)

const rack = computed(() => rackStore.currentRack)

const ledStatus = computed(() => {
  if (!rack.value) return 'off'
  switch (rack.value.status) {
    case 'available': return 'green'
    case 'occupied': return 'red'
    case 'maintenance': return 'yellow'
    default: return 'off'
  }
})

const statusColor = computed(() => {
  if (!rack.value) return 'text-gray-600'
  switch (rack.value.status) {
    case 'available': return 'text-green-600'
    case 'occupied': return 'text-red-600'
    case 'maintenance': return 'text-yellow-600'
    default: return 'text-gray-600'
  }
})

const accessTime = computed(() => {
  if (!rack.value) return '-'
  const times = { small: '30 min', medium: '1 hour', large: '2 hours' }
  return times[rack.value.size] || '30 min'
})

onMounted(async () => {
  try {
    await rackStore.fetchRackById(props.id)
    
    // Subscribe to real-time updates
    rackStore.subscribeToRackUpdates(props.id, {
      onLockStatus: (data) => {
        console.log('Lock status updated:', data)
      },
      onLEDStatus: (data) => {
        console.log('LED status updated:', data)
      }
    })
  } catch (err) {
    error.value = err.response?.data?.error || 'Failed to load rack details'
  } finally {
    loading.value = false
  }
})

onUnmounted(() => {
  if (props.id) {
    rackStore.unsubscribeFromRackUpdates(props.id)
  }
})
</script>
