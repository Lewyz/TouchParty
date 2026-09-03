# Historial de Cambios Diarios — Jueves 03 de Septiembre de 2026

## CHG-019 — Reemplazo de fuente por Press Start 2P con soporte de tildes
- Inicio: 2026-09-02 23:55:00 -06:00
- Entrega: 2026-09-03 00:00:36 -06:00
- Duración de implementación: ~6 minutos
- Testeado: Sí
- Hora de prueba final: 2026-09-03 00:10:00 -06:00
- Prompt: El font actual no tiene tildes. Buscar un nuevo font con soporte completo de acentos.
- Archivos modificados: `press_start_2p.ttf`, `OFL-press_start_2p.txt`, `Renderer.cpp`, `GameUI.h`.
- Estado: Testeado

## CHG-020 — UI/UX profesional: auto-ajuste y rediseño de pantallas
- Inicio: 2026-09-03 00:05:00 -06:00
- Entrega: 2026-09-03 00:15:00 -06:00
- Duración de implementación: ~10 minutos
- Testeado: Sí
- Hora de prueba final: 2026-09-03 00:15:00 -06:00
- Prompt: Auto-ajuste de textos para evitar que la fuente desborde o se solape.
- Archivos modificados: `GameUI.h`.
- Estado: Testeado

## CHG-021 — Fix crash BadTokenException y mejora de contraste en Crear Sala
- Inicio: 2026-09-03 00:16:00 -06:00
- Entrega: 2026-09-03 00:24:00 -06:00
- Duración de implementación: ~8 minutos
- Testeado: Sí
- Hora de prueba final: 2026-09-03 00:25:00 -06:00
- Prompt: Fix de Fatal Exception BadTokenException en AlertDialog y mejora de contraste.
- Archivos modificados: `MainActivity.kt`, `GameUI.h`.
- Estado: Testeado

## CHG-022 — Agrandar card/botones y hacer legibles campos editables y nickname
- Inicio: 2026-09-03 00:26:00 -06:00
- Entrega: 2026-09-03 00:32:00 -06:00
- Duración de implementación: ~6 minutos
- Testeado: Sí
- Hora de prueba final: 2026-09-03 00:35:00 -06:00
- Prompt: Agrandar card y botones; hacer legible el campo de texto y el badge de nickname.
- Archivos modificados: `GameUI.h`.
- Estado: Testeado

## CHG-023 — Sincronizar zonas de toque con el nuevo layout y truncar textos largos
- Inicio: 2026-09-03 00:36:00 -06:00
- Entrega: 2026-09-03 00:44:00 -06:00
- Duración de implementación: ~8 minutos
- Testeado: Sí
- Hora de prueba final: 2026-09-03 00:44:00 -06:00
- Prompt: Sincronizar zonas de toque e hitpad en Crear Sala.
- Archivos modificados: `GameUI.h`.
- Estado: Testeado

## CHG-024 — Arreglar truncado de textos (menú), nickname y colores del lobby
- Inicio: 2026-09-03 00:45:00 -06:00
- Entrega: 2026-09-03 00:56:00 -06:00
- Duración de implementación: ~11 minutos
- Testeado: Sí
- Hora de prueba final: 2026-09-03 00:57:00 -06:00
- Prompt: Corregir truncado en el menú principal y colores del lobby.
- Archivos modificados: `GameUI.h`.
- Estado: Testeado

## CHG-025 — Mostrar nickname del jugador local en la fila del lobby
- Inicio: 2026-09-03 00:58:00 -06:00
- Entrega: 2026-09-03 01:04:00 -06:00
- Duración de implementación: ~6 minutos
- Testeado: Sí
- Hora de prueba final: 2026-09-03 01:05:00 -06:00
- Prompt: Mostrar nickname del jugador local en la fila del equipo en el lobby.
- Archivos modificados: `GameUI.h`, `MainActivity.kt`.
- Estado: Testeado

