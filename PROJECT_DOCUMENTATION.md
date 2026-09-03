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
│   ├── GameUI.h              # 2D Orthographic UI state coordinator (state container & navigation router)
│   ├── GameUIStructs.h       # Shared UI data structures (ServerRoomEntry, PlayerInfo)
│   ├── UITheme.h             # Centralized UI Theme & Color Palette Manager (Light/Dark themes, ColorRGBA)
│   ├── UIButton.h            # Reusable C++ UI Button & Banner Component (rendering, borders, auto-fitting, touch hitbox)
│   ├── UICard.h              # Reusable C++ UI Container Card & Panel Component (borders, background, titles, underlines)
│   ├── UIBanner.h            # Reusable Top/Bottom Banners (Server status, NickName badge, notifications)
│   ├── UIDrawHelpers.h       # Primitive 2D rendering helpers (quads, textures, icons, digits, back button)
│   ├── UISetupScreen.h       # Composable Screen: First-run setup & language initialization
│   ├── UIWelcomeScreen.h     # Composable Screen: Main menu screen (Title card, Search, Create room, Test 1v1, Settings)
│   ├── UIRoomListScreen.h    # Composable Screen: Available rooms screen (Filters, scrollable rows, Join/Refresh)
│   ├── UICreateRoomScreen.h   # Composable Screen: Create room screen (Name, privacy toggle, PIN field)
│   ├── UIRoomLobbyScreen.h   # Composable Screen: Expanded Room Lobby screen (Blue/Red team columns, player rows, arrows)
│   ├── UILanguageScreen.h    # Composable Screen: Modal language selection panel (Flag buttons ES/EN)
│   ├── UIInGameOverlay.h     # Composable Screen: HUD overlay (Match timer, score panels, winner banner, leave confirm modal)
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

### 3.3. User Interface & Overlay Architecture (Arquitectura Modular Estilo Jetpack Compose)
- **Coordinador de UI (`GameUI.h`)**: Contenedor principal de estado (State Container) que gestiona temporizadores, transiciones de estado y callbacks JNI/WebSocket, delegando el renderizado a clases de pantalla independientes.
- **Estructuras Compartidas (`GameUIStructs.h`)**: Define las estructuras de datos `ServerRoomEntry` y `PlayerInfo` utilizadas a lo largo de los componentes de la UI.
- **Sistema Centralizado de Temas (`UITheme.h`)**:
  - Encapsula toda la paleta de colores (`ColorRGBA`) con soporte para temas `LIGHT` y `DARK`.
  - **Prohibición de valores RGB hardcodeados**: Todos los colores de la interfaz deben declararse en `UITheme.h` (`BADGE_*`, `TEAM_ROW_*`, `CARD_*`, `BTN_SUCCESS_*`, `BTN_PRIMARY_*`, `BTN_TEST_*`, `SERVER_*`).
- **Componentes Primitivos Reutilizables**:
  - **Botones y Banners (`UIButton.h`)**: Estandariza el renderizado de botones y banners interactivos con `UIButtonSpec`, bordes de acento y tipografía auto-ajustada (`FontRenderer`). Propiedad `isClickable = false` para elementos estáticos.
  - **Tarjetas Contenedoras (`UICard.h`)**: Paneles contenedores con bordes, color de fondo y título de encabezado con subrayado decorativo (`UICardSpec`).
  - **Banners de Estado (`UIBanner.h`)**: Banners de conexión del servidor, Badge de NickName en la esquina superior derecha y notificaciones emergentes de ingreso/salida de jugadores.
  - **Ayudantes de Dibujo (`UIDrawHelpers.h`)**: Primitivas de renderizado de bajo nivel (`drawQuad`, `drawIcon`, `drawCursorSprite`, `drawTextFitted`, `renderBackButton`).
