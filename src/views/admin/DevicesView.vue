<template>
  <div class="min-h-screen bg-gray-900 text-white p-3 sm:p-6">
    <div class="max-w-7xl mx-auto">
      <!-- Header -->
      <div class="mb-6 sm:mb-8">
        <h1 class="text-2xl sm:text-3xl font-bold mb-2">Shelf Controllers</h1>
        <p class="text-gray-400 text-sm sm:text-base">ESP32 controllers managing multiple locker doors per shelf</p>
      </div>

      <!-- Stats Cards -->
      <div class="grid grid-cols-2 lg:grid-cols-4 gap-3 sm:gap-6 mb-6 sm:mb-8">
        <div class="bg-gray-800 rounded-xl p-4 sm:p-6">
          <div class="flex items-center gap-3 sm:gap-4">
            <div class="w-10 h-10 sm:w-12 sm:h-12 bg-purple-900/50 rounded-lg flex items-center justify-center">
              <svg class="w-5 h-5 sm:w-6 sm:h-6 text-purple-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M9 3v2m6-2v2M9 19v2m6-2v2M5 9H3m2 6H3m18-6h-2m2 6h-2M7 19h10a2 2 0 002-2V7a2 2 0 00-2-2H7a2 2 0 00-2 2v10a2 2 0 002 2zM9 9h6v6H9V9z" />
              </svg>
            </div>
            <div>
              <p class="text-gray-400 text-xs sm:text-sm">Shelves Online</p>
              <p class="text-xl sm:text-2xl font-bold text-purple-400">{{ shelves.length }}</p>
            </div>
          </div>
        </div>

        <div class="bg-gray-800 rounded-xl p-4 sm:p-6">
          <div class="flex items-center gap-3 sm:gap-4">
            <div class="w-10 h-10 sm:w-12 sm:h-12 bg-green-900/50 rounded-lg flex items-center justify-center">
              <svg class="w-5 h-5 sm:w-6 sm:h-6 text-green-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M9 12l2 2 4-4m6 2a9 9 0 11-18 0 9 9 0 0118 0z" />
              </svg>
            </div>
            <div>
              <p class="text-gray-400 text-xs sm:text-sm">Doors Online</p>
              <p class="text-xl sm:text-2xl font-bold text-green-400">{{ connectedDoorsCount }}</p>
            </div>
          </div>
        </div>

        <div class="bg-gray-800 rounded-xl p-4 sm:p-6">
          <div class="flex items-center gap-3 sm:gap-4">
            <div class="w-10 h-10 sm:w-12 sm:h-12 bg-blue-900/50 rounded-lg flex items-center justify-center">
              <svg class="w-5 h-5 sm:w-6 sm:h-6 text-blue-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M12 15v2m-6 4h12a2 2 0 002-2v-6a2 2 0 00-2-2H6a2 2 0 00-2 2v6a2 2 0 002 2zm10-10V7a4 4 0 00-8 0v4h8z" />
              </svg>
            </div>
            <div>
              <p class="text-gray-400 text-xs sm:text-sm">Locked</p>
              <p class="text-xl sm:text-2xl font-bold text-blue-400">{{ lockedCount }}</p>
            </div>
          </div>
        </div>

        <div class="bg-gray-800 rounded-xl p-4 sm:p-6">
          <div class="flex items-center gap-3 sm:gap-4">
            <div class="w-10 h-10 sm:w-12 sm:h-12 bg-orange-900/50 rounded-lg flex items-center justify-center">
              <svg class="w-5 h-5 sm:w-6 sm:h-6 text-orange-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M8 11V7a4 4 0 118 0m-4 8v2m-6 4h12a2 2 0 002-2v-6a2 2 0 00-2-2H6a2 2 0 00-2 2v6a2 2 0 002 2z" />
              </svg>
            </div>
            <div>
              <p class="text-gray-400 text-xs sm:text-sm">Unlocked</p>
              <p class="text-xl sm:text-2xl font-bold text-orange-400">{{ unlockedCount }}</p>
            </div>
          </div>
        </div>
      </div>

      <!-- Refresh Button -->
      <div class="mb-6 flex flex-col sm:flex-row sm:justify-between sm:items-center gap-4">
        <div class="flex items-center gap-3 sm:gap-4">
          <h2 class="text-lg sm:text-xl font-semibold">Connected Shelves</h2>
          <span v-if="wsConnected" class="flex items-center gap-2 text-sm text-green-400">
            <span class="w-2 h-2 bg-green-500 rounded-full animate-pulse"></span>
            Live
          </span>
          <span v-else class="flex items-center gap-2 text-sm text-red-400">
            <span class="w-2 h-2 bg-red-500 rounded-full"></span>
            Disconnected
          </span>
        </div>
        <div class="flex gap-2 w-full sm:w-auto">
          <button
            @click="testAllDoors"
            :disabled="shelves.length === 0"
            class="flex-1 sm:flex-none px-3 sm:px-4 py-2 bg-purple-600 hover:bg-purple-700 rounded-lg flex items-center justify-center gap-2 disabled:opacity-50 text-sm sm:text-base"
          >
            <svg class="w-4 h-4 sm:w-5 sm:h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M13 10V3L4 14h7v7l9-11h-7z" />
            </svg>
            <span class="hidden sm:inline">Test All</span>
            <span class="sm:hidden">Test</span>
          </button>
          <button
            @click="fetchDevices"
            :disabled="loading"
            class="flex-1 sm:flex-none px-3 sm:px-4 py-2 bg-blue-600 hover:bg-blue-700 rounded-lg flex items-center justify-center gap-2 disabled:opacity-50 text-sm sm:text-base"
          >
            <svg 
              :class="{ 'animate-spin': loading }"
              class="w-4 h-4 sm:w-5 sm:h-5" 
              fill="none" 
              stroke="currentColor" 
              viewBox="0 0 24 24"
            >
              <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M4 4v5h.582m15.356 2A8.001 8.001 0 004.582 9m0 0H9m11 11v-5h-.581m0 0a8.003 8.003 0 01-15.357-2m15.357 2H15" />
            </svg>
            Refresh
          </button>
        </div>
      </div>

      <!-- Loading State -->
      <div v-if="loading" class="text-center py-12">
        <div class="animate-spin rounded-full h-12 w-12 border-b-2 border-blue-500 mx-auto"></div>
        <p class="text-gray-400 mt-4">Loading shelf controllers...</p>
      </div>

      <!-- Shelves List -->
      <div v-else-if="shelves.length > 0" class="space-y-4 sm:space-y-6">
        <div 
          v-for="shelf in shelves" 
          :key="shelf.shelfId"
          class="bg-gray-800 rounded-xl p-4 sm:p-6 border border-gray-700"
        >
          <!-- Shelf Header -->
          <div class="flex flex-col sm:flex-row sm:items-center justify-between gap-3 sm:gap-0 mb-4 sm:mb-6">
            <div class="flex items-center gap-3 sm:gap-4">
              <div class="w-10 h-10 sm:w-12 sm:h-12 bg-purple-900/50 rounded-lg flex items-center justify-center">
                <svg class="w-5 h-5 sm:w-6 sm:h-6 text-purple-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                  <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M9 3v2m6-2v2M9 19v2m6-2v2M5 9H3m2 6H3m18-6h-2m2 6h-2M7 19h10a2 2 0 002-2V7a2 2 0 00-2-2H7a2 2 0 00-2 2v10a2 2 0 002 2zM9 9h6v6H9V9z" />
                </svg>
              </div>
              <div>
                <h3 class="text-lg sm:text-xl font-bold">{{ shelf.shelfId }}</h3>
                <p class="text-gray-400 text-xs sm:text-sm">{{ shelf.numDoors }} doors • IP: {{ shelf.ip || 'N/A' }}</p>
              </div>
            </div>
            <div class="flex items-center gap-2 sm:gap-3 ml-auto sm:ml-0">
              <span class="px-2 sm:px-3 py-1 bg-green-900/50 text-green-400 rounded-full text-xs sm:text-sm">
                Connected
              </span>
              <span class="text-gray-400 text-xs sm:text-sm hidden sm:inline">
                Since {{ formatTime(shelf.connectedAt) }}
              </span>
            </div>
          </div>

          <!-- Door Grid -->
          <div class="grid grid-cols-2 sm:grid-cols-3 md:grid-cols-4 lg:grid-cols-6 gap-2 sm:gap-3">
            <div 
              v-for="(rack, index) in shelf.racks" 
              :key="rack.rackId || index"
              class="bg-gray-900 rounded-lg p-4 border border-gray-700 hover:border-gray-600 transition-colors"
            >
              <!-- Door Header -->
              <div class="flex items-center justify-between mb-3">
                <span class="font-mono text-sm font-bold">Door {{ index }}</span>
                <span 
                  :class="rack.isLocked !== false ? 'bg-green-900/50 text-green-400' : 'bg-orange-900/50 text-orange-400'"
                  class="px-2 py-0.5 rounded text-xs uppercase"
                >
                  {{ rack.isLocked !== false ? 'Locked' : 'Open' }}
                </span>
              </div>

              <!-- Rack ID -->
              <p class="text-gray-400 text-xs mb-2 truncate" :title="rack.rackId">
                {{ rack.rackId }}
              </p>

              <!-- Stock Status based on lock state -->
              <div class="flex items-center gap-2 mb-3">
                <span 
                  :class="rack.isLocked !== false ? 'bg-green-500' : 'bg-orange-500'"
                  class="w-2 h-2 rounded-full"
                ></span>
                <span class="text-xs text-gray-400">
                  {{ rack.isLocked !== false ? 'Stocked' : 'Empty - Needs Refill' }}
                </span>
              </div>

              <!-- Actions -->
              <div class="flex gap-1">
                <!-- Show Unlock button when door is locked -->
                <button
                  v-if="rack.isLocked !== false"
                  @click="testUnlock(rack.rackId)"
                  class="flex-1 px-2 py-1.5 bg-blue-600 hover:bg-blue-700 rounded text-xs font-medium transition-colors"
                  title="Unlock this door"
                >
                  🔓 Unlock
                </button>
                <!-- Show Refill & Lock button when door is unlocked (empty) -->
                <button
                  v-else
                  @click="testLock(rack.rackId)"
                  class="flex-1 px-2 py-1.5 bg-green-600 hover:bg-green-700 rounded text-xs font-medium transition-colors"
                  title="Mark as refilled and lock"
                >
                  ✓ Refill & Lock
                </button>
              </div>
            </div>
          </div>

          <!-- Shelf Actions -->
          <div class="mt-4 pt-4 border-t border-gray-700 flex flex-col sm:flex-row justify-end gap-2">
            <button
              @click="unlockAllDoors(shelf)"
              class="w-full sm:w-auto px-4 py-2 bg-blue-600 hover:bg-blue-700 rounded-lg text-sm font-medium flex items-center justify-center gap-2"
            >
              <svg class="w-4 h-4" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M8 11V7a4 4 0 118 0m-4 8v2m-6 4h12a2 2 0 002-2v-6a2 2 0 00-2-2H6a2 2 0 00-2 2v6a2 2 0 002 2z" />
              </svg>
              Unlock All
            </button>
            <button
              @click="lockAllDoors(shelf)"
              class="w-full sm:w-auto px-4 py-2 bg-gray-600 hover:bg-gray-700 rounded-lg text-sm font-medium flex items-center justify-center gap-2"
            >
              <svg class="w-4 h-4" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M12 15v2m-6 4h12a2 2 0 002-2v-6a2 2 0 00-2-2H6a2 2 0 00-2 2v6a2 2 0 002 2zm10-10V7a4 4 0 00-8 0v4h8z" />
              </svg>
              Lock All
            </button>
          </div>
        </div>
      </div>

      <!-- No Shelves Connected -->
      <div v-else class="text-center py-8 sm:py-12 bg-gray-800 rounded-xl px-4">
        <svg class="w-12 h-12 sm:w-16 sm:h-16 mx-auto text-gray-600 mb-4" fill="none" stroke="currentColor" viewBox="0 0 24 24">
          <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M9 3v2m6-2v2M9 19v2m6-2v2M5 9H3m2 6H3m18-6h-2m2 6h-2M7 19h10a2 2 0 002-2V7a2 2 0 00-2-2H7a2 2 0 00-2 2v10a2 2 0 002 2zM9 9h6v6H9V9z" />
        </svg>
        <h3 class="text-lg sm:text-xl font-semibold text-gray-400 mb-2">No Shelf Controllers Connected</h3>
        <p class="text-gray-500 text-sm sm:text-base max-w-md mx-auto">
          ESP32 shelf controllers will appear here once they connect to the WebSocket server.
        </p>
      </div>

      <!-- Setup Instructions -->
      <div class="mt-6 sm:mt-8 bg-gray-800 rounded-xl p-4 sm:p-6">
        <h3 class="text-base sm:text-lg font-semibold mb-4">Shelf Controller Setup</h3>
        <div class="grid md:grid-cols-2 gap-4 sm:gap-6">
          <div>
            <h4 class="font-medium text-gray-300 mb-2 text-sm sm:text-base">1. ESP32 Configuration</h4>
            <div class="bg-gray-900 rounded-lg p-3 sm:p-4 font-mono text-xs sm:text-sm overflow-x-auto">
              <p>SHELF_ID: <span class="text-purple-400">"SHELF_001"</span></p>
              <p>NUM_DOORS: <span class="text-blue-400">9</span> or <span class="text-blue-400">12</span></p>
              <p>Server IP: <span class="text-green-400">{{ serverIP }}</span></p>
              <p>Server Port: <span class="text-green-400">5000</span></p>
            </div>
          </div>
          <div>
            <h4 class="font-medium text-gray-300 mb-2">2. Door Mapping</h4>
            <div class="bg-gray-900 rounded-lg p-4 font-mono text-sm">
              <p>Door 0 → <span class="text-yellow-400">RACK-001</span> (GPIO 13)</p>
              <p>Door 1 → <span class="text-yellow-400">RACK-002</span> (GPIO 27)</p>
              <p>Door 2 → <span class="text-yellow-400">RACK-003</span> (GPIO 25)</p>
              <p class="text-gray-500">... configure in shelf_controller.ino</p>
            </div>
          </div>
        </div>
        <div class="mt-4 text-gray-400 text-sm">
          <p>📁 Firmware: <code class="bg-gray-900 px-2 py-1 rounded">esp32/locker_controller/shelf_controller.ino</code></p>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted, onUnmounted, computed } from 'vue'
