# Project Documentation - Touchparty 3D Game Engine

Comprehensive technical documentation for the **Touchparty** Android 3D Game Engine built with C++20, OpenGL ES 3.0, and GameActivity.

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
- **Window & Layout**: Immersive Sticky Fullscreen Landscape (`NoActionBar` theme, display cutout support)

---

## 2. Architecture & File Structure

```
app/src/main/
├── cpp/
│   ├── main.cpp              # GameActivity entry point & ALooper event loop
│   ├── Renderer.h / .cpp     # Render pipeline, camera matrices, EGL context, frame timing, input raycasting
│   ├── MatrixMath.h          # Column-major 4x4 matrix math & 3D unprojection raycast algorithms
│   ├── CubeGrid.h            # 12x10 3D cube grid (120 cubes), 3D floor platform, animations & rotation
│   ├── ParticleSystem.h      # Board-relative expanding shockwave particle system
│   ├── GameUI.h              # 2D Orthographic UI (Score panels, 3D mini-cubes, timer, countdown, winner card)
│   ├── AudioEngine.h         # Procedural AAudio PCM synth & JNI MP3 trigger
│   ├── VibrationEngine.h     # JNI bridge for device haptic feedback
│   ├── Shader.h / .cpp       # GLES 3.0 shader program management (MVP, Color, Texture)
│   ├── TextureAsset.h / .cpp # Asset loader using Android AImageDecoder
│   ├── Model.h / Utility.h   # Vertex structures and EGL/GL error helpers
│   └── CMakeLists.txt        # Native C++ build script (links EGL, GLESv3, AAudio, GameActivity)
├── java/com/lewyzstudio/touchparty/
│   └── MainActivity.kt       # GameActivity subclass, full-screen cutout, Vibrator & MediaPlayer integration
├── assets/
│   └── background_cubes.jpeg # Sci-fi laboratory background image texture
└── res/raw/
    └── countdown_party.mp3   # Countdown audio track (3, 2, 1, GO!)
```

---

## 3. Core Engine Systems

### 3.1. 3D Camera & Matrix Math (`MatrixMath.h`, `Renderer.cpp`)
- **Projection**: Perspective camera with adaptive FOV based on screen aspect ratio (`width / height`) so the entire 12x10 grid is visible on any display size.
- **View Matrix**: LookAt camera positioned at `Vec3(0.0, 11.5, 10.5)` looking down at board center `Vec3(0.0, -0.4, 0.0)`.
- **Unprojection / Raycasting**:
  Converts 2D screen touch $(x, y)$ in pixels to normalized device coordinates (NDC) $[-1, 1]$, then applies inverse View-Projection matrix $(P \times V)^{-1}$ to generate a 3D Ray (origin & normalized direction).

### 3.2. 12x10 Cube Grid & Platform (`CubeGrid.h`)
- **Dimensions**: 12 columns $\times$ 10 rows = 120 cubes.
- **Base Platform**: 3D metallic tech floor rendered underneath the grid with bevel borders (`R: 0.35, G: 0.40, B: 0.46`).
- **State Machine**:
  - `CUBE_STATE_WHITE` (0): Base neutral state (uncounted).
  - `CUBE_STATE_BLUE` (1): Blue state (+1 Blue score).
  - `CUBE_STATE_RED` (2): Red state (-1 Blue, +1 Red score).
  - Tapping state 2 resets cube back to `WHITE` (-1 Red score).
- **Animations**:
  - Jump Y-offset: $\text{sin}(t \cdot \pi) \times 0.75\text{f}$ over $\sim 0.33$s.
  - Spin Rotation: $t \times 360^\circ$ Y-axis rotation on touch.
- **Rotating Difficulty (Half-Time)**:
  When remaining match time $\le 15$s, the board smoothly rotates on its Y-axis (`boardRotationY_ += dt * 22.0f`).
- **Rotation-Aware Raycasting**:
  When picking cubes during board rotation, the incoming 3D Ray is inverse-rotated by $-\text{boardRotationY\_}$ around the Y-axis before testing against static cube AABBs. This achieves **100% picking precision** at any rotation angle.

