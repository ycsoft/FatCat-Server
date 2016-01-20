
#include <cstdlib>
#include "NetWork/tcpserver.h"

#include "memManage/diskdbmanager.h"
#include "memManage/memdbmanager.h"

#include "Game/log.h"
#include "Monster/monster.h"
#include "PlayerLogin/playerlogin.h"
#include "GameTask/gametask.h"
#include "GameAttack/gameattack.h"
#include "PlayerLogin/playerlogin.h"
#include "TeamFriend/teamfriend.h"
#include "GameInterchange/gameinterchange.h"
#include "OperationGoods/operationgoods.h"
#include "OperationPostgres/operationpostgres.h"
#include "Game/userposition.hpp"
#include "GameChat/gamechat.h"
#include "Game/cmdparse.h"

#include "server.h"

Server* Server::m_instance = NULL;


Server::Server() :
    m_MemDB(new MemDBManager),
    m_DiskDB(new DiskDBManager),
    m_task_pool(ThreadCount),
    m_pack_pool(2),
    m_memory_factory(CHUNK_SIZE,CHUNK_COUNT),
    m_monster( new Monster()),
    m_playerLogin( new PlayerLogin),
    m_gameTask(new GameTask),
    m_teamFriend(new TeamFriend),
    m_gameAttack(new GameAttack),
    m_gameInterchange(new GameInterchange),
    m_operationGoods(new OperationGoods),
    m_operationPostgres(new OperationPostgres),
//    m_cmdParse(new CmdParse),
    m_gameChat(new GameChat),
    m_package(new boost::lockfree::queue<STR_Package>(PackageCount))
{
    _pop_times = 0;
    _push_times = 0;
}

Server::~Server()
{
    delete m_MemDB;
    delete m_DiskDB;
    delete m_monster;
    delete m_playerLogin;
    delete m_gameTask;
    delete m_teamFriend;
    delete m_gameAttack;
    delete m_gameInterchange;
    delete m_operationGoods;
    delete m_operationPostgres;
//    delete m_cmdParse;
    delete m_gameChat;
}


void Server::InitDB()
{
//    Configuration Diskcon;
    Configuration MemCon;

//    memset(&Diskcon, 0, sizeof(Diskcon));
    memset(&MemCon, 0, sizeof(MemCon));
//    Diskcon.ip = "139.196.165.107";
//    Diskcon.ip = "10.183.100.13";
//    Diskcon.port = "5433";
//    Diskcon.dbName = "game";
//    Diskcon.user = "game";
//    Diskcon.password = "houfang2015";

    MemCon.ip = "127.0.0.1";
    MemCon.port = "6379";

//   if(!m_DiskDB->Connect(Diskcon))
//   {
//        Logger::GetLogger()->Debug("Connect postgres error");
//   }

   if(!m_DiskDB->Connect())
   {
       Logger::GetLogger()->Debug("Connect postgres error");
   }
   if(!m_MemDB->Connect(MemCon))
   {
       Logger::GetLogger()->Debug("Connect redis errorr");
   }

   m_monster->CreateMonster();
   m_monster->QueryMonsterLoot();
   m_monster->QueryNpcInfo();
   m_gameTask->QueryTaskData();
   m_gameAttack->QuerySkillInfo();
   m_operationGoods->QueryGoodsPrice();
   m_operationGoods->QueryConsumableAttr();
   m_operationGoods->QueryEquAttr();
   m_operationGoods->SetEquIDInitialValue();
   m_playerLogin->QueryRoleJobAttribute();


   Server  *srv = Server::GetInstance();
   GameAttack* t_attack = srv->GetGameAttack();
   Monster* t_monster = srv->GetMonster();
   OperationPostgres* t_opePost = srv->GetOperationPostgres();

   srv->RunTask(boost::bind(&Server::PopPackage, srv));
//   srv->RunTask(boost::bind(&Server::PopPackage, srv));
//   srv->RunTask(boost::bind(&Server::PopPackage, srv));
//   srv->RunTask(boost::bind(&Server::PopPackage, srv));
//   srv->RunTask(boost::bind(&Server::PopPackage, srv));

   srv->RunTask(boost::bind(&Monster::Monsteractivity, t_monster));
   //技能伤害线程
   srv->RunTask(boost::bind(&GameAttack::RoleSkillAttack, t_attack));
   //删除过了时间的掉落物品
   srv->RunTask(boost::bind(&GameAttack::DeleteOverTimeGoods, t_attack));
   //操作数据库更新用户数据
   srv->RunTask(boost::bind(&OperationPostgres::UpdatePostgresData, t_opePost));
}

//
//Two threads to provide network io
//
void Server::InitListen()
{
    boost::asio::io_service io;

    TCPServer netsrv(io);
    netsrv.Start();
    io.run();
//    boost::thread_group group;
//    group.create_thread(boost::bind(& boost::asio::io_service::run, &io));
//    group.create_thread(boost::bind(& boost::asio::io_service::run, &io));
//    group.join_all();
}


Server* Server::GetInstance()
{

    if ( NULL == m_instance)
    {
        m_instance = new Server();
    }
    return m_instance;
}

void Server::PopPackage()
{
    STR_Package pack;
    umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
    while(1)
    {
        if(m_package->pop(pack))
        {
            _pop_times += 1;
            TCPConnection::Pointer conn = (*t_roleSock)[pack.roleid];
            if(conn == NULL)
            {
                usleep(1);
                continue;
            }
            else
            {
                //CommandParse(conn, pack.data);
                RunPackTask(boost::bind(&CommandParse,conn,pack.data));
            }
        }
        else
        {
            usleep(1);
        }
    }
}
