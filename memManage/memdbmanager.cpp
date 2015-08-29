#include "memdbmanager.h"
#include <stdlib.h>
#include "diskdbmanager.h"
#include <stdio.h>
#include <strings.h>
#include <string.h>


MemDBManager::MemDBManager()
{

}

MemDBManager::~MemDBManager()
{

}

bool MemDBManager::Connect(Configuration con)
{
    m_redis = redisConnect(con.ip, atoi(con.port));
    if(m_redis == NULL || m_redis->err)
    {
        printf("Error: %s\n", m_redis->errstr);
        redisFree(m_redis);
        return false;
    }
    else
        return true;
}

bool MemDBManager::Disconnect()
{
    redisFree(m_redis);
    return true;
}

//可变参
//void *MemDBManager::Set(const char *format,...)
//{
//    va_list ap;
//    void *reply = NULL;
//    va_start(ap,format);
//    reply = redisvCommand(m_redis,format,ap);
//    va_end(ap);
//    return reply;
//}

//void *MemDBManager::SetB(const char *key, void *value, int len)
//{
//    char buf[256] = {0};
////    return Set("LPUSH %s %b",key,value,len);
//    return Set("SET %s %b",key,value,len);
//}

hf_int32 MemDBManager::Set(const char *format,...)
{
//    return 0;
}

void* MemDBManager::SetA(const char* setStr)
{
//    redisReply* t_reply = (redisReply*)redisCommand(m_redis, setStr);
//    Result* t_result = new Result;
//    if(!(t_reply->type == REDIS_REPLY_STATUS && strcasecmp(t_reply->str, "OK") == 0))
//    {
//       printf("redisCommand %s error\n",setStr);
//       freeReplyObject(t_reply);
//       redisFree(m_redis);
//       t_result->result = false;
//       return t_result;
//    }
//    else
//    {
//        freeReplyObject(t_reply);
//        t_result->result = true;
//        return t_result;
//    }
}


void* MemDBManager::Get(const char* getStr)
{
//    redisReply* t_reply = (redisReply*)redisCommand(m_redis, getStr);
//    Result* t_result = new Result;
//    if(t_reply == NULL || t_reply->type == REDIS_REPLY_ERROR)
//    {
//        printf("redisCommand %s error\n",getStr);
//        freeReplyObject(t_reply);
//        redisFree(m_redis);
//        t_result->result = false;
//        return t_result;
//    }
//    else if(t_reply->type == REDIS_REPLY_NIL)
//    {
//        printf("key value does not exist\n");
//        freeReplyObject(t_reply);
//        t_result->result = false;
//        return t_result;
//    }
//    else if(t_reply->type == REDIS_REPLY_STRING) //字符串
//    {
//        int t_length = t_reply->len;
//        char* t_res = new char[t_length + sizeof(Result) + 1];
//        if(t_res == NULL)
//        {
//            t_result->result = false;
//            freeReplyObject(t_reply);
//            return t_result;
//        }
//        else
//        {
//            t_result->result = true;
//            memcpy(t_res, t_result,sizeof(Result));
//            memcpy(t_res, t_reply->str, t_length);
//            freeReplyObject(t_reply);
//            delete t_result;
//            return t_res;
//        }
//    }
//    else if(t_reply->type == REDIS_REPLY_INTEGER) //整数
//    {

//        long long num = t_reply->integer;
//        char* t_res = new char[sizeof(num) + sizeof(Result) + 1];
//        if(t_res == NULL)
//        {
//            t_result->result = false;
//            freeReplyObject(t_reply);
//            return t_result;
//        }
//        else
//        {
//            t_result->result = true;
//            memset(t_res, 0, sizeof(num) + sizeof(Result) + 1);
//            memcpy(t_res, t_result, sizeof(Result));
//            memcpy(t_res+sizeof(Result), &num, sizeof(num));
//            freeReplyObject(t_reply);
//            delete t_result;
//            return t_res;
//        }
//    }
//    else if(t_reply->type == REDIS_REPLY_ARRAY) //数组
//    {
//        for(int i = 0; i < t_reply->elements; i++)
//            redisReply* childReply = r_element[i];
//        if(childReply->type ==)

//    }
}

//bool MemDBManager::GetDbData(Configuration DiskCon, Configuration MemCon)
//{
//    DiskDBManager t_disk;
//    if(!t_disk.Connect(DiskCon))
//        return false;
//    if(!Connect(MemCon))
//        return false;
////    People   p;
////    p.age = 10;
////    strcpy(p.name,"Yang");
//    const char* str = (const char*)t_disk.Get("select * from test;");
//    Result t_result;
//    memset(&t_result, 0, sizeof(t_result));
//    memcpy(&t_result, str, sizeof(t_result));
//    char* t_colname = (char*)malloc(20);
//    char* t_colvalue = (char*)malloc(20);
//    char* t_str = (char*)malloc(50);

//    char cmd[128] = {0};
//    char buf[256] = {0};
//    string   s;
////    memcpy(buf,&p,sizeof(People));

////    Set("SET yang %b",(void*)&p,sizeof(p));
////    SetB("yang2",&p,sizeof(p));
////    p.age = 11;
////    SetB("yang2",&p,sizeof(p));
//    //Get
////    People *ptr = (People*)Get("RPOP yang2");
////    ptr = (People*)Get("RPOP yang2");
////    ptr = (People*)Get("RPOP yang2");

//    if(t_result.result == true)
//    {
//        str += sizeof(t_result);
//        printf("%s\n", str);
//        int num = strlen(str);
//        while(num > 0)
//        {
//            memcpy(t_colname, str, 2);
//            str += 2;
//            memcpy(t_colvalue, str, 1);
//            str += 1;
//            sprintf(t_str, "SET %s %s", t_colname, t_colvalue);

//            memset(&t_result, 0, sizeof(t_result));
//            memcpy(&t_result, Set(t_str), sizeof(t_result));
//            if(t_result.result == false)
//                break;

//            memcpy(t_colname, str, 3);
//            str += 3;
//            memcpy(t_colvalue, str, 2);
//            str += 2;
//            sprintf(t_str, "SET %s %s", t_colname, t_colvalue);

//            memset(&t_result, 0, sizeof(t_result));
//            memcpy(&t_result, Set(t_str), sizeof(t_result));
//            if(t_result.result == false)
//                break;
//            num -= 8;
//        }
//        free(t_colname);
//        free(t_colvalue);
//        free(t_str);
//        return t_result.result;
//    }
//    else
//    {
//        free(t_colname);
//        free(t_colvalue);
//        free(t_str);
//        return t_result.result;
//    }
//}
