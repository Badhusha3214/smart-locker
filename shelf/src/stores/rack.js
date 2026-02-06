import { defineStore } from 'pinia'
import { ref, computed } from 'vue'
import api from '@/services/api'
import websocketService from '@/services/websocket'

export const useRackStore = defineStore('rack', () => {
  const racks = ref([])
  const currentRack = ref(null)
  const loading = ref(false)
  const error = ref(null)

  const availableRacks = computed(() => 
    racks.value.filter(r => r.status === 'available')
  )

  const occupiedRacks = computed(() => 
    racks.value.filter(r => r.status === 'occupied')
  )

  async function fetchRacks(filters = {}) {
    loading.value = true
    error.value = null
    try {
      const response = await api.get('/racks', { params: filters })
      racks.value = response.data.racks
      return response.data
    } catch (err) {
      error.value = err.response?.data?.error || 'Failed to fetch racks'
      throw err
    } finally {
      loading.value = false
    }
  }

  async function fetchRackById(id) {
    loading.value = true
    error.value = null
    try {
      const response = await api.get(`/racks/${id}`)
      currentRack.value = response.data.rack
      return response.data.rack
    } catch (err) {
      error.value = err.response?.data?.error || 'Failed to fetch rack'
      throw err
    } finally {
      loading.value = false
    }
  }

  async function fetchRackByQR(qrData) {
    loading.value = true
    error.value = null
    try {
      const response = await api.post('/racks/qr', { qrData })
      currentRack.value = response.data.rack
      return response.data.rack
    } catch (err) {
      error.value = err.response?.data?.error || 'Invalid QR code'
      throw err
    } finally {
      loading.value = false
    }
  }

  async function unlockRack(rackId, transactionId) {
    loading.value = true
    error.value = null
    try {
      const response = await api.post(`/racks/${rackId}/unlock`, { transactionId })
      if (currentRack.value?._id === rackId) {
        currentRack.value = response.data.rack
      }
      return response.data
    } catch (err) {
      error.value = err.response?.data?.error || 'Failed to unlock rack'
      throw err
    } finally {
      loading.value = false
    }
  }

  async function lockRack(rackId) {
    loading.value = true
    error.value = null
    try {
      const response = await api.post(`/racks/${rackId}/lock`)
      if (currentRack.value?._id === rackId) {
        currentRack.value = response.data.rack
      }
      return response.data
    } catch (err) {
      error.value = err.response?.data?.error || 'Failed to lock rack'
      throw err
    } finally {
      loading.value = false
    }
  }

  function subscribeToRackUpdates(rackId, callbacks = {}) {
    websocketService.subscribeToRack(rackId)
    
    if (callbacks.onLockStatus) {
      websocketService.onLockStatus((data) => {
        if (data.rackId === rackId) {
          callbacks.onLockStatus(data)
          // Update local state
          if (currentRack.value?._id === rackId) {
            currentRack.value.status = data.status === 'locked' ? 'available' : 'occupied'
          }
        }
      })
    }
    
    if (callbacks.onLEDStatus) {
      websocketService.onLEDStatus((data) => {
        if (data.rackId === rackId) {
          callbacks.onLEDStatus(data)
          if (currentRack.value?._id === rackId) {
            currentRack.value.ledStatus = data.color
          }
        }
      })
    }
  }

  function unsubscribeFromRackUpdates(rackId) {
    websocketService.unsubscribeFromRack(rackId)
    websocketService.off('lock-status')
    websocketService.off('led-status')
  }

  function clearCurrentRack() {
    currentRack.value = null
  }

  // Admin functions
  async function fetchAllRacks(filters = {}) {
    loading.value = true
    try {
      const response = await api.get('/admin/racks', { params: filters })
      racks.value = response.data.racks
      return response.data
    } catch (err) {
      error.value = err.response?.data?.error || 'Failed to fetch racks'
      throw err
    } finally {
      loading.value = false
    }
  }

  async function createRack(rackData) {
    loading.value = true
    try {
      const response = await api.post('/admin/racks', rackData)
      racks.value.push(response.data.rack)
      return response.data
    } catch (err) {
      error.value = err.response?.data?.error || 'Failed to create rack'
      throw err
    } finally {
      loading.value = false
    }
  }

  async function updateRack(rackId, rackData) {
    loading.value = true
    try {
      const response = await api.put(`/admin/racks/${rackId}`, rackData)
      const index = racks.value.findIndex(r => r._id === rackId)
      if (index !== -1) {
        racks.value[index] = response.data.rack
      }
      return response.data
    } catch (err) {
      error.value = err.response?.data?.error || 'Failed to update rack'
      throw err
    } finally {
      loading.value = false
    }
  }

  async function deleteRack(rackId) {
    loading.value = true
    try {
      await api.delete(`/admin/racks/${rackId}`)
      racks.value = racks.value.filter(r => r._id !== rackId)
      return { success: true }
    } catch (err) {
      error.value = err.response?.data?.error || 'Failed to delete rack'
      throw err
    } finally {
      loading.value = false
    }
  }

  async function manualUnlock(rackId) {
    const response = await api.post(`/admin/racks/${rackId}/manual-unlock`)
    return response.data
  }

  async function manualLock(rackId) {
    const response = await api.post(`/admin/racks/${rackId}/manual-lock`)
    return response.data
  }

  async function refillRack(rackId) {
    loading.value = true
    try {
      const response = await api.post(`/racks/${rackId}/refill`)
      const index = racks.value.findIndex(r => r._id === rackId)
      if (index !== -1) {
        racks.value[index] = response.data.rack
      }
      return response.data
    } catch (err) {
      error.value = err.response?.data?.error || 'Failed to mark rack as refilled'
      throw err
    } finally {
      loading.value = false
    }
  }

  // Computed for racks needing refill
  const racksNeedingRefill = computed(() => 
    racks.value.filter(r => r.needsRefill)
  )

  return {
    racks,
    currentRack,
    loading,
    error,
    availableRacks,
    occupiedRacks,
    racksNeedingRefill,
    fetchRacks,
    fetchRackById,
    fetchRackByQR,
    unlockRack,
    lockRack,
    subscribeToRackUpdates,
    unsubscribeFromRackUpdates,
    clearCurrentRack,
    fetchAllRacks,
    createRack,
    updateRack,
    deleteRack,
    manualUnlock,
    manualLock,
    refillRack
  }
})
