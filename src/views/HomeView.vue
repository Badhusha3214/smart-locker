<template>
  <div class="min-h-screen bg-gradient-to-br from-blue-50 to-indigo-100">
    <!-- Hero Section -->
    <section class="relative overflow-hidden">
      <div class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-20 lg:py-32">
        <div class="grid lg:grid-cols-2 gap-12 items-center">
          <div>
            <h1 class="text-4xl lg:text-6xl font-bold text-gray-900 leading-tight">
              Smart QR-Controlled
              <span class="text-blue-600">Locker System</span>
            </h1>
            <p class="mt-6 text-lg text-gray-600 leading-relaxed">
              Secure, contactless locker access with QR code technology. 
              Simply scan, pay, and access your locker in seconds.
            </p>
            <div class="mt-8 flex flex-col sm:flex-row gap-4">
              <router-link 
                to="/scan"
                class="inline-flex items-center justify-center px-6 py-3 bg-blue-600 hover:bg-blue-700 text-white font-medium rounded-lg transition-colors shadow-lg hover:shadow-xl"
              >
                <svg class="w-5 h-5 mr-2" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                  <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
                    d="M12 4v1m6 11h2m-6 0h-2v4m0-11v3m0 0h.01M12 12h4.01M16 20h4M4 12h4m12 0h.01M5 8h2a1 1 0 001-1V5a1 1 0 00-1-1H5a1 1 0 00-1 1v2a1 1 0 001 1zm12 0h2a1 1 0 001-1V5a1 1 0 00-1-1h-2a1 1 0 00-1 1v2a1 1 0 001 1zM5 20h2a1 1 0 001-1v-2a1 1 0 00-1-1H5a1 1 0 00-1 1v2a1 1 0 001 1z" />
                </svg>
                Scan QR Code
              </router-link>
              <router-link 
                v-if="!userStore.isAuthenticated"
                to="/register"
                class="inline-flex items-center justify-center px-6 py-3 bg-white hover:bg-gray-50 text-gray-900 font-medium rounded-lg border border-gray-200 transition-colors"
              >
                Create Account
              </router-link>
            </div>
          </div>
          <div class="relative">
            <div class="bg-white rounded-2xl shadow-2xl p-8">
              <div class="grid grid-cols-3 gap-4">
                <div v-for="i in 9" :key="i" class="aspect-square bg-gray-100 rounded-lg flex items-center justify-center">
                  <div 
                    :class="[
                      'w-4 h-4 rounded-full',
                      i % 3 === 0 ? 'bg-green-500' : i % 2 === 0 ? 'bg-red-500' : 'bg-gray-300'
                    ]"
                  ></div>
                </div>
              </div>
              <div class="mt-6 text-center">
                <p class="text-sm text-gray-500">Live Locker Status</p>
              </div>
            </div>
          </div>
        </div>
      </div>
    </section>

    <!-- Features Section -->
    <section class="py-20 bg-white">
      <div class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        <div class="text-center">
          <h2 class="text-3xl font-bold text-gray-900">How It Works</h2>
          <p class="mt-4 text-gray-600">Three simple steps to secure locker access</p>
        </div>

        <div class="mt-16 grid md:grid-cols-3 gap-8">
          <div class="text-center">
            <div class="w-16 h-16 mx-auto bg-blue-100 rounded-full flex items-center justify-center">
              <svg class="w-8 h-8 text-blue-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
                  d="M12 4v1m6 11h2m-6 0h-2v4m0-11v3m0 0h.01M12 12h4.01M16 20h4M4 12h4m12 0h.01M5 8h2a1 1 0 001-1V5a1 1 0 00-1-1H5a1 1 0 00-1 1v2a1 1 0 001 1zm12 0h2a1 1 0 001-1V5a1 1 0 00-1-1h-2a1 1 0 00-1 1v2a1 1 0 001 1zM5 20h2a1 1 0 001-1v-2a1 1 0 00-1-1H5a1 1 0 00-1 1v2a1 1 0 001 1z" />
              </svg>
            </div>
            <h3 class="mt-6 text-xl font-semibold text-gray-900">1. Scan QR Code</h3>
            <p class="mt-2 text-gray-600">Scan the QR code on the locker using your phone's camera</p>
          </div>

          <div class="text-center">
            <div class="w-16 h-16 mx-auto bg-green-100 rounded-full flex items-center justify-center">
              <svg class="w-8 h-8 text-green-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
                  d="M3 10h18M7 15h1m4 0h1m-7 4h12a3 3 0 003-3V8a3 3 0 00-3-3H6a3 3 0 00-3 3v8a3 3 0 003 3z" />
              </svg>
            </div>
            <h3 class="mt-6 text-xl font-semibold text-gray-900">2. Make Payment</h3>
            <p class="mt-2 text-gray-600">Secure payment through Razorpay - UPI, Cards, and more</p>
          </div>

          <div class="text-center">
            <div class="w-16 h-16 mx-auto bg-purple-100 rounded-full flex items-center justify-center">
              <svg class="w-8 h-8 text-purple-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
                  d="M8 11V7a4 4 0 118 0m-4 8v2m-6 4h12a2 2 0 002-2v-6a2 2 0 00-2-2H6a2 2 0 00-2 2v6a2 2 0 002 2z" />
              </svg>
            </div>
            <h3 class="mt-6 text-xl font-semibold text-gray-900">3. Access Locker</h3>
            <p class="mt-2 text-gray-600">Locker unlocks automatically - store your items safely</p>
          </div>
        </div>
      </div>
    </section>

    <!-- Stats Section -->
    <section class="py-16 bg-blue-600">
      <div class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        <div class="grid grid-cols-2 md:grid-cols-4 gap-8 text-center">
          <div>
            <p class="text-4xl font-bold text-white">500+</p>
            <p class="mt-2 text-blue-200">Lockers Available</p>
          </div>
          <div>
            <p class="text-4xl font-bold text-white">10k+</p>
            <p class="mt-2 text-blue-200">Happy Users</p>
          </div>
          <div>
            <p class="text-4xl font-bold text-white">99.9%</p>
            <p class="mt-2 text-blue-200">Uptime</p>
          </div>
          <div>
            <p class="text-4xl font-bold text-white">24/7</p>
            <p class="mt-2 text-blue-200">Support</p>
          </div>
        </div>
      </div>
    </section>

    <!-- Pricing Section -->
    <section class="py-20 bg-gray-50">
      <div class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        <div class="text-center">
          <h2 class="text-3xl font-bold text-gray-900">Simple Pricing</h2>
          <p class="mt-4 text-gray-600">Pay per use - no subscriptions required</p>
        </div>

        <div class="mt-12 grid md:grid-cols-3 gap-8 max-w-4xl mx-auto">
          <div class="bg-white rounded-xl shadow-lg p-8 text-center">
            <h3 class="text-lg font-semibold text-gray-900">Small Locker</h3>
            <p class="mt-4 text-4xl font-bold text-gray-900">₹10<span class="text-lg font-normal text-gray-500">/use</span></p>
            <ul class="mt-6 space-y-3 text-gray-600">
              <li>Small items</li>
              <li>30 minutes access</li>
              <li>Basic security</li>
            </ul>
          </div>

          <div class="bg-blue-600 rounded-xl shadow-lg p-8 text-center transform scale-105">
            <h3 class="text-lg font-semibold text-white">Medium Locker</h3>
            <p class="mt-4 text-4xl font-bold text-white">₹20<span class="text-lg font-normal text-blue-200">/use</span></p>
            <ul class="mt-6 space-y-3 text-blue-100">
              <li>Bags & backpacks</li>
              <li>1 hour access</li>
              <li>Enhanced security</li>
            </ul>
          </div>

          <div class="bg-white rounded-xl shadow-lg p-8 text-center">
            <h3 class="text-lg font-semibold text-gray-900">Large Locker</h3>
            <p class="mt-4 text-4xl font-bold text-gray-900">₹30<span class="text-lg font-normal text-gray-500">/use</span></p>
            <ul class="mt-6 space-y-3 text-gray-600">
              <li>Luggage & large items</li>
              <li>2 hours access</li>
              <li>Premium security</li>
            </ul>
          </div>
        </div>
      </div>
    </section>

    <!-- CTA Section -->
    <section class="py-20 bg-white">
      <div class="max-w-3xl mx-auto text-center px-4">
        <h2 class="text-3xl font-bold text-gray-900">Ready to Get Started?</h2>
        <p class="mt-4 text-lg text-gray-600">
          Find a locker near you and experience hassle-free storage
        </p>
        <router-link 
          to="/scan"
          class="mt-8 inline-flex items-center justify-center px-8 py-4 bg-blue-600 hover:bg-blue-700 text-white font-medium rounded-lg transition-colors shadow-lg hover:shadow-xl text-lg"
        >
          Start Scanning Now
        </router-link>
      </div>
    </section>
  </div>
</template>

<script setup>
import { useUserStore } from '@/stores/user'

const userStore = useUserStore()
</script>
