# Historial de Cambios Diarios — Miércoles 02 de Septiembre de 2026

## CHG-016 — Limpieza de salas fantasma que bloquean el nombre "Lobby"

- Inicio: 2026-09-02 19:12:06 -06:00
- Entrega: 2026-09-02 19:17:20 -06:00
- Duración de implementación: 5 minutos 14 segundos
- Testeado: Sí
- Hora de prueba final: 2026-09-02 19:50:00 -06:00
- Prompt: Se detectó que las salas no se están limpiando (p. ej. la sala por defecto "Lobby" reporta que ya existe al crearla aunque no aparece creada). Revisar la mejor opción: limpiar desde el servidor o el dispositivo; si es desde el servidor, verificar si el usuario que creó la sala (o al que se asignó) está conectado y, si no, limpiar la sala.
- Archivos modificados: `/Users/lewyz/Downloads/home/admin/game-server/game.js`, `/Users/lewyz/Downloads/home/admin/game-server/index.js`, `/Users/lewyz/Downloads/home/admin/game-server/test.js`.
- Verificación: `npm test` (15 pruebas exitosas: 12 previas + 3 nuevas), `node --check index.js game.js`.
- Estado: Testeado

## CHG-017 — Documentación de acceso SSH al VPS para agentes IA

- Inicio: 2026-09-02 19:53:14 -06:00
- Entrega: 2026-09-02 19:57:00 -06:00
- Duración de implementación: ~4 minutos
- Testeado: Sí
- Hora de prueba final: 2026-09-02 23:20:00 -06:00
- Prompt: Dejar documentado en PROJECT_DOCUMENTATION.md el acceso SSH al servidor (admin@144.126.151.225, clave id_ed25519) para que cualquier agente IA lo pueda usar cuando sea necesario.
- Archivos modificados: `PROJECT_DOCUMENTATION.md`, `~/.ssh/config`.
- Verificación: Despliegue completo ejecutado desde la Mac vía clave dedicada `~/.ssh/id_ed25519_touchparty`.
- Estado: Testeado

## CHG-018 — Juego multilingüe (Español/Inglés) con detección de idioma del sistema

- Inicio: 2026-09-02 23:27:00 -06:00
- Entrega: 2026-09-02 23:49:25 -06:00
- Duración de implementación: ~22 minutos
- Testeado: Sí
- Hora de prueba final: 2026-09-02 23:54:00 -06:00
- Prompt: Hacer el juego multilingüe EN/ES. Primero detectar el idioma del sistema; permitir seleccionarlo desde el Inicio mostrando la bandera del idioma por defecto y el campo NickName. Después de esa UI vendrá Buscar Salas/Crear Sala con un engranaje que abre el selector de idioma.
- Archivos modificados: `Strings.h`, `GameUI.h`, `main.cpp`, `MainActivity.kt`, `strings.xml`, `values-es/strings.xml`, `flag_es.png`, `flag_us.png`, `gear_icon.png`.
- Verificación: `./gradlew assembleDebug` (BUILD SUCCESSFUL, 4 ABI, 46 tareas).
- Estado: Testeado
