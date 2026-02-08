# Smart Locker System - Deployment Guide

## 🌐 Deployment Options

### Option 1: Vercel (Frontend) + Render (Backend) - FREE

This is the easiest and most cost-effective option for getting started.

---

## 📦 Step 1: Deploy Backend to Render

### 1.1 Prepare Backend
```bash
cd backend
```

### 1.2 Create MongoDB Atlas Database (Free Tier)

1. Go to [MongoDB Atlas](https://cloud.mongodb.com)
2. Create free account → Create Cluster (M0 Free Tier)
3. Create Database User with password
4. Network Access → Add `0.0.0.0/0` (allow all IPs)
5. Get connection string: `mongodb+srv://user:pass@cluster.mongodb.net/smartlocker`

### 1.3 Deploy to Render

1. Go to [Render](https://render.com) and sign up
2. Connect your GitHub repository
3. Create **New Web Service**
4. Select your repo → Set root directory to `backend`
5. Configure:
   - **Name**: `smart-locker-api`
   - **Runtime**: Node
   - **Build Command**: `npm install`
   - **Start Command**: `npm start`

6. Add Environment Variables:
   ```
   NODE_ENV=production
   PORT=5000
   MONGODB_URI=mongodb+srv://...your-atlas-uri...
   JWT_SECRET=your-random-secret-key-here
   FRONTEND_URL=https://your-frontend.vercel.app
   ```

7. Deploy! Your backend URL will be: `https://smart-locker-api.onrender.com`

---

## 🎨 Step 2: Deploy Frontend to Vercel

### 2.1 Update Frontend Environment

Create `.env.production` in frontend root:
```env
VITE_API_URL=https://smart-locker-api.onrender.com/api
VITE_WS_URL=https://smart-locker-api.onrender.com
```

### 2.2 Deploy to Vercel

1. Go to [Vercel](https://vercel.com) and sign up
2. Import your GitHub repository
3. Configure:
   - **Framework Preset**: Vite
   - **Root Directory**: `.` (or `shelf` if nested)
   - **Build Command**: `npm run build`
   - **Output Directory**: `dist`

4. Add Environment Variables:
   ```
   VITE_API_URL=https://smart-locker-api.onrender.com/api
   VITE_WS_URL=https://smart-locker-api.onrender.com
   ```

5. Deploy! Your frontend URL: `https://smart-locker.vercel.app`

---

## 🔄 Step 3: Update CORS on Backend

After deploying frontend, update backend environment on Render:
```
FRONTEND_URL=https://smart-locker.vercel.app
CORS_ORIGINS=https://smart-locker.vercel.app
```

---

## 🔧 Step 4: Configure ESP32 for Production

Update your ESP32 code:

```cpp
// For production server
const char* SERVER_HOST = "smart-locker-api.onrender.com";
const int SERVER_PORT = 443;  // HTTPS
const bool USE_SSL = true;    // Enable SSL

// Your WiFi at the locker location
const char* WIFI_SSID = "LockerLocation_WiFi";
const char* WIFI_PASSWORD = "wifi_password";
```

---

## 🚀 Option 2: VPS Deployment (DigitalOcean/AWS/Linode)

For full control and better WebSocket performance.

### 2.1 Setup Ubuntu VPS

```bash
# SSH into your server
ssh root@your-server-ip

# Update system
sudo apt update && sudo apt upgrade -y

# Install Node.js 20
curl -fsSL https://deb.nodesource.com/setup_20.x | sudo -E bash -
sudo apt install -y nodejs

# Install MongoDB
# Or use MongoDB Atlas (recommended)

# Install PM2 for process management
sudo npm install -g pm2

# Install Nginx
sudo apt install -y nginx

# Install Certbot for SSL
sudo apt install -y certbot python3-certbot-nginx
```

### 2.2 Clone and Setup

```bash
# Clone your repo
cd /var/www
git clone https://github.com/yourusername/shelf.git
cd shelf

# Install backend dependencies
cd backend
npm install

# Create production env file
nano .env
# Add your production environment variables

# Start with PM2
pm2 start src/server.js --name smart-locker-api
pm2 save
pm2 startup
```

### 2.3 Configure Nginx

```bash
sudo nano /etc/nginx/sites-available/smart-locker
```

```nginx
# API Server (Backend)
server {
    listen 80;
    server_name api.yourlocker.com;

    location / {
        proxy_pass http://localhost:5000;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection 'upgrade';
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
        proxy_cache_bypass $http_upgrade;
    }
}

# Frontend (if hosting on same server)
server {
    listen 80;
    server_name yourlocker.com www.yourlocker.com;
    root /var/www/shelf/dist;
    index index.html;

    location / {
        try_files $uri $uri/ /index.html;
    }

    location /api {
        proxy_pass http://localhost:5000;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection 'upgrade';
        proxy_set_header Host $host;
        proxy_cache_bypass $http_upgrade;
    }

    location /socket.io {
        proxy_pass http://localhost:5000;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection 'upgrade';
        proxy_set_header Host $host;
        proxy_cache_bypass $http_upgrade;
    }
}
```

```bash
# Enable site
sudo ln -s /etc/nginx/sites-available/smart-locker /etc/nginx/sites-enabled/
sudo nginx -t
sudo systemctl restart nginx

# Setup SSL
sudo certbot --nginx -d api.yourlocker.com -d yourlocker.com
```

### 2.4 Build Frontend

```bash
cd /var/www/shelf
npm install
npm run build
# Files will be in /var/www/shelf/dist
```

---

## 🔒 SSL/HTTPS for ESP32

For ESP32 to connect via HTTPS/WSS, you need:

```cpp
// In ESP32 code
#include <WiFiClientSecure.h>

// Use SSL connection
const bool USE_SSL = true;

// For self-signed certs (not recommended for production)
// webSocket.setInsecure();

// For proper SSL, add your CA certificate
const char* root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\n" \
// ... rest of certificate
"-----END CERTIFICATE-----\n";
```

---

## 📱 Domain Setup

1. Buy domain from Namecheap, GoDaddy, or Cloudflare
2. Add DNS records:
   - `A` record: `yourlocker.com` → Your server IP
   - `A` record: `api.yourlocker.com` → Your server IP
   - `CNAME` record: `www` → `yourlocker.com`

---

## ✅ Deployment Checklist

- [ ] MongoDB Atlas database created
- [ ] Backend deployed with environment variables
- [ ] Frontend deployed with correct API URLs
- [ ] CORS configured for frontend domain
- [ ] SSL certificates installed
- [ ] ESP32 updated with production server details
- [ ] Test QR scanning on mobile
- [ ] Test payment flow
- [ ] Test ESP32 connection and lock/unlock

---

## 🐛 Troubleshooting

### Backend won't start
- Check `MONGODB_URI` is correct
- Verify environment variables are set

### CORS errors
- Add your frontend URL to `FRONTEND_URL` and `CORS_ORIGINS`

### ESP32 won't connect
- Check WiFi credentials
- Verify server URL and port
- For HTTPS, ensure SSL is enabled in ESP32 code
- Check firewall allows WebSocket connections

### WebSocket disconnects
- Render free tier may sleep after inactivity
- Consider upgrading or using a VPS

---

## 💰 Cost Breakdown

| Service | Free Tier | Paid |
|---------|-----------|------|
| MongoDB Atlas | 512 MB | From $9/mo |
| Render | 750 hrs/mo | From $7/mo |
| Vercel | 100 GB bandwidth | From $20/mo |
| **Total** | **$0** | **~$36/mo** |

For production with good uptime, budget ~$20-50/month.