import axios from 'axios'
import { io } from 'socket.io-client'

const shelves = ref([])
const loading = ref(true)
const socket = ref(null)
const wsConnected = ref(false)

const API_URL = import.meta.env.VITE_API_URL || 'http://localhost:5000/api'
const WS_URL = import.meta.env.VITE_WS_URL || 'http://localhost:5000'

// Computed stats
const connectedDoorsCount = computed(() => {
  return shelves.value.reduce((acc, shelf) => acc + (shelf.racks?.length || shelf.numDoors || 0), 0)
})

const lockedCount = computed(() => {
  let count = 0
  shelves.value.forEach(shelf => {
    if (shelf.racks) {
      count += shelf.racks.filter(r => r.isLocked !== false).length
    }
  })
  return count
})

const unlockedCount = computed(() => {
  return connectedDoorsCount.value - lockedCount.value
})

const serverIP = computed(() => {
  const url = new URL(WS_URL)
  return url.hostname === 'localhost' ? 'YOUR_SERVER_IP' : url.hostname
})

async function fetchDevices() {
  loading.value = true
  try {
    const token = localStorage.getItem('token')
    const response = await axios.get(`${API_URL}/racks/devices/connected`, {
      headers: { Authorization: `Bearer ${token}` }
    })
    
    // Process response - group by shelf if needed
    const devices = response.data.devices || []
    const shelvesData = response.data.shelves || []
    
    // If we have shelf data directly, use it
    if (shelvesData.length > 0) {
      shelves.value = shelvesData
    } else if (devices.length > 0) {
      // Group devices by shelfId if they have one
      const shelfMap = new Map()
      
      devices.forEach(device => {
        const shelfId = device.shelfId || 'SHELF_001'
        
        if (!shelfMap.has(shelfId)) {
          shelfMap.set(shelfId, {
            shelfId,
            numDoors: 0,
            racks: [],
            connectedAt: device.connectedAt,
            ip: device.ip
          })
        }
        
        const shelf = shelfMap.get(shelfId)
        shelf.racks.push({
          rackId: device.rackId,
          isLocked: device.status === 'locked',
          itemPresent: device.itemPresent,
          doorIndex: device.doorIndex || shelf.racks.length
        })
        shelf.numDoors = shelf.racks.length
      })
      
      shelves.value = Array.from(shelfMap.values())
    } else {
      shelves.value = []
    }
  } catch (error) {
    console.error('Error fetching devices:', error)
    shelves.value = []
  } finally {
    loading.value = false
  }
}

