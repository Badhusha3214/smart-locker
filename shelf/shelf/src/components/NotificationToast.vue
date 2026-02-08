<template>
  <Teleport to="body">
    <Transition name="toast">
      <div
        v-if="visible"
        :class="[
          'fixed z-50 flex items-center space-x-3 px-4 py-3 rounded-lg shadow-lg max-w-md',
          positionClasses,
          typeClasses
        ]"
      >
        <!-- Icon -->
        <div class="flex-shrink-0">
          <component :is="iconComponent" class="w-5 h-5" />
        </div>

        <!-- Content -->
        <div class="flex-1 min-w-0">
          <p v-if="title" class="font-medium">{{ title }}</p>
          <p :class="['text-sm', { 'mt-1': title }]">{{ message }}</p>
        </div>

        <!-- Close button -->
        <button 
          v-if="closable"
          @click="close"
          class="flex-shrink-0 opacity-70 hover:opacity-100 transition-opacity"
        >
          <svg class="w-4 h-4" fill="none" stroke="currentColor" viewBox="0 0 24 24">
            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M6 18L18 6M6 6l12 12" />
          </svg>
        </button>
      </div>
    </Transition>
  </Teleport>
</template>

<script setup>
import { ref, computed, watch, onMounted, h } from 'vue'

const props = defineProps({
  type: {
    type: String,
    default: 'info',
    validator: (value) => ['success', 'error', 'warning', 'info'].includes(value)
  },
  title: {
    type: String,
    default: ''
  },
  message: {
    type: String,
    required: true
  },
  duration: {
    type: Number,
    default: 3000 // ms, 0 for no auto-close
  },
  position: {
    type: String,
    default: 'top-right',
    validator: (value) => ['top-right', 'top-left', 'bottom-right', 'bottom-left', 'top-center', 'bottom-center'].includes(value)
  },
  closable: {
    type: Boolean,
    default: true
  },
  modelValue: {
    type: Boolean,
    default: true
  }
})

const emit = defineEmits(['update:modelValue', 'close'])

const visible = ref(props.modelValue)
let timeoutId = null

const typeClasses = computed(() => {
  const classes = {
    success: 'bg-green-50 text-green-800 border border-green-200',
    error: 'bg-red-50 text-red-800 border border-red-200',
    warning: 'bg-yellow-50 text-yellow-800 border border-yellow-200',
    info: 'bg-blue-50 text-blue-800 border border-blue-200'
  }
  return classes[props.type]
})

const positionClasses = computed(() => {
  const positions = {
    'top-right': 'top-4 right-4',
    'top-left': 'top-4 left-4',
    'bottom-right': 'bottom-4 right-4',
    'bottom-left': 'bottom-4 left-4',
    'top-center': 'top-4 left-1/2 -translate-x-1/2',
    'bottom-center': 'bottom-4 left-1/2 -translate-x-1/2'
  }
  return positions[props.position]
})

const iconComponent = computed(() => {
  const icons = {
    success: {
      render() {
        return h('svg', { fill: 'none', stroke: 'currentColor', viewBox: '0 0 24 24' }, [
          h('path', { 
            'stroke-linecap': 'round', 
            'stroke-linejoin': 'round', 
            'stroke-width': '2', 
            d: 'M5 13l4 4L19 7' 
          })
        ])
      }
    },
    error: {
      render() {
        return h('svg', { fill: 'none', stroke: 'currentColor', viewBox: '0 0 24 24' }, [
          h('path', { 
            'stroke-linecap': 'round', 
            'stroke-linejoin': 'round', 
            'stroke-width': '2', 
            d: 'M6 18L18 6M6 6l12 12' 
          })
        ])
      }
    },
    warning: {
      render() {
        return h('svg', { fill: 'none', stroke: 'currentColor', viewBox: '0 0 24 24' }, [
          h('path', { 
            'stroke-linecap': 'round', 
            'stroke-linejoin': 'round', 
            'stroke-width': '2', 
            d: 'M12 9v2m0 4h.01m-6.938 4h13.856c1.54 0 2.502-1.667 1.732-3L13.732 4c-.77-1.333-2.694-1.333-3.464 0L3.34 16c-.77 1.333.192 3 1.732 3z' 
          })
        ])
      }
    },
    info: {
      render() {
        return h('svg', { fill: 'none', stroke: 'currentColor', viewBox: '0 0 24 24' }, [
          h('path', { 
            'stroke-linecap': 'round', 
            'stroke-linejoin': 'round', 
            'stroke-width': '2', 
            d: 'M13 16h-1v-4h-1m1-4h.01M21 12a9 9 0 11-18 0 9 9 0 0118 0z' 
          })
        ])
      }
    }
  }
  return icons[props.type]
})

function close() {
  visible.value = false
  emit('update:modelValue', false)
  emit('close')
  clearTimeout(timeoutId)
}

function startTimer() {
  if (props.duration > 0) {
    timeoutId = setTimeout(close, props.duration)
  }
}

watch(() => props.modelValue, (newValue) => {
  visible.value = newValue
  if (newValue) {
    startTimer()
  }
})

onMounted(() => {
  if (visible.value) {
    startTimer()
  }
})

defineExpose({ close })
</script>

<style scoped>
.toast-enter-active,
.toast-leave-active {
  transition: all 0.3s ease;
}

.toast-enter-from {
  opacity: 0;
  transform: translateX(100%);
}

.toast-leave-to {
  opacity: 0;
  transform: translateX(100%);
}
</style>