- **Clases de Pantalla Independientes (Composables)**:
  - **Pantalla Principal (`UIWelcomeScreen.h`)**: Menú principal con tarjeta de título `"TOUCHPARTY CUBE ARENA"`, botón `"TEST 1V1"`, engranaje de configuración e idioma y botones `"BUSCAR SALAS"` y `"CREAR SALA"`.
  - **Pantalla de Salas Disponibles (`UIRoomListScreen.h`)**: Muestra la lista de salas filtrables (`BUSCAR: TODAS` / `PUBLICAS` / `PRIVADAS`), scroll con botones `UP ^` / `DWN v`, y botones de acción `[UNIRSE]`, `[PIN]` y `[ACTUALIZAR]`.
  - **Pantalla de Crear Sala (`UICreateRoomScreen.h`)**: Formulario para crear salas públicas o privadas, nombre de sala, toggle de privacidad, PIN de acceso y botón `"CREAR SALA"`.
  - **Pantalla de Lobby de Sala (`UIRoomLobbyScreen.h`)**: Vista expandida (+50% de escala) con columnas separadas para el equipo Azul y Rojo, filas de jugadores con resaltado de propietario, cursores de cambio de equipo (`arrow_*`) y botón de inicio.
  - **Panel de Selección de Idioma (`UILanguageScreen.h`)**: Diálogo modal con banderas de España e Inglés (EE.UU.) para cambio inmediato de idioma.
  - **Overlays y HUD de Partida (`UIInGameOverlay.h`)**: Marcadores superiores, temporizador (`01:30`), cuadro de jugadores en partida, banner de victoria/derrota, modal de confirmación de salida (`"¿DESEAS SALIR DE LA SALA?"`) y conteo regresivo (`3, 2, 1, GO!`).
  - **Pantalla de Primer Arranque (`UISetupScreen.h`)**: Pantalla inicial de bienvenida e ingreso de nickname.

### 3.4. Networking & Real-Time Sync (`GameWebSocketManager.kt`, `MainActivity.kt`, `index.js`)
- **Server Health Check**: Background HTTP GET to `${BuildConfig.GAME_SERVER_HTTP_URL}/health` every 5s.
- **Unique Device Identification (`deviceId`)**: UUID persistido en `SharedPreferences` que previene duplicación de usuarios (`Lewyz1` / `LEWYZ1`) al reconectar.
- **Active Server Heartbeat & Ghost Room Audit (`cleanGhostRooms`)**:
  - Ping/pong activo cada 5s en Node.js.
  - Detecta sockets muertos/cerrados, remueve jugadores desconectados, reasigna `ownerId` al primer integrante activo o destruye salas vacías.
  - **Ventana de reconexión de 60s (`DISCONNECT_RECONNECT_GRACE_MS`)**: un jugador que cae durante `PLAYING` se conserva en `disconnectedPlayers` hasta 60s para permitir la reconexión automática de su dispositivo; al expirar el lapso, `purgeExpiredDisconnected()` libera su asiento y sus cubos vuelven a neutro.
  - **Fin de partida sin salas fantasma**: `endGame()` libera el pool de desconectados justo después de emitir `game_over` (la reconexión solo aplica en `PLAYING`), de modo que una sala terminada sin jugadores conectados se destruye y su nombre (p. ej. el default `Lobby`) queda disponible de nuevo.
