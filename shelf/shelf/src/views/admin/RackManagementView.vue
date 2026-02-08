<template>
  <div>
    <div class="flex flex-col sm:flex-row sm:items-center sm:justify-between mb-6 gap-4">
      <h1 class="text-xl sm:text-2xl font-bold text-gray-900">Rack Management</h1>
      <button
        @click="openModal()"
        class="inline-flex items-center justify-center px-4 py-2 bg-blue-600 hover:bg-blue-700 text-white font-medium rounded-lg transition-colors w-full sm:w-auto"
      >
        <svg class="w-5 h-5 mr-2" fill="none" stroke="currentColor" viewBox="0 0 24 24">
          <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M12 6v6m0 0v6m0-6h6m-6 0H6" />
        </svg>
        Add Rack
      </button>
    </div>

    <!-- Alert Banner for Unlocked (Empty) Racks -->
    <div v-if="unlockedRacksCount > 0" class="bg-orange-50 border-l-4 border-orange-400 p-4 mb-6 rounded-r-lg">
      <div class="flex items-center">
        <div class="flex-shrink-0">
          <svg class="h-5 w-5 text-orange-400" fill="currentColor" viewBox="0 0 20 20">
            <path fill-rule="evenodd" d="M8.257 3.099c.765-1.36 2.722-1.36 3.486 0l5.58 9.92c.75 1.334-.213 2.98-1.742 2.98H4.42c-1.53 0-2.493-1.646-1.743-2.98l5.58-9.92zM11 13a1 1 0 11-2 0 1 1 0 012 0zm-1-8a1 1 0 00-1 1v3a1 1 0 002 0V6a1 1 0 00-1-1z" clip-rule="evenodd"/>
          </svg>
        </div>
        <div class="ml-3">
          <p class="text-sm text-orange-700">
            <span class="font-bold">{{ unlockedRacksCount }} rack(s) are unlocked and need refilling!</span>
            Click the <span class="font-medium">✓</span> button to mark as refilled and lock.
          </p>
        </div>
      </div>
    </div>

    <!-- Filters -->
    <div class="bg-white rounded-xl shadow-lg p-4 mb-6">
      <div class="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 gap-3">
        <select 
          v-model="filters.status"
          @change="fetchRacks"
          class="w-full px-4 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent"
        >
          <option value="">All Status</option>
          <option value="available">Available</option>
          <option value="occupied">Occupied</option>
          <option value="maintenance">Maintenance</option>
        </select>
        <select 
          v-model="filters.size"
          @change="fetchRacks"
          class="w-full px-4 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent"
        >
          <option value="">All Sizes</option>
          <option value="small">Small</option>
          <option value="medium">Medium</option>
          <option value="large">Large</option>
        </select>
        <input
          v-model="filters.search"
          @input="debounceSearch"
          type="text"
          placeholder="Search racks..."
          class="w-full px-4 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent"
        />
      </div>
    </div>

    <!-- Loading State -->
    <div v-if="loading" class="bg-white rounded-xl shadow-lg p-8 text-center">
      <div class="animate-spin w-12 h-12 border-4 border-blue-600 border-t-transparent rounded-full mx-auto"></div>
      <p class="mt-4 text-gray-600">Loading racks...</p>
    </div>

    <!-- Mobile Card View (shown on small screens) -->
    <div v-else class="block lg:hidden space-y-4">
      <div v-if="racks.length === 0" class="bg-white rounded-xl shadow-lg p-8 text-center text-gray-500">
        No racks found
      </div>
      <div 
        v-for="rack in racks" 
        :key="rack._id + '-mobile'"
        class="bg-white rounded-xl shadow-lg p-4"
      >
        <div class="flex items-start justify-between mb-3">
          <div>
            <h3 class="font-bold text-gray-900">{{ rack.rackNumber }}</h3>
            <p class="text-sm text-gray-500">{{ rack.location || 'No location' }}</p>
          </div>
          <span :class="['px-2 py-1 text-xs font-medium rounded-full', statusBadge(rack.status)]">
            {{ rack.status }}
          </span>
        </div>
        
        <div class="grid grid-cols-2 gap-3 mb-4 text-sm">
          <div>
            <span class="text-gray-500">Price:</span>
            <span class="font-medium text-gray-900 ml-1">₹{{ rack.pricePerUse }}</span>
          </div>
          <div>
            <span v-if="!rack.isLocked" class="px-2 py-1 text-xs font-medium rounded-full bg-orange-100 text-orange-800">
              🔓 Empty
            </span>
            <span v-else class="px-2 py-1 text-xs font-medium rounded-full bg-green-100 text-green-800">
              🔒 Stocked
            </span>
          </div>
        </div>
        
        <div class="flex items-center justify-between pt-3 border-t border-gray-100">
          <div class="flex items-center space-x-3">
            <button
              v-if="!rack.isLocked"
              @click="handleRefillAndLock(rack)"
              class="p-2 text-green-600 hover:bg-green-50 rounded-lg"
              title="Mark as Refilled & Lock"
            >
              <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M5 13l4 4L19 7" />
              </svg>
            </button>
            <button @click="showQRCode(rack)" class="p-2 text-purple-600 hover:bg-purple-50 rounded-lg">
              <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M12 4v1m6 11h2m-6 0h-2v4m0-11v3m0 0h.01M12 12h4.01M16 20h4M4 12h4m12 0h.01M5 8h2a1 1 0 001-1V5a1 1 0 00-1-1H5a1 1 0 00-1 1v2a1 1 0 001 1zm12 0h2a1 1 0 001-1V5a1 1 0 00-1-1h-2a1 1 0 00-1 1v2a1 1 0 001 1zM5 20h2a1 1 0 001-1v-2a1 1 0 00-1-1H5a1 1 0 00-1 1v2a1 1 0 001 1z" />
              </svg>
            </button>
            <button @click="openModal(rack)" class="p-2 text-blue-600 hover:bg-blue-50 rounded-lg">
              <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M11 5H6a2 2 0 00-2 2v11a2 2 0 002 2h11a2 2 0 002-2v-5m-1.414-9.414a2 2 0 112.828 2.828L11.828 15H9v-2.828l8.586-8.586z" />
              </svg>
            </button>
          </div>
          <button @click="confirmDelete(rack)" class="p-2 text-red-600 hover:bg-red-50 rounded-lg">
            <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M19 7l-.867 12.142A2 2 0 0116.138 21H7.862a2 2 0 01-1.995-1.858L5 7m5 4v6m4-6v6m1-10V4a1 1 0 00-1-1h-4a1 1 0 00-1 1v3M4 7h16" />
            </svg>
          </button>
        </div>
      </div>
    </div>

    <!-- Desktop Table View (hidden on small screens) -->
    <div v-if="!loading" class="hidden lg:block bg-white rounded-xl shadow-lg overflow-hidden">
      <div class="overflow-x-auto">
        <table class="min-w-full divide-y divide-gray-200">
          <thead class="bg-gray-50">
            <tr>
              <th class="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase tracking-wider">Rack Number</th>
              <th class="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase tracking-wider">Location</th>
              <th class="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase tracking-wider">Price/Use</th>
              <th class="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase tracking-wider">Status</th>
              <th class="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase tracking-wider">Stock</th>
              <th class="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase tracking-wider">Actions</th>
            </tr>
          </thead>
          <tbody class="bg-white divide-y divide-gray-200">
            <tr v-for="rack in racks" :key="rack._id" class="hover:bg-gray-50">
              <td class="px-6 py-4 whitespace-nowrap">
                <div class="font-medium text-gray-900">{{ rack.rackNumber }}</div>
                <div class="text-sm text-gray-500">ID: {{ rack._id?.slice(-8) }}</div>
              </td>
              <td class="px-6 py-4 whitespace-nowrap text-gray-600">
                {{ rack.location || 'N/A' }}
              </td>
              <td class="px-6 py-4 whitespace-nowrap font-medium text-gray-900">
                ₹{{ rack.pricePerUse }}
              </td>
              <td class="px-6 py-4 whitespace-nowrap">
                <span :class="['px-2 py-1 text-xs font-medium rounded-full', statusBadge(rack.status)]">
                  {{ rack.status }}
                </span>
              </td>
              <td class="px-6 py-4 whitespace-nowrap">
                <!-- Stock based on lock status: unlocked = empty, locked = stocked -->
                <span v-if="!rack.isLocked" class="px-2 py-1 text-xs font-medium rounded-full bg-orange-100 text-orange-800">
                  🔓 Unlocked - Empty
                </span>
                <span v-else class="px-2 py-1 text-xs font-medium rounded-full bg-green-100 text-green-800">
                  🔒 Locked - Stocked
                </span>
              </td>
              <td class="px-6 py-4 whitespace-nowrap">
                <div class="flex items-center space-x-2">
                  <!-- Refill Button - show when unlocked (empty) -->
                  <button
                    v-if="!rack.isLocked"
                    @click="handleRefillAndLock(rack)"
                    class="text-green-600 hover:text-green-700"
                    title="Mark as Refilled & Lock"
                  >
                    <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                      <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
                        d="M5 13l4 4L19 7" />
                    </svg>
                  </button>
                  <button
                    @click="showQRCode(rack)"
                    class="text-purple-600 hover:text-purple-700"
                    title="View QR Code"
                  >
                    <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                      <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
                        d="M12 4v1m6 11h2m-6 0h-2v4m0-11v3m0 0h.01M12 12h4.01M16 20h4M4 12h4m12 0h.01M5 8h2a1 1 0 001-1V5a1 1 0 00-1-1H5a1 1 0 00-1 1v2a1 1 0 001 1zm12 0h2a1 1 0 001-1V5a1 1 0 00-1-1h-2a1 1 0 00-1 1v2a1 1 0 001 1zM5 20h2a1 1 0 001-1v-2a1 1 0 00-1-1H5a1 1 0 00-1 1v2a1 1 0 001 1z" />
                    </svg>
                  </button>
                  <button
                    @click="openModal(rack)"
                    class="text-blue-600 hover:text-blue-700"
                    title="Edit"
                  >
                    <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                      <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
                        d="M11 5H6a2 2 0 00-2 2v11a2 2 0 002 2h11a2 2 0 002-2v-5m-1.414-9.414a2 2 0 112.828 2.828L11.828 15H9v-2.828l8.586-8.586z" />
                    </svg>
                  </button>
                  <button
                    v-if="rack.status === 'occupied'"
                    @click="handleManualUnlock(rack._id)"
                    class="text-green-600 hover:text-green-700"
                    title="Manual Unlock"
                  >
                    <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                      <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
                        d="M8 11V7a4 4 0 118 0m-4 8v2m-6 4h12a2 2 0 002-2v-6a2 2 0 00-2-2H6a2 2 0 00-2 2v6a2 2 0 002 2z" />
                    </svg>
                  </button>
                  <button
                    @click="confirmDelete(rack)"
                    class="text-red-600 hover:text-red-700"
                    title="Delete"
                  >
                    <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                      <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
                        d="M19 7l-.867 12.142A2 2 0 0116.138 21H7.862a2 2 0 01-1.995-1.858L5 7m5 4v6m4-6v6m1-10V4a1 1 0 00-1-1h-4a1 1 0 00-1 1v3M4 7h16" />
                    </svg>
                  </button>
                </div>
              </td>
            </tr>
            <tr v-if="racks.length === 0">
              <td colspan="6" class="px-6 py-12 text-center text-gray-500">
                No racks found
              </td>
            </tr>
          </tbody>
        </table>
      </div>
    </div>

    <!-- Add/Edit Modal -->
    <div v-if="showModal" class="fixed inset-0 z-50 overflow-y-auto">
      <div class="flex items-center justify-center min-h-screen px-4">
        <div class="fixed inset-0 bg-black bg-opacity-50" @click="closeModal"></div>
        <div class="relative bg-white rounded-xl shadow-xl max-w-md w-full p-6">
          <h2 class="text-xl font-bold text-gray-900 mb-4">
            {{ editingRack ? 'Edit Rack' : 'Add New Rack' }}
          </h2>
          
          <form @submit.prevent="handleSubmit" class="space-y-4">
            <div>
              <label class="block text-sm font-medium text-gray-700 mb-1">Rack Number</label>
              <input
                v-model="form.rackNumber"
                type="text"
                required
                class="w-full px-4 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent"
                placeholder="RACK-001"
              />
            </div>
            
            <div>
              <label class="block text-sm font-medium text-gray-700 mb-1">Location</label>
              <input
                v-model="form.location"
                type="text"
                class="w-full px-4 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent"
                placeholder="Floor 1, Zone A"
              />
            </div>
            
            <div>
              <label class="block text-sm font-medium text-gray-700 mb-1">Price per Use (₹)</label>
              <input
                v-model.number="form.pricePerUse"
                type="number"
                required
                min="1"
                class="w-full px-4 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent"
                placeholder="10"
              />
            </div>
            
            <div>
              <label class="block text-sm font-medium text-gray-700 mb-1">Status</label>
              <select
                v-model="form.status"
                class="w-full px-4 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent"
              >
                <option value="available">Available</option>
                <option value="occupied">Occupied</option>
                <option value="maintenance">Maintenance</option>
              </select>
            </div>
            
            <div>
              <label class="block text-sm font-medium text-gray-700 mb-1">ESP32 Device ID</label>
              <input
                v-model="form.esp32DeviceId"
                type="text"
                class="w-full px-4 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent"
                placeholder="ESP32_001"
              />
            </div>

            <div v-if="error" class="p-3 bg-red-50 border border-red-200 rounded-lg">
              <p class="text-red-700 text-sm">{{ error }}</p>
            </div>

            <div class="flex space-x-3 pt-4">
              <button
                type="button"
                @click="closeModal"
                class="flex-1 py-2 bg-gray-100 hover:bg-gray-200 text-gray-700 font-medium rounded-lg transition-colors"
              >
                Cancel
              </button>
              <button
                type="submit"
                :disabled="submitting"
                class="flex-1 py-2 bg-blue-600 hover:bg-blue-700 disabled:bg-gray-400 text-white font-medium rounded-lg transition-colors"
              >
                {{ submitting ? 'Saving...' : (editingRack ? 'Update' : 'Create') }}
              </button>
            </div>
          </form>
        </div>
      </div>
    </div>

    <!-- Delete Confirmation Modal -->
    <div v-if="showDeleteModal" class="fixed inset-0 z-50 overflow-y-auto">
      <div class="flex items-center justify-center min-h-screen px-4">
        <div class="fixed inset-0 bg-black bg-opacity-50" @click="showDeleteModal = false"></div>
        <div class="relative bg-white rounded-xl shadow-xl max-w-sm w-full p-6 text-center">
          <div class="w-16 h-16 mx-auto bg-red-100 rounded-full flex items-center justify-center mb-4">
            <svg class="w-8 h-8 text-red-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
                d="M19 7l-.867 12.142A2 2 0 0116.138 21H7.862a2 2 0 01-1.995-1.858L5 7m5 4v6m4-6v6m1-10V4a1 1 0 00-1-1h-4a1 1 0 00-1 1v3M4 7h16" />
            </svg>
          </div>
          <h3 class="text-lg font-semibold text-gray-900 mb-2">Delete Rack?</h3>
          <p class="text-gray-600 mb-6">Are you sure you want to delete "{{ deletingRack?.rackNumber }}"? This action cannot be undone.</p>
          <div class="flex space-x-3">
            <button
              @click="showDeleteModal = false"
              class="flex-1 py-2 bg-gray-100 hover:bg-gray-200 text-gray-700 font-medium rounded-lg transition-colors"
            >
              Cancel
            </button>
            <button
              @click="handleDelete"
              :disabled="deleting"
              class="flex-1 py-2 bg-red-600 hover:bg-red-700 disabled:bg-gray-400 text-white font-medium rounded-lg transition-colors"
            >
              {{ deleting ? 'Deleting...' : 'Delete' }}
            </button>
          </div>
        </div>
      </div>
    </div>

    <!-- QR Code Modal -->
    <div v-if="showQRModal" class="fixed inset-0 z-50 overflow-y-auto">
      <div class="flex items-center justify-center min-h-screen px-4">
        <div class="fixed inset-0 bg-black bg-opacity-50" @click="showQRModal = false"></div>
        <div class="relative bg-white rounded-xl shadow-xl max-w-sm w-full p-6 text-center">
          <h3 class="text-lg font-semibold text-gray-900 mb-4">QR Code for {{ qrRack?.rackNumber }}</h3>
          
          <div v-if="qrRack?.qrCode" class="mb-4">
            <img :src="qrRack.qrCode" :alt="'QR Code for ' + qrRack.rackNumber" class="mx-auto border rounded-lg" />
          </div>
          <div v-else class="mb-4 p-8 bg-gray-100 rounded-lg">
            <p class="text-gray-500">QR Code not available</p>
          </div>
          
          <p class="text-sm text-gray-500 mb-4">
            Rack ID: {{ qrRack?._id }}<br>
            Location: {{ qrRack?.location }}
          </p>
          
          <div class="flex space-x-3">
            <button
              @click="showQRModal = false"
              class="flex-1 py-2 bg-gray-100 hover:bg-gray-200 text-gray-700 font-medium rounded-lg transition-colors"
            >
              Close
            </button>
            <button
              v-if="qrRack?.qrCode"
              @click="downloadQRCode"
              class="flex-1 py-2 bg-blue-600 hover:bg-blue-700 text-white font-medium rounded-lg transition-colors"
            >
              Download
            </button>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, computed, onMounted } from 'vue'