### 3.3. Particle System (`ParticleSystem.h`)
- Spawns expanding shockwave ring particles around tapped cubes.
- Particles store local board coordinates and render with `boardRotationY_` matrix transformation, ensuring waves **continuously rotate in lockstep with the board** and stay attached to the tapped cube.

### 3.4. User Interface Overlay (`GameUI.h`)
- Rendered in a dedicated 2D orthographic overlay pass with depth testing disabled.
- **Score Panels**: Top-Left panel (`RED: X`) and Top-Right panel (`BLUE: Y`) with animated **spinning 3D mini-cubes**.
- **Reset Button**: Top-Left button directly below Red panel (`Y = 0.65f`) for quick match restart.
- **Match Timer**: Top-Center digital clock (`00:30` $\rightarrow$ `00:00`) pulsing red when $\le 10$s.
- **Play Button**: Centered circular button in `GameState::MENU`.
- **Countdown Sequence**: Animated numbers `3`, `2`, `1`, `GO!` with scaling/alpha transitions.
- **Winner Card Overlay (`GameState::MATCH_OVER`)**: Appears at `00:00`, locks cube input, announces winner ("RED WINS!", "BLUE WINS!", "DRAW!"), displays final scores, and provides a "PLAY AGAIN" button.

### 3.5. Audio Engine (`AudioEngine.h`, `MainActivity.kt`)
- **Procedural Synthesizer (AAudio)**: Low-latency 44.1 kHz stereo PCM synthesis:
  - Cube Tap Pops: Frequency sweep tailored per color state.
  - Countdown Beeps: Pure 523 Hz (C5) synth beep.
  - Victory Chime: Ascending two-tone chime (G5 $\rightarrow$ C6).
- **Countdown MP3 Track**: Triggers `MainActivity.playCountdownAudio()` via JNI to play `R.raw.countdown_party` (`3_2_1_GO_party.mp3`).

### 3.6. Haptic Vibration Engine (`VibrationEngine.h`, `MainActivity.kt`)
- Invokes `MainActivity.triggerVibration(colorState)` via JNI using `app->activity->javaGameActivity`.
- **Tactile Patterns**:
  - White (0): Light click (~15 ms).
  - Blue (1): Sharp double click (~35 ms).
  - Red (2): Heavy double pulse (~60 ms).
  - Green (3 - Future): Triple tick pattern (~25 ms $\times 3$).
  - Yellow (4 - Future): Ramp pulse (~75 ms).

---

## 4. Game States & Flow

```
[MENU] ──(Tap Play / Reset)──> [COUNTDOWN] ──(4 seconds)──> [PLAYING]
   ▲                                                           │
   │                                                    (30s Timeout)
   │                                                           ▼
   └───────────────(Tap Reset / Play Again)──────────── [MATCH_OVER]
```

1. **MENU**: Play button displayed over board. Cubes locked.
2. **COUNTDOWN**: Plays `countdown_party.mp3` while showing `3`, `2`, `1`, `GO!`.
3. **PLAYING**:
   - 30-second match timer runs.
   - Cubes touchable (triggering jump, spin, haptic vibration, procedural sound, and wave particles).
   - Board rotates when timer $\le 15$s.
4. **MATCH_OVER**:
   - Input locked.
   - Winner Card overlay displays winner and scores.
   - Tapping "PLAY AGAIN" or Reset resets grid and starts new match.

---

## 5. Future Extensibility (Green & Yellow States)

The codebase is pre-configured for expanding cube color states to include Green (3) and Yellow (4):
- **AudioEngine**: Procedural frequencies defined for `SOUND_TAP_GREEN` (1050 Hz) and `SOUND_TAP_YELLOW` (1250 Hz).
- **ParticleSystem**: Colors configured for Green (`R: 0.1, G: 0.9, B: 0.2`) and Yellow (`R: 1.0, G: 0.85, B: 0.05`).
- **VibrationEngine**: Tactile patterns defined for states 3 and 4 in `MainActivity.kt`.
