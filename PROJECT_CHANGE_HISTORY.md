# Project Change History

## CHG-001 — Navegación de salida de partida y equipos en sala

- Inicio: 2026-09-01 20:24:00 -06:00
- Entrega: 2026-09-01 20:41:24 -06:00
- Duración de implementación: 17 minutos 24 segundos
- Testeado: Sí
- Hora de prueba final: 2026-09-01 20:54:11 -06:00
- Prompt: Desde el juego, regresar a la sala en lugar del menú de elección; conservar el liderazgo; finalizar la partida si quedan menos de dos jugadores; mostrar jugadores y equipos; distribuir equipos azul/rojo; permitir cambiar de equipo desde la sala; sincronizar todo visualmente y en el servidor.
- Archivos modificados: `PROJECT_DOCUMENTATION.md`, `app/src/main/cpp/GameUI.h`, `app/src/main/cpp/Renderer.cpp`, `app/src/main/cpp/main.cpp`, `app/src/main/java/com/lewyzstudio/touchparty/GameWebSocketManager.kt`, `app/src/main/java/com/lewyzstudio/touchparty/MainActivity.kt`, `/Users/lewyz/Downloads/home/admin/game-server/game.js`, `/Users/lewyz/Downloads/home/admin/game-server/index.js`, `/Users/lewyz/Downloads/home/admin/game-server/test.js`.
- Verificación: `npm test` (12 pruebas exitosas), `node --check index.js game.js test.js`, `./gradlew assembleDebug` (BUILD SUCCESSFUL) y `git diff --check`.
- Estado: Testeado

## CHG-002 — Ajuste de UI de equipos, botón de salida y duración real de partida

- Inicio: 2026-09-01 21:06:34 -06:00
- Entrega: 2026-09-01 21:08:52 -06:00
- Duración de implementación: 2 minutos 18 segundos
- Testeado: Pendiente
- Hora de prueba final: Pendiente
- Prompt: Retirar el botón `SALIR` superior, subir los nombres, eliminar los textos de equipo, mostrar siempre el equipo local y su contador a la izquierda, identificar equipos mediante cubos, mostrar al líder igual que los demás con borde amarillo y compensar los 4 segundos de presentación en la duración de la partida si resuelve el desfase.
- Archivos modificados: `PROJECT_DOCUMENTATION.md`, `app/src/main/cpp/GameUI.h`, `/Users/lewyz/Downloads/home/admin/game-server/game.js`, `/Users/lewyz/Downloads/home/admin/game-server/test.js`.
- Verificación: `npm test` (12 pruebas exitosas), `node --check index.js game.js test.js`, `./gradlew assembleDebug` (BUILD SUCCESSFUL) y `git diff --check`.
- Estado: Pendiente de prueba

## CHG-003 — Ajuste visual del botón para cambiar de equipo

- Inicio: 2026-09-01 21:18:13 -06:00
- Entrega: 2026-09-01 21:19:07 -06:00
- Duración de implementación: 54 segundos
- Testeado: Sí
- Hora de prueba final: 2026-09-01 21:28:09 -06:00
- Prompt: Mejorar el botón `< >` para cambiar de equipo porque estaba muy arriba y agregarle un fondo adaptado a la UI.
- Archivos modificados: `app/src/main/cpp/GameUI.h`.
- Verificación: `./gradlew assembleDebug` (BUILD SUCCESSFUL) y `git diff --check`.
- Estado: Testeado

## CHG-004 — Bloqueo de multitouch durante la captura de cubos

- Inicio: 2026-09-01 21:28:09 -06:00
- Entrega: 2026-09-01 21:30:24 -06:00
- Duración de implementación: 2 minutos 15 segundos
- Testeado: Pendiente
- Hora de prueba final: Pendiente
- Prompt: Desactivar el multitouch para que ninguna pulsación simultánea cambie cubos; solo un touch individual debe capturar un cubo. No corregir todavía la secuencia de colores al tocar.
- Archivos modificados: `app/src/main/cpp/Renderer.cpp`, `app/src/main/cpp/Renderer.h`, `PROJECT_DOCUMENTATION.md`.
- Verificación: `./gradlew assembleDebug` (BUILD SUCCESSFUL) y `git diff --check`.
- Estado: Pendiente de prueba

## CHG-007 — Flecha transparente y más grande

- Inicio: 2026-09-01 22:35:17 -06:00
- Entrega: 2026-09-01 22:35:46 -06:00
- Duración de implementación: 29 segundos
- Testeado: Pendiente
- Hora de prueba final: Pendiente
- Prompt: Quitar el fondo negro para dejar únicamente la flecha transparente y aumentar un poco el tamaño de la imagen.
- Archivos modificados: `app/src/main/cpp/GameUI.h`, `PROJECT_DOCUMENTATION.md`.
- Verificación: `./gradlew assembleDebug` (BUILD SUCCESSFUL) y `git diff --check`.
- Estado: Pendiente de prueba

## CHG-008 — Alineación de la flecha azul

