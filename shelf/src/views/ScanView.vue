<template>
  <div class="min-h-screen bg-gray-50 py-12">
    <div class="max-w-lg mx-auto px-4">
      <div class="bg-white rounded-2xl shadow-lg p-8">
        <div class="text-center mb-8">
          <div class="w-16 h-16 mx-auto bg-blue-100 rounded-full flex items-center justify-center mb-4">
            <svg class="w-8 h-8 text-blue-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
                d="M12 4v1m6 11h2m-6 0h-2v4m0-11v3m0 0h.01M12 12h4.01M16 20h4M4 12h4m12 0h.01M5 8h2a1 1 0 001-1V5a1 1 0 00-1-1H5a1 1 0 00-1 1v2a1 1 0 001 1zm12 0h2a1 1 0 001-1V5a1 1 0 00-1-1h-2a1 1 0 00-1 1v2a1 1 0 001 1zM5 20h2a1 1 0 001-1v-2a1 1 0 00-1-1H5a1 1 0 00-1 1v2a1 1 0 001 1z" />
            </svg>
          </div>
          <h1 class="text-2xl font-bold text-gray-900">Scan QR Code</h1>
          <p class="mt-2 text-gray-600">Point your camera at the locker's QR code</p>
        </div>

        <QRScanner 
          @scan-success="handleScanSuccess" 
          @scan-error="handleScanError"
        />

        <div v-if="error" class="mt-6 p-4 bg-red-50 rounded-lg border border-red-200">
          <div class="flex items-center space-x-2">
            <svg class="w-5 h-5 text-red-500" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
                d="M12 8v4m0 4h.01M21 12a9 9 0 11-18 0 9 9 0 0118 0z" />
            </svg>
            <p class="text-red-700">{{ error }}</p>
          </div>
        </div>

        <div v-if="loading" class="mt-6 text-center">
          <div class="inline-flex items-center space-x-2 text-blue-600">
            <svg class="animate-spin w-5 h-5" fill="none" viewBox="0 0 24 24">
              <circle class="opacity-25" cx="12" cy="12" r="10" stroke="currentColor" stroke-width="4"></circle>
              <path class="opacity-75" fill="currentColor" d="M4 12a8 8 0 018-8V0C5.373 0 0 5.373 0 12h4zm2 5.291A7.962 7.962 0 014 12H0c0 3.042 1.135 5.824 3 7.938l3-2.647z"></path>
            </svg>
            <span>Processing QR code...</span>
          </div>
        </div>

        <!-- Manual Entry Option -->
        <div class="mt-8 pt-8 border-t border-gray-200">
          <p class="text-center text-gray-500 text-sm mb-4">Or enter locker code manually</p>
          <form @submit.prevent="handleManualEntry" class="flex space-x-2">
            <input
              v-model="manualCode"
              type="text"
              placeholder="Enter locker code"
              class="flex-1 px-4 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent"
            />
            <button
              type="submit"
              :disabled="!manualCode || loading"
              class="px-4 py-2 bg-blue-600 hover:bg-blue-700 disabled:bg-gray-400 text-white font-medium rounded-lg transition-colors"
            >
              Go
            </button>
          </form>
        </div>
      </div>

      <!-- Instructions -->
      <div class="mt-8 bg-white rounded-xl shadow-lg p-6">
        <h3 class="font-semibold text-gray-900 mb-4">Scanning Tips</h3>
        <ul class="space-y-3 text-gray-600 text-sm">
          <li class="flex items-start space-x-2">
            <svg class="w-5 h-5 text-green-500 flex-shrink-0 mt-0.5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M5 13l4 4L19 7" />
            </svg>
            <span>Ensure good lighting for best results</span>
          </li>
          <li class="flex items-start space-x-2">
            <svg class="w-5 h-5 text-green-500 flex-shrink-0 mt-0.5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M5 13l4 4L19 7" />
            </svg>
            <span>Hold your phone steady while scanning</span>
          </li>
          <li class="flex items-start space-x-2">
            <svg class="w-5 h-5 text-green-500 flex-shrink-0 mt-0.5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M5 13l4 4L19 7" />
            </svg>
            <span>Keep the QR code within the scanning frame</span>
          </li>
        </ul>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref } from 'vue'
import { useRouter } from 'vue-router'
import QRScanner from '@/components/QRScanner.vue'
import { useRackStore } from '@/stores/rack'

const router = useRouter()
const rackStore = useRackStore()

const loading = ref(false)
const error = ref(null)
const manualCode = ref('')

async function handleScanSuccess(rackId) {
  loading.value = true
  error.value = null

  try {
    // The QR scanner already extracts the rack ID from the URL
    // Navigate directly to rack details
    if (rackId && rackId.match(/^[a-f0-9]{24}$/i)) {
      // Valid MongoDB ObjectId - navigate to rack details
      router.push({ name: 'rack-details', params: { id: rackId } })
    } else {
      // Try to fetch by QR data (legacy support)
      const rack = await rackStore.fetchRackByQR(rackId)
      router.push({ name: 'rack-details', params: { id: rack._id } })
    }
  } catch (err) {
    error.value = err.response?.data?.error || 'Invalid QR code. Please try again.'
    loading.value = false
  }
}

function handleScanError(err) {
  error.value = 'Camera access denied. Please enable camera permissions in your browser settings.'
}

async function handleManualEntry() {
  if (!manualCode.value) return
  
  loading.value = true
  error.value = null

  try {
    const rack = await rackStore.fetchRackByQR(manualCode.value)
    router.push({ name: 'rack-details', params: { id: rack._id } })
  } catch (err) {
    error.value = err.response?.data?.error || 'Invalid locker code. Please try again.'
  } finally {
    loading.value = false
  }
}
</script>
