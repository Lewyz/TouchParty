# Project Documentation - Touchparty 3D Game Engine

Comprehensive technical documentation for the **Touchparty** Android 3D Game Engine built with C++20, OpenGL ES 3.0, GameActivity, and OkHttp WebSocket real-time multiplayer networking.

---

## 1. Tech Stack & Platform Overview

- **Platform**: Android Native (`GameActivity` AGDK + C++20 / Android NDK)
- **Graphics API**: OpenGL ES 3.0
- **Audio System**:
  - AAudio NDK (Real-time procedural PCM waveform synthesis)
  - Android `MediaPlayer` via JNI (Resource audio playback)
- **Input & Haptics**:
  - `GameActivity` motion event polling
  - Android NDK JNI bridge to `Vibrator` / `VibratorManager`
- **Networking & Real-Time Sync**:
  - **OkHttp 4.12.0 WebSocket** client (`wss://game.tutaxi502.com`)
  - **Node.js ES Module Backend** (`/Users/lewyz/Downloads/home/admin/game-server/`)
  - **JNI Thread-Safe Bridge** between Kotlin and C++ engine
- **Window & Layout**: Immersive Sticky Fullscreen Landscape (`NoActionBar` theme, display cutout support)

---

## 2. Architecture & File Structure

```
app/src/main/
├── cpp/
│   ├── main.cpp              # GameActivity entry point, JNI exports & ALooper event loop
│   ├── Renderer.h / .cpp     # Render pipeline, camera matrices, EGL context, 3D raycast & WS triggers
│   ├── MatrixMath.h          # Column-major 4x4 matrix math & 3D unprojection raycast algorithms
│   ├── CubeGrid.h            # 12x10 3D cube grid (120 cubes), 3D floor platform, animations & real-time sync
│   ├── ParticleSystem.h      # Board-relative expanding shockwave particle system
│   ├── GameUI.h              # 2D Orthographic UI (NickName badge, server status, room list, confirmation modal)
│   ├── AudioEngine.h         # Procedural AAudio PCM synth & JNI MP3 trigger
│   ├── VibrationEngine.h     # JNI bridge for device haptic feedback
│   ├── Shader.h / .cpp       # GLES 3.0 shader program management (MVP, Color, Texture)
│   ├── TextureAsset.h / .cpp # Asset loader using Android AImageDecoder
│   ├── Model.h / Utility.h   # Vertex structures and EGL/GL error helpers
│   └── CMakeLists.txt        # Native C++ build script (links EGL, GLESv3, AAudio, GameActivity)
├── java/com/lewyzstudio/touchparty/
│   ├── MainActivity.kt       # GameActivity subclass, health check loop, nickname persistence & JNI bridges
│   └── GameWebSocketManager.kt # OkHttp WebSocket client, JSON message parser & real-time event router
├── assets/
│   └── background_cubes.jpeg # Sci-fi laboratory background image texture
└── res/raw/
    └── countdown_party.mp3   # Countdown audio track (3, 2, 1, GO!)

Servidor Node.js Backend:
/Users/lewyz/Downloads/home/admin/game-server/
├── index.js                  # WebSocket server, HTTP /health endpoint, message router
├── game.js                   # Room class, Player class, 12x10 board state & game logic
├── package.json              # Node.js ES module package definition
└── test.js                   # Unit tests for room lifecycle & game mechanics
```

---

## 3. Core Engine Systems

### 3.1. 3D Camera & Matrix Math (`MatrixMath.h`, `Renderer.cpp`)
- **Projection**: Perspective camera with adaptive FOV based on screen aspect ratio (`width / height`).
- **View Matrix**: LookAt camera positioned at `Vec3(0.0, 11.5, 10.5)` looking down at board center `Vec3(0.0, -0.4, 0.0)`.
- **Unprojection / Raycasting**: Converts 2D screen touch $(x, y)$ in pixels to normalized device coordinates (NDC) $[-1, 1]$, applying inverse View-Projection matrix $(P \times V)^{-1}$ to generate a 3D Ray.

