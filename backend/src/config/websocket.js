const socketIO = require('socket.io');
const jwt = require('jsonwebtoken');
const Rack = require('../models/Rack');

// Store connected ESP32 devices (single rack controllers)
const connectedDevices = new Map();

// Store connected shelf controllers (multi-door)
// Key: shelfId, Value: { socketId, shelfId, racks: [rackId1, rackId2, ...] }
const connectedShelves = new Map();

// Map rack IDs to their shelf controller (for routing)
// Key: rackId, Value: shelfId
const rackToShelfMap = new Map();

/**
 * Initialize WebSocket server with Socket.IO
 * 
 * Events:
 * - subscribe-rack: Client subscribes to rack updates
 * - unsubscribe-rack: Client unsubscribes from rack
 * - subscribe-admin: Admin subscribes to all rack updates
 * - esp:register: ESP32 registers with server
 * - esp:status: ESP32 sends status update
 * - esp:itemStatus: ESP32 reports item sensor status
 * 
 * Server emits:
 * - lock-status: { status: 'locked'|'unlocked', rackId }
 * - led-status: { color: 'green'|'red'|'blue', rackId }
 * - rack-status: { rackId, status } (admin only)
 * - locker:unlock: Command ESP32 to unlock
 * - locker:lock: Command ESP32 to lock
 */
