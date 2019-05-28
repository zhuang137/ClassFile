//
// Created by whulzz on 2019/5/9.
//
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <jni.h>
#define LOG_TAG "NATIVE"
#define DEBUG
#include "log/log.h"
#include "classfile/ClassFileReader.h"

static jboolean init(JNIEnv* env, jclass clazz, jstring classFilePath) {
    const char* path = env->GetStringUTFChars(classFilePath, JNI_FALSE);
    ClassFileReader cfReader(path);
    cfReader.init();
    cfReader.printInfo();
    env->ReleaseStringUTFChars(classFilePath, path);
    return JNI_TRUE;
}

static const char *classPathName = "com/whulzz/classfile/MyApplication";
static JNINativeMethod methods[] = {
    {"init", "(Ljava/lang/String;)V", (void*)init},
};
typedef union {
    JNIEnv* env;
    void* venv;
} UnionJNIEnvToVoid;

static int registerNativeMethods(JNIEnv* env, const char* className,
    const JNINativeMethod* gMethods, int numMethods)
{
    jclass clazz;

    clazz = env->FindClass(className);
    if (clazz == NULL) {
        LOGE("Native registration unable to find class '%s'", className);
        return JNI_FALSE;
    }
    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
        LOGE("RegisterNatives failed for '%s'", className);
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

static int registerNatives(JNIEnv* env)
{
  if (!registerNativeMethods(env, classPathName,
                 methods, sizeof(methods) / sizeof(methods[0]))) {
    return JNI_FALSE;
  }

  return JNI_TRUE;
}


jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    UnionJNIEnvToVoid uenv;
    uenv.venv = NULL;
    jint result = -1;
    JNIEnv* env = NULL;

    LOGI("JNI_OnLoad");

    if (vm->GetEnv(&uenv.venv, JNI_VERSION_1_4) != JNI_OK) {
        LOGE("ERROR: GetEnv failed");
        goto bail;
    }
    env = uenv.env;

    if (registerNatives(env) != JNI_TRUE) {
        LOGE("ERROR: registerNatives failed");
        goto bail;
    }

    result = JNI_VERSION_1_4;

bail:
    return result;
}

