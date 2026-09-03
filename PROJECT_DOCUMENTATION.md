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
  - Cube capture accepts only a completed single-pointer touch; any multi-touch gesture is cancelled without changing a cube.
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
│   ├── background_cubes.jpeg # Sci-fi laboratory background image texture
│   ├── arrow_red_left.png     # Red team left arrow sprite
│   ├── arrow_blue_left.png    # Blue team left arrow sprite
│   ├── arrow_blue_right.png   # Blue team right arrow sprite
│   └── arrow_red_right.png    # Red team right arrow sprite
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
  - **Equipo azul a la izquierda y equipo rojo a la derecha**: cada jugador aparece en su columna según el equipo autoritativo recibido del servidor.
  - El creador conserva el resaltado dorado `[CREADOR]`; su resaltado no impide que pueda cambiar de equipo.
  - Cada jugador puede pulsar la flecha de su propia fila para cambiar entre `BLUE` y `RED` mientras la sala está en lobby.
  - El conteo y la lista se actualizan en tiempo real; los jugadores desconectados temporalmente durante una partida no se muestran como conectados.
  - En el lobby, el panel de conteo queda elevado y ampliado; el aviso de jugadores requeridos usa un panel más ancho y contraste claro para no mezclar texto rojo con fondo rojo.
  - El botón de inicio solo está disponible para el propietario cuando hay al menos dos jugadores y no hay una partida activa.
  - El cambio de equipo usa sprites individuales (`arrow_red_left.png`, `arrow_blue_left.png`, `arrow_blue_right.png`, `arrow_red_right.png`) y conserva su transparencia y brillo originales.
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
  - **Ventana de reconexión de 60s (`DISCONNECT_RECONNECT_GRACE_MS`)**: un jugador que cae durante `PLAYING` se conserva en `disconnectedPlayers` hasta 60s para permitir la reconexión automática de su dispositivo; al expirar el lapso, `purgeExpiredDisconnected()` libera su asiento y sus cubos vuelven a neutro.
  - **Fin de partida sin salas fantasma**: `endGame()` libera el pool de desconectados justo después de emitir `game_over` (la reconexión solo aplica en `PLAYING`), de modo que una sala terminada sin jugadores conectados se destruye y su nombre (p. ej. el default `Lobby`) queda disponible de nuevo.
- **Local NickName Persistence**: Stored in `SharedPreferences` (`"user_nickname"`). Prompted ONLY on first launch. Sent to C++ via thread-safe JNI retry loop.
- **WebSocket Protocol**:
  - `list_rooms` -> `rooms_list`: Populates `serverRooms_` vector dynamically in C++.
  - `join` -> `joined` / `room_state`: Joins room, assigns stable player ID, team (`BLUE`/`RED`) and team color.
  - `set_team` -> `room_state`: Changes the requesting player's team while waiting in the lobby.
  - `return_to_room` -> `returned_to_room` / `game_aborted` / `room_state`: Returns from the active match without leaving the room; an active match with fewer than two players is aborted and routed to the room lobby.
  - `leave` / `leave_room` -> `left_room_confirmed`: Clean exit from room.
  - `start` -> `game_started`: Synchronizes game start across all connected devices in room.
  - `tap` -> `room_state`: Transmits 3D cube taps `(col, row)` and broadcasts updated 12x10 board to all players in that room in real time.
- **Reconexión Automática en Partida (`PLAYING`)**:
  - Al reestablecerse el WebSocket en `GameWebSocketManager.kt` (`onOpen`), re-envía automáticamente la solicitud `join` con `currentRoomId` y `deviceId`.
  - En el servidor (`game.js`), los jugadores desconectados se retienen en la piscina `disconnectedPlayers` para resguardar su estado en la partida activa.
  - Durante el breve lapso de desconexión, las celdas pueden pasar temporalmente a blanco neutral (`#cccccc`), pero inmediatamente al completar el handshake de reconexión, el servidor envía la ráfaga `room_state` que restaura los colores originales del tablero 12x10 y re-sincroniza las puntuaciones de todos los jugadores en pantalla.
- **Reinicio de Partidas ("OTRA VEZ")**:
  - Sincronización del estado de fin de juego (`FINISHED` / `game_over`) hacia C++ para transicionar a `GameState::MATCH_OVER`.
  - Al presionar "OTRA VEZ" desde la pantalla de resultados, el creador envía la orden `start`, reiniciando el temporizador a 94s del servidor (90s jugables después de los 4s de presentación) y el tablero a neutral para todos los integrantes en la sala.
