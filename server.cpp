
#include <cstdlib>
#include "NetWork/tcpserver.h"

#include "memManage/diskdbmanager.h"
#include "memManage/memdbmanager.h"

#include "Game/log.hpp"
#include "Monster/monster.h"
#include "PlayerLogin/playerlogin.h"
#include "GameTask/gametask.h"
#include "GameAttack/gameattack.h"
#include "PlayerLogin/playerlogin.h"
#include "TeamFriend/teamfriend.h"
#include "GameInterchange/gameinterchange.h"
#include "OperationGoods/operationgoods.h"
#include "OperationPostgres/operationpostgres.h"

#include "server.h"

Server* Server::m_instance = NULL;


Server::Server() :
    m_MemDB(new MemDBManager),
    m_DiskDB(new DiskDBManager),
    m_task_pool(20),
    m_memory_factory(CHUNK_SIZE,CHUNK_COUNT),
    m_monster( new Monster()),
    m_playerLogin( new PlayerLogin),
    m_gameTask(new GameTask),
    m_teamFriend(new TeamFriend),
    m_gameAttack(new GameAttack),
    m_gameInterchange(new GameInterchange),
    m_operationGoods(new OperationGoods),
    m_operationPostgres(new OperationPostgres)
{

}

Server::~Server()
{

}


void Server::InitDB()
{
    Configuration Diskcon;
    Configuration MemCon;

    memset(&Diskcon, 0, sizeof(Diskcon));
    memset(&MemCon, 0, sizeof(MemCon));
    Diskcon.ip = "127.0.0.1";
//    Diskcon.ip = "10.183.100.13";
    Diskcon.port = "5432";
    Diskcon.dbName = "my_database";
    Diskcon.user = "postgres";
    Diskcon.password = "postgres";

    MemCon.ip = "127.0.0.1";
    MemCon.port = "6379";

   if(!m_DiskDB->Connect(Diskcon))
   {
        Logger::GetLogger()->Debug("Connect postgres error");
   }

//   if(!m_MemDB->Connect(MemCon))
//   {
//       Logger::GetLogger()->Debug("Connect redis errorr");
//   }

   m_monster->CreateMonster();
   m_monster->QueryMonsterLoot();
   m_monster->QueryNpcInfo();
   m_gameTask->QueryTaskData();
   m_gameAttack->QuerySkillInfo();
   m_operationGoods->QueryGoodsPrice();
   m_operationGoods->QueryEquAttr();
   m_operationGoods->SetEquIDInitialValue();

}

//
//Two threads to provide network io
//
void Server::InitListen()
{
    boost::asio::io_service io;

    TCPServer netsrv(io);
    netsrv.Start();
    boost::thread_group group;
    group.create_thread(boost::bind(& boost::asio::io_service::run, &io));
    group.create_thread(boost::bind(& boost::asio::io_service::run, &io));
    group.join_all();
}


Server* Server::GetInstance()
{

    if ( NULL == m_instance)
    {
        m_instance = new Server();
    }
    return m_instance;
}

