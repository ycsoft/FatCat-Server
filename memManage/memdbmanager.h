#ifndef MEMDBMANAGER_H
#define MEMDBMANAGER_H

#include "idbmanager.h"

/**
 * @brief The MemDBManager class
 * 操作redis结构体
 */
class MemDBManager : public IDBManager
{
public:
    MemDBManager();
    ~MemDBManager();
    bool Connect(Configuration con);
    bool Connect();
    bool Disconnect();
    /**
     * @brief Set
     * @param str
     * @return
     */
    hf_int32 Set(const char* str,...);
    void* SetA(const char *str);
    void* SetB(const char *key, void *value, int len);

    /**
     * @brief Set
     * @param str
     * @param ptr
     * @param len
     * @return
     * 用于存储二进制数据类型
     */
    //void* Set(const char *str,void *ptr,size_t len);
    /**
     * @brief Get  向redis中提取数据
     * @param str  get语句
     * @return
     */
    void* Get(const char* str);

    bool GetDbData(Configuration DiskCon, Configuration MemCon);
private:
    redisContext* m_redis;
};

#endif // MEMDBMANAGER_H
