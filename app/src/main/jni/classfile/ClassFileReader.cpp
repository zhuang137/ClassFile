//
// Created by whulzz on 2019/5/9.
//
#include "ClassFileReader.h"
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string>

#include "BigLittleEndian.h"
#define LOG_TAG "CFREADER"
#define DEBUG
#include "log.h"

ClassFileReader::ClassFileReader(const char* filePath) : path(filePath){
    fd = open(filePath, O_RDONLY);
    if (fd > 0) {
        fileLen = lseek(fd, 0, SEEK_END);
        lseek(fd, 0, SEEK_SET);
    }
    mapAddr = mmap(NULL, fileLen, PROT_READ, MAP_PRIVATE, fd, 0);
    LOGD("ClassFileReader mapAddr(%p) fileLen(%lx)", mapAddr, fileLen);
}

ClassFileReader::~ClassFileReader() {
    uint16_t count  = IGNORE_ENDIAN_INT16(constantPoolCount);
    uint16_t index = 1;
    while(index < count) {
        if (constantPool[index]) {
            delete constantPool[index];
        }
        index ++;
    }
    if (mapAddr) {
        munmap(mapAddr, fileLen);
    }
    if (fd > 0) {
        close(fd);
    }
}

void ClassFileReader::printInfo() {
    uint16_t count  = IGNORE_ENDIAN_INT16(constantPoolCount);
    uint16_t index = 1;
    //LOGD("printInfo count(%d)");
    while(index < count) {
        if (constantPool[index]) {
            constantPool[index]->printInfo();
        }
        index ++;
    }
}

void ClassFileReader::init() {
    verifyClassFile() && initConstantPool();
}

bool ClassFileReader::verifyClassFile() {
    if (fd <= 0) {
        LOGE("verifyClassFile fd(%d)", fd);
        return false;
    }

    if (!mapAddr) {
        LOGE("verifyClassFile mapAddr(%p)", mapAddr);
        return false;
    }

    long mapEndAddr = (long)((char*)mapAddr + fileLen);

    const char* x = (const char*)mapAddr;
    LOGD("verifyClassFile x[0](%x) x[1](%x) x[2](%x) x[3](%x)", x[0], x[1], x[2], x[3]);
    magic = (int32_t*)mapAddr;
    if (mapEndAddr < (long) (&magic[1])) {
        LOGE("verifyClassFile fail");
        return false;
    }

    if (IGNORE_ENDIAN_INT32(magic) != 0xCAFEBABE) {
        LOGE("verifyClassFile magic(%lx)", *magic);
        return false;
    }
    minorVersion = (int16_t*)&magic[1];
    majorVersion = &minorVersion[1];
    if (IGNORE_ENDIAN_INT16(majorVersion) < 0x2D) {
        LOGE("verifyClassFile majorVersion(%d)", *majorVersion);
        return false;
    }
    return true;
}