function testUnlock(rackId) {
  if (socket.value && wsConnected.value) {
    socket.value.emit('locker:unlock', { rackId })
    console.log('Sent unlock command to:', rackId)
    
    // Update UI optimistically
    updateDoorState(rackId, false)
  } else {
    alert('WebSocket not connected!')
  }
}

function testLock(rackId) {
  if (socket.value && wsConnected.value) {
    socket.value.emit('locker:lock', { rackId })
    console.log('Sent lock command to:', rackId)
    
    // Update UI optimistically
    updateDoorState(rackId, true)
  } else {
    alert('WebSocket not connected!')
  }
}

function updateDoorState(rackId, isLocked) {
  shelves.value.forEach(shelf => {
    if (shelf.racks) {
      const rack = shelf.racks.find(r => r.rackId === rackId)
      if (rack) {
        rack.isLocked = isLocked
      }
    }
  })
}

function unlockAllDoors(shelf) {
  if (!confirm(`Unlock all ${shelf.numDoors} doors on ${shelf.shelfId}?`)) return
  
  shelf.racks?.forEach(rack => {
    testUnlock(rack.rackId)
  })
}

function lockAllDoors(shelf) {
  if (!confirm(`Lock all ${shelf.numDoors} doors on ${shelf.shelfId}?`)) return
  
  shelf.racks?.forEach(rack => {
    testLock(rack.rackId)
  })
}

