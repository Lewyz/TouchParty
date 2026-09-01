#include <jni.h>

#include <game-activity/native_app_glue/android_native_app_glue.h>
#include <game-activity/GameActivity.h>

#include "AndroidOut.h"
#include "Renderer.h"

static Renderer* g_pRenderer = nullptr;

extern "C" {

JNIEXPORT jboolean JNICALL
Java_com_lewyzstudio_touchparty_MainActivity_nativeOnTextInputResult(
        JNIEnv *env, jclass clazz, jint fieldType, jstring text) {
    if (!g_pRenderer) {
        return JNI_FALSE;
    }
    const char *nativeString = env->GetStringUTFChars(text, nullptr);
    if (nativeString) {
        g_pRenderer->getUI().onTextInputResult(fieldType, nativeString);
        env->ReleaseStringUTFChars(text, nativeString);
        return JNI_TRUE;
    }
    return JNI_FALSE;
}

JNIEXPORT void JNICALL
Java_com_lewyzstudio_touchparty_MainActivity_nativeSetServerConnected(
        JNIEnv *env, jclass clazz, jboolean connected) {
    if (g_pRenderer) {
        g_pRenderer->getUI().setServerConnected(connected == JNI_TRUE);
    }
}

JNIEXPORT jboolean JNICALL
Java_com_lewyzstudio_touchparty_MainActivity_nativeSetServerRooms(
        JNIEnv *env, jclass clazz, jstring jsonRooms) {
    if (!g_pRenderer) return JNI_FALSE;
    const char *nativeString = env->GetStringUTFChars(jsonRooms, nullptr);
    if (nativeString) {
        g_pRenderer->getUI().setServerRoomsJson(nativeString);
        env->ReleaseStringUTFChars(jsonRooms, nativeString);
        return JNI_TRUE;
    }
    return JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_lewyzstudio_touchparty_MainActivity_nativeOnRoomJoined(
        JNIEnv *env, jclass clazz, jstring roomId, jboolean isOwner) {
    if (!g_pRenderer) return JNI_FALSE;
    const char *nativeString = env->GetStringUTFChars(roomId, nullptr);
    if (nativeString) {
        g_pRenderer->getUI().onRoomJoined(nativeString, isOwner == JNI_TRUE);
        env->ReleaseStringUTFChars(roomId, nativeString);
        return JNI_TRUE;
    }
    return JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_lewyzstudio_touchparty_MainActivity_nativeOnRoomStateUpdated(
        JNIEnv *env, jclass clazz, jstring roomId, jstring roomName, jint playerCount, jboolean isPrivate, jstring ownerId, jboolean isOwner, jstring state, jstring playersJson) {
    if (!g_pRenderer) return JNI_FALSE;
    const char *nativeName = env->GetStringUTFChars(roomName, nullptr);
    const char *nativeOwnerId = env->GetStringUTFChars(ownerId, nullptr);
    const char *nativeState = env->GetStringUTFChars(state, nullptr);
    const char *nativePlayersJson = env->GetStringUTFChars(playersJson, nullptr);
    if (nativeName && nativeOwnerId && nativeState && nativePlayersJson) {
        g_pRenderer->getUI().onRoomStateUpdated(nativeName, playerCount, isPrivate == JNI_TRUE, nativeOwnerId, isOwner == JNI_TRUE, nativeState, nativePlayersJson);
        env->ReleaseStringUTFChars(roomName, nativeName);
        env->ReleaseStringUTFChars(ownerId, nativeOwnerId);
        env->ReleaseStringUTFChars(state, nativeState);
        env->ReleaseStringUTFChars(playersJson, nativePlayersJson);
        return JNI_TRUE;
    }
    return JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_lewyzstudio_touchparty_MainActivity_nativeUpdateBoardCell(
        JNIEnv *env, jclass clazz, jint x, jint y, jint colorState) {
    if (!g_pRenderer) return JNI_FALSE;
    g_pRenderer->getCubeGrid().setCubeState(x, y, static_cast<CubeState>(colorState));
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL
Java_com_lewyzstudio_touchparty_MainActivity_nativeStartGameFromNetwork(
        JNIEnv *env, jclass clazz) {
    if (!g_pRenderer) return JNI_FALSE;
    g_pRenderer->getCubeGrid().reset();
    g_pRenderer->getUI().startCountdown();
    return JNI_TRUE;
}

void nativeWsSendTap(android_app* app, int x, int y) {
    if (!app || !app->activity || !app->activity->vm) return;
    JavaVM* vm = app->activity->vm;
    JNIEnv* env = nullptr;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        if (vm->AttachCurrentThread(&env, nullptr) != 0) return;
    }
    jobject activityObj = app->activity->javaGameActivity;
    jclass activityClass = env->GetObjectClass(activityObj);
    jmethodID method = env->GetMethodID(activityClass, "requestSendTap", "(II)V");
    if (method) {
        env->CallVoidMethod(activityObj, method, static_cast<jint>(x), static_cast<jint>(y));
    }
}

void nativeWsRequestCreateRoom(android_app* app, const std::string& name, bool isPrivate, const std::string& pin) {
    if (!app || !app->activity || !app->activity->vm) return;
    JavaVM* vm = app->activity->vm;
    JNIEnv* env = nullptr;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        if (vm->AttachCurrentThread(&env, nullptr) != 0) return;
    }
    jobject activityObj = app->activity->javaGameActivity;
    jclass activityClass = env->GetObjectClass(activityObj);
    jmethodID method = env->GetMethodID(activityClass, "requestCreateRoom", "(Ljava/lang/String;ZLjava/lang/String;)V");
    if (method) {
        jstring jName = env->NewStringUTF(name.c_str());
        jstring jPin = env->NewStringUTF(pin.c_str());
        env->CallVoidMethod(activityObj, method, jName, static_cast<jboolean>(isPrivate), jPin);
        env->DeleteLocalRef(jName);
        env->DeleteLocalRef(jPin);
    }
}

void nativeWsRequestListRooms(android_app* app) {
    if (!app || !app->activity || !app->activity->vm) return;
    JavaVM* vm = app->activity->vm;
    JNIEnv* env = nullptr;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        if (vm->AttachCurrentThread(&env, nullptr) != 0) return;
    }
    jobject activityObj = app->activity->javaGameActivity;
    jclass activityClass = env->GetObjectClass(activityObj);
    jmethodID method = env->GetMethodID(activityClass, "requestListRooms", "()V");
    if (method) {
        env->CallVoidMethod(activityObj, method);
    }
}

void nativeWsRequestJoinRoom(android_app* app, const std::string& roomId, const std::string& pin) {
    if (!app || !app->activity || !app->activity->vm) return;
    JavaVM* vm = app->activity->vm;
    JNIEnv* env = nullptr;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        if (vm->AttachCurrentThread(&env, nullptr) != 0) return;
    }
    jobject activityObj = app->activity->javaGameActivity;
    jclass activityClass = env->GetObjectClass(activityObj);
    jmethodID method = env->GetMethodID(activityClass, "requestJoinRoom", "(Ljava/lang/String;Ljava/lang/String;)V");
    if (method) {
        jstring jRoomId = env->NewStringUTF(roomId.c_str());
        jstring jPin = env->NewStringUTF(pin.c_str());
        env->CallVoidMethod(activityObj, method, jRoomId, jPin);
        env->DeleteLocalRef(jRoomId);
        env->DeleteLocalRef(jPin);
    }
}

void nativeWsRequestLeaveRoom(android_app* app) {
    if (!app || !app->activity || !app->activity->vm) return;
    JavaVM* vm = app->activity->vm;
    JNIEnv* env = nullptr;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        if (vm->AttachCurrentThread(&env, nullptr) != 0) return;
    }
    jobject activityObj = app->activity->javaGameActivity;
    jclass activityClass = env->GetObjectClass(activityObj);
    jmethodID method = env->GetMethodID(activityClass, "requestLeaveRoom", "()V");
    if (method) {
        env->CallVoidMethod(activityObj, method);
    }
}

