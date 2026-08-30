#ifndef ANDROIDGLINVESTIGATIONS_VIBRATIONENGINE_H
#define ANDROIDGLINVESTIGATIONS_VIBRATIONENGINE_H

#include <jni.h>
#include <game-activity/native_app_glue/android_native_app_glue.h>
#include <game-activity/GameActivity.h>
#include "AndroidOut.h"

class VibrationEngine {
public:
    static void triggerVibration(android_app* app, int colorState) {
        if (!app || !app->activity || !app->activity->vm || !app->activity->javaGameActivity) return;

        JavaVM* vm = app->activity->vm;
        JNIEnv* env = nullptr;
        bool isAttached = false;

        jint res = vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
        if (res == JNI_EDETACHED) {
            if (vm->AttachCurrentThread(&env, nullptr) == JNI_OK) {
                isAttached = true;
            } else {
                return;
            }
        }

        if (env) {
            jobject activityObj = app->activity->javaGameActivity;
            jclass activityClass = env->GetObjectClass(activityObj);
            if (activityClass) {
                jmethodID method = env->GetMethodID(activityClass, "triggerVibration", "(I)V");
                if (method) {
                    env->CallVoidMethod(activityObj, method, static_cast<jint>(colorState));
                } else {
                    env->ExceptionClear();
                }
                env->DeleteLocalRef(activityClass);
            } else {
                env->ExceptionClear();
            }
        }

        if (isAttached) {
            vm->DetachCurrentThread();
        }
    }
};

#endif // ANDROIDGLINVESTIGATIONS_VIBRATIONENGINE_H