- Inicio: 2026-09-01 22:38:08 -06:00
- Entrega: 2026-09-01 22:38:29 -06:00
- Duración de implementación: 21 segundos
- Testeado: Pendiente
- Hora de prueba final: Pendiente
- Prompt: Ajustar la flecha azul porque la flecha roja ya se veía bien; separarla un poco del borde derecho manteniendo dirección y tamaño.
- Archivos modificados: `app/src/main/cpp/GameUI.h`.
- Verificación: `./gradlew assembleDebug` (BUILD SUCCESSFUL) y `git diff --check`.
- Estado: Pendiente de prueba

## CHG-005 — Flechas de equipo como sprite

- Inicio: 2026-09-01 22:26:00 -06:00
- Entrega: 2026-09-01 22:26:48 -06:00
- Duración de implementación: 48 segundos
- Testeado: Pendiente
- Hora de prueba final: Pendiente
- Prompt: Reemplazar las flechas de texto para cambiar de equipo por `cursor.png`, tratándolo como un sprite con cuatro flechas.
- Archivos modificados: `app/src/main/cpp/GameUI.h`, `app/src/main/cpp/Renderer.cpp`, `app/src/main/assets/cursor.png`, `PROJECT_DOCUMENTATION.md`.
- Verificación: `file app/src/main/assets/cursor.png`, `./gradlew assembleDebug` (BUILD SUCCESSFUL) y `git diff --check`.
- Estado: Pendiente de prueba

## CHG-006 — Corrección de dirección y tamaño del sprite de equipo

- Inicio: 2026-09-01 22:30:59 -06:00
- Entrega: 2026-09-01 22:31:58 -06:00
- Duración de implementación: 59 segundos
- Testeado: Pendiente
- Hora de prueba final: Pendiente
- Prompt: Corregir la dirección de las flechas del sprite porque se mostraban invertidas, subirlas ligeramente y aumentar su tamaño.
- Archivos modificados: `app/src/main/cpp/GameUI.h`.
- Verificación: `./gradlew assembleDebug` (BUILD SUCCESSFUL) y `git diff --check`.
- Estado: Pendiente de prueba

## CHG-009 — Ajuste de paneles del lobby y flecha izquierda

- Inicio: 2026-09-01 22:46:48 -06:00
- Entrega: 2026-09-01 22:47:37 -06:00
- Duración de implementación: 49 segundos
- Testeado: Pendiente
- Hora de prueba final: Pendiente
- Prompt: Ajustar la flecha del lado izquierdo; subir y ensanchar el fondo de `JUGADORES: 1/8`; ensanchar el fondo de `REQUERIDOS 2 JUGADORES` para cubrir el texto completo y mejorar el contraste entre texto y fondo.
- Archivos modificados: `PROJECT_DOCUMENTATION.md`, `PROJECT_CHANGE_HISTORY.md`, `app/src/main/cpp/GameUI.h`.
- Verificación: `./gradlew assembleDebug` (BUILD SUCCESSFUL) y `git diff --check`.
- Estado: Pendiente de prueba

## CHG-010 — Centrado vertical de la flecha de equipo

- Inicio: 2026-09-01 22:53:08 -06:00
- Entrega: 2026-09-01 22:53:29 -06:00
- Duración de implementación: 21 segundos
- Testeado: Pendiente
- Hora de prueba final: Pendiente
- Prompt: Bajar la flecha porque su ajuste vertical aún no se veía bien.
- Archivos modificados: `PROJECT_CHANGE_HISTORY.md`, `app/src/main/cpp/GameUI.h`.
- Verificación: `./gradlew assembleDebug` (BUILD SUCCESSFUL) y `git diff --check`.
- Estado: Pendiente de prueba

## CHG-011 — Descenso adicional de las flechas de equipo

- Inicio: 2026-09-01 22:55:23 -06:00
- Entrega: 2026-09-01 22:55:41 -06:00
- Duración de implementación: 18 segundos
- Testeado: Pendiente
- Hora de prueba final: Pendiente
- Prompt: Bajar un poco más la flecha izquierda y aplicar el mismo ajuste a la flecha roja.
- Archivos modificados: `PROJECT_CHANGE_HISTORY.md`, `app/src/main/cpp/GameUI.h`.
- Verificación: `./gradlew assembleDebug` (BUILD SUCCESSFUL) y `git diff --check`.
- Estado: Pendiente de prueba

## CHG-012 — Reemplazo del atlas por flechas individuales

- Inicio: 2026-09-01 23:06:59 -06:00
- Entrega: 2026-09-01 23:07:57 -06:00
- Duración de implementación: 58 segundos
- Testeado: Pendiente
- Hora de prueba final: Pendiente
- Prompt: Dejar de usar el atlas recortado y utilizar `arrow_red_left.png`, `arrow_blue_left.png`, `arrow_blue_right.png` y `arrow_red_right.png` como sprites individuales.
- Archivos modificados: `PROJECT_DOCUMENTATION.md`, `PROJECT_CHANGE_HISTORY.md`, `app/src/main/cpp/GameUI.h`, `app/src/main/cpp/Renderer.cpp`, `app/src/main/assets/arrow_red_left.png`, `app/src/main/assets/arrow_blue_left.png`, `app/src/main/assets/arrow_blue_right.png`, `app/src/main/assets/arrow_red_right.png`.
- Verificación: `file` confirmó los cuatro PNG como `82x80 RGBA`, `./gradlew assembleDebug` (BUILD SUCCESSFUL) y `git diff --check`.
- Estado: Pendiente de prueba

