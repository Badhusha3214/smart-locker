<template>
  <div class="qr-scanner">
    <!-- Manual Entry Mode -->
    <div v-if="showManualEntry" class="text-center p-6 bg-gray-50 rounded-xl">
      <div class="w-16 h-16 mx-auto bg-gray-200 rounded-full flex items-center justify-center mb-4">
        <svg class="w-8 h-8 text-gray-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
          <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
            d="M11 5H6a2 2 0 00-2 2v11a2 2 0 002 2h11a2 2 0 002-2v-5m-1.414-9.414a2 2 0 112.828 2.828L11.828 15H9v-2.828l8.586-8.586z" />
        </svg>
      </div>
      <h3 class="text-lg font-semibold text-gray-900 mb-2">Enter Rack ID Manually</h3>
      <p class="text-gray-600 mb-4 text-sm">Find the rack ID on the locker label</p>
      
      <form @submit.prevent="submitManualEntry" class="space-y-4 max-w-sm mx-auto">
        <input
          v-model="manualRackId"
          type="text"
          placeholder="Enter Rack ID"
          class="w-full px-4 py-3 border border-gray-300 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent text-center"
        />
        <div class="flex space-x-3">
          <button
            type="button"
            @click="showManualEntry = false"
            class="flex-1 py-2 bg-gray-200 hover:bg-gray-300 text-gray-700 font-medium rounded-lg transition-colors"
          >
            Use Camera
          </button>
          <button
            type="submit"
            :disabled="!manualRackId.trim()"
            class="flex-1 py-2 bg-blue-600 hover:bg-blue-700 disabled:bg-gray-400 text-white font-medium rounded-lg transition-colors"
          >
            Submit
          </button>
        </div>
      </form>
    </div>

    <!-- Camera Mode -->
    <div v-else>
      <!-- QR Reader Container - always visible when in camera mode -->
      <div id="qr-reader-container" class="qr-reader-wrapper" v-show="isScanning">
        <div id="qr-reader"></div>
      </div>
      
      <!-- Loading state -->
      <div v-if="isStarting" class="text-center p-8">
        <div class="animate-spin w-12 h-12 border-4 border-blue-600 border-t-transparent rounded-full mx-auto mb-4"></div>
        <p class="text-gray-600">Starting camera...</p>
        <p class="text-gray-400 text-xs mt-2">{{ statusMessage }}</p>
      </div>
      
      <!-- Start button -->
      <div v-if="!isScanning && !isStarting" class="text-center p-6">
        <div class="w-20 h-20 mx-auto bg-blue-100 rounded-full flex items-center justify-center mb-4">
          <svg class="w-10 h-10 text-blue-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
              d="M3 9a2 2 0 012-2h.93a2 2 0 001.664-.89l.812-1.22A2 2 0 0110.07 4h3.86a2 2 0 011.664.89l.812 1.22A2 2 0 0018.07 7H19a2 2 0 012 2v9a2 2 0 01-2 2H5a2 2 0 01-2-2V9z" />
            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M15 13a3 3 0 11-6 0 3 3 0 016 0z" />
          </svg>
        </div>
        <h3 class="text-lg font-semibold text-gray-900 mb-2">Scan QR Code</h3>
        <p class="text-gray-600 mb-4 text-sm">Point your camera at the QR code on the locker</p>
        
        <button 
          @click="startCamera"
          class="bg-blue-600 hover:bg-blue-700 text-white font-medium py-3 px-8 rounded-lg inline-flex items-center space-x-2 transition-colors"
        >
          <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
              d="M15 10l4.553-2.276A1 1 0 0121 8.618v6.764a1 1 0 01-1.447.894L15 14M5 18h8a2 2 0 002-2V8a2 2 0 00-2-2H5a2 2 0 00-2 2v8a2 2 0 002 2z" />
          </svg>
          <span>Start Camera</span>
        </button>
        
        <div class="mt-4">
          <button 
            @click="showManualEntry = true"
            class="text-gray-500 hover:text-gray-700 text-sm underline"
          >
            Or enter rack ID manually
          </button>
        </div>
      </div>
      
      <!-- Scanning controls -->
      <div v-if="isScanning" class="mt-4 text-center">
        <p class="text-gray-600 text-sm mb-3">{{ statusMessage || 'Position the QR code within the frame' }}</p>
        <div class="flex justify-center space-x-3">
          <button 
            @click="stopCamera"
            class="bg-red-600 hover:bg-red-700 text-white font-medium py-2 px-4 rounded-lg inline-flex items-center space-x-2 transition-colors"
          >
            <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M6 18L18 6M6 6l12 12" />
            </svg>
            <span>Stop</span>
          </button>
          <button 
            @click="stopCamera(); showManualEntry = true"
            class="bg-gray-600 hover:bg-gray-700 text-white font-medium py-2 px-4 rounded-lg transition-colors"
          >
            Enter ID
          </button>
        </div>
      </div>
    </div>

    <!-- Error display -->
    <div v-if="error" class="mt-4 p-4 bg-red-50 border border-red-200 rounded-lg">
      <p class="text-red-600 text-center text-sm">{{ error }}</p>
      <div class="mt-3 text-center">
        <button 
          @click="showManualEntry = true; error = null"
          class="text-blue-600 hover:text-blue-700 text-sm underline"
        >
          Use manual entry instead
        </button>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, onUnmounted, nextTick } from 'vue'