### 3.2. 12x10 Cube Grid & Platform (`CubeGrid.h`)
- **Dimensions**: 12 columns $\times$ 10 rows = 120 cubes.
- **Base Platform**: 3D metallic tech floor rendered underneath the grid.
- **State Machine**:
  - `CUBE_STATE_WHITE` (0): Base neutral state.
  - `CUBE_STATE_BLUE` (1): Blue state (+1 Blue score).
  - `CUBE_STATE_RED` (2): Red state (-1 Blue, +1 Red score).
- **Real-Time Synchronized State**: `setCubeState(col, row, newState)` updates cell state instantly upon receiving WebSocket `room_state` board updates from Node.js server.
- **Rotation-Aware Raycasting**: 100% picking precision at any rotation angle during half-time board rotation.

### 3.3. User Interface & Overlay (`GameUI.h`)
- **Server Status Banner**: Displays `SERVIDOR: CONECTADO` (green) or `SERVIDOR: DESCONECTADO (MODO LOCAL)` (red).
- **Top-Right Registered NickName Badge**: Rendered in a crisp white background box (`1.0, 1.0, 1.0, 1.0`) with dark text and blue accent border. Non-editable once saved.
- **Redesigned Room List (`renderRoomListScreen`)**:
  - Title: `SALAS DISPONIBLES`.
  - Filter Selector: `BUSCAR: TODAS` / `PUBLICAS` / `PRIVADAS` (sin acentos para compatibilidad de fuente).
  - Displays visible room rows with player count (e.g. `room-name (1/8)`).
  - **Salas Públicas**: Texto en cian brillante (`#33E6FF`) con botón `[UNIRSE]`.
  - **Salas Privadas**: Texto en naranja brillante (`#FF8C1A`) con botón `[PIN]`.
  - Deduplicación estricta por `entry.id` para evitar filas duplicadas.
- **Redesigned Expanded Lobby Screen (`renderRoomLobbyScreen`)**:
  - Escala ampliada (+50%) abarcando la pantalla landscape (`cardW = 2.65f`, `cardH = 1.25f`).
  - **Columna Izquierda**: Estado de la sala, conteo `JUGADORES: X/8` y botón `INICIAR PARTIDA (X/8)` para el Creador o `ESPERANDO AL CREADOR...` para integrantes.
  - **Columna Derecha (`INTEGRANTES DE LA SALA`)**: Lista en tiempo real de jugadores conectados. Creador destacado en dorado `[CREADOR] NickName` y miembros regulares en cian.
- **Confirmation Exit Modal (`renderLeaveConfirmModal`)**:
  - Displays `"¿DESEAS SALIR DE LA SALA?"` modal card when tapping "SALIR DE SALA" or "VOLVER" in lobby.
  - `[SÍ, SALIR]` sends WebSocket `leave` to server, destroys room if empty, and returns to main menu.
  - `[CANCELAR]` closes modal.

### 3.4. Networking & Real-Time Sync (`GameWebSocketManager.kt`, `MainActivity.kt`, `index.js`)
- **Server Health Check**: Background HTTP GET to `${BuildConfig.GAME_SERVER_HTTP_URL}/health` every 5s.
- **Unique Device Identification (`deviceId`)**: UUID persistido en `SharedPreferences` que previene duplicación de usuarios (`Lewyz1` / `LEWYZ1`) al reconectar.
- **Active Server Heartbeat & Ghost Room Audit (`cleanGhostRooms`)**:
  - Ping/pong activo cada 5s en Node.js.
  - Detecta sockets muertos/cerrados, remueve jugadores desconectados, reasigna `ownerId` al primer integrante activo o destruye salas vacías.
- **Local NickName Persistence**: Stored in `SharedPreferences` (`"user_nickname"`). Prompted ONLY on first launch. Sent to C++ via thread-safe JNI retry loop.
- **WebSocket Protocol**:
  - `list_rooms` -> `rooms_list`: Populates `serverRooms_` vector dynamically in C++.
  - `join` -> `joined` / `room_state`: Joins room, assigns stable player ID and unique color.
  - `leave` / `leave_room` -> `left_room_confirmed`: Clean exit from room.
  - `start` -> `game_started`: Synchronizes game start across all connected devices in room.
  - `tap` -> `room_state`: Transmits 3D cube taps `(col, row)` and broadcasts updated 12x10 board to all players in that room in real time.