import { useRackStore } from '@/stores/rack'

const rackStore = useRackStore()

const loading = ref(true)
const showModal = ref(false)
const showDeleteModal = ref(false)
const showQRModal = ref(false)
const submitting = ref(false)
const deleting = ref(false)
const error = ref(null)

const editingRack = ref(null)
const deletingRack = ref(null)
const qrRack = ref(null)

const filters = ref({
  status: '',
  size: '',
  search: ''
})

const form = ref({
  rackNumber: '',
  location: '',
  pricePerUse: 10,
  status: 'available',
  esp32DeviceId: ''
})

const racks = computed(() => rackStore.racks)
const emptyRacksCount = computed(() => racks.value.filter(r => r.needsRefill).length)
const unlockedRacksCount = computed(() => racks.value.filter(r => !r.isLocked).length)

let searchTimeout = null

function debounceSearch() {
  clearTimeout(searchTimeout)
  searchTimeout = setTimeout(fetchRacks, 300)
}

function statusBadge(status) {
  const badges = {
    available: 'bg-green-100 text-green-700',
    occupied: 'bg-red-100 text-red-700',
    maintenance: 'bg-yellow-100 text-yellow-700'
  }
  return badges[status] || 'bg-gray-100 text-gray-700'
}

function openModal(rack = null) {
  editingRack.value = rack
  if (rack) {
    form.value = {
      rackNumber: rack.rackNumber,
      location: rack.location || '',
      pricePerUse: rack.pricePerUse,
      status: rack.status,
      esp32DeviceId: rack.esp32DeviceId || ''
    }
  } else {
    form.value = {
      rackNumber: '',
      location: '',
      pricePerUse: 10,
      status: 'available',
      esp32DeviceId: ''
    }
  }
  error.value = null
  showModal.value = true
}

