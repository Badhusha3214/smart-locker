import { io } from 'socket.io-client'
import { ref } from 'vue'

class WebSocketService {
  constructor() {
    this.socket = null
    this.connected = ref(false)
    this.reconnectAttempts = 0
    this.maxReconnectAttempts = 5
  }

  connect(token = null) {
    if (this.socket?.connected) return

    const wsUrl = import.meta.env.VITE_WS_URL || 'http://localhost:3000'
    
    this.socket = io(wsUrl, {
      transports: ['websocket', 'polling'],
      reconnection: true,
      reconnectionAttempts: this.maxReconnectAttempts,
      reconnectionDelay: 1000,
      auth: token ? { token } : {}
    })

    this.socket.on('connect', () => {
      console.log('[WS] Connected')
      this.connected.value = true
      this.reconnectAttempts = 0
    })

    this.socket.on('disconnect', (reason) => {
      console.log('[WS] Disconnected:', reason)
      this.connected.value = false
    })

    this.socket.on('connect_error', (error) => {
      console.error('[WS] Connection error:', error.message)
      this.reconnectAttempts++
    })
  }

  disconnect() {
    if (this.socket) {
      this.socket.disconnect()
      this.socket = null
      this.connected.value = false
    }
  }

  subscribeToRack(rackId) {
    if (this.socket?.connected) {
      this.socket.emit('subscribe-rack', rackId)
    }
  }

  unsubscribeFromRack(rackId) {
    if (this.socket?.connected) {
      this.socket.emit('unsubscribe-rack', rackId)
    }
  }

  subscribeToAdmin() {
    if (this.socket?.connected) {
      this.socket.emit('subscribe-admin')
    }
  }

  onLockStatus(callback) {
    if (this.socket) {
      this.socket.on('lock-status', callback)
    }
  }

  onLEDStatus(callback) {
    if (this.socket) {
      this.socket.on('led-status', callback)
    }
  }

  onRackStatus(callback) {
    if (this.socket) {
      this.socket.on('rack-status', callback)
    }
  }

  onAutoLockFailed(callback) {
    if (this.socket) {
      this.socket.on('auto-lock-failed', callback)
    }
  }

  off(event) {
    if (this.socket) {
      this.socket.off(event)
    }
  }

  isConnected() {
    return this.connected.value
  }
}

export default new WebSocketService()