const initializeWebSocket = (server) => {
  const allowedOrigins = [
    'http://localhost:5173',
    'http://localhost:5174',
    'http://127.0.0.1:5173',
    'http://127.0.0.1:5174',
    process.env.FRONTEND_URL
  ].filter(Boolean);

  const io = socketIO(server, {
    cors: {
      origin: (origin, callback) => {
        // Allow requests with no origin (ESP32, mobile apps, curl, etc.)
        if (!origin) {
          return callback(null, true);
        }
        // Allow listed origins
        if (allowedOrigins.includes(origin)) {
          return callback(null, true);
        }
        // Allow all in development
        if (process.env.NODE_ENV !== 'production') {
          return callback(null, true);
        }
        callback(new Error('Not allowed by CORS'));
      },
      methods: ['GET', 'POST'],
      credentials: true
    },
    transports: ['websocket', 'polling'],
    allowEIO3: true,
    pingTimeout: 60000,
    pingInterval: 25000
  });

  // Optional: Authentication middleware
  io.use((socket, next) => {
    const token = socket.handshake.auth.token;
    
    if (token) {
      try {
        const decoded = jwt.verify(token, process.env.JWT_SECRET);
        socket.user = decoded;
      } catch (err) {
        // Token invalid, but still allow connection for public updates
        console.log('WebSocket: Invalid token, connecting as guest');
      }
    }
    
    next();
  });

  io.on('connection', (socket) => {
    console.log(`[WS] Client connected: ${socket.id}`);

    // Subscribe to specific rack updates
    socket.on('subscribe-rack', (rackId) => {
      socket.join(`rack-${rackId}`);
      console.log(`[WS] ${socket.id} subscribed to rack ${rackId}`);
      
      // Acknowledge subscription
      socket.emit('subscribed', { rackId });
    });

    // Unsubscribe from rack
    socket.on('unsubscribe-rack', (rackId) => {
      socket.leave(`rack-${rackId}`);
      console.log(`[WS] ${socket.id} unsubscribed from rack ${rackId}`);
    });

    // Admin subscribe to all updates
    socket.on('subscribe-admin', () => {
      // Verify admin role
      if (socket.user && socket.user.role === 'admin') {
        socket.join('admin');
        console.log(`[WS] Admin ${socket.id} subscribed to admin channel`);
        socket.emit('admin-subscribed');
      } else {
        socket.emit('error', { message: 'Admin access required' });
      }
    });

    // Handle ping for connection health
    socket.on('ping', () => {
      socket.emit('pong');
    });

    // ============== ADMIN LOCKER CONTROL HANDLERS ==============
    
    // Admin sends unlock command to ESP32
    socket.on('locker:unlock', (data) => {
      console.log(`[WS] Admin unlock command received for: ${data.rackId}`);
      
      // Check if this rack is on a shelf controller (multi-door)
      if (rackToShelfMap.has(data.rackId)) {
        const shelfId = rackToShelfMap.get(data.rackId);
        console.log(`[WS] Forwarding unlock to shelf ${shelfId} for rack: ${data.rackId}`);
        io.to(`shelf-${shelfId}`).emit('locker:unlock', { rackId: data.rackId });
        socket.emit('command-sent', { success: true, command: 'unlock', rackId: data.rackId, shelfId });
        return;
      }
      
      // Check if single rack ESP32 is connected
      if (connectedDevices.has(data.rackId)) {
        console.log(`[WS] Forwarding unlock to ESP32: ${data.rackId}`);
        io.to(`esp-${data.rackId}`).emit('locker:unlock', { rackId: data.rackId });
        socket.emit('command-sent', { success: true, command: 'unlock', rackId: data.rackId });
      } else {
        console.log(`[WS] No ESP32 or shelf controller connected for: ${data.rackId}`);
        socket.emit('command-error', { error: 'ESP32 not connected', rackId: data.rackId });
      }
    });

    // Admin sends lock command to ESP32
    socket.on('locker:lock', (data) => {
      console.log(`[WS] Admin lock command received for: ${data.rackId}`);
      
      // Check if this rack is on a shelf controller (multi-door)
      if (rackToShelfMap.has(data.rackId)) {
        const shelfId = rackToShelfMap.get(data.rackId);
        console.log(`[WS] Forwarding lock to shelf ${shelfId} for rack: ${data.rackId}`);
        io.to(`shelf-${shelfId}`).emit('locker:lock', { rackId: data.rackId });
        socket.emit('command-sent', { success: true, command: 'lock', rackId: data.rackId, shelfId });
        return;
      }
      
      // Check if single rack ESP32 is connected
      if (connectedDevices.has(data.rackId)) {
        console.log(`[WS] Forwarding lock to ESP32: ${data.rackId}`);
        io.to(`esp-${data.rackId}`).emit('locker:lock', { rackId: data.rackId });
        socket.emit('command-sent', { success: true, command: 'lock', rackId: data.rackId });
      } else {
        console.log(`[WS] No ESP32 or shelf controller connected for: ${data.rackId}`);
        socket.emit('command-error', { error: 'ESP32 not connected', rackId: data.rackId });
      }
    });

    // ============== ESP32 DEVICE HANDLERS ==============
    
    // Shelf controller registers (multi-door)
    socket.on('shelf:register', async (data) => {
      console.log(`[WS] Shelf controller registered: ${data.shelfId} with ${data.numDoors} doors`);
      
      // Extract rack IDs from the racks array
      const rackIds = data.racks ? data.racks.map(r => r.rackId) : [];
      
      // Store rack details for status tracking
      const rackDetails = {};
      if (data.racks) {
        data.racks.forEach((r, index) => {
          rackDetails[index] = {
            rackId: r.rackId,
            locked: r.isLocked !== false,
            item: r.itemPresent || false
          };
        });
      }
      
      connectedShelves.set(data.shelfId, {
        socketId: socket.id,
        shelfId: data.shelfId,
        numDoors: data.numDoors,
        racks: rackIds,
        doors: rackDetails,
        ip: data.ip,
        connectedAt: new Date()
      });
      
      // Map each rack to this shelf for routing
      rackIds.forEach(rackId => {
        rackToShelfMap.set(rackId, data.shelfId);
        console.log(`[WS]   Rack ${rackId} → Shelf ${data.shelfId}`);
      });
      
      // Join shelf-specific room
      socket.join(`shelf-${data.shelfId}`);
      
      // Also join each rack room for individual rack updates
      rackIds.forEach(rackId => {
        socket.join(`rack-${rackId}`);
        socket.join(`esp-${rackId}`);  // For backward compatibility
      });
      
      // Mark as shelf controller
      socket.isShelf = true;
      socket.shelfId = data.shelfId;
      socket.rackIds = rackIds;
      
      // Notify admin of new shelf controller
      io.to('admin').emit('shelf:connected', {
        shelfId: data.shelfId,
        numDoors: data.numDoors,
        racks: data.racks
      });
      
      // Acknowledge registration
      socket.emit('shelf:registered', { success: true, shelfId: data.shelfId });
    });
    
    // Shelf heartbeat
    socket.on('shelf:heartbeat', (data) => {
      if (connectedShelves.has(data.shelfId)) {
        const shelf = connectedShelves.get(data.shelfId);
        shelf.lastHeartbeat = new Date();
        shelf.rssi = data.rssi;
        shelf.uptime = data.uptime;
        
        // Update door states if provided (from ESP32 heartbeat)
        if (data.doors && Array.isArray(data.doors)) {
          data.doors.forEach((door, index) => {
            if (!shelf.doors) shelf.doors = {};
            shelf.doors[index] = {
              rackId: door.rackId || shelf.racks[index],
              locked: door.locked !== false,
              item: door.item || false
            };
          });
        }
      }
    });

    // Single rack ESP32 registers with server
    socket.on('esp:register', async (data) => {
      console.log(`[WS] ESP32 registered: ${data.rackId}`);
      
      // Store device connection
      connectedDevices.set(data.rackId, {
        socketId: socket.id,
        rackId: data.rackId,
        status: data.status,
        itemPresent: data.itemPresent,
        connectedAt: new Date()
      });
      
      // Join rack-specific room
      socket.join(`esp-${data.rackId}`);
      socket.join(`rack-${data.rackId}`);
      
      // Mark as ESP device
      socket.isEsp = true;
      socket.rackId = data.rackId;
      
      // Notify admin of new device
      io.to('admin').emit('esp:connected', {
        rackId: data.rackId,
        status: data.status,
        itemPresent: data.itemPresent
      });
      
      // Acknowledge registration
      socket.emit('esp:registered', { success: true });
    });
    
    // ESP32 status update (lock status) - handles both single and shelf controllers
    socket.on('esp:status', async (data) => {
      console.log(`[WS] ESP32 status update: ${data.rackId}`, data);
      
      try {
        // If this is from a shelf controller, update the shelf's door state
        if (data.shelfId && connectedShelves.has(data.shelfId)) {
          const shelf = connectedShelves.get(data.shelfId);
          const doorIndex = data.doorIndex;
          if (doorIndex !== undefined && shelf.doors) {
            shelf.doors[doorIndex] = {
              rackId: data.rackId,
              locked: data.isLocked,
              item: data.itemPresent
            };
          }
        }
        
        // Also update if this rack is mapped to a shelf
        if (rackToShelfMap.has(data.rackId)) {
          const shelfId = rackToShelfMap.get(data.rackId);
          const shelf = connectedShelves.get(shelfId);
          if (shelf && shelf.racks) {
            const doorIndex = shelf.racks.indexOf(data.rackId);
            if (doorIndex >= 0) {
              if (!shelf.doors) shelf.doors = {};
              shelf.doors[doorIndex] = {
                rackId: data.rackId,
                locked: data.isLocked,
                item: data.itemPresent
              };
            }
          }
        }
        
        // Update rack in database
        const rack = await Rack.findOne({ rackNumber: data.rackId });
        if (rack) {
          rack.isLocked = data.isLocked;
          await rack.save();
          
          // Notify all subscribers
          io.to(`rack-${rack._id}`).emit('lock-status', {
            rackId: rack._id,
            status: data.isLocked ? 'locked' : 'unlocked'
          });
          
          // Notify admin
          io.to('admin').emit('rack-status', {
            rackId: rack._id,
            rackNumber: rack.rackNumber,
            isLocked: data.isLocked
          });
        }
      } catch (error) {
        console.error('[WS] Error updating rack status:', error);
      }
    });
    
    // ESP32 item sensor status
    socket.on('esp:itemStatus', async (data) => {
      console.log(`[WS] ESP32 item status: ${data.rackId}`, data);
      
      try {
        // Update rack in database
        const rack = await Rack.findOne({ rackNumber: data.rackId });
        if (rack) {
          rack.needsRefill = data.needsRefill || !data.itemPresent;
          await rack.save();
          
          // Notify admin of refill needed
          if (rack.needsRefill) {
            io.to('admin').emit('refill-needed', {
              rackId: rack._id,
              rackNumber: rack.rackNumber,
              needsRefill: true
            });
          }
          
          // Notify rack subscribers
          io.to(`rack-${rack._id}`).emit('item-status', {
            rackId: rack._id,
            itemPresent: data.itemPresent
          });
        }
      } catch (error) {
        console.error('[WS] Error updating item status:', error);
      }
    });

    // Handle disconnect
    socket.on('disconnect', (reason) => {
      console.log(`[WS] Client disconnected: ${socket.id}, reason: ${reason}`);
      
      // If shelf controller, remove from connected shelves
      if (socket.isShelf && socket.shelfId) {
        console.log(`[WS] Shelf controller disconnected: ${socket.shelfId}`);
        
        // Remove rack to shelf mappings
        if (socket.rackIds) {
          socket.rackIds.forEach(rackId => {
            rackToShelfMap.delete(rackId);
          });
        }
        
        connectedShelves.delete(socket.shelfId);
        
        // Notify admin
        io.to('admin').emit('shelf:disconnected', {
          shelfId: socket.shelfId,
          rackIds: socket.rackIds
        });
      }
      
      // If single rack ESP32, remove from connected devices
      if (socket.isEsp && socket.rackId) {
        connectedDevices.delete(socket.rackId);
        
        // Notify admin
        io.to('admin').emit('esp:disconnected', {
          rackId: socket.rackId
        });
      }
    });

    // Handle errors
    socket.on('error', (error) => {
      console.error(`[WS] Socket error for ${socket.id}:`, error);
    });
  });

  // Helper function to emit to specific rack room
  io.emitToRack = (rackId, event, data) => {
    io.to(`rack-${rackId}`).emit(event, data);
  };

  // Helper function to emit to admin room
  io.emitToAdmin = (event, data) => {
    io.to('admin').emit(event, data);
  };

  // Helper function to send command to ESP32
  io.sendToEsp = (rackId, event, data) => {
    io.to(`esp-${rackId}`).emit(event, data);
  };

  // Helper function to unlock a locker via ESP32 or shelf controller
  io.unlockLocker = (rackNumber) => {
    console.log(`[WS] Sending unlock command for rack: ${rackNumber}`);
    
    // Check if rack is on a shelf controller
    if (rackToShelfMap.has(rackNumber)) {
      const shelfId = rackToShelfMap.get(rackNumber);
      console.log(`[WS] Routing to shelf controller: ${shelfId}`);
      io.to(`shelf-${shelfId}`).emit('locker:unlock', { rackId: rackNumber });
    } else {
      // Direct ESP32 connection
      io.to(`esp-${rackNumber}`).emit('locker:unlock', { rackId: rackNumber });
    }
  };

  // Helper function to lock a locker via ESP32 or shelf controller
  io.lockLocker = (rackNumber) => {
    console.log(`[WS] Sending lock command for rack: ${rackNumber}`);
    
    // Check if rack is on a shelf controller
    if (rackToShelfMap.has(rackNumber)) {
      const shelfId = rackToShelfMap.get(rackNumber);
      console.log(`[WS] Routing to shelf controller: ${shelfId}`);
      io.to(`shelf-${shelfId}`).emit('locker:lock', { rackId: rackNumber });
    } else {
      // Direct ESP32 connection
      io.to(`esp-${rackNumber}`).emit('locker:lock', { rackId: rackNumber });
    }
  };

  // Get connected ESP32 devices (both single and shelf controllers)
  io.getConnectedDevices = () => {
    const singleDevices = Array.from(connectedDevices.values());
    const shelfDevices = [];
    
    // Add all racks from shelf controllers
    connectedShelves.forEach((shelf) => {
      if (shelf.racks) {
        shelf.racks.forEach((rackId, index) => {
          shelfDevices.push({
            rackId: rackId,
            shelfId: shelf.shelfId,
            doorIndex: index,
            socketId: shelf.socketId,
            connectedAt: shelf.connectedAt,
            status: shelf.doors?.[index]?.locked ? 'locked' : 'unlocked',
            itemPresent: shelf.doors?.[index]?.item || false
          });
        });
      }
    });
    
    return [...singleDevices, ...shelfDevices];
  };

  // Get connected shelf controllers (formatted for frontend)
  io.getConnectedShelves = () => {
    const shelvesArray = [];
    
    connectedShelves.forEach((shelf) => {
      // Format racks as objects with rackId for frontend
      const formattedRacks = (shelf.racks || []).map((rackId, index) => ({
        rackId: rackId,
        doorIndex: index,
        isLocked: shelf.doors?.[index]?.locked !== false,
        itemPresent: shelf.doors?.[index]?.item || false
      }));
      
      shelvesArray.push({
        shelfId: shelf.shelfId,
        numDoors: shelf.numDoors || formattedRacks.length,
        racks: formattedRacks,
        ip: shelf.ip,
        socketId: shelf.socketId,
        connectedAt: shelf.connectedAt,
        rssi: shelf.rssi,
        uptime: shelf.uptime
      });
    });
    
    return shelvesArray;
  };

  // Check if ESP32 is connected for a rack (single or via shelf controller)
  io.isEspConnected = (rackNumber) => {
    return connectedDevices.has(rackNumber) || rackToShelfMap.has(rackNumber);
  };

  console.log('[WS] WebSocket server initialized');
  
  return io;
};

module.exports = initializeWebSocket;
