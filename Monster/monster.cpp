#include <cmath>
#include "memManage/diskdbmanager.h"
#include "utils/stringbuilder.hpp"
#include "Game/session.hpp"
#include "Game/log.hpp"
#include "monster.h"
#include "server.h"




void _STR_MonsterInfo::SetMonsterSpawns()
{
    umap_monsterSpawns* monsterSpawns = Server::GetInstance()->GetMonster()->GetMonsterSpawns();
    mtx.lock();
    umap_monsterSpawns::iterator it = monsterSpawns->find(monster.MonsterTypeID);
    monster.HP = monster.MaxHP;
    monster.Direct = 0;
    monster.ActID = 1;

    Server::GetInstance()->GetMonster()->CreateEffectivePos(&monster, &it->second); //生成怪物有效位置
    Flag = MonsterAlive;  //怪物复活
    mtx.unlock();
}


//计算玩家与怪物之间的距离
hf_float    Monster::caculateDistanceWithMonster( STR_PackPlayerPosition *usr,  STR_MonsterBasicInfo *monster)
{
    hf_float res = 0.0f;

    int dx = usr->Pos_x - monster->Current_x,
            dy = usr->Pos_y - monster->Current_y,
            dz = usr->Pos_z - monster->Current_z;
    res = sqrt(dx*dx + dy *dy + dz *dz);

    return res;
}

//推送新看到的怪物数据
void Monster::PushViewMonsters( TCPConnection::Pointer conn)
{
    Server* srv = Server::GetInstance();
    SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();
    STR_PackPlayerPosition* playerPosition =   &((*smap)[conn].m_position);
    umap_monsterBasicInfo t_viewMonster = (*smap)[conn].m_viewMonster;

    STR_PackHead t_packHead;
    t_packHead.Flag = FLAG_MonsterCome;

    STR_MonsterBasicInfo t_monsterInfo;

    hf_char* comebuff = (hf_char*)srv->malloc();
    hf_char* leavebuff = (hf_char*)srv->malloc();
    hf_uint16 pushcount = 0;
    hf_uint16 popcount = 0;

    for(_umap_monsterBasicInfo::iterator it = m_monsterBasic->begin(); it != m_monsterBasic->end(); it++)
    {
        memcpy(&t_monsterInfo, &(it->second), sizeof(STR_MonsterBasicInfo));

        if(t_monsterInfo.MapID != playerPosition->MapID)
        {
            break;
        }
        _umap_monsterBasicInfo::iterator monster = t_viewMonster->find(t_monsterInfo.MonsterID);
        if(monster == t_viewMonster->end()) //不存在
        {
            if ( caculateDistanceWithMonster(playerPosition,&t_monsterInfo) > PlayerView) //不在可视范围
            {
                continue;
            }
            (*t_viewMonster)[t_monsterInfo.MonsterID] = t_monsterInfo;  //保存为新看到的怪
            //将怪物可视范围内的玩家保存到怪物可视范围内
            _umap_roleSock* t_roleSock = &(*m_monsterViewRole)[t_monsterInfo.MonsterID];
            hf_uint32 roleid = (*smap)[conn].m_roleid;
            (*t_roleSock)[roleid] = conn;
            memcpy(comebuff + sizeof(STR_PackHead) + sizeof(STR_MonsterBasicInfo)*pushcount, &t_monsterInfo, sizeof(STR_MonsterBasicInfo));
            pushcount++;

            if(pushcount == (CHUNK_SIZE - sizeof(STR_PackHead))/sizeof(STR_MonsterBasicInfo))
            {
                t_packHead.Flag = FLAG_MonsterCome;
                t_packHead.Len = sizeof(STR_MonsterBasicInfo) * pushcount;
                memcpy(comebuff, &t_packHead, sizeof(STR_PackHead));

                //发送新看到的怪物
                conn->Write_all(comebuff, sizeof(STR_PackHead) + t_packHead.Len);
                pushcount = 0;
            }
        }
        else   //原来在可视范围，判断是否还在可视范围
        {
            if ( caculateDistanceWithMonster(playerPosition,&t_monsterInfo) <= PlayerView)
            {
                continue;
            }
            //保存离开视野范围的怪
            memcpy(leavebuff + sizeof(STR_PackHead) + popcount * sizeof(t_monsterInfo.MonsterID), &(t_monsterInfo.MonsterID), sizeof(t_monsterInfo.MonsterID));
            popcount++;

            _umap_roleSock* t_roleSock = &(*m_monsterViewRole)[t_monsterInfo.MonsterID];
            t_roleSock->erase((*smap)[conn].m_roleid);
            t_viewMonster->erase(monster);
        }
    }

    if(pushcount != (CHUNK_SIZE - sizeof(STR_PackHead))/sizeof(STR_MonsterBasicInfo) && pushcount != 0)
    {
        t_packHead.Len = pushcount * sizeof(STR_MonsterBasicInfo);
        t_packHead.Flag = FLAG_MonsterCome;
        memcpy(comebuff, &t_packHead, sizeof(STR_PackHead));
        conn->Write_all(comebuff, t_packHead.Len + sizeof(STR_PackHead));
    }

    //发送离开视野范围的怪物
    if(popcount != 0)
    {
        t_packHead.Len = popcount * sizeof(t_monsterInfo.MonsterID);
        t_packHead.Flag = FLAG_MonsterLeave;
        memcpy(leavebuff, &t_packHead, sizeof(STR_PackHead));
        conn->Write_all(leavebuff, t_packHead.Len + sizeof(STR_PackHead));
    }

    srv->free(comebuff);
    srv->free(leavebuff);
}