function testAllDoors() {
  if (!confirm('This will unlock and lock all doors briefly. Continue?')) return
  
  shelves.value.forEach(shelf => {
    shelf.racks?.forEach((rack, index) => {
      // Stagger unlocks to avoid power surge
      setTimeout(() => {
        testUnlock(rack.rackId)
        setTimeout(() => testLock(rack.rackId), 1000)
      }, index * 500)
    })
  })
}

function formatTime(dateString) {
  if (!dateString) return 'N/A'
  const date = new Date(dateString)
  return date.toLocaleTimeString()
}

function setupWebSocket() {
  const token = localStorage.getItem('token')
  
  socket.value = io(WS_URL, {
    auth: { token },
    transports: ['websocket', 'polling']
  })

  socket.value.on('connect', () => {
    console.log('Admin WebSocket connected')
    wsConnected.value = true
    socket.value.emit('subscribe-admin')
  })

  socket.value.on('disconnect', () => {
    console.log('Admin WebSocket disconnected')
    wsConnected.value = false
  })

  // Shelf controller events
  socket.value.on('shelf:connected', (data) => {
    console.log('Shelf controller connected:', data)
    fetchDevices()
  })

  socket.value.on('shelf:disconnected', (data) => {
    console.log('Shelf controller disconnected:', data)
    fetchDevices()
  })

  // Legacy single ESP32 events
  socket.value.on('esp:connected', (data) => {
    console.log('ESP32 connected:', data)
    fetchDevices()
  })

  socket.value.on('esp:disconnected', (data) => {
    console.log('ESP32 disconnected:', data)
    fetchDevices()
  })

  // Rack status updates
  socket.value.on('rack-status', (data) => {
    console.log('Rack status update:', data)
    updateDoorState(data.rackNumber || data.rackId, data.isLocked)
  })

  // Command feedback
  socket.value.on('command-sent', (data) => {
    console.log('Command sent:', data)
  })

  socket.value.on('command-error', (data) => {
    console.error('Command error:', data)
    alert(`Error: ${data.error} for ${data.rackId}`)
  })
}

onMounted(() => {
  fetchDevices()
  setupWebSocket()
})

onUnmounted(() => {
  if (socket.value) {
    socket.value.disconnect()
  }
})
</script>
