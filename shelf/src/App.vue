<script setup>
import { computed } from 'vue'
import { useRoute } from 'vue-router'
import Navbar from './components/Navbar.vue'

const route = useRoute()

// Don't show navbar on admin pages (they have their own layout)
const showNavbar = computed(() => {
  return !route.path.startsWith('/admin')
})
</script>

<template>
  <div id="app" class="min-h-screen bg-gray-50">
    <Navbar v-if="showNavbar" />
    <router-view />
  </div>
</template>

<style>
/* Global styles */
#app {
  min-height: 100vh;
}
</style>