//从数据库中提取所有的怪物数据
void Monster::QueryMonstersAll()
{
    DiskDBManager *db = Server::GetInstance()->getDiskDB();

    hf_int32 count = db->GetMonsterSpawns(m_monsterSpawns);
    if ( count < 0 )
    {
        Logger::GetLogger()->Error("Query monster spawns error");
        return;
    }
    count = db->GetMonsterType(m_monsterType);
    if ( count < 0 )
    {
        Logger::GetLogger()->Error("Query monster type error");
        return;
    }   
}

//生成怪物
void Monster::CreateMonster()
{
    QueryMonstersAll();

    STR_MonsterBasicInfo t_MonsterBasicInfo;
    STR_MonsterAttackInfo    t_MonsterAttackInfo;
    hf_uint32 monsterID = 30000000;
    _umap_roleSock t_roleSock;
    for(umap_monsterSpawns::iterator it = m_monsterSpawns->begin(); it != m_monsterSpawns->end(); it++)
    {
        umap_monsterType::iterator iter = m_monsterType->find(it->second.MonsterTypeID);
        if(iter == m_monsterType->end())
        {
            continue;
        }

        t_MonsterBasicInfo.MonsterTypeID = iter->second.MonsterTypeID;
        memcpy(t_MonsterBasicInfo.MonsterName, iter->second.MonsterName, 32);
        t_MonsterBasicInfo.MapID = it->second.MapID;
        t_MonsterBasicInfo.MoveRate = iter->second.MoveRate;
        t_MonsterBasicInfo.HP = iter->second.HP;
        t_MonsterBasicInfo.MaxHP = iter->second.HP;
        t_MonsterBasicInfo.Direct = 50;
        t_MonsterBasicInfo.Level = iter->second.Level;
        t_MonsterBasicInfo.RankID = iter->second.RankID;
        t_MonsterBasicInfo.ActID = 1;
        t_MonsterBasicInfo.Flag = 1;

        t_MonsterAttackInfo.Hp = iter->second.HP;
        t_MonsterAttackInfo.Level = iter->second.Level;
        t_MonsterAttackInfo.PhysicalAttack = iter->second.PhysicalAttack;
        t_MonsterAttackInfo.MagicAttack = iter->second.MagicAttack;
        t_MonsterAttackInfo.PhysicalDefense = iter->second.PhysicalDefense;
        t_MonsterAttackInfo.MagicDefense = iter->second.MagicDefense;

        srand(time(NULL));
        for(int j = 0; j < it->second.Amount; j++)
        {
            //生成怪物有效位置
            CreateEffectivePos(&t_MonsterBasicInfo, &it->second);

            t_MonsterBasicInfo.MonsterID = monsterID;
            t_MonsterAttackInfo.MonsterID = monsterID;
            monsterID++;

            (*m_monsterBasic)[t_MonsterBasicInfo.MonsterID] = t_MonsterBasicInfo;
            (*m_monsterAttack)[t_MonsterAttackInfo.MonsterID] = t_MonsterAttackInfo;
            (*m_monsterSpawnsPos)[t_MonsterBasicInfo.MonsterID] = it->second.SpawnsPosID; //保存怪物刷怪点

            //给怪物可视范围内的玩家初始化为空_umap_roleSock
            (*m_monsterViewRole)[t_MonsterBasicInfo.MonsterID] = t_roleSock;
        }
    }
}

