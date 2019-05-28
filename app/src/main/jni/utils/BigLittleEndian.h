//
// Created by whulzz on 2019/5/9.
//

#ifndef CLASSFILE_BIGLITTLEENDIAN_H
#define CLASSFILE_BIGLITTLEENDIAN_H

#include <sys/types.h>
typedef enum {
    BIG_ENDIAN,
    LITTLE_ENDIAN,
} ENDIAN_MODEL;

int16_t IGNORE_ENDIAN_INT16(void*);
int32_t IGNORE_ENDIAN_INT32(void*);
int64_t IGNORE_ENDIAN_INT64(void*);

#endif //CLASSFILE_BIGLITTLEENDIAN_H
