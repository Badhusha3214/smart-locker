<template>
  <nav class="bg-white shadow-lg sticky top-0 z-50">
    <div class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
      <div class="flex justify-between h-16">
        <div class="flex items-center">
          <router-link to="/" class="flex items-center space-x-2">
            <div class="w-10 h-10 bg-blue-600 rounded-lg flex items-center justify-center">
              <svg class="w-6 h-6 text-white" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
                  d="M12 15v2m-6 4h12a2 2 0 002-2v-6a2 2 0 00-2-2H6a2 2 0 00-2 2v6a2 2 0 002 2zm10-10V7a4 4 0 00-8 0v4h8z" />
              </svg>
            </div>
            <span class="text-xl font-bold text-gray-900">Smart Locker</span>
          </router-link>
        </div>

        <!-- Desktop Navigation -->
        <div class="hidden md:flex items-center space-x-4">
          <router-link 
            to="/" 
            class="text-gray-600 hover:text-blue-600 px-3 py-2 rounded-md text-sm font-medium transition-colors"
          >
            Home
          </router-link>
          <router-link 
            to="/scan" 
            class="text-gray-600 hover:text-blue-600 px-3 py-2 rounded-md text-sm font-medium transition-colors"
          >
            Scan QR
          </router-link>
          
          <template v-if="userStore.isAuthenticated">
            <router-link 
              to="/history" 
              class="text-gray-600 hover:text-blue-600 px-3 py-2 rounded-md text-sm font-medium transition-colors"
            >
              History
            </router-link>
            <router-link 
              v-if="userStore.isAdmin"
              to="/admin" 
              class="text-gray-600 hover:text-blue-600 px-3 py-2 rounded-md text-sm font-medium transition-colors"
            >
              Admin
            </router-link>
            <div class="relative" ref="profileDropdown">
              <button 
                @click="showDropdown = !showDropdown"
                class="flex items-center space-x-2 text-gray-600 hover:text-blue-600 px-3 py-2 rounded-md text-sm font-medium transition-colors"
              >
                <div class="w-8 h-8 bg-blue-100 rounded-full flex items-center justify-center">
                  <span class="text-blue-600 font-medium">{{ userInitial }}</span>
                </div>
                <span>{{ userStore.userName }}</span>
                <svg class="w-4 h-4" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                  <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M19 9l-7 7-7-7" />
                </svg>
              </button>
              <div 
                v-if="showDropdown"
                class="absolute right-0 mt-2 w-48 bg-white rounded-md shadow-lg py-1 ring-1 ring-black ring-opacity-5"
              >
                <button 
                  @click="handleLogout"
                  class="block w-full text-left px-4 py-2 text-sm text-gray-700 hover:bg-gray-100"
                >
                  Sign out
                </button>
              </div>
            </div>
          </template>
          
          <template v-else>
            <router-link 
              to="/login" 
              class="text-gray-600 hover:text-blue-600 px-3 py-2 rounded-md text-sm font-medium transition-colors"
            >
              Login
            </router-link>
            <router-link 
              to="/register" 
              class="bg-blue-600 hover:bg-blue-700 text-white px-4 py-2 rounded-md text-sm font-medium transition-colors"
            >
              Sign Up
            </router-link>
          </template>
        </div>

        <!-- Mobile menu button -->
        <div class="md:hidden flex items-center">
          <button 
            @click="showMobileMenu = !showMobileMenu"
            class="text-gray-600 hover:text-blue-600 p-2"
          >
            <svg v-if="!showMobileMenu" class="w-6 h-6" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M4 6h16M4 12h16M4 18h16" />
            </svg>
            <svg v-else class="w-6 h-6" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M6 18L18 6M6 6l12 12" />
            </svg>
          </button>
        </div>
      </div>

      <!-- Mobile Navigation -->
      <div v-if="showMobileMenu" class="md:hidden pb-4">
        <div class="space-y-1">
          <router-link 
            to="/" 
            class="block text-gray-600 hover:text-blue-600 px-3 py-2 rounded-md text-base font-medium"
            @click="showMobileMenu = false"
          >
            Home
          </router-link>
          <router-link 
            to="/scan" 
            class="block text-gray-600 hover:text-blue-600 px-3 py-2 rounded-md text-base font-medium"
            @click="showMobileMenu = false"
          >
            Scan QR
          </router-link>
          
          <template v-if="userStore.isAuthenticated">
            <router-link 
              to="/history" 
              class="block text-gray-600 hover:text-blue-600 px-3 py-2 rounded-md text-base font-medium"
              @click="showMobileMenu = false"
            >
              History
            </router-link>
            <router-link 
              v-if="userStore.isAdmin"
              to="/admin" 
              class="block text-gray-600 hover:text-blue-600 px-3 py-2 rounded-md text-base font-medium"
              @click="showMobileMenu = false"
            >
              Admin
            </router-link>
            <button 
              @click="handleLogout"
              class="block w-full text-left text-gray-600 hover:text-blue-600 px-3 py-2 rounded-md text-base font-medium"
            >
              Sign out
            </button>
          </template>
          
          <template v-else>
            <router-link 
              to="/login" 
              class="block text-gray-600 hover:text-blue-600 px-3 py-2 rounded-md text-base font-medium"
              @click="showMobileMenu = false"
            >
              Login
            </router-link>
            <router-link 
              to="/register" 
              class="block text-gray-600 hover:text-blue-600 px-3 py-2 rounded-md text-base font-medium"
              @click="showMobileMenu = false"
            >
              Sign Up
            </router-link>
          </template>
        </div>
      </div>
    </div>
  </nav>
</template>

<script setup>
import { ref, computed, onMounted, onUnmounted } from 'vue'
import { useRouter } from 'vue-router'
import { useUserStore } from '@/stores/user'

const router = useRouter()
const userStore = useUserStore()

const showDropdown = ref(false)
const showMobileMenu = ref(false)
const profileDropdown = ref(null)

const userInitial = computed(() => {
  return userStore.user?.name?.charAt(0).toUpperCase() || 'U'
})

async function handleLogout() {
  showDropdown.value = false
  showMobileMenu.value = false
  await userStore.logout()
  router.push('/')
}

function handleClickOutside(event) {
  if (profileDropdown.value && !profileDropdown.value.contains(event.target)) {
    showDropdown.value = false
  }
}

onMounted(() => {
  document.addEventListener('click', handleClickOutside)
})

onUnmounted(() => {
  document.removeEventListener('click', handleClickOutside)
})
</script>
