/*
* logger.h
*
*  Created on:
*      Author: lijy
 */

//typedef enum {
//    /** fatal */    LOG4C_PRIORITY_FATAL    = 000,
//    /** alert */    LOG4C_PRIORITY_ALERT    = 100,
//    /** crit */              LOG4C_PRIORITY_CRIT    = 200,
//    /** error */    LOG4C_PRIORITY_ERROR    = 300,
//    /** warn */              LOG4C_PRIORITY_WARN    = 400,
//    /** notice */    LOG4C_PRIORITY_NOTICE    = 500,
//    /** info */              LOG4C_PRIORITY_INFO    = 600,
//    /** debug */    LOG4C_PRIORITY_DEBUG    = 700,
//    /** trace */    LOG4C_PRIORITY_TRACE    = 800,
//    /** notset */    LOG4C_PRIORITY_NOTSET    = 900,
//    /** unknown */    LOG4C_PRIORITY_UNKNOWN    = 1000
//} log4c_priority_level_t;

#ifndef LOGGER_H
#define LOGGER_H

#include "log4c.h"

#define LOG_ERROR(category, msg, args...) \
    { \
    const log4c_location_info_t locinfo = LOG4C_LOCATION_INFO_INITIALIZER(NULL);\
    log4c_category_log_locinfo(mycat, &locinfo, LOG4C_PRIORITY_ERROR, msg, ##args); \
    }

#define LOG_WARN(category, msg, args...) \
    { \
    const log4c_location_info_t locinfo = LOG4C_LOCATION_INFO_INITIALIZER(NULL);\
    log4c_category_log_locinfo(mycat, &locinfo, LOG4C_PRIORITY_WARN, msg, ##args); \
    }

#define LOG_INFO(category, msg, args...) \
    { \
    const log4c_location_info_t locinfo = LOG4C_LOCATION_INFO_INITIALIZER(NULL); \
    log4c_category_log_locinfo(mycat, &locinfo, LOG4C_PRIORITY_INFO, msg, ##args); \
    }

#define LOG_DEBUG(category, msg, args...) \
    { \
    const log4c_location_info_t locinfo = LOG4C_LOCATION_INFO_INITIALIZER(NULL); \
    log4c_category_log_locinfo(mycat, &locinfo, LOG4C_PRIORITY_DEBUG, msg, ##args); \
    }

#define LOG_TRACE(category, msg, args...) \
    { \
    const log4c_location_info_t locinfo = LOG4C_LOCATION_INFO_INITIALIZER(NULL);\
    log4c_category_log_locinfo(mycat, &locinfo, LOG4C_PRIORITY_TRACE, msg, ##args); \
    }

#endif /* LOGGER_H */
