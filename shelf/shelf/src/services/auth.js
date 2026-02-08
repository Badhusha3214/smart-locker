import api from './api'

export default {
  async register(userData) {
    const response = await api.post('/auth/register', userData)
    return response.data
  },

  async login(credentials) {
    const response = await api.post('/auth/login', credentials)
    return response.data
  },

  async logout() {
    try {
      const response = await api.post('/auth/logout')
      return response.data
    } catch (error) {
      // Still logout locally even if API fails
      return { success: true }
    }
  },

  async getMe() {
    const response = await api.get('/auth/me')
    return response.data
  },

  async updateProfile(profileData) {
    const response = await api.put('/auth/profile', profileData)
    return response.data
  }
}
