#include <log4c.h>
#include <assert.h>
#include "log.h"


static log4c_category_t *log_category = NULL;

Logger* Logger::m_log4c = NULL;
Logger* Logger::GetLogger()
{
    if(m_log4c == NULL)
    {
        m_log4c = new Logger;
       printf("new logger!\n");
    }
    return m_log4c;
}
int Logger::log_open(const char *category)
{
    if (log4c_init() == 1)
    {
        return -1;
    }
    log_category = log4c_category_get(category);
    return 0 ;
}

void Logger::log_trace(const char *file, int line, const char *fun,
            const char *fmt , ...)
{
    char new_fmt[2048];
    const char *head_fmt = "[file:%s, line:%d, function:%s]";
    va_list ap;
    int n;

    assert(log_category != NULL);
    n = sprintf(new_fmt, head_fmt , file , line , fun);
    strcat(new_fmt + n , fmt);

    va_start(ap , fmt);
    log4c_category_vlog(log_category , LOG4C_PRIORITY_TRACE, new_fmt , ap);
    va_end(ap);
}

void Logger::Fatal(const char* fmt, ...)
{
    va_list ap;
    assert(log_category != NULL);
    va_start(ap, fmt);
    log4c_category_vlog(log_category , LOG_PRI_FATAL , fmt , ap);
    va_end(ap);
}

void Logger::Error(const char* fmt, ...)
{
    va_list ap;
    assert(log_category != NULL);
    va_start(ap, fmt);
    log4c_category_vlog(log_category , LOG_PRI_ERROR , fmt , ap);
    va_end(ap);
}

void Logger::Warning(const char* fmt, ...)
{
    va_list ap;
    assert(log_category != NULL);
    va_start(ap, fmt);
    log4c_category_vlog(log_category , LOG_PRI_WARN , fmt , ap);
    va_end(ap);
}

void Logger::Notice(const char* fmt, ...)
{
    va_list ap;
    assert(log_category != NULL);
    va_start(ap, fmt);
    log4c_category_vlog(log_category , LOG_PRI_NOTICE , fmt , ap);
    va_end(ap);
}

void Logger::Info(const char* fmt, ...)
{
    va_list ap;

    assert(log_category != NULL);

    va_start(ap, fmt);
    log4c_category_vlog(log_category , LOG_PRI_INFO , fmt , ap);
    va_end(ap);
}

void Logger::Debug(const char* fmt, ...)
{
    va_list ap;
    assert(log_category != NULL);
    va_start(ap, fmt);
    log4c_category_vlog(log_category , LOG_PRI_DEBUG , fmt , ap);
    va_end(ap);
}

void Logger::Trace(const char* fmt, ...)
{
    va_list ap;
    assert(log_category != NULL);
    va_start(ap, fmt);
    log4c_category_vlog(log_category , LOG_PRI_TRACE , fmt , ap);
    va_end(ap);
}


int Logger::log_close()
{
    return (log4c_fini());
}





