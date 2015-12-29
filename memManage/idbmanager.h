#ifndef IDBMANAGER_H
#define IDBMANAGER_H

#include <iostream>
#include <hiredis.h>
#include <libpq-fe.h>

#include "./../Game/cmdtypes.h"

using namespace std;

/**
 * @brief The IDBManager class
 * 内存管理抽象类
 */
class IDBManager
{
public:
    IDBManager();
    ~IDBManager();
    virtual bool Connect(Configuration con) = 0;
    virtual bool Connect() = 0;
    virtual bool Disconnect() = 0;
    virtual hf_int32 Set(const char* str,...) = 0;
    virtual void* Get(const char* str) = 0;
};

#endif // IDBMANAGER_H


