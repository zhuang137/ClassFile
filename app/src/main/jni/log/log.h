//
// Created by whulzz on 2019/5/9.
//

#ifndef CLASSFILE_LOG_H
#define CLASSFILE_LOG_H
#include <android/log.h>
#ifdef DEBUG
#define LOGD(fmt, args...)  do {__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, fmt, ##args);} while(0)
#define LOGI(fmt, args...)  do {__android_log_print(ANDROID_LOG_INFO, LOG_TAG, fmt, ##args);} while(0)
#define LOGE(fmt, args...)  do {__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, fmt, ##args);} while(0)
#else
#define LOGD(fmt, args...) do {} while(0)
#define LOGI(fmt, args...) do {} while(0)
#define LOGE(fmt, args...) do {} while(0)
#endif
#endif //CLASSFILE_LOG_H
