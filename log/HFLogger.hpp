#ifndef HFLOGGER_H
#define HFLOGGER_H


#define LOG4CPP_HAVE_SNPRINTF

#if defined(_MSC_VER)
    #define VSNPRINTF _vsnprintf
#else
#ifdef LOG4CPP_HAVE_SNPRINTF
    #define VSNPRINTF vsnprintf
#else
/* use alternative snprintf() from http://www.ijs.si/software/snprintf/ */

#define HAVE_SNPRINTF
#define PREFER_PORTABLE_SNPRINTF

#include <stdlib.h>
#include <stdarg.h>
#include <string>

#define VSNPRINTF portable_vsnprintf

#endif // LOG4CPP_HAVE_SNPRINTF
#endif // _MSC_VER

#include <log4cpp/Category.hh>
#include <log4cpp/PropertyConfigurator.hh>

class HFLogger
{
public:
    ~HFLogger() {}

    static HFLogger *GetLogger()
    {
        static HFLogger log;
        return &log;
    }
    void Fatal(const char* fmt, ...)
    {
        va_list ap;
        va_start(ap,fmt);
        std::string msg = vform(fmt,ap);
        __log->fatal(msg);
        va_end(ap);

    }
    void Error(const char* fmt, ...)
    {
        va_list ap;
        va_start(ap,fmt);
        std::string msg = vform(fmt,ap);
        __log->error(msg);
        va_end(ap);

    }
    void Warning(const char* fmt, ...)
    {
        va_list ap;
        va_start(ap,fmt);
        std::string msg = vform(fmt,ap);
        __log->warn(msg);
        va_end(ap);

    }
    void Notice(const char* fmt, ...)
    {
        va_list ap;
        va_start(ap,fmt);
        std::string msg = vform(fmt,ap);
        __log->notice(msg);
        va_end(ap);

    }
    void Debug(const char* fmt, ...)
    {
        va_list ap;
        va_start(ap,fmt);
        std::string msg = vform(fmt,ap);
        __log->debug(msg);
        va_end(ap);

    }
    void Info(const char* fmt, ...)
    {
        va_list ap;
        va_start(ap,fmt);
        std::string msg = vform(fmt,ap);
        __log->info(msg);
        va_end(ap);

    }

private:

    HFLogger()
    {
        log4cpp::PropertyConfigurator::configure("log4cpp.properties");
        __log = &(log4cpp::Category::getInstance(std::string("houfang")));
    }

    std::string vform(const char *format, va_list args)
    {
        size_t size = 1024;
               char* buffer = new char[size];
               while (1) {
                   va_list args_copy;

       #if defined(_MSC_VER) || defined(__BORLANDC__)
                   args_copy = args;
       #else
                   va_copy(args_copy, args);
       #endif

                   int n = VSNPRINTF(buffer, size, format, args_copy);

                   va_end(args_copy);

                   // If that worked, return a string.
                   if ((n > -1) && (static_cast<size_t>(n) < size)) {
                       std::string s(buffer);
                       delete [] buffer;
                       return s;
                   }

                   // Else try again with more space.
                   size = (n > -1) ?
                       n + 1 :   // ISO/IEC 9899:1999
                       size * 2; // twice the old size

                   delete [] buffer;
                   buffer = new char[size];
               }
    }

    log4cpp::Category*  __log;

};

#endif // HFLOGGER_H
