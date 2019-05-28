//
// Created by whulzz on 2019/5/9.
//
#include "BigLittleEndian.h"
#include <sys/types.h>
ENDIAN_MODEL machineEndian() {
    uint16_t value = 1;
    if (*(char*)(&value) == 1) {
        return LITTLE_ENDIAN;
    }
    return BIG_ENDIAN;
}

int16_t IGNORE_ENDIAN_INT16(void* arg) {
    const char* value = (const char*)arg;
    return (value[0] << 8) | (value[1] << 0);
}
int32_t IGNORE_ENDIAN_INT32(void* arg) {
    const char* value = (const char*)arg;
    return (value[0] << 24) | (value[1] << 16) | (value[2] << 8) | (value[3] << 0);
}
int64_t IGNORE_ENDIAN_INT64(void* arg) {
    const char* value = (const char*)arg;
    return (value[0] << 56) | (value[1] << 48) | (value[2] << 40) | (value[3] << 32) |
                (value[4] << 24) | (value[5] << 16) | (value[6] << 8) | (value[7] << 0);
}