function closeModal() {
  showModal.value = false
  editingRack.value = null
}

function confirmDelete(rack) {
  deletingRack.value = rack
  showDeleteModal.value = true
}

async function fetchRacks() {
  loading.value = true
  try {
    const params = { ...filters.value }
    if (!params.status) delete params.status
    if (!params.size) delete params.size
    if (!params.search) delete params.search
    
    await rackStore.fetchAllRacks(params)
  } finally {
    loading.value = false
  }
}

async function handleSubmit() {
  submitting.value = true
  error.value = null

  try {
    if (editingRack.value) {
      await rackStore.updateRack(editingRack.value._id, form.value)
    } else {
      await rackStore.createRack(form.value)
    }
    closeModal()
    fetchRacks()
  } catch (err) {
    error.value = err.response?.data?.error || 'Operation failed'
  } finally {
    submitting.value = false
  }
}

async function handleDelete() {
  deleting.value = true
  try {
    await rackStore.deleteRack(deletingRack.value._id)
    showDeleteModal.value = false
    fetchRacks()
  } catch (err) {
    console.error('Delete failed:', err)
  } finally {
    deleting.value = false
  }
}

async function handleManualUnlock(rackId) {
  try {
    await rackStore.manualUnlock(rackId)
    fetchRacks()
  } catch (err) {
    console.error('Manual unlock failed:', err)
  }
}

async function handleRefill(rackId) {
  try {
    await rackStore.refillRack(rackId)
    fetchRacks()
  } catch (err) {
    console.error('Refill failed:', err)
  }
}

// Mark as refilled and lock the rack
async function handleRefillAndLock(rack) {
  try {
    // Lock the rack via API (this will send WebSocket command to ESP32)
    await rackStore.lockRack(rack._id)
    
    // Mark as refilled in database
    await rackStore.refillRack(rack._id)
    
    // Refresh the list to get updated state
    await fetchRacks()
    
    console.log('Refilled and locked:', rack.rackNumber)
  } catch (err) {
    console.error('Refill and lock failed:', err)
    alert('Failed to lock rack: ' + (err.message || 'Unknown error'))
  }
}

function showQRCode(rack) {
  qrRack.value = rack
  showQRModal.value = true
}

function downloadQRCode() {
  if (!qrRack.value?.qrCode) return
  
  const link = document.createElement('a')
  link.href = qrRack.value.qrCode
  link.download = `qr-${qrRack.value.name.replace(/\s+/g, '-')}.png`
  document.body.appendChild(link)
  link.click()
  document.body.removeChild(link)
}

onMounted(() => {
  fetchRacks()
})
</script>