bool ClassFileReader::initConstantPool() {
    if (!mapAddr || !majorVersion) {
        LOGE("initConstantPool fail");
        return false;
    }
    long mapEndAddr = (long)mapAddr + fileLen;

    if (mapEndAddr <= (long)&majorVersion[1]) {
        LOGE("initConstantPool fail 1");
        return false;
    }
    constantPoolCount = (uint16_t*)&majorVersion[1];
    //constantPool[0] is null
    //if constantPoolCount = 20,than we should get array whose size is 21 and array[21] set null
    uint16_t count = IGNORE_ENDIAN_INT16(constantPoolCount);
    size_t constanPoolSize = (count + 1)*(sizeof(constantPool));
    constantPool = (PoolInfoBase**)malloc(constanPoolSize);
    if (!constantPool) {
        LOGE("initConstantPool malloc failed");
        return false;
    }
    memset(constantPool, 0, constanPoolSize);

    uint16_t index = 1;
    for (;index < count;index++) {
        constantPool[index] = new PoolInfoBase();
    }
    index = 1;
    uint8_t* poolStart = (uint8_t*)&constantPoolCount[1];
    while (index < count && mapEndAddr > (long)&(poolStart[1])) {
        LOGD("initConstantPool poolStart(%lx) tag(%d)", poolStart, *poolStart);
        uint8_t tag = *poolStart;
        off64_t off_addr = 0;
        uint16_t off_fix = 0;
        switch (tag) {
            case CONSTANT_UTF8: {
                uint16_t *utf8Length = (uint16_t *)&poolStart[1];
                if (*utf8Length <= 0) {
                    LOGE("initConstantPool fail utf8Length <= 0");
                    return false;
                }
                off_addr = sizeof(uint8_t)/*tag*/ + sizeof(uint16_t)/*length*/ + IGNORE_ENDIAN_INT16(utf8Length);
                break;
            }
            case CONSTANT_INTEGER: {
                off_addr = INTEGERINFO_LENGTH;
                break;
            }
            case CONSTANT_FLOAT: {
                off_addr = FLOATINFO_LENGTH;
                break;
            }
            case CONSTANT_LONG: {
                off_addr = LONGINFO_LENGTH;
                off_fix = 1;
                break;
            }
            case CONSTANT_DOUBLE: {
                off_addr = DOUBLEINFO_LENGTH;
                off_fix = 1;
                break;
            }
            case CONSTANT_CLASS: {
                off_addr = CLASSINFO_LENGTH;
                break;
            }
            case CONSTANT_STRING: {
                off_addr = STRINGINFO_LENGTH;
                break;
            }
            case CONSTANT_FIELDREF: {
                off_addr = FIELDREFINFO_LENGTH;
                break;
            }
            case CONSTANT_METHODREF: {
                off_addr = METHODREFINFO_LENGTH;
                break;
            }
            case CONSTANT_INTERFACEMETHODREF: {
                off_addr = INTERFACEMETHODREFINFO_LENGTH;
                break;
            }
            case CONSTANT_NAMEANDTYPE: {
                off_addr = NAMEANDTYPEINFO_LENGTH;
                break;
            }
            case CONSTANT_METHODHANDLE: {
                off_addr = METHODHANDLEINFO_LENGTH;
                break;
            }
            case CONSTANT_METHODTYPE: {
                off_addr = METHODTYPEINFO_LENGTH;
                break;
            }
            case CONSTANT_INVOKEDYNAMIC: {
                off_addr = INVOKEDYNAMICINFO_LENGTH;
                break;
            }
            default:
                LOGE("initConstantPool UNKNOWN CONSTANTPOOL tag");
                return false;
        }
        poolStart = poolStart + off_addr;
        constantPool[index]->index = index;
        constantPool[index]->tag = tag;
        constantPool[index]->addrStart = poolStart - off_addr;
        constantPool[index]->addrEnd = (void*)(poolStart - 1);
        LOGD("initConstantPool poolCount(%d) index(%d)", count, index);
        index += off_fix;
        index++;
    }
    LOGD("initConstantPool successful");
    return true;
}
void PoolInfoBase::printInfo() {
    std::string logStr;
    logStr.append(std::to_string(index));
    logStr.append(":");
    switch (tag) {
        case CONSTANT_UTF8: {
            logStr.append("Utf8 ");
            uint16_t* len = (uint16_t*)(&((uint8_t*)addrStart)[1]);
            const char* strStart = (const char*)&len[1];
            logStr.append(strStart, IGNORE_ENDIAN_INT16(len));
            break;
        }
        case CONSTANT_INTEGER: {
            logStr.append("Integer ");
            int32_t* value = (int32_t*)(&((uint8_t*)addrStart)[1]);
            logStr.append(std::to_string(IGNORE_ENDIAN_INT32(value)));
            break;
        }
        case CONSTANT_FLOAT: {
            logStr.append("Float ");
            int32_t* value = (int32_t*)(&((uint8_t*)addrStart)[1]);
            logStr.append(std::to_string(IGNORE_ENDIAN_INT32(value)));
            break;
        }
        case CONSTANT_LONG: {
            logStr.append("Long ");
            int32_t* highValue = (int32_t*)(&((uint8_t*)addrStart)[1]);
            int32_t* lowValue = &highValue[1];
            long long value = (long long)(*highValue) << 32 | *lowValue;
            logStr.append(std::to_string(value));
            break;
        }
        case CONSTANT_DOUBLE: {
            logStr.append("Double ");
            int32_t* highValue = (int32_t*)(&((uint8_t*)addrStart)[1]);
            int32_t* lowValue = &highValue[1];
            double value = (long long)IGNORE_ENDIAN_INT32(highValue) << 32 | IGNORE_ENDIAN_INT32(lowValue);
            logStr.append(std::to_string(value));
            break;
        }
        case CONSTANT_CLASS: {
            logStr.append("Class #");
            uint16_t* value = (uint16_t*)(&((uint8_t*)addrStart)[1]);
            logStr.append(std::to_string(IGNORE_ENDIAN_INT16(value)));
            break;
        }
        case CONSTANT_STRING: {
            logStr.append("String #");
            uint16_t* value = (uint16_t*)(&((uint8_t*)addrStart)[1]);
            logStr.append(std::to_string(IGNORE_ENDIAN_INT16(value)));
            break;
        }
        case CONSTANT_FIELDREF: {
            logStr.append("FieldRef ");
            logStr.append("classIndex-#");
            uint16_t* value = (uint16_t*)(&((uint8_t*)addrStart)[1]);
            logStr.append(std::to_string(IGNORE_ENDIAN_INT16(value)));
            logStr.append("nameAndTypeIndex-#");
            value = &value[1];
            logStr.append(std::to_string(IGNORE_ENDIAN_INT16(value)));
            break;
        }
        case CONSTANT_METHODREF: {
            logStr.append("MethodRef ");
            logStr.append("classIndex-#");
            uint16_t* value = (uint16_t*)(&((uint8_t*)addrStart)[1]);
            logStr.append(std::to_string(IGNORE_ENDIAN_INT16(value)));
            logStr.append("nameAndTypeIndex-#");
            value = &value[1];
            logStr.append(std::to_string(IGNORE_ENDIAN_INT16(value)));
            break;
        }
        case CONSTANT_INTERFACEMETHODREF: {
            logStr.append("InterfaceMethodRef ");
            logStr.append("classIndex-#");
            uint16_t* value = (uint16_t*)(&((uint8_t*)addrStart)[1]);
            logStr.append(std::to_string(IGNORE_ENDIAN_INT16(value)));
            logStr.append("nameAndTypeIndex-#");
            value = &value[1];
            logStr.append(std::to_string(IGNORE_ENDIAN_INT16(value)));
            break;
        }
        case CONSTANT_NAMEANDTYPE: {
            logStr.append("NameAndType ");
            logStr.append("classIndex-#");
            uint16_t* value = (uint16_t*)(&((uint8_t*)addrStart)[1]);
            logStr.append(std::to_string(IGNORE_ENDIAN_INT16(value)));
            logStr.append("descriptorIndex-#");
            value = &value[1];
            logStr.append(std::to_string(IGNORE_ENDIAN_INT16(value)));
            break;
        }
        case CONSTANT_METHODHANDLE: {
            logStr.append("MethodHandle ");
            break;
        }
        case CONSTANT_METHODTYPE: {
            logStr.append("MethodType ");
            break;
        }
        case CONSTANT_INVOKEDYNAMIC: {
            logStr.append("InvokeDynamic ");
            break;
        }
    }
    LOGD("%s", logStr.c_str());
}