void nativeWsRequestStartGame(android_app* app) {
    if (!app || !app->activity || !app->activity->vm) return;
    JavaVM* vm = app->activity->vm;
    JNIEnv* env = nullptr;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        if (vm->AttachCurrentThread(&env, nullptr) != 0) return;
    }
    jobject activityObj = app->activity->javaGameActivity;
    jclass activityClass = env->GetObjectClass(activityObj);
    jmethodID method = env->GetMethodID(activityClass, "requestStartGame", "()V");
    if (method) {
        env->CallVoidMethod(activityObj, method);
    }
}

/*!
 * Handles commands sent to this Android application
 * @param pApp the app the commands are coming from
 * @param cmd the command to handle
 */
void handle_cmd(android_app *pApp, int32_t cmd) {
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            g_pRenderer = new Renderer(pApp);
            pApp->userData = g_pRenderer;
            break;
        case APP_CMD_TERM_WINDOW:
            if (pApp->userData) {
                auto *pRenderer = reinterpret_cast<Renderer *>(pApp->userData);
                pApp->userData = nullptr;
                if (g_pRenderer == pRenderer) {
                    g_pRenderer = nullptr;
                }
                delete pRenderer;
            }
            break;
        default:
            break;
    }
}

/*!
 * Enable the motion events you want to handle; not handled events are
 * passed back to OS for further processing. For this example case,
 * only pointer and joystick devices are enabled.
 *
 * @param motionEvent the newly arrived GameActivityMotionEvent.
 * @return true if the event is from a pointer or joystick device,
 *         false for all other input devices.
 */
bool motion_event_filter_func(const GameActivityMotionEvent *motionEvent) {
    auto sourceClass = motionEvent->source & AINPUT_SOURCE_CLASS_MASK;
    return (sourceClass == AINPUT_SOURCE_CLASS_POINTER ||
            sourceClass == AINPUT_SOURCE_CLASS_JOYSTICK);
}

/*!
 * This the main entry point for a native activity
 */
void android_main(struct android_app *pApp) {
    aout << "Welcome to android_main" << std::endl;

    pApp->onAppCmd = handle_cmd;
    android_app_set_motion_event_filter(pApp, motion_event_filter_func);

    do {
        bool done = false;
        while (!done) {
            int timeout = 0;
            int events;
            android_poll_source *pSource;
            int result = ALooper_pollOnce(timeout, nullptr, &events,
                                          reinterpret_cast<void**>(&pSource));
            switch (result) {
                case ALOOPER_POLL_TIMEOUT:
                    [[clang::fallthrough]];
                case ALOOPER_POLL_WAKE:
                    done = true;
                    break;
                case ALOOPER_EVENT_ERROR:
                    aout << "ALooper_pollOnce returned an error" << std::endl;
                    break;
                case ALOOPER_POLL_CALLBACK:
                    break;
                default:
                    if (pSource) {
                        pSource->process(pApp, pSource);
                    }
            }
        }

        if (pApp->userData) {
            auto *pRenderer = reinterpret_cast<Renderer *>(pApp->userData);
            pRenderer->handleInput();
            pRenderer->render();
        }
    } while (!pApp->destroyRequested);
}
}