import { Html5Qrcode } from 'html5-qrcode'

const emit = defineEmits(['scan-success', 'scan-error'])

const isScanning = ref(false)
const isStarting = ref(false)
const error = ref(null)
const showManualEntry = ref(false)
const manualRackId = ref('')
const statusMessage = ref('')

let html5QrCode = null

function submitManualEntry() {
  if (manualRackId.value.trim()) {
    emit('scan-success', manualRackId.value.trim())
    manualRackId.value = ''
    showManualEntry.value = false
  }
}

async function startCamera() {
  error.value = null
  isStarting.value = true
  isScanning.value = true  // Show the container immediately so html5-qrcode can use it
  statusMessage.value = 'Requesting camera access...'
  
  await nextTick()
  await new Promise(r => setTimeout(r, 200))  // Give DOM time to update
  
  try {
    // Cleanup any existing instance
    if (html5QrCode) {
      try {
        await html5QrCode.stop()
        html5QrCode.clear()
      } catch (e) {}
      html5QrCode = null
    }
    
    // Ensure element exists
    const readerElement = document.getElementById('qr-reader')
    if (!readerElement) {
      throw new Error('Scanner element not ready. Please refresh.')
    }
    
    // Clear any old content
    readerElement.innerHTML = ''
    
    html5QrCode = new Html5Qrcode('qr-reader')
    
    statusMessage.value = 'Looking for cameras...'
    
    // Get available cameras
    let cameraId = null
    try {
      const cameras = await Html5Qrcode.getCameras()
      console.log('Available cameras:', cameras)
      
      if (cameras && cameras.length > 0) {
        // Filter out virtual/smart cameras (Smart Connect, OBS, etc.)
        const virtualKeywords = ['smart', 'virtual', 'obs', 'screen', 'capture', 'snap']
        const realCameras = cameras.filter(c => 
          !virtualKeywords.some(keyword => c.label.toLowerCase().includes(keyword))
        )
        
        console.log('Real cameras (excluding virtual):', realCameras)
        
        const camerasToUse = realCameras.length > 0 ? realCameras : cameras
        
        // Prefer back camera on mobile, or first real camera on desktop
        const backCamera = camerasToUse.find(c => 
          c.label.toLowerCase().includes('back') || 
          c.label.toLowerCase().includes('rear') ||
          c.label.toLowerCase().includes('environment')
        )
        
        // On desktop, prefer integrated/built-in webcam
        const integratedCamera = camerasToUse.find(c =>
          c.label.toLowerCase().includes('integrated') ||
          c.label.toLowerCase().includes('built-in') ||
          c.label.toLowerCase().includes('webcam') ||
          c.label.toLowerCase().includes('hd camera') ||
          c.label.toLowerCase().includes('facetime')
        )
        
        // Priority: back camera > integrated > first real camera
        cameraId = backCamera?.id || integratedCamera?.id || camerasToUse[0]?.id
        
        const selectedCamera = cameras.find(c => c.id === cameraId)
        statusMessage.value = `Using: ${selectedCamera?.label || 'Camera'}`
        console.log('Selected camera:', selectedCamera?.label)
      }
    } catch (e) {
      console.log('Camera enumeration failed:', e)
      statusMessage.value = 'Using default camera...'
    }
    
    const config = {
      fps: 10,
      qrbox: { width: 250, height: 250 },
      aspectRatio: 1.0,
      showTorchButtonIfSupported: true,
      showZoomSliderIfSupported: true,
      defaultZoomValueIfSupported: 2
    }
    
    statusMessage.value = 'Starting video stream...'
    
    // Start scanning
    if (cameraId) {
      await html5QrCode.start(
        cameraId,
        config,
        onScanSuccess,
        onScanError
      )
    } else {
      // Try with facingMode
      try {
        await html5QrCode.start(
          { facingMode: 'environment' },
          config,
          onScanSuccess,
          onScanError
        )
      } catch (e) {
        console.log('Back camera failed, trying front:', e.message)
        await html5QrCode.start(
          { facingMode: 'user' },
          config,
          onScanSuccess,
          onScanError
        )
      }
    }
    
    isStarting.value = false
    statusMessage.value = 'Scanning for QR codes...'
    
  } catch (err) {
    console.error('Camera start error:', err)
    isStarting.value = false
    isScanning.value = false  // Reset scanning state on error
    statusMessage.value = ''
    
    const msg = err.message || err.toString()
    
    if (msg.includes('Permission') || err.name === 'NotAllowedError') {
      error.value = 'Camera permission denied. Please allow camera access and try again.'
    } else if (msg.includes('not found') || err.name === 'NotFoundError' || msg.includes('Requested device not found')) {
      error.value = 'No camera found on this device.'
    } else if (msg.includes('in use') || err.name === 'NotReadableError' || msg.includes('Could not start video source')) {
      error.value = 'Camera is busy. Close other camera apps and try again.'
    } else if (msg.includes('secure') || err.name === 'SecurityError') {
      error.value = 'Camera requires HTTPS. Please use a secure connection.'
    } else {
      error.value = `Failed to start camera: ${msg}`
    }
    
    emit('scan-error', err)
  }
}

