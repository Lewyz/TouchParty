# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

cmake_minimum_required(VERSION 3.22.1)

project("touchparty")

if(NOT DEFINED GAME_SERVER_HTTP_URL)
    set(GAME_SERVER_HTTP_URL "https://game.tutaxi502.com")
endif()
if(NOT DEFINED GAME_SERVER_WS_URL)
    set(GAME_SERVER_WS_URL "wss://game.tutaxi502.com")
endif()

# Creates your game shared library. The name must be the same as the
# one used for loading in your Kotlin/Java or AndroidManifest.txt files.
add_library(touchparty SHARED
        main.cpp
        AndroidOut.cpp
        Renderer.cpp
        Shader.cpp
        TextureAsset.cpp
        FontRenderer.cpp
        Utility.cpp)

target_compile_definitions(touchparty PRIVATE
        GAME_SERVER_HTTP_URL="${GAME_SERVER_HTTP_URL}"
        GAME_SERVER_WS_URL="${GAME_SERVER_WS_URL}"
)

# Searches for a package provided by the game activity dependency
find_package(game-activity REQUIRED CONFIG)

# Forces the linker to keep the JNI entry point for GameActivity
set(CMAKE_SHARED_LINKER_FLAGS
        "${CMAKE_SHARED_LINKER_FLAGS} -u Java_com_google_androidgamesdk_GameActivity_initializeNativeCode")

# Configure libraries CMake uses to link your target library.
target_link_libraries(touchparty
        # The game activity
        game-activity::game-activity_static

        # EGL, AAudio, and other dependent libraries required for drawing
        # and interacting with Android system
        EGL
        GLESv3
        aaudio
        jnigraphics
        android
        log)