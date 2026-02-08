<template>
  <div class="min-h-screen bg-gray-100">
    <!-- Admin Sidebar -->
    <aside class="fixed inset-y-0 left-0 w-64 bg-gray-900 text-white z-50 transform transition-transform duration-200 md:translate-x-0"
      :class="{ '-translate-x-full': !showSidebar }">
      <div class="flex items-center justify-between h-16 px-4 bg-gray-800">
        <router-link to="/admin" class="flex items-center space-x-2">
          <div class="w-8 h-8 bg-blue-600 rounded-lg flex items-center justify-center">
            <svg class="w-5 h-5 text-white" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
                d="M12 15v2m-6 4h12a2 2 0 002-2v-6a2 2 0 00-2-2H6a2 2 0 00-2 2v6a2 2 0 002 2zm10-10V7a4 4 0 00-8 0v4h8z" />
            </svg>
          </div>
          <span class="font-bold">Admin Panel</span>
        </router-link>
        <button @click="showSidebar = false" class="md:hidden text-gray-400 hover:text-white">
          <svg class="w-6 h-6" fill="none" stroke="currentColor" viewBox="0 0 24 24">
            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M6 18L18 6M6 6l12 12" />
          </svg>
        </button>
      </div>
      
      <nav class="mt-6 px-4">
        <router-link 
          to="/admin"
          :class="[
            'flex items-center space-x-3 px-4 py-3 rounded-lg transition-colors',
            $route.path === '/admin' ? 'bg-blue-600 text-white' : 'text-gray-300 hover:bg-gray-800'
          ]"
        >
          <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
              d="M3 12l2-2m0 0l7-7 7 7M5 10v10a1 1 0 001 1h3m10-11l2 2m-2-2v10a1 1 0 01-1 1h-3m-6 0a1 1 0 001-1v-4a1 1 0 011-1h2a1 1 0 011 1v4a1 1 0 001 1m-6 0h6" />
          </svg>
          <span>Dashboard</span>
        </router-link>
        
        <router-link 
          to="/admin/racks"
          :class="[
            'flex items-center space-x-3 px-4 py-3 rounded-lg transition-colors mt-2',
            $route.path === '/admin/racks' ? 'bg-blue-600 text-white' : 'text-gray-300 hover:bg-gray-800'
          ]"
        >
          <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
              d="M19 11H5m14 0a2 2 0 012 2v6a2 2 0 01-2 2H5a2 2 0 01-2-2v-6a2 2 0 012-2m14 0V9a2 2 0 00-2-2M5 11V9a2 2 0 012-2m0 0V5a2 2 0 012-2h6a2 2 0 012 2v2M7 7h10" />
          </svg>
          <span>Racks</span>
        </router-link>
        
        <router-link 
          to="/admin/transactions"
          :class="[
            'flex items-center space-x-3 px-4 py-3 rounded-lg transition-colors mt-2',
            $route.path === '/admin/transactions' ? 'bg-blue-600 text-white' : 'text-gray-300 hover:bg-gray-800'
          ]"
        >
          <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
              d="M9 5H7a2 2 0 00-2 2v12a2 2 0 002 2h10a2 2 0 002-2V7a2 2 0 00-2-2h-2M9 5a2 2 0 002 2h2a2 2 0 002-2M9 5a2 2 0 012-2h2a2 2 0 012 2" />
          </svg>
          <span>Transactions</span>
        </router-link>
        
        <router-link 
          to="/admin/devices"
          :class="[
            'flex items-center space-x-3 px-4 py-3 rounded-lg transition-colors mt-2',
            $route.path === '/admin/devices' ? 'bg-blue-600 text-white' : 'text-gray-300 hover:bg-gray-800'
          ]"
        >
          <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
              d="M9 3v2m6-2v2M9 19v2m6-2v2M5 9H3m2 6H3m18-6h-2m2 6h-2M7 19h10a2 2 0 002-2V7a2 2 0 00-2-2H7a2 2 0 00-2 2v10a2 2 0 002 2zM9 9h6v6H9V9z" />
          </svg>
          <span>ESP32 Devices</span>
        </router-link>

        <div class="mt-8 pt-8 border-t border-gray-700">
          <router-link 
            to="/"
            class="flex items-center space-x-3 px-4 py-3 rounded-lg text-gray-300 hover:bg-gray-800 transition-colors"
          >
            <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M10 19l-7-7m0 0l7-7m-7 7h18" />
            </svg>
            <span>Back to Site</span>
          </router-link>
          
          <button 
            @click="handleLogout"
            class="flex items-center space-x-3 px-4 py-3 rounded-lg text-gray-300 hover:bg-gray-800 transition-colors w-full mt-2"
          >
            <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
                d="M17 16l4-4m0 0l-4-4m4 4H7m6 4v1a3 3 0 01-3 3H6a3 3 0 01-3-3V7a3 3 0 013-3h4a3 3 0 013 3v1" />
            </svg>
            <span>Logout</span>
          </button>
        </div>
      </nav>
    </aside>

    <!-- Main Content -->
    <div class="md:ml-64">
      <!-- Top Bar -->
      <header class="bg-white shadow-sm h-16 flex items-center justify-between px-6">
        <button @click="showSidebar = true" class="md:hidden text-gray-600 hover:text-gray-900">
          <svg class="w-6 h-6" fill="none" stroke="currentColor" viewBox="0 0 24 24">
            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M4 6h16M4 12h16M4 18h16" />
          </svg>
        </button>
        
        <div class="flex items-center space-x-4">
          <span class="text-gray-600">Welcome, {{ userStore.userName }}</span>
          <div class="w-10 h-10 bg-blue-100 rounded-full flex items-center justify-center">
            <span class="text-blue-600 font-medium">{{ userStore.userName.charAt(0).toUpperCase() }}</span>
          </div>
        </div>
      </header>

      <!-- Page Content -->
      <main class="p-6">
        <router-view />
      </main>
    </div>

    <!-- Sidebar Overlay -->
    <div 
      v-if="showSidebar"
      @click="showSidebar = false"
      class="fixed inset-0 bg-black bg-opacity-50 z-40 md:hidden"
    ></div>
  </div>
</template>

<script setup>
import { ref } from 'vue'
import { useRouter } from 'vue-router'
import { useUserStore } from '@/stores/user'

const router = useRouter()
const userStore = useUserStore()

const showSidebar = ref(false)

async function handleLogout() {
  await userStore.logout()
  router.push('/login')
}
</script>