function onScanSuccess(decodedText, decodedResult) {
  console.log('QR Code scanned:', decodedText)
  
  // Stop scanning first
  stopCamera()
  
  // Parse the QR code
  let rackId = null
  
  try {
    if (decodedText.startsWith('http')) {
      const url = new URL(decodedText)
      const pathMatch = url.pathname.match(/\/rack\/([a-f0-9]{24})/i)
      if (pathMatch) {
        rackId = pathMatch[1]
      } else {
        rackId = url.searchParams.get('rack') || url.searchParams.get('id')
      }
    } else {
      try {
        const data = JSON.parse(decodedText)
        rackId = data.rackId || data.id
      } catch {
        rackId = decodedText
      }
    }
  } catch (e) {
    rackId = decodedText
  }
  
  emit('scan-success', rackId || decodedText)
}

function onScanError(errorMessage) {
  // This is called when no QR code is found in a frame - ignore it
}

async function stopCamera() {
  statusMessage.value = ''
  
  if (html5QrCode) {
    try {
      const state = html5QrCode.getState()
      if (state === 2) { // SCANNING
        await html5QrCode.stop()
      }
      html5QrCode.clear()
    } catch (e) {
      console.log('Error stopping camera:', e)
    }
    html5QrCode = null
  }
  
  isScanning.value = false
}

onUnmounted(() => {
  stopCamera()
})

defineExpose({ startCamera, stopCamera })
</script>

<style scoped>
.qr-reader-wrapper {
  width: 100%;
  max-width: 400px;
  margin: 0 auto;
}

#qr-reader {
  width: 100%;
  border-radius: 0.75rem;
  overflow: hidden;
}

#qr-reader {
  width: 100%;
  min-height: 300px;
  background: #1f2937;
  border-radius: 0.75rem;
  overflow: hidden;
}

/* Style the html5-qrcode elements */
:deep(#qr-reader video) {
  width: 100% !important;
  height: auto !important;
  min-height: 300px !important;
  border-radius: 0.5rem !important;
  object-fit: cover !important;
  display: block !important;
}

:deep(#qr-reader__scan_region) {
  background: #1f2937 !important;
  min-height: 300px !important;
}

:deep(#qr-reader__scan_region img) {
  display: none !important;
}

:deep(#qr-reader__scan_region video) {
  width: 100% !important;
  object-fit: cover !important;
  display: block !important;
}

:deep(#qr-reader__dashboard) {
  padding: 10px !important;
  background: #f3f4f6 !important;
  border-radius: 0 0 0.75rem 0.75rem !important;
}

:deep(#qr-reader__dashboard_section_csr) {
  display: none !important;
}

:deep(#qr-reader__dashboard_section_swaplink) {
  display: none !important;
}

:deep(#qr-reader__header_message) {
  display: none !important;
}

:deep(#qr-shaded-region) {
  border-color: rgba(59, 130, 246, 0.5) !important;
}
</style>
