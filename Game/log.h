#ifndef LOG_H
#define LOG_H

#include "log/HFLogger.hpp"
class Logger
{
public:
    ~Logger(){}

    static HFLogger *GetLogger();

private:
    Logger() {}



};

#endif // LOG_H
