<template>
  <div class="countdown-timer text-center">
    <div 
      :class="[
        'relative inline-flex items-center justify-center',
        sizeClasses.container
      ]"
    >
      <!-- Circular progress -->
      <svg :class="sizeClasses.svg" viewBox="0 0 100 100">
        <!-- Background circle -->
        <circle
          cx="50"
          cy="50"
          r="45"
          fill="none"
          stroke="#e5e7eb"
          stroke-width="8"
        />
        <!-- Progress circle -->
        <circle
          cx="50"
          cy="50"
          r="45"
          fill="none"
          :stroke="progressColor"
          stroke-width="8"
          stroke-linecap="round"
          :stroke-dasharray="circumference"
          :stroke-dashoffset="dashOffset"
          class="transition-all duration-1000 ease-linear"
          transform="rotate(-90 50 50)"
        />
      </svg>
      
      <!-- Time display -->
      <div class="absolute inset-0 flex flex-col items-center justify-center">
        <span :class="['font-bold', sizeClasses.time, timeColor]">
          {{ formattedTime }}
        </span>
        <span v-if="showLabel" class="text-gray-500 text-xs mt-1">
          {{ label }}
        </span>
      </div>
    </div>

    <!-- Warning message -->
    <p v-if="isWarning && showWarning" class="mt-2 text-yellow-600 text-sm font-medium animate-pulse">
      Lock closing soon!
    </p>
  </div>
</template>

<script setup>
import { ref, computed, watch, onMounted, onUnmounted } from 'vue'

const props = defineProps({
  duration: {
    type: Number,
    default: 30 // seconds
  },
  autoStart: {
    type: Boolean,
    default: false
  },
  warningThreshold: {
    type: Number,
    default: 10 // seconds
  },
  size: {
    type: String,
    default: 'md',
    validator: (value) => ['sm', 'md', 'lg'].includes(value)
  },
  label: {
    type: String,
    default: 'seconds'
  },
  showLabel: {
    type: Boolean,
    default: true
  },
  showWarning: {
    type: Boolean,
    default: true
  }
})

const emit = defineEmits(['complete', 'tick', 'warning'])

const timeRemaining = ref(props.duration)
const isRunning = ref(false)
let intervalId = null

const circumference = 2 * Math.PI * 45

const sizeClasses = computed(() => {
  const sizes = {
    sm: { container: 'w-20 h-20', svg: 'w-20 h-20', time: 'text-lg' },
    md: { container: 'w-32 h-32', svg: 'w-32 h-32', time: 'text-2xl' },
    lg: { container: 'w-40 h-40', svg: 'w-40 h-40', time: 'text-4xl' }
  }
  return sizes[props.size]
})

const progress = computed(() => {
  return timeRemaining.value / props.duration
})

const dashOffset = computed(() => {
  return circumference * (1 - progress.value)
})

const isWarning = computed(() => {
  return timeRemaining.value <= props.warningThreshold && timeRemaining.value > 0
})

const progressColor = computed(() => {
  if (timeRemaining.value <= 5) return '#ef4444' // red
  if (isWarning.value) return '#f59e0b' // yellow
  return '#3b82f6' // blue
})

const timeColor = computed(() => {
  if (timeRemaining.value <= 5) return 'text-red-600'
  if (isWarning.value) return 'text-yellow-600'
  return 'text-gray-900'
})

const formattedTime = computed(() => {
  const minutes = Math.floor(timeRemaining.value / 60)
  const seconds = timeRemaining.value % 60
  
  if (minutes > 0) {
    return `${minutes}:${seconds.toString().padStart(2, '0')}`
  }
  return seconds.toString()
})

function start() {
  if (isRunning.value) return
  
  isRunning.value = true
  intervalId = setInterval(() => {
    if (timeRemaining.value > 0) {
      timeRemaining.value--
      emit('tick', timeRemaining.value)
      
      if (timeRemaining.value === props.warningThreshold) {
        emit('warning')
      }
    } else {
      stop()
      emit('complete')
    }
  }, 1000)
}

function stop() {
  isRunning.value = false
  if (intervalId) {
    clearInterval(intervalId)
    intervalId = null
  }
}

function reset() {
  stop()
  timeRemaining.value = props.duration
}

function restart() {
  reset()
  start()
}

watch(() => props.duration, (newDuration) => {
  timeRemaining.value = newDuration
})

onMounted(() => {
  if (props.autoStart) {
    start()
  }
})

onUnmounted(() => {
  stop()
})

defineExpose({ start, stop, reset, restart, timeRemaining, isRunning })
</script>