## CHG-013 — Elevación de las flechas individuales

- Inicio: 2026-09-01 23:09:45 -06:00
- Entrega: 2026-09-01 23:10:02 -06:00
- Duración de implementación: 17 segundos
- Testeado: Pendiente
- Hora de prueba final: Pendiente
- Prompt: Subir un poco las flechas porque las dos se veían demasiado abajo.
- Archivos modificados: `PROJECT_CHANGE_HISTORY.md`, `app/src/main/cpp/GameUI.h`.
- Verificación: `./gradlew assembleDebug` (BUILD SUCCESSFUL) y `git diff --check`.
- Estado: Pendiente de prueba

## CHG-014 — Flechas más altas y pequeñas

- Inicio: 2026-09-01 23:12:48 -06:00
- Entrega: 2026-09-01 23:13:08 -06:00
- Duración de implementación: 20 segundos
- Testeado: Pendiente
- Hora de prueba final: Pendiente
- Prompt: Subir un poco las flechas y reducir su tamaño.
- Archivos modificados: `PROJECT_CHANGE_HISTORY.md`, `app/src/main/cpp/GameUI.h`.
- Verificación: `./gradlew assembleDebug` (BUILD SUCCESSFUL) y `git diff --check`.
- Estado: Pendiente de prueba

## CHG-015 — Avance horizontal de las flechas

- Inicio: 2026-09-01 23:15:47 -06:00
- Entrega: 2026-09-01 23:19:46 -06:00
- Duración de implementación: 3 minutos 59 segundos
- Testeado: Sí
- Hora de prueba final: 2026-09-01 23:19:46 -06:00
- Prompt: Mover cada flecha un poco más hacia adelante, siguiendo la dirección a la que apunta.
- Archivos modificados: `PROJECT_CHANGE_HISTORY.md`, `app/src/main/cpp/GameUI.h`.
- Verificación: Confirmación visual del usuario mediante `Testeado`; la compilación `assembleDebug` quedó interrumpida dos veces por falta de salida del proceso, sin error de código reportado.
- Estado: Testeado

## CHG-016 — Limpieza de salas fantasma que bloquean el nombre "Lobby"

- Inicio: 2026-09-02 19:12:06 -06:00
- Entrega: 2026-09-02 19:17:20 -06:00
- Duración de implementación: 5 minutos 14 segundos
- Testeado: Pendiente
- Hora de prueba final: Pendiente
- Prompt: Se detectó que las salas no se están limpiando (p. ej. la sala por defecto "Lobby" reporta que ya existe al crearla aunque no aparece creada). Revisar la mejor opción: limpiar desde el servidor o el dispositivo; si es desde el servidor, verificar si el usuario que creó la sala (o al que se asignó) está conectado y, si no, limpiar la sala. Elegir lo más efectivo sin afectar la UI/UX.
- Archivos modificados: `PROJECT_DOCUMENTATION.md`, `/Users/lewyz/Downloads/home/admin/game-server/game.js`, `/Users/lewyz/Downloads/home/admin/game-server/index.js`, `/Users/lewyz/Downloads/home/admin/game-server/test.js`.
- Verificación: `npm test` (15 pruebas exitosas: 12 previas + 3 nuevas), `node --check index.js game.js`.
- Estado: Pendiente de prueba

## CHG-017 — Documentación de acceso SSH al VPS para agentes IA

- Inicio: 2026-09-02 19:53:14 -06:00
- Entrega: 2026-09-02 19:57:00 -06:00
- Duración de implementación: ~4 minutos
- Testeado: Pendiente
- Hora de prueba final: Pendiente
- Prompt: Dejar documentado en PROJECT_DOCUMENTATION.md el acceso SSH al servidor (admin@144.126.151.225, clave id_ed25519) para que cualquier agente IA lo pueda usar cuando sea necesario. Contexto: el usuario usa dos terminales — una local (Mac) y otra conectada por SSH al servidor. Los cambios de despliegue del servidor ya los había realizado el usuario manualmente.
- Archivos modificados: `PROJECT_DOCUMENTATION.md` (sección 5.3 con clave dedicada, sudo NOPASSWD y alias SSH `game-server`), `~/.ssh/config` (alias `game-server` creado en la Mac).
- Verificación: Despliegue completo ejecutado por un agente desde la Mac vía clave dedicada `~/.ssh/id_ed25519_touchparty`: `npm test` (15/15), `sudo cp` a `/home/deployer/supabase/docker/game-server/` (CP_OK), `docker compose stop/up --build game-server` (solo game-server recreado; supabase-db y demás servicios intactos), `/health` responde `{"status":"ok","uptime":4.2,"activeRooms":0}`. `sudo NOPASSWD: /usr/bin/docker, /usr/bin/cp` confirmado con `sudo -l`.
- Estado: Pendiente de prueba