## CHG-026 — Refactorización de Arquitectura UI Modular Estilo Jetpack Compose (C++)
- Inicio: 2026-09-03 01:10:00 -06:00
- Entrega: 2026-09-03 01:30:00 -06:00
- Duración de implementación: 20 minutos
- Testeado: Sí
- Hora de prueba final: 2026-09-03 01:35:00 -06:00
- Prompt: Refactorización modular de GameUI.h en componentes primitivos y clases de pantalla.
- Archivos modificados: `GameUI.h`, `GameUIStructs.h`, `UIButton.h`, `UICard.h`, `UIBanner.h`, `UIDrawHelpers.h`, `UISetupScreen.h`, `UIWelcomeScreen.h`, `UIRoomListScreen.h`, `UICreateRoomScreen.h`, `UIRoomLobbyScreen.h`, `UILanguageScreen.h`, `UIInGameOverlay.h`.
- Estado: Testeado

## CHG-027 — Margen en botón Back, NickName en amarillo y relayout de Crear Sala
- Inicio: 2026-09-03 01:40:00 -06:00
- Entrega: 2026-09-03 01:52:00 -06:00
- Duración de implementación: 12 minutos
- Testeado: Sí
- Hora de prueba final: 2026-09-03 01:55:00 -06:00
- Prompt: Margen en botón Back, NickName en texto amarillo y ocultamiento de PIN en sala pública.
- Archivos modificados: `UIDrawHelpers.h`, `UIBanner.h`, `UICreateRoomScreen.h`, `GameUI.h`.
- Estado: Testeado

## CHG-028 — Renderizado del nombre de sala e input field con marco
- Inicio: 2026-09-03 01:56:00 -06:00
- Entrega: 2026-09-03 02:02:00 -06:00
- Duración de implementación: 6 minutos
- Testeado: Sí
- Hora de prueba final: 2026-09-03 02:05:00 -06:00
- Prompt: Corregir visibilidad del nombre de sala dentro de la caja editable.
- Archivos modificados: `UICreateRoomScreen.h`, `GameUI.h`.
- Estado: Testeado

## CHG-029 — Nombre de sala dinámico (NickName_Lobby), renderizado de texto puro y botón EDITAR
- Inicio: 2026-09-03 02:10:00 -06:00
- Entrega: 2026-09-03 02:22:00 -06:00
- Duración de implementación: 12 minutos
- Testeado: Sí
- Hora de prueba final: 2026-09-03 02:25:00 -06:00
- Prompt: Mostrar el nombre de la sala directamente como texto puro para evitar problemas de renderizado en dispositivos físicos; colocar a la derecha el botón [EDITAR] y cambiar el nombre por defecto a `NickName_Lobby`.
- Archivos modificados: `UICreateRoomScreen.h`, `GameUI.h`, `history/SEPTIEMBRE_JUEVES_03_2026.md`.
- Estado: Testeado

## CHG-030 — Normalización MAYÚSCULAS/sin tildes, refresco a 1s de inicio y espacio para LEWYZ1_LOBBY
- Inicio: 2026-09-03 02:26:00 -06:00
- Entrega: 2026-09-03 02:30:00 -06:00
- Duración de implementación: 4 minutos
- Testeado: Sí
- Hora de prueba final: 2026-09-03 02:31:00 -06:00
- Prompt: Normalizar automáticamente a MAYÚSCULAS y remover acentos/tildes en Nicknames y nombres de sala tanto en C++ como en Kotlin (ej. `ì` -> `I`, `wywy` -> `WYWY`). Programar resincronización a 1s del inicio en `MainActivity.kt`. Ajustar fuente a `0.092f` y punto de origen a `-0.28f` (`maxTextW = 1.35f`) para que `LEWYZ1_LOBBY` quepa completo sin recortarse a `LE1_LOBB`.
- Archivos modificados: `MainActivity.kt`, `UICreateRoomScreen.h`, `GameUI.h`, `history/SEPTIEMBRE_JUEVES_03_2026.md`.
- Verificación: Compilación Gradle `:app:assembleDebug` exitosa para las 4 ABIs y confirmación visual en celular físico enviada por el usuario (`Testeado`).
- Estado: Testeado