- **Duplicate Room Name Check**: Server rejects creation if a room with the same name already exists.
- **Nickname Disambiguation**: Auto-appends `02`, `03` (e.g. `Lewyz02`) if multiple players with identical nickname join the same room.
- **Room Ownership Transfer**: If owner leaves, ownership automatically passes to the next player. If empty (`0` players), room is immediately destroyed on server (`rooms.delete(roomId)`).

---

### 3.5. Puntos Pendientes / En Revisión

> [!NOTE]
> - **Flujo de Verificación de PIN para Salas Privadas**: Revisión pendiente para el diálogo emergente de PIN antes de unirse y la visibilidad esporádica de salas privadas en la pantalla `SALAS DISPONIBLES`.

---

## 4. Game States & Flow

```
[WELCOME / MENU] ──(Buscar / Crear)──> [ROOM_LIST / CREATE_ROOM]
        │                                       │
        ▼                                       ▼
  [ROOM_LOBBY] ◄──────────────(Join)────────────┘
        │
   (Owner Starts)
        ▼
   [COUNTDOWN] ──(4 seconds)──> [PLAYING (Real-Time 3D Taps)] ──(30s)──> [MATCH_OVER]
```

---

## 5. Instrucciones Mandatorias para Agentes IA (Verificación y Despliegue)

> [!IMPORTANT]
> **REGLA OBLIGATORIA PARA TODOS LOS AGENTES DE IA**:
> 1. Al finalizar cualquier cambio o tarea en este proyecto, DEBES indicar al usuario cómo verificar los cambios realizados.
> 2. **INSTRUCCIONES DE DESPLIEGUE DEL SERVIDOR**: Las instrucciones de despliegue del VPS ÚNICAMENTE se deben incluir en el resumen final **SI Y SOLO SI se realizaron modificaciones en los archivos del servidor Node.js** (`/Users/lewyz/Downloads/home/admin/game-server/`).
> 3. Si **NO** se modificaron archivos del servidor (por ejemplo, si solo se cambió código en Android / C++ / Kotlin), **NO DEBES incluir** la Guía de Despliegue del Servidor en tu respuesta final para evitar confusiones.

### Guía de Despliegue del Servidor (Incluir SOLO cuando se modifique el servidor Node.js):

#### 1. En la Terminal de tu Mac (Subir archivos actualizados al VPS)
```bash
scp /Users/lewyz/Downloads/home/admin/game-server/{game.js,index.js,test.js,package.json} admin@144.126.151.225:/home/admin/game-server/
```

#### 2. En el VPS (SSH, ejecutar tests y reiniciar Docker)
Conéctate por SSH al servidor:
```bash
ssh admin@144.126.151.225
```
Y ejecuta este bloque completo para correr las pruebas unitarias, copiar los archivos a la carpeta de despliegue y reiniciar el contenedor:
```bash
# 1. Ejecutar pruebas unitarias para validar los cambios
cd /home/admin/game-server && npm test

# 2. Copiar archivos a la carpeta de despliegue
sudo cp /home/admin/game-server/{game.js,index.js,test.js,package.json} /home/deployer/supabase/docker/game-server/

# 3. Detener ÚNICAMENTE el contenedor game-server (sin apagar los demás servicios)
sudo docker compose -f /home/deployer/supabase/docker/docker-compose.yml stop game-server

# 4. Reconstruir la imagen e iniciar SOLAMENTE el servicio game-server
sudo docker compose -f /home/deployer/supabase/docker/docker-compose.yml up -d --build game-server
```

#### 3. Verificación Inmediata
Verifica que el servidor esté activo y respondiendo correctamente:
```bash
curl https://game.tutaxi502.com/health
```