- **Local NickName Persistence**: Stored in `SharedPreferences` (`"user_nickname"`). Prompted ONLY on first launch. Sent to C++ via thread-safe JNI retry loop.
- **Multilingüe EN/ES (CHG-018)**: el juego soporta Español e Inglés.
  - **Detección del idioma del sistema**: `MainActivity` lee `Locale.getDefault()` (es → español; cualquier otro → inglés) y lo envía a C++ vía `nativeSetLanguage` antes de renderizar.
  - **Persistencia local**: `SharedPreferences` clave `"app_language"` (`"es"`/`"en"`), guardada al confirmar el NickName en la pantalla de primer arranque y al cambiar idioma desde el engranaje.
  - **Pantalla de primer arranque (`GameState::SETUP`)**: muestra título, banderas de idioma (ES `flag_es.png`, EN `flag_us.png`), campo de NickName (teclado nativo) y botón `GUARDAR Y CONTINUAR`. Solo aparece si no hay nickname guardado.
  - **Selector desde el menú (engranaje)**: icono `gear_icon.png` debajo del botón `TEST 1V1` en la pantalla principal abre el panel `renderLanguagePanel`; al elegir bandera el idioma cambia al instante (los textos se leen en cada frame vía `Strings::get`) y se persiste.
  - **Catálogo centralizado**: `Strings.h` enumera todos los StringId con pares ES/EN; los textos hardcodeados de `GameUI.h` se migraron a `Strings::get`. Los diálogos nativos de Android usan `res/values/strings.xml` (EN) y `res/values-es/strings.xml` (ES).
  - **Fuente con tildes**: se reemplazó `calculator.ttf` por **Press Start 2P** (`press_start_2p.ttf`, licencia SIL OFL 1.1 en `assets/OFL-press_start_2p.txt`), que rasteriza correctamente los acentos españoles (Á É Í Ó Ú Ü Ñ á é í ó ú ü ñ ¡ ¿). `FontRenderer` ya soporta UTF-8/Latin-1; el fallback sin tildes ya no se activa.
  - **UI profesional (auto-ajuste)**: el font píxel Press Start 2P es más ancho que el anterior, por lo que se añadió `drawTextFitted` que mide el ancho del texto y reduce el tamaño para que **nunca desborde su contenedor**. Se rediseñaron las pantallas (menú, crear sala, salas disponibles, lobby, setup, panel de idioma, modal de salir, winner overlay, banners) con tamaños coherentes, separación vertical, subrayados de acento y sin colisiones de texto.
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
> 5. **REGLAS MANDATORIAS PARA DESARROLLO DE UI Y COMPONENTES EN C++**:
>    - **Gestor de Colores (`UITheme.h`)**: Todos los agentes de IA DEBEN usar `UITheme.h` para obtener colores (`ColorRGBA`). Queda estrictamente prohibido hardcodear valores RGB (`1.0f, 0.5f, ...`) en `GameUI.h`.
>    - **Componente Reutilizable de Botones (`UIButton.h`)**: Todo botón o banner estático DEBE renderizarse usando `UIButtonSpec` y la función `UIButton::render()`. Para componentes estáticos sin respuesta táctil, establecer `isClickable = false`.
>    - **Prohibido el texto recortado con puntos suspensivos ("...")**: NUNCA permitir que títulos o textos de botones se muestren truncados (`"TE..."`, `"SEAR..."`, `"CREA..."`). Si un texto se trunca, el agente DEBE ampliar el contenedor (`cardW`, `btnW`, `w`) o ajustar el tamaño de fuente (`fontSize`).
>    - **Ajuste de Tipografía y Márgenes**: Asegurar que las tarjetas y badges tengan altura (`h`) y holgura suficiente para que los textos queden verticalmente centrados sin cortar los bordes superior o inferior.

### 5.1. Historial obligatorio de cambios por día para agentes IA

Todos los agentes IA deben registrar y mantener el historial cronológico de cada cambio organizado **por archivos diarios independientes** dentro del directorio `history/`:

- **Formato y Ruta de archivo diario**: `/Users/lewyz/Documents/ProyectsEnero2026.nosync/AgostoGames/history/MES_DIA_DD_AÑO.md` (por ejemplo: `history/SEPTIEMBRE_MARTES_01_2026.md`, `history/SEPTIEMBRE_MIERCOLES_02_2026.md`, `history/SEPTIEMBRE_JUEVES_03_2026.md`).

Cada entrada en el archivo diario del día correspondiente debe incluir los siguientes datos por cambio:

- **ID del cambio**: identificador consecutivo (ej. `CHG-001`, `CHG-029`).
- **Prompt solicitado**: solicitud original del usuario.
- **Hora de inicio**: momento en que el usuario solicita el cambio.
- **Hora de entrega**: momento en que el agente entrega la implementación.
- **Duración de implementación**: tiempo transcurrido entre inicio y entrega.
- **Archivos modificados**: rutas de todos los archivos afectados.
- **Verificación realizada**: pruebas y compilación ejecutadas.
- **Estado**: `Pendiente de prueba` hasta que el usuario confirme con `Testeado`.
- **Hora de prueba final**: momento en que el usuario responde `Testeado`.

> [!IMPORTANT]
> **ZONA HORARIA Y MEDICIÓN DE TIEMPO Y PRODUCTIVIDAD ("Testeado")**:
> 1. **Zona Horaria Oficial**: Todos los registros de hora (`Inicio`, `Entrega`, `Hora de prueba final`) DEBEN registrarse siempre en la **zona horaria de Guatemala** (`America/Guatemala`, UTC-06:00), por ejemplo: `2026-09-03 02:31:00 -06:00`.
> 2. **Confirmación con `Testeado`**: Cuando el usuario responde **`Testeado`**, significa que el cambio fue verificado y **solucionado exitosamente**. La duración total del ciclo de un cambio/tarea abarca desde la hora de inicio de la solicitud hasta la hora en que se recibe la respuesta `Testeado`.
> 3. Al recibir `Testeado`, el agente debe actualizar la hora de prueba final y marcar el estado como `Testeado` en el archivo diario del día correspondiente (`history/MES_DIA_DD_AÑO.md`).

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