//保存死亡的怪
void Monster::SaveDeathMonster(hf_uint32 monsterID)
{
    umap_monsterSpawnsPos::iterator it = m_monsterSpawnsPos->find(monsterID);
    if(it == m_monsterSpawnsPos->end())
    {
        return;
    }
    time_t timep;
    time(&timep);
    STR_MonsterDeath t_monsterDeath;
    t_monsterDeath.SpawnsTime = (hf_uint32)timep + MonsterDeathTime;
    t_monsterDeath.MonsterID = monsterID;
    t_monsterDeath.SpawnsPos = it->second;
    (*m_monsterDeath)[t_monsterDeath.MonsterID] = t_monsterDeath;
}

//怪物复活
void Monster::MonsterSpawns()
{
    SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();
    time_t timep;
    while(1)
    {
        time(&timep);
        for(_umap_monsterDeath::iterator death_it = m_monsterDeath->begin(); death_it != m_monsterDeath->end();)
        {           
            if((hf_uint32)timep < death_it->second.SpawnsTime)
            {
//                hf_uint32 t_time = (hf_uint32)timep;
//                cout << "当前时间" << t_time << endl;
//                cout << "怪物复活时间" << death_it->second.SpawnsTime << endl;
                death_it++;
                continue;
            }
            umap_monsterSpawns::iterator it = m_monsterSpawns->find(death_it->second.SpawnsPos);
            if(it == m_monsterSpawns->end())
            {
                continue;
            }
            umap_monsterType::iterator iter = m_monsterType->find(it->second.MonsterTypeID);
            if(iter == m_monsterType->end())
            {
                continue;
            }
            STR_MonsterBasicInfo t_MonsterBasicInfo;
            t_MonsterBasicInfo.MonsterTypeID = iter->second.MonsterTypeID;
            memcpy(t_MonsterBasicInfo.MonsterName, iter->second.MonsterName, 32);
            t_MonsterBasicInfo.MapID = it->second.MapID;
            t_MonsterBasicInfo.MoveRate = iter->second.MoveRate;
            t_MonsterBasicInfo.HP = iter->second.HP;
            t_MonsterBasicInfo.MaxHP = iter->second.HP;
            t_MonsterBasicInfo.Direct = 50;
            t_MonsterBasicInfo.Level = iter->second.Level;
            t_MonsterBasicInfo.RankID = iter->second.RankID;
            t_MonsterBasicInfo.ActID = 1;
            t_MonsterBasicInfo.Flag = 1;

            CreateEffectivePos(&t_MonsterBasicInfo, &it->second); //生成怪物有效位置

            t_MonsterBasicInfo.MonsterID = death_it->first;
            (*m_monsterBasic)[t_MonsterBasicInfo.MonsterID] = t_MonsterBasicInfo;

            //查找怪物周围的玩家,给玩家发送新生成的怪物
            _umap_roleSock t_roleSock;
            hf_char* buff = (hf_char*)Server::GetInstance()->malloc();
            STR_PackHead t_packHead;
            t_packHead.Len = sizeof(STR_MonsterBasicInfo);
            t_packHead.Flag = FLAG_MonsterCome;
            memcpy(buff, &t_packHead, sizeof(STR_PackHead));
            memcpy(buff + sizeof(STR_PackHead), &t_MonsterBasicInfo, sizeof(STR_MonsterBasicInfo));

            for(SessionMgr::SessionMap::iterator role_it = smap->begin();role_it != smap->end(); role_it++)
            {
                STR_PackPlayerPosition* t_pos = &(role_it->second.m_position);
                if (caculateDistanceWithMonster(t_pos,&t_MonsterBasicInfo) > PlayerView)
                {
                    continue;
                }
                role_it->first->Write_all(buff, sizeof(STR_PackHead) + t_packHead.Len);
                t_roleSock[role_it->second.m_roleid] = role_it->first;
                (*(role_it->second.m_viewMonster))[t_MonsterBasicInfo.MonsterID] = t_MonsterBasicInfo;
            }

            cout << t_MonsterBasicInfo.MonsterID << "复活" << endl;
            cout << "Current_x:" << t_MonsterBasicInfo.Current_x << "Current_y:" << t_MonsterBasicInfo.Current_y << "Current_z:" << t_MonsterBasicInfo.Current_z << endl;

            (*m_monsterViewRole)[t_MonsterBasicInfo.MonsterID] = t_roleSock;
            Server::GetInstance()->free(buff);
            _umap_monsterDeath::iterator _death_it = death_it;
            death_it++;
            m_monsterDeath->erase(_death_it);
        }
        sleep(1);
    }   
}

