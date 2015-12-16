/*
 * main.cpp
 *
 *  Created on: 2015年4月26日
 *      Author: yang
 */

#include <iostream>
#include <sys/time.h>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <hash_map>
#include <unistd.h>
#include <iostream>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "server.h"
#include "Game/log.hpp"

//#include "fileOperation/fileoperation.h"
//#include "Game/log.hpp"

//using namespace std;
//using namespace hf_types;

//#define UNITSIZE	1024*10
//#define TIMES		20

//#define MEMSIZE 4

//const int maxlength = 10000;


int main()
{
//    fileOperation file;
//    file.ReadFile ("/home/hf02/桌面/heightmap.raw");
//    file.ReadFile("/home/hf02/桌面/aaaaaaaaaaaaaa");

//    log4c::GetLog4c()->log_open("mycat");
//    LOG_TRACE("trace");
//    LOG_ERROR("error");
//    LOG_WARN("warn");
//    LOG_NOTICE("notice");
//    LOG_DEBUG("hello log4c!");

    //数据库初始化
//     Logger::GetLogger()->log_open("mycat");
     Logger::GetLogger()->Info("Init DB ........");
     Server::GetInstance()->InitDB();
     Logger::GetLogger()->Debug("Init DB Finished!");

    //开始监听客户端连接
    Logger::GetLogger()->Info("Init Listen.......");
    Server::GetInstance()->InitListen();

//    Logger::GetLogger()->log_close();
	return 0;
}

