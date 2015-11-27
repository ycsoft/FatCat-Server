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

#include "hf_types.h"

#include "md5.h"
#include "server.h"


#include "Game/session.hpp"
#include "Game/log.hpp"
#include "Game/userposition.hpp"

#include "fileOperation/fileoperation.h"


using namespace std;
using namespace hf_types;

#define UNITSIZE	1024*10
#define TIMES		20

#define MEMSIZE 4

const int maxlength = 10000;


int main()
{
//    fileOperation file;
//    file.ReadFile ("/home/hf02/桌面/heightmap.raw");
//    file.ReadFile("/home/hf02/桌面/aaaaaaaaaaaaaa");

    //数据库初始化
     Logger::GetLogger()->Info("Init DB ........");
     Server::GetInstance()->InitDB();
     Logger::GetLogger()->Debug("Init DB Finished!");

    //开始监听客户端连接
    Logger::GetLogger()->Info("Init Listen.......");
    Server::GetInstance()->InitListen();

	return 0;
}

