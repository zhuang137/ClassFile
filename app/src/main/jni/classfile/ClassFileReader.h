//
// Created by whulzz on 2019/5/9.
//

#ifndef CLASSFILE_CLASSFILEREADER_H
#define CLASSFILE_CLASSFILEREADER_H

#include <stdint.h>
#include <sys/types.h>

struct PoolInfoBase {
    uint16_t index;
    uint8_t tag;
    void* addrStart;
    void* addrEnd;
    virtual void printInfo();
};
struct IntegerInfo : PoolInfoBase {
    int32_t value;
    void printInfo();
};
struct FloatInfo : PoolInfoBase {
    int32_t value;
    void printInfo();
};
struct LongInfo : PoolInfoBase {
    int64_t value;
    void printInfo();
};
struct DoubleInfo : PoolInfoBase {
    int64_t value;
    void printInfo();
};
struct Utf8Info : PoolInfoBase {
    uint16_t length;
    const char* str;
    void printInfo();
};
struct ClassInfo : PoolInfoBase {
    uint16_t nameIndex;
    const char* nameStr;
    void printInfo();
};
struct FieldRefInfo : PoolInfoBase {
    uint16_t classIndex;
    uint16_t nameAndTypeIndex;
    const char* classStr;
    const char* nameAndTypeStr;
    void printInfo();
};
struct StringInfo : PoolInfoBase {
    uint16_t strIndex;
    const char* strStr;
    void printInfo();
};
struct MethodTypeInfo : PoolInfoBase {
    uint16_t descriptorIndex;
    const char* descriptorStr;
    void printInfo();
};
struct MethodRefInfo : PoolInfoBase {
    uint16_t classIndex;
    uint16_t nameAndTypeIndex;
    const char* classStr;
    const char* nameAndTypeStr;
    void printInfo();
};
struct InterfaceMethodRefInfo : PoolInfoBase {
    uint16_t classIndex;
    uint16_t nameAndTypeIndex;
    const char* classStr;
    const char* nameAndTypeStr;
    void printInfo();
};
struct NameAndTypeInfo : PoolInfoBase {
    uint16_t classIndex;
    uint16_t descriptorIndex;
    const char* classStr;
    const char* descriptorStr;
    void printInfo();
};

/*PoolInfoLength*/
#define INTEGERINFO_LENGTH 5
#define FLOATINFO_LENGTH 5
#define LONGINFO_LENGTH 9
#define DOUBLEINFO_LENGTH 9
#define FIELDREFINFO_LENGTH 5
#define CLASSINFO_LENGTH 3
#define STRINGINFO_LENGTH 3
#define METHODTYPEINFO_LENGTH 3
#define METHODREFINFO_LENGTH 5
#define INTERFACEMETHODREFINFO_LENGTH 5
#define NAMEANDTYPEINFO_LENGTH 5
#define METHODHANDLEINFO_LENGTH 4
#define INVOKEDYNAMICINFO_LENGTH 5

enum {
    CONSTANT_UTF8 = 1,                               //1
    CONSTANT_IGNORE2,                                //2
    CONSTANT_INTEGER,                                //3
    CONSTANT_FLOAT,                                  //4
    CONSTANT_LONG,                                   //5
    CONSTANT_DOUBLE,                                 //6
    CONSTANT_CLASS,                                  //7
    CONSTANT_STRING,                                 //8
    CONSTANT_FIELDREF,                               //9
    CONSTANT_METHODREF,                              //10
    CONSTANT_INTERFACEMETHODREF,                     //11
    CONSTANT_NAMEANDTYPE,                            //12
    CONSTANT_IGNORE13,                               //13
    CONSTANT_IGNORE14,                               //14
    CONSTANT_METHODHANDLE,                           //15
    CONSTANT_METHODTYPE,                             //16
    CONSTANT_IGNORE17,                               //17
    CONSTANT_INVOKEDYNAMIC,                          //18
};

struct ClassFileReader{

public:
    ClassFileReader(const char*);
    ~ClassFileReader();
    void init();
    void printInfo();

private:
    bool verifyClassFile();
    bool initConstantPool();
    bool initInterfaces();
    bool initFields();
    bool initMethods();
    bool initAttributes();

private:
    int fd;
    void* mapAddr;
    const char* path;
    off64_t fileLen;
    int32_t* magic;
    int16_t* minorVersion;
    int16_t* majorVersion;
    uint16_t* constantPoolCount;
    PoolInfoBase** constantPool;
    const char* accessFlags;
    const char* classFullyQualifiedName;
    const char* superClassFullyQualifiedName;
    uint16_t* interfacesCount;
    void* interfaces;
    uint16_t* fieldsCount;
    void* fields;
    uint16_t* methodsCount;
    void* methods;
    uint16_t* attributeCount;
    void* attributes;
};

#endif //CLASSFILE_CLASSFILEREADER_H
