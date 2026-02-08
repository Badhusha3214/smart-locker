import { createRouter, createWebHistory } from 'vue-router'
import { useUserStore } from '@/stores/user'

const routes = [
  {
    path: '/',
    name: 'home',
    component: () => import('@/views/HomeView.vue'),
    meta: { title: 'Home' }
  },
  {
    path: '/scan',
    name: 'scan',
    component: () => import('@/views/ScanView.vue'),
    meta: { title: 'Scan QR Code' }
  },
  {
    path: '/rack/:id',
    name: 'rack-details',
    component: () => import('@/views/RackDetailsView.vue'),
    meta: { title: 'Rack Details' },
    props: true
  },
  {
    path: '/payment/:rackId',
    name: 'payment',
    component: () => import('@/views/PaymentView.vue'),
    meta: { title: 'Payment', requiresAuth: true },
    props: true
  },
  {
    path: '/access/:rackId',
    name: 'access',
    component: () => import('@/views/AccessView.vue'),
    meta: { title: 'Access Locker', requiresAuth: true },
    props: true
  },
  {
    path: '/history',
    name: 'history',
    component: () => import('@/views/HistoryView.vue'),
    meta: { title: 'Transaction History', requiresAuth: true }
  },
  {
    path: '/login',
    name: 'login',
    component: () => import('@/views/auth/LoginView.vue'),
    meta: { title: 'Login', guest: true }
  },
  {
    path: '/register',
    name: 'register',
    component: () => import('@/views/auth/RegisterView.vue'),
    meta: { title: 'Register', guest: true }
  },
  {
    path: '/admin',
    component: () => import('@/views/admin/AdminLayout.vue'),
    meta: { requiresAuth: true, requiresAdmin: true },
    children: [
      {
        path: '',
        name: 'admin-dashboard',
        component: () => import('@/views/admin/DashboardView.vue'),
        meta: { title: 'Admin Dashboard' }
      },
      {
        path: 'racks',
        name: 'admin-racks',
        component: () => import('@/views/admin/RackManagementView.vue'),
        meta: { title: 'Manage Racks' }
      },
      {
        path: 'transactions',
        name: 'admin-transactions',
        component: () => import('@/views/admin/TransactionsView.vue'),
        meta: { title: 'All Transactions' }
      },
      {
        path: 'devices',
        name: 'admin-devices',
        component: () => import('@/views/admin/DevicesView.vue'),
        meta: { title: 'ESP32 Devices' }
      }
    ]
  },
  {
    path: '/:pathMatch(.*)*',
    name: 'not-found',
    component: () => import('@/views/NotFoundView.vue'),
    meta: { title: '404 - Not Found' }
  }
]

const router = createRouter({
  history: createWebHistory(),
  routes,
  scrollBehavior(to, from, savedPosition) {
    if (savedPosition) {
      return savedPosition
    }
    return { top: 0 }
  }
})

// Navigation guards
router.beforeEach((to, from, next) => {
  // Get auth state from localStorage first (in case store not ready)
  const token = localStorage.getItem('token')
  const userStr = localStorage.getItem('user')
  const user = userStr ? JSON.parse(userStr) : null
  
  const isAuthenticated = !!token && !!user
  const isAdmin = user?.role === 'admin'
  
  // Set page title
  document.title = to.meta.title 
    ? `${to.meta.title} - Smart Locker` 
    : 'Smart Locker System'

  // Check authentication
  if (to.meta.requiresAuth && !isAuthenticated) {
    next({ name: 'login', query: { redirect: to.fullPath } })
  } else if (to.meta.guest && isAuthenticated) {
    next({ name: 'home' })
  } else if (to.meta.requiresAdmin && !isAdmin) {
    next({ name: 'home' })
  } else {
    next()
  }
})

export default router
