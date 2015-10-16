#include <cmath>
#include "memManage/diskdbmanager.h"
#include "utils/stringbuilder.hpp"
#include "Game/session.hpp"
#include "Game/log.hpp"
#include "monster.h"
#include "server.h"
#include "GameAttack/gameattack.h"

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
    umap_playerViewMonster t_playerViewMonster = (*smap)[conn].m_viewMonster;
    STR_PackHead t_packHead;
    t_packHead.Flag = FLAG_MonsterCome;

//    STR_MonsterBasicInfo t_monsterInfo;

    hf_char* comebuff = (hf_char*)srv->malloc();
    hf_char* leavebuff = (hf_char*)srv->malloc();
    hf_uint16 pushcount = 0;
    hf_uint16 popcount = 0;

    for(_umap_monsterInfo::iterator it = m_monsterBasic->begin(); it != m_monsterBasic->end(); it++)
    {
//        memcpy(&t_monsterInfo, &(it->second.monster), sizeof(STR_MonsterBasicInfo));

        hf_uint32 monsterID = it->second.monster.MonsterID;
        if(it->second.monster.MapID != playerPosition->MapID)
        {
            break;
        }       
         hf_float t_distance = caculateDistanceWithMonster(playerPosition, &it->second.monster);
         _umap_playerViewMonster::iterator monster = t_playerViewMonster->find(monsterID);
        if(monster == t_playerViewMonster->end()) //不存在
        {          
            if (t_distance > PlayerView) //不在可视范围
            {
                continue;
            }
            (*t_playerViewMonster)[monsterID] = monsterID;  //保存为新看到的怪
            //在怪物可视范围内的玩家保存到怪物可视范围内

            _umap_viewRole* t_roleSock = &(*m_monsterViewRole)[monsterID];
            hf_uint32 roleid = (*smap)[conn].m_roleid;

            STR_MonsterViewRole t_viewRole(conn);
            (*t_roleSock)[roleid] = t_viewRole;

            memcpy(comebuff + sizeof(STR_PackHead) + sizeof(STR_MonsterBasicInfo)*pushcount, &it->second.monster, sizeof(STR_MonsterBasicInfo));
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
            if (t_distance <= PlayerView)
            {
                continue;
            }

            _umap_viewRole* t_roleSock = &(*m_monsterViewRole)[monsterID];
            t_roleSock->erase((*smap)[conn].m_roleid);

            t_playerViewMonster->erase(monsterID);
            //保存离开视野范围的怪
            memcpy(leavebuff + sizeof(STR_PackHead) + popcount * sizeof(monsterID), &monsterID, sizeof(monsterID));
            popcount++;           
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
        t_packHead.Len = popcount * 4;
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

    STR_MonsterInfo t_monsterInfo;
    STR_MonsterAttackInfo    t_MonsterAttackInfo;
    hf_uint32 monsterID = 30000000;
    _umap_viewRole t_roleSock;
    hf_double currentTime = GetCurrentTime();
    for(umap_monsterSpawns::iterator it = m_monsterSpawns->begin(); it != m_monsterSpawns->end(); it++)
    {
        umap_monsterType::iterator iter = m_monsterType->find(it->second.MonsterTypeID);
        if(iter == m_monsterType->end())
        {
            continue;
        }

        memset(&t_monsterInfo, 0, sizeof(STR_MonsterInfo));
        t_monsterInfo.monster.MonsterTypeID = iter->second.MonsterTypeID;
        memcpy(t_monsterInfo.monster.MonsterName, iter->second.MonsterName, 32);
        t_monsterInfo.monster.MapID = it->second.MapID;
        t_monsterInfo.monster.MoveRate = iter->second.MoveRate;
        t_monsterInfo.monster.MaxHP = iter->second.HP;
        t_monsterInfo.monster.Level = iter->second.Level;
        t_monsterInfo.monster.RankID = iter->second.RankID;
        t_monsterInfo.monster.Flag = 1;

        t_MonsterAttackInfo.HP = iter->second.HP;
        t_MonsterAttackInfo.Level = iter->second.Level;
        t_MonsterAttackInfo.PhysicalAttack = iter->second.PhysicalAttack;
        t_MonsterAttackInfo.MagicAttack = iter->second.MagicAttack;
        t_MonsterAttackInfo.PhysicalDefense = iter->second.PhysicalDefense;
        t_MonsterAttackInfo.MagicDefense = iter->second.MagicDefense;

        t_MonsterAttackInfo.MonsterID = t_monsterInfo.monster.MonsterTypeID;
        (*m_monsterAttack)[t_monsterInfo.monster.MonsterTypeID] = t_MonsterAttackInfo;
        srand(time(NULL));
        for(int j = 0; j < it->second.Amount; j++)
        {
            //生成怪物有效位置
            MonsterSpawns(&t_monsterInfo, &it->second);

            t_monsterInfo.monster.MonsterID = monsterID;            
            hf_float dx = t_monsterInfo.monster.Target_x - t_monsterInfo.monster.Current_x;
            hf_float dz = t_monsterInfo.monster.Target_z - t_monsterInfo.monster.Current_z;

            t_monsterInfo.monster.Direct = CalculationDirect(dx, dz);

            t_monsterInfo.aimTime = currentTime + sqrt(dx*dx + dz*dz) / ((hf_double)t_monsterInfo.monster.MoveRate/100 * MonsterMoveDistance);


            (*m_monsterBasic)[monsterID] = t_monsterInfo;

             monsterID++;

            //给怪物可视范围内的玩家初始化为空_umap_roleSock
            (*m_monsterViewRole)[t_monsterInfo.monster.MonsterID] = t_roleSock;

        }
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

//怪物运动
void Monster::Monsteractivity()
{
    SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();
    umap_monsterInfo t_monsterBasic = Server::GetInstance()->GetMonster()->GetMonsterBasic();
    umap_monsterSpawns* monsterSpawns = Server::GetInstance()->GetMonster()->GetMonsterSpawns();
    umap_monsterViewRole  monsterViewRole = Server::GetInstance()->GetMonster()->GetMonsterViewRole();

    hf_char* buff = (hf_char*)Server::GetInstance()->malloc();
    STR_PackHead t_packHead;
    t_packHead.Len = sizeof(STR_MonsterBasicInfo);
    t_packHead.Flag = FLAG_MonsterCome;
    memcpy(buff, &t_packHead, sizeof(STR_PackHead));

    umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();

    while(1)
    {
        hf_double currentTime = GetCurrentTime();
        for(_umap_monsterInfo::iterator it = t_monsterBasic->begin(); it != t_monsterBasic->end(); it++)
        {
            if(currentTime >= it->second.aimTime)//怪物复活时间到了
            {
                STR_MonsterSpawns* t_monsterSpawns = &(*monsterSpawns)[it->second.spawnsPos];
                if(it->second.monster.HP == 0 )
                {
                    MonsterSpawns(&it->second, t_monsterSpawns); //怪物复活

                    //查找怪物周围的玩家,给玩家发送新生成的怪物
                    _umap_roleSock t_roleSock;
                    memcpy(buff + sizeof(STR_PackHead), &it->second.monster, sizeof(STR_MonsterBasicInfo));
                    for(SessionMgr::SessionMap::iterator role_it = smap->begin(); role_it != smap->end(); role_it++)
                    {
                        STR_PackPlayerPosition* t_pos = &(role_it->second.m_position);
                        if (caculateDistanceWithMonster(t_pos, &it->second.monster) > PlayerView)
                        {
                            continue;
                        }
                        role_it->first->Write_all(buff, sizeof(STR_PackHead) + t_packHead.Len);
                        t_roleSock[role_it->second.m_roleid] = role_it->first;
                        (*(role_it->second.m_viewMonster))[it->first] = it->first;

                        STR_MonsterViewRole t_viewRole(role_it->first);
                        ((*monsterViewRole)[it->first])[role_it->second.m_roleid] = t_viewRole;
                    }
                 }
                else
                {
                    STR_MonsterBasicInfo* t_monster = &(it->second.monster);
                    if(it->second.hatredRoleid != 0)  //怪物有追击目标
                    {
                        _umap_roleSock::iterator role_it = t_roleSock->find(it->second.hatredRoleid);
                        t_monster->Current_x = t_monster->Target_x;
                        t_monster->Current_z = t_monster->Target_z;

                        if(role_it != t_roleSock->end())
                        {
                            STR_PackPlayerPosition*  t_playerPos = &(*smap)[role_it->second].m_position;
                            hf_float dx = t_playerPos->Pos_x - t_monster->Current_x;
                            hf_float dz = t_playerPos->Pos_z - t_monster->Current_z;
                            hf_float dis = sqrt(dx*dx + dz*dz);
                            hf_float direct = CalculationDirect(dx, dz);
                            if(dis >= PursuitFarDistance*2) //距离较远
                            {
                                t_monster->Target_x += PursuitFarDistance*cos(direct);
                                t_monster->Target_z += PursuitFarDistance*sin(direct);
                            }
                            else if(dis < PursuitFarDistance && dis > PursuitNearlyDistance)
                            {
                                t_monster->Target_x += PursuitNearlyDistance*cos(direct);
                                t_monster->Target_z += PursuitNearlyDistance*sin(direct);
                            }
                            else //攻击玩家
                            {
                                hf_uint32 roleid = (*smap)[role_it->second].m_roleid;
                                hf_uint32 hp = (*m_monsterAttack)[t_monster->MonsterTypeID].HP;
                                cout << "玩家血量减少" << hp << endl;
                                STR_RoleAttribute t_roleAttr(roleid, hp);
                                it->second.aimTime += MonsterAttackSpeed;

                                role_it->second->Write_all(&t_roleAttr, sizeof(STR_RoleAttribute));
                                umap_roleSock t_viewRole = (*smap)[role_it->second].m_viewRole;
                                for(_umap_roleSock::iterator view_it = t_viewRole->begin(); view_it != t_viewRole->end(); view_it++)
                                {
                                    view_it->second->Write_all(&t_roleAttr, sizeof(STR_RoleAttribute));
                                }
                            }
                        }
                        else  //确定新目标 玩家已经退出游戏
                        {

                        }
                    }
                    else
                    {
                        NewMovePosition(&it->second, t_monsterSpawns);//自由活动
                    }
////                    printf("%lf,%lf\n", currentTime, it->second.aimTime);
////                    cout << it->second.monster.Current_x << it->second.monster.Current_z << endl;
                    SendMonsterToViewRole(&it->second.monster);
                }
            }
        }
    }
    Server::GetInstance()->free(buff);
}

//生成怪物
void Monster::MonsterSpawns(STR_MonsterInfo* monsterInfo, STR_MonsterSpawns* monsterSpawns)
{
    hf_uint8 Flag = 1;
    while(Flag)
    {        
        monsterInfo->monster.Current_x = (monsterSpawns->Pos_x - monsterSpawns->Boundary) + (float)rand()/(float)RAND_MAX * (monsterSpawns->Boundary*2);
        monsterInfo->monster.Current_y = monsterSpawns->Pos_y;
        monsterInfo->monster.Current_z = (monsterSpawns->Pos_z - monsterSpawns->Boundary) + (float)rand()/RAND_MAX * (monsterSpawns->Boundary*2);
        hf_float dx = monsterInfo->monster.Current_x - monsterSpawns->Pos_x;
        hf_float dz = monsterInfo->monster.Current_z - monsterSpawns->Pos_z;
        if(dx*dx + dz*dz <= (monsterSpawns->Boundary)*(monsterSpawns->Boundary))
        {
            Flag = 0;
            monsterInfo->pos.Come_x = monsterInfo->monster.Current_x;
            monsterInfo->pos.Come_y = monsterInfo->monster.Current_y;
            monsterInfo->pos.Come_z = monsterInfo->monster.Current_z;

            monsterInfo->monster.Direct = 1;
            monsterInfo->monster.ActID = 1;
            monsterInfo->spawnsPos = monsterSpawns->SpawnsPosID;
            monsterInfo->monster.HP = monsterInfo->monster.MaxHP;


            hf_uint32 boundary = monsterSpawns->Boundary/monsterSpawns->Amount;
            monsterInfo->monster.Target_x = monsterSpawns->Pos_x - boundary + (float)rand()/(float)RAND_MAX * boundary*2;
            monsterInfo->monster.Target_y = monsterInfo->monster.Current_y;
            monsterInfo->monster.Target_z = monsterSpawns->Pos_z - boundary + (float)rand()/RAND_MAX * boundary*2;
        }
    }
}

//怪物新的移动点
void Monster::NewMovePosition(STR_MonsterInfo* monsterInfo, STR_MonsterSpawns* monsterSpawns)
{
    monsterInfo->monster.Current_x = monsterInfo->monster.Target_x;
    monsterInfo->monster.Current_y = monsterInfo->monster.Target_y;
    monsterInfo->monster.Current_z = monsterInfo->monster.Target_z;

    hf_uint32 boundary = monsterSpawns->Boundary/*/monsterSpawns->Amount*/;
    monsterInfo->monster.Target_x = monsterSpawns->Pos_x - boundary + (float)rand()/(float)RAND_MAX * boundary*2;
    monsterInfo->monster.Target_y = monsterInfo->monster.Target_y;
    monsterInfo->monster.Target_z = monsterSpawns->Pos_z - boundary + (float)rand()/RAND_MAX * boundary*2;

    hf_float dx = monsterInfo->monster.Target_x - monsterInfo->monster.Current_x;
    hf_float dz = monsterInfo->monster.Target_z - monsterInfo->monster.Current_z;

    monsterInfo->monster.Direct = CalculationDirect(dx, dz);
    monsterInfo->aimTime += sqrt(dx*dx + dz*dz) / ((hf_double)monsterInfo->monster.MoveRate/100 * MonsterMoveDistance);
}

//将变化的怪物信息发送给怪物可视范围内的玩家
void Monster::SendMonsterToViewRole(STR_MonsterBasicInfo* monster)
{
    _umap_viewRole* t_roleSock = &(*m_monsterViewRole)[monster->MonsterID];
    STR_PackMonsterBasicInfo t_monster(monster);
    for(_umap_viewRole::iterator it = t_roleSock->begin(); it != t_roleSock->end(); it++)
    {
        it->second.conn->Write_all(&t_monster, sizeof(STR_PackMonsterBasicInfo));
    }
}

//发送伤害给可视范围内的玩家
void Monster::SendMonsterHPToViewRole(STR_PackMonsterAttrbt* monsterBt)
{
    _umap_viewRole* t_roleSock = &(*m_monsterViewRole)[monsterBt->MonsterID];

    for(_umap_viewRole::iterator it = t_roleSock->begin(); it != t_roleSock->end(); it++)
    {
        it->second.conn->Write_all(monsterBt, sizeof(STR_PackMonsterAttrbt));
    }
}

//计算方向
hf_float Monster::CalculationDirect(hf_float dx, hf_float dz)
{
    if(dx == 0)
    {
        if(dz > 0)
            return 90;
        else
            return 270;
    }
    if(dz > 0)
    {
        if(dx > 0)
            return atan2(dz, dx)/PI*180;
        else
            return atan2(dz, dx)/PI*180 + 180;
    }
    else
    {
        if(dx > 0)
            return atan2(dz, dx)/PI*180 + 180;
        else
            return atan2(dz, dx)/PI*180 + 360;
    }
}
