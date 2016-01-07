#ifndef LOG_H
#define LOG_H

#include <string.h>
#include <stdlib.h>

//#ifdef __cplusplus
//extern "C"
//{
//#endif

#include <log4c.h>
//#ifdef __cplusplus
//}
//#endif

class Logger
{
public:
    int log_open(const char *category);
    void log_trace(const char *file , int line , const char *func, const char *fmt ,...);


    void Fatal(const char* fmt, ...);
    void Error(const char* fmt, ...);
    void Warning(const char* fmt, ...);
    void Notice(const char* fmt, ...);
    void Debug(const char* fmt, ...);
    void Info(const char* fmt, ...);
    void Trace(const char* fmt, ...);

    int log_close();
    static Logger* GetLogger();

private:
    Logger()
    {
    }

    ~Logger()
    {

    }
    static Logger* m_log4c;
};

#define LOG_PRI_FATAL       LOG4C_PRIORITY_FATAL
#define LOG_PRI_ERROR       LOG4C_PRIORITY_ERROR
#define LOG_PRI_WARN        LOG4C_PRIORITY_WARN
#define LOG_PRI_NOTICE      LOG4C_PRIORITY_NOTICE
#define LOG_PRI_DEBUG       LOG4C_PRIORITY_DEBUG
#define LOG_PRI_INFO        LOG4C_PRIORITY_INFO
#define LOG_PRI_TRACE       LOG4C_PRIORITY_TRACE

#endif // LOG_H