## CHG-031 — Rediseño del Buscador de Salas (SERVER BROWSER), Búsqueda Local, Indicador [FULL] en rojo y Desplazamiento Táctil
- Inicio: 2026-09-03 02:35:00 -06:00
- Entrega: 2026-09-03 02:48:00 -06:00
- Duración de implementación: ~13 minutos
- Testeado: Sí
- Hora de prueba final: 2026-09-03 03:15:00 -06:00
- Prompt: Rediseño del Buscador de Salas (`SERVER BROWSER`), sub-encabezado `LOBBIES`, barra de búsqueda local por texto (`BUSCAR | ...`), selector de filtro `[ALL, PUBLIC, PRIVATE]`, indicador `[FULL]` en rojo para salas llenas (8/8) y clase dedicada `UIScrollContainer.h` para desplazamiento táctil por arrastre y barra de scroll.
- Archivos modificados: `UIScrollContainer.h`, `Strings.h`, `UIRoomListScreen.h`, `GameUI.h`, `Renderer.cpp`.
- Verificación: Compilación nativa Gradle `:app:assembleDebug` exitosa para las 4 ABIs y confirmación visual recibida del usuario (`Testeado`).
- Estado: Testeado

## CHG-032 — Ajuste de layout: centrado de LOBBIES, elevación de barra de búsqueda y respuesta sobre la barra de scroll
- Inicio: 2026-09-03 03:00:00 -06:00
- Entrega: 2026-09-03 03:08:00 -06:00
- Duración de implementación: ~8 minutos
- Testeado: Sí
- Hora de prueba final: 2026-09-03 03:15:00 -06:00
- Prompt: Centrar el texto `LOBBIES` horizontalmente en el panel, subir la barra de búsqueda `SEARCH | ...` y el filtro `[ALL, PUBLIC, PRIVATE]` lo más alto posible en el panel con margen respecto al borde superior para evitar solapamientos, y responder sobre la visibilidad dinámica de la barra de scroll.
- Archivos modificados: `UIRoomListScreen.h`, `GameUI.h`.
- Verificación: Compilación Gradle `:app:assembleDebug` exitosa para las 4 ABIs y confirmación visual recibida del usuario (`Testeado`).
- Estado: Testeado

## CHG-033 — Elevar banner SERVER CONNECTED y agregar botón [X] para limpiar búsqueda
- Inicio: 2026-09-03 03:16:00 -06:00
- Entrega: 2026-09-03 03:24:00 -06:00
- Duración de implementación: ~8 minutos
- Testeado: Sí
- Hora de prueba final: 2026-09-03 03:30:00 -06:00
- Prompt: Subir un poco más el banner `SERVER: CONNECTED` para evitar solapamiento con el borde superior de la tarjeta `SERVER BROWSER` y agregar el botón `[X]` de limpiado al final de la barra de búsqueda para borrar el texto y restaurar todo el listado de salas.
- Archivos modificados: `UIBanner.h`, `UIRoomListScreen.h`, `GameUI.h`.
- Verificación: Compilación Gradle `:app:assembleDebug` exitosa para las 4 ABIs y confirmación visual recibida del usuario (`Testeado`).
- Estado: Testeado

## CHG-034 — Posicionar botón [X] a la derecha, margen inferior para LOBBIES, separación vertical de filas en Lobby y restricción de equipos distintos para INICIAR PARTIDA
- Inicio: 2026-09-03 03:31:00 -06:00
- Entrega: 2026-09-03 03:42:00 -06:00
- Duración de implementación: ~11 minutos
- Testeado: Sí
- Hora de prueba final: 2026-09-03 03:45:00 -06:00
- Prompt: Posicionar el botón `[X]` más a la derecha sin encimarse en `SEARCH`, añadir margen inferior para separar `LOBBIES` de la lista de salas, aumentar margen superior y separación vertical entre filas de jugadores en el Lobby para evitar superposición de bordes, y exigir la presencia de jugadores en ambos equipos (`BLUE` y `RED`) para habilitar el botón `INICIAR PARTIDA`.
- Archivos modificados: `Strings.h`, `UIRoomListScreen.h`, `UIRoomLobbyScreen.h`, `GameUI.h`.
- Verificación: Compilación Gradle `:app:assembleDebug` exitosa para las 4 ABIs y confirmación visual recibida del usuario (`Testing`/`Testeado`).
- Estado: Testeado