- **Duplicate Room Name Check**: Server rejects creation if a room with the same name already exists.
- **Nickname Disambiguation**: Auto-appends `02`, `03` (e.g. `Lewyz02`) if multiple players with identical nickname join the same room.
- **Room Ownership Transfer**: If owner leaves, ownership automatically passes to the next player. If empty (`0` players), room is immediately destroyed on server (`rooms.delete(roomId)`).
- **Room Default Name**: A room created without a name uses `Lobby`.
- **Team Assignment and Ownership**: The server assigns new players to the smaller team (`BLUE` first, `RED` second), sends each player's `team` and `color`, and accepts `set_team` only while the room is in `LOBBY`. A lobby reconnection preserves the existing owner role.
- **Return From Match**: `return_to_room` keeps the player in the same room and returns its UI to `ROOM_LOBBY`. If the active match would have fewer than two players, the server changes the room to `LOBBY`, clears the match timer/board, and broadcasts `game_aborted` with the reason.
- **In-Match Player List**: `room_state.players` includes `connected: true|false`; Android marks the local player with `isLocal`, and the C++ UI displays connected players grouped by team during the match.

---

### 3.5. Puntos Pendientes / En Revisión

> [!NOTE]
> - **Flujo de Verificación de PIN para Salas Privadas**: Revisión pendiente para el diálogo emergente de PIN antes de unirse y la visibilidad esporádica de salas privadas en la pantalla `SALAS DISPONIBLES`.
> - **Transición de colores al tocar un cubo**: Pendiente de definir y corregir la secuencia de estados para que el cubo vaya directamente al color del equipo local. No modificar como parte del bloqueo de multitouch.

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
   [COUNTDOWN] ──(4 seconds)──> [PLAYING (Real-Time 3D Taps)] ──(94s server / 90s playable)──> [MATCH_OVER]
                                  │                         │
                         (Salir / return_to_room)   (return_to_room)
                                  ▼                         ▼
                         [ROOM_LOBBY] ◄────────────── [ROOM_LOBBY]
                                  │
                         (<2 activos: game_aborted)
```

---

## 5. Instrucciones Mandatorias para Agentes IA (Verificación y Despliegue)

> [!IMPORTANT]
> **REGLA OBLIGATORIA PARA TODOS LOS AGENTES DE IA**:
> 1. Al finalizar cualquier cambio o tarea en este proyecto, DEBES indicar al usuario cómo verificar los cambios realizados.
> 2. **INSTRUCCIONES DE DESPLIEGUE DEL SERVIDOR**: Las instrucciones de despliegue del VPS ÚNICAMENTE se deben incluir en el resumen final **SI Y SOLO SI se realizaron modificaciones en los archivos del servidor Node.js** (`/Users/lewyz/Downloads/home/admin/game-server/`).
> 3. Si **NO** se modificaron archivos del servidor (por ejemplo, si solo se cambió código en Android / C++ / Kotlin), **NO DEBES incluir** la Guía de Despliegue del Servidor en tu respuesta final para evitar confusiones.
> 4. La respuesta final DEBE incluir una sección breve y concreta de comprobación, con los comandos de prueba, compilación o pasos manuales necesarios según el tipo de cambio.

### 5.1. Historial obligatorio de cambios para agentes IA

Todos los agentes IA deben mantener un historial cronológico de cada cambio realizado en:

`/Users/lewyz/Documents/ProyectsEnero2026.nosync/AgostoGames/PROJECT_CHANGE_HISTORY.md`

El historial debe conservarse como un archivo Markdown separado de esta documentación técnica y debe incluir, como mínimo, una entrada por cambio con los siguientes datos:

- **ID del cambio**: identificador consecutivo o único.
- **Prompt solicitado**: solicitud original del usuario, o un resumen fiel si es demasiado extensa.
- **Hora de inicio**: momento en que el usuario solicita el cambio.
- **Hora de entrega**: momento en que el agente devuelve el resultado del cambio.
- **Duración de implementación**: tiempo transcurrido entre inicio y entrega.
- **Archivos modificados**: rutas de todos los archivos afectados, incluyendo archivos del backend si aplica.
- **Verificación realizada**: pruebas, compilación y/o validaciones ejecutadas.
- **Estado**: `Pendiente de prueba` hasta que el usuario confirme.
- **Hora de prueba final**: momento en que el usuario escribe exactamente `Testeado`.

Las horas deben registrarse en formato ISO 8601 usando la zona horaria local del proyecto (`America/Guatemala`, UTC-06:00), por ejemplo: `2026-09-01 19:48:00 -06:00`.

Cuando el usuario escriba `Testeado`, el agente debe localizar el último cambio con estado `Pendiente de prueba`, agregar la hora de esa confirmación y cambiar su estado a `Testeado`. No se debe marcar un cambio como testeado antes de recibir esa confirmación explícita.

Formato recomendado:

```markdown
## CHG-001 — Título breve del cambio

