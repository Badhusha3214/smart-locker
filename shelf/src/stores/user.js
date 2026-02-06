import { defineStore } from 'pinia'
import { ref, computed } from 'vue'
import authService from '@/services/auth'
import websocketService from '@/services/websocket'

export const useUserStore = defineStore('user', () => {
  const user = ref(JSON.parse(localStorage.getItem('user')) || null)
  const token = ref(localStorage.getItem('token') || null)
  const loading = ref(false)
  const error = ref(null)

  const isAuthenticated = computed(() => !!token.value && !!user.value)
  const isAdmin = computed(() => user.value?.role === 'admin')
  const userName = computed(() => user.value?.name || 'Guest')

  async function register(userData) {
    loading.value = true
    error.value = null
    try {
      const data = await authService.register(userData)
      user.value = data.user
      token.value = data.token
      localStorage.setItem('token', data.token)
      localStorage.setItem('user', JSON.stringify(data.user))
      websocketService.connect(data.token)
      return { success: true }
    } catch (err) {
      error.value = err.response?.data?.error || 'Registration failed'
      return { success: false, error: error.value }
    } finally {
      loading.value = false
    }
  }

  async function login(credentials) {
    loading.value = true
    error.value = null
    try {
      const data = await authService.login(credentials)
      user.value = data.user
      token.value = data.token
      localStorage.setItem('token', data.token)
      localStorage.setItem('user', JSON.stringify(data.user))
      websocketService.connect(data.token)
      return { success: true }
    } catch (err) {
      error.value = err.response?.data?.error || 'Login failed'
      return { success: false, error: error.value }
    } finally {
      loading.value = false
    }
  }

  async function logout() {
    try {
      await authService.logout()
    } finally {
      user.value = null
      token.value = null
      localStorage.removeItem('token')
      localStorage.removeItem('user')
      websocketService.disconnect()
    }
  }

  async function fetchUser() {
    if (!token.value) return
    
    loading.value = true
    try {
      const data = await authService.getMe()
      user.value = data.user
      localStorage.setItem('user', JSON.stringify(data.user))
    } catch (err) {
      logout()
    } finally {
      loading.value = false
    }
  }

  function initialize() {
    if (token.value) {
      websocketService.connect(token.value)
      fetchUser()
    }
  }

  return {
    user,
    token,
    loading,
    error,
    isAuthenticated,
    isAdmin,
    userName,
    register,
    login,
    logout,
    fetchUser,
    initialize
  }
})
