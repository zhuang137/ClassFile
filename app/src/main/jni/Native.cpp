//
// Created by whulzz on 2019/5/9.
//
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <jni.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <dlfcn.h>

#define LOG_TAG "NATIVE"
#define DEBUG
#include "log/log.h"
#include "classfile/ClassFileReader.h"

static void popenTest(const char* command) {
    char result_buf[1024];
    int rc = 0; // 用于接收命令返回值
    FILE *fp;

    /*将要执行的命令写入buf*/
    /*执行预先设定的命令，并读出该命令的标准输出*/
    fp = popen(command, "r");
    if(NULL == fp)
    {
        LOGD("popen执行失败！");
        exit(1);
    }
    while(fgets(result_buf, sizeof(result_buf), fp) != NULL)
    {
        /*为了下面输出好看些，把命令返回的换行符去掉*/
        if('\n' == result_buf[strlen(result_buf)-1])
        {
            result_buf[strlen(result_buf)-1] = '\0';
        }
        LOGD("命令【%s】 输出【%s】\r\n", command, result_buf);
    }

    /*等待命令执行完毕并关闭管道及文件指针*/
    rc = pclose(fp);
    if(-1 == rc)
    {
        LOGD("关闭文件指针失败");
        exit(1);
    }
    else
    {
        LOGD("命令【%s】子进程结束状态【%d】命令返回值【%d】\r\n", command, rc, WEXITSTATUS(rc));
    }
}

static void testBin(const char* path) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        LOGD("open bin(%s) failed", path);
        return;
    }
    LOGD("open bin(%s) success", path);
    LOGD("close bin ret: %d",close(fd));
    LOGD("close bin ret: %d",close(fd));
}

static void cpToybox() {
    const char* path = "/system/bin/toybox";
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        LOGD("openToybox failed");
        return;
    }
    LOGD("openToybox success");
    const char* savedPath = "/sdcard/toybox";
    int saveFd = open(savedPath, O_RDWR|O_CREAT, 0777);
    if (saveFd < 0) {
        LOGD("savedPath open failed: %s", strerror(errno));
        close(fd);
        return;
    }
    char buf[1024];
    int ret = 0;
    while ((ret = read(fd, buf, 1024)) > 0) {
        write(saveFd, buf, ret);
    }
    fsync(saveFd);
    close(saveFd);
    close(fd);
}

static void testMySystem(const char* command) {
    pid_t pid, cpid;
    struct sigaction intsave, quitsave;
    sigset_t mask, omask;
    int pstat;
    char *argp[] = {"sh", "-c", NULL, NULL};

    if (!command)           /* just checking... */
    {
        LOGD("testMySystem failed, cmd: null");
        return;
    }

    argp[2] = (char *)command;

    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask, &omask);
    switch (cpid = vfork()) {
        case -1:                        /* error */
            sigprocmask(SIG_SETMASK, &omask, NULL);
            LOGD("testMySystem vfork failed");
            return;
        case 0:                         /* child */
            sigprocmask(SIG_SETMASK, &omask, NULL);
            execve("/system/bin/sh", argp, environ);
            _exit(127);
    }

    sigaction(SIGINT, NULL, &intsave);
    sigaction(SIGQUIT, NULL, &quitsave);
    do {
        pid = waitpid(cpid, &pstat, 0);
    } while (pid == -1 && errno == EINTR);
    sigprocmask(SIG_SETMASK, &omask, NULL);
    sigaction(SIGINT, &intsave, NULL);
    sigaction(SIGQUIT, &quitsave, NULL);
    int ret = (pid == -1 ? -1 : pstat);
    LOGD("testMySystem ret:%d", ret);
}

static void test_memcmp() {
    const char src[] = "arm";
    long mapLen = (1UL<<12)*20;
    void* mapAddr = mmap((void*)0x864b9000, mapLen, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    void* mapEnd = (void*)((long)mapAddr + mapLen);
    void* strToAddrBegin = (void*)((long)mapEnd - 13);
    memcpy(strToAddrBegin, src, sizeof(src));
    const char my_str[] = "data@data@com.ledou.mhhy.huawei@lebian@com.ledou.mhhy.huawei-1@base.odex.apk@classes.dex";
    LOGD("test_memcmp begin mapAddr %p size: %d s1:%s(@%p) s2: %s(@%p)", mapAddr, sizeof(my_str),
            strToAddrBegin, strToAddrBegin, my_str, my_str);
    int xx = strncmp((const char*)strToAddrBegin, my_str, sizeof(my_str));
    LOGD("test_memcmp ret: %d",xx);
}

static jboolean init(JNIEnv* env, jclass clazz, jstring classFilePath) {
//    int ret = system("/system/bin/sh sh -c env");
//    LOGD("system ret:%d", ret);
//    //cpToybox();
testBin("/system/bin/sh");
//    //testBin("/system/bin/rm");
//    //testMySystem("env > /sdcard/env.txt");
//    popenTest("env");
//    const char* path = env->GetStringUTFChars(classFilePath, JNI_FALSE);
//    ClassFileReader cfReader(path);
//    cfReader.init();
//    cfReader.printInfo();
//    env->ReleaseStringUTFChars(classFilePath, path);
//
//    test_memcmp();
    void* handle = dlopen("libfjni_wrapper.so", 0);
    if (!handle) {
        LOGD("libfjni dlopen failed, error: %s", dlerror());
    }
    LOGD("libfjni handle: %p", handle);
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
    gettid();
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