//查询NPC信息
void Monster::QueryNpcInfo()
{
    DiskDBManager *db = Server::GetInstance()->getDiskDB();
    hf_int32 count = db->GetNPCInfo(m_npcInfo);
    if ( count < 0 )
    {
        Logger::GetLogger()->Error("Query npc info error");
    }
}

//查询怪物掉落物品
void Monster::QueryMonsterLoot()
{
    DiskDBManager *db = Server::GetInstance()->getDiskDB();
    hf_int32 count = db->GetMonsterLoot(m_monsterLoot);
    if ( count < 0 )
    {
        Logger::GetLogger()->Error("Query monster loot error");
    }
}

//生成怪物有效位置
void Monster::CreateEffectivePos(STR_MonsterBasicInfo* monsterInfo, STR_MonsterSpawns* monsterSpawns)
{
    hf_uint8 Flag = 1;
    while(Flag)
    {
        monsterInfo->Current_x = (monsterSpawns->Pos_x - monsterSpawns->Boundary) + (float)rand()/(float)RAND_MAX * (monsterSpawns->Boundary*2);
        monsterInfo->Current_y = monsterSpawns->Pos_y;
        monsterInfo->Current_z = (monsterSpawns->Pos_z - monsterSpawns->Boundary) + (float)rand()/RAND_MAX * (monsterSpawns->Boundary*2);
        hf_float dx = monsterInfo->Current_x - monsterSpawns->Pos_x;
        hf_float dz = monsterInfo->Current_z - monsterSpawns->Pos_z;
        if(dx*dx + dz*dz <= (monsterSpawns->Boundary)*(monsterSpawns->Boundary))
        {
            Flag = 0;
        }
    }
    Flag = 1;
    while(Flag)
    {
        monsterInfo->Target_x = (monsterSpawns->Pos_x - monsterSpawns->Boundary) + (float)rand()/RAND_MAX * (monsterSpawns->Boundary*2);
        monsterInfo->Target_y = monsterSpawns->Pos_y;
        monsterInfo->Target_z = (monsterSpawns->Pos_z - monsterSpawns->Boundary) + (float)rand()/RAND_MAX * (monsterSpawns->Boundary*2);
        hf_float dx = monsterInfo->Target_x - monsterSpawns->Pos_x;
        hf_float dz = monsterInfo->Target_z - monsterSpawns->Pos_z;
        if(dx*dx + dz*dz <= (monsterSpawns->Boundary)*(monsterSpawns->Boundary))
        {
            Flag = 0;
        }
    }
}