- Inicio: 2026-09-01 19:30:00 -06:00
- Entrega: 2026-09-01 19:48:00 -06:00
- Duración de implementación: 18 minutos
- Testeado: Pendiente
- Hora de prueba final: Pendiente
- Prompt: Corrección de transferencia de líder y residuos de salas.
- Archivos modificados: `game.js`, `index.js`, `test.js`
- Verificación: 9 pruebas backend y compilación Android exitosa.
- Estado: Pendiente de prueba
```

El historial debe actualizarse sin borrar entradas anteriores. Si el agente no puede escribir el archivo, debe informarlo claramente al usuario y entregar la entrada completa para copiarla manualmente.

### 5.2. Comandos abreviados para agentes IA

Las siguientes palabras son convenciones permanentes del proyecto. No son comandos nativos de Git; el agente debe interpretarlas como instrucciones completas:

- **`Rcommit`**: significa “realiza un commit del último cambio solicitado”. Antes de crear el commit, el agente debe revisar `git status` y el diff, incluir únicamente los archivos correspondientes a la última tarea y excluir cambios previos o ajenos. No debe realizar `push` al repositorio.
- **`Testeado`**: significa que el usuario validó el último cambio. El agente debe localizar la última entrada con estado `Pendiente de prueba`, registrar la hora de confirmación en `America/Guatemala` y cambiar su estado a `Testeado`.

Si el usuario necesita confirmar todos los cambios pendientes, puede escribir **`Rcommit todo`**. En ese caso, el agente debe mostrar primero el alcance de los archivos que serán incluidos y solicitar confirmación si existen cambios ajenos o ambiguos.

### 5.3. Acceso SSH al VPS para agentes IA (habilitado 2026-09-02)

> [!IMPORTANT]
> **Contexto de trabajo del usuario**: el usuario usa **dos terminales** — una terminal **local (Mac)** para subir archivos (`scp`) y verificar (`curl`), y una segunda terminal **conectada por SSH al servidor** para ejecutar pruebas y reiniciar Docker. Cuando un agente IA necesite desplegar, debe seguir este mismo esquema (comandos locales + comando remoto vía `ssh`), no reemplazar el servidor.

Datos de acceso (ya configurados y verificados el 2026-09-02):

- **Servidor**: `admin@144.126.151.225` (hostname VPS: `vmi3019967`; dominio público: `https://game.tutaxi502.com`).
- **Autenticación**: clave pública dedicada de despliegue en `/Users/lewyz/.ssh/id_ed25519_touchparty` (SIN passphrase, para uso no interactivo de agentes), ya autorizada en `/home/admin/.ssh/authorized_keys` del VPS (el servidor responde `Server accepts key`).
- **Clave dedicada (agentes)**: en `/Users/lewyz/.ssh/config` está definido el alias **`game-server`** (`HostName 144.126.151.225`, `User admin`, `IdentityFile ~/.ssh/id_ed25519_touchparty`, `IdentitiesOnly yes`). Usar siempre el alias `game-server` en `scp`/`ssh`; no requiere passphrase ni ssh-agent. La clave antigua `~/.ssh/id_ed25519` quedó con passphrase desconocida y NO debe usarse para despliegues automáticos.


- **Comprobación de conectividad** (terminal local del agente):

```bash
ssh -o BatchMode=yes -o ConnectTimeout=15 game-server 'echo SSH_OK && hostname'
# Esperado: SSH_OK y vmi3019967
```
- **Estado del servicio** (verificación rápida desde cualquier terminal):

```bash
curl -sS -m 15 https://game.tutaxi502.com/health
# Esperado: {"status":"ok","uptime":...,"activeRooms":0}
```

**Flujo de despliegue autoritativo que deben usar los agentes IA** (misma estructura que la Guía de Despliegue de abajo):

1. **Terminal local (Mac) — subir archivos**:

```bash
scp /Users/lewyz/Downloads/home/admin/game-server/{game.js,index.js,test.js,package.json} game-server:/home/admin/game-server/
```

2. **Comando remoto (un solo `ssh`) — pruebas + copia + reinicio SOLO de game-server** (no detiene los demás servicios):

```bash
ssh game-server 'cd /home/admin/game-server && npm test && sudo cp /home/admin/game-server/{game.js,index.js,test.js,package.json} /home/deployer/supabase/docker/game-server/ && sudo docker compose -f /home/deployer/supabase/docker/docker-compose.yml stop game-server && sudo docker compose -f /home/deployer/supabase/docker/docker-compose.yml up -d --build game-server'
```

3. **Terminal local — verificación**:

```bash
curl https://game.tutaxi502.com/health
```

> [!NOTE]
> **`sudo` sin contraseña ya configurado (2026-09-02)**: en `/etc/sudoers` del VPS está autorizado `admin ALL=(root) NOPASSWD: /usr/bin/docker, /usr/bin/cp`. Por ello el comando remoto del paso 2 corre sin pedir contraseña en sesión no interactiva. Si en el futuro se restringe, revisar `sudo -l | grep -A3 "User admin"`.

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