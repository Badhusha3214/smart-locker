<template>
  <div class="flex items-center space-x-2">
    <div 
      :class="[
        'w-4 h-4 rounded-full transition-all duration-300',
        statusClasses,
        { 'animate-pulse': isAnimating }
      ]"
    ></div>
    <span :class="['text-sm font-medium', textColorClass]">
      {{ statusText }}
    </span>
  </div>
</template>

<script setup>
import { computed } from 'vue'

const props = defineProps({
  status: {
    type: String,
    default: 'off',
    validator: (value) => ['green', 'red', 'blue', 'yellow', 'off'].includes(value)
  },
  showText: {
    type: Boolean,
    default: true
  }
})

const statusConfig = {
  green: {
    classes: 'bg-green-500 shadow-green-500/50 shadow-lg',
    text: 'Available',
    textColor: 'text-green-600'
  },
  red: {
    classes: 'bg-red-500 shadow-red-500/50 shadow-lg',
    text: 'Occupied',
    textColor: 'text-red-600'
  },
  blue: {
    classes: 'bg-blue-500 shadow-blue-500/50 shadow-lg',
    text: 'Processing',
    textColor: 'text-blue-600'
  },
  yellow: {
    classes: 'bg-yellow-500 shadow-yellow-500/50 shadow-lg',
    text: 'Warning',
    textColor: 'text-yellow-600'
  },
  off: {
    classes: 'bg-gray-400',
    text: 'Offline',
    textColor: 'text-gray-600'
  }
}

const statusClasses = computed(() => {
  return statusConfig[props.status]?.classes || statusConfig.off.classes
})

const statusText = computed(() => {
  return props.showText ? (statusConfig[props.status]?.text || 'Unknown') : ''
})

const textColorClass = computed(() => {
  return statusConfig[props.status]?.textColor || 'text-gray-600'
})

const isAnimating = computed(() => {
  return ['blue', 'yellow'].includes(props.status)
})
</script>
