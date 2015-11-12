#include <cmath>
#include "memManage/diskdbmanager.h"
#include "utils/stringbuilder.hpp"
#include "Game/session.hpp"
#include "Game/log.hpp"
#include "monster.h"
#include "server.h"
#include "GameAttack/gameattack.h"

hf_uint8 Monster::JudgeDisAndDirect(STR_PackPlayerPosition *usr,  STR_MonsterInfo *monster, hf_double currentTime)
{
    STR_MonsterBasicInfo t_monster;
    memcpy(&t_monster, &monster->monster, sizeof(STR_MonsterBasicInfo));

    hf_float dx = t_monster.Target_x - t_monster.Current_x;
    hf_float dz = t_monster.Target_z - t_monster.Current_z;
    hf_float dis = sqrt(dx*dx + dz*dz);
    hf_double userTime = dis/(hf_double)(t_monster.MoveRate/100*MonsterMoveDistance);

    t_monster.Current_x = t_monster.Target_x - (monster->aimTime - currentTime)/userTime * dx;
    t_monster.Current_z = t_monster.Target_z - (monster->aimTime - currentTime)/userTime * dz;

    dx = usr->Pos_x - t_monster.Current_x;
    dz = usr->Pos_z - t_monster.Current_z;
    dis = sqrt(dx*dx + dz *dz);
    if(dis > PlayerAttackView)
        return 1;
    else if(dx*cos(usr->Direct) + dz*sin(usr->Direct) < 0)
        return 2;
    else
        return 0;
}

hf_float Monster::caculateDistanceWithRole( STR_PackPlayerPosition *usr,  STR_MonsterBasicInfo *monster)
{
    hf_float res = 0.0f;

    int dx = usr->Pos_x - monster->Current_x,
            dy = usr->Pos_y - monster->Current_y,
            dz = usr->Pos_z - monster->Current_z;
    res = sqrt(dx*dx + dy *dy + dz *dz);

    return res;
}

//计算玩家与怪物之间的距离
hf_float    Monster::caculateDistanceWithMonster( STR_PackPlayerPosition *usr,  STR_MonsterInfo *monster, hf_double currentTime, STR_MonsterBasicInfo* monsterBasicInfo)
{
//    cout << "memcpy怪物坐标来源,x:" << monster->monster.Current_x << ",y:" << monster->monster.Current_y << ",z:" << monster->monster.Current_z << endl;
    memcpy(monsterBasicInfo, &monster->monster, sizeof(STR_MonsterBasicInfo));

    if(monsterBasicInfo->Current_x == monsterBasicInfo->Target_x &&
            monsterBasicInfo->Current_z == monsterBasicInfo->Target_z)
    {
        hf_float dx = usr->Pos_x - monsterBasicInfo->Current_x;
        hf_float dz = usr->Pos_z - monsterBasicInfo->Current_z;
        hf_float dis = sqrt(dx*dx + dz *dz);
        return dis;
    }
    hf_float dx = monsterBasicInfo->Target_x - monsterBasicInfo->Current_x;
    hf_float dz = monsterBasicInfo->Target_z - monsterBasicInfo->Current_z;
    hf_float dis = sqrt(dx*dx + dz*dz);
    hf_double userTime = dis/(hf_double)(monsterBasicInfo->MoveRate/100*MonsterMoveDistance);

    monsterBasicInfo->Current_x = monsterBasicInfo->Target_x - (monster->aimTime - currentTime)/userTime * dx;
    monsterBasicInfo->Current_z = monsterBasicInfo->Target_z - (monster->aimTime - currentTime)/userTime * dz;

    dx = usr->Pos_x - monsterBasicInfo->Current_x;
    dz = usr->Pos_z - monsterBasicInfo->Current_z;
    dis = sqrt(dx*dx + dz *dz);

    return dis;
}

//推送新看到的怪物数据
void Monster::PushViewMonsters( TCPConnection::Pointer conn)
{
    Server* srv = Server::GetInstance();
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    STR_PackPlayerPosition* playerPosition =   &((*smap)[conn].m_position);
    umap_playerViewMonster t_playerViewMonster = (*smap)[conn].m_viewMonster;
    STR_PackHead t_packHead;
    t_packHead.Flag = FLAG_MonsterCome;

    STR_MonsterBasicInfo t_monsterBasicInfo;

    hf_char* comebuff = (hf_char*)srv->malloc();
    hf_char* leavebuff = (hf_char*)srv->malloc();
    hf_uint16 pushcount = 0;
    hf_uint16 popcount = 0;

    hf_double currentTime = GetCurrentTime();
    for(_umap_monsterInfo::iterator it = m_monsterBasic->begin(); it != m_monsterBasic->end(); it++)
    {
        hf_uint32 monsterID = it->second.monster.MonsterID;
        if(it->second.monster.MapID != playerPosition->MapID)
        {
            break;
        }       
         hf_float t_distance = caculateDistanceWithMonster(playerPosition, &it->second, currentTime, &t_monsterBasicInfo);
//         cout << t_distance << endl;
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

            (*t_roleSock)[roleid] = 0;

//            cout << "新进入玩家可视范围的怪," << t_monsterBasicInfo.MonsterID << ",x:" << t_monsterBasicInfo.Current_x << ",y:" << t_monsterBasicInfo.Current_y << ",z:" << t_monsterBasicInfo.Current_z << endl;
            memcpy(comebuff + sizeof(STR_PackHead) + sizeof(STR_MonsterBasicInfo)*pushcount, &t_monsterBasicInfo, sizeof(STR_MonsterBasicInfo));
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
        t_monsterInfo.monster.ActID = Action_Walk;
        t_monsterInfo.monster.Flag = 1;

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
            MonsterSpawns(&t_monsterInfo, &it->second, currentTime);

            t_monsterInfo.monster.MonsterID = monsterID;            

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
//    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
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
        SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
        hf_double currentTime = GetCurrentTime();
        for(_umap_monsterInfo::iterator it = t_monsterBasic->begin(); it != t_monsterBasic->end(); it++)
        {
            if(currentTime < it->second.aimTime)//时间没到
            {
                continue;
            }
            STR_MonsterSpawns* t_monsterSpawns = &(*monsterSpawns)[it->second.spawnsPos];
            if(it->second.monster.HP == 0 ) //怪物复活
            {
                cout << "monster hp 0: " << it->second.monster.MonsterID << endl;
                MonsterSpawns(&it->second, t_monsterSpawns, currentTime); //怪物复活

                //查找怪物周围的玩家,给玩家发送新生成的怪物
                _umap_roleSock t_monsterViewRole;
                memcpy(buff + sizeof(STR_PackHead), &it->second.monster, sizeof(STR_MonsterBasicInfo));
                for(SessionMgr::SessionMap::iterator role_it = smap->begin(); role_it != smap->end(); role_it++)
                {
                    STR_PackPlayerPosition* t_pos = &(role_it->second.m_position);
                    if (caculateDistanceWithRole(t_pos, &it->second.monster) > PlayerView)
                    {
                        continue;
                    }
                    role_it->first->Write_all(buff, sizeof(STR_PackHead) + t_packHead.Len);
                    t_monsterViewRole[role_it->second.m_roleid] = role_it->first;
                    (*(role_it->second.m_viewMonster))[it->first] = it->first;
                    ((*monsterViewRole)[it->first])[role_it->second.m_roleid] = 0;
                }
                continue;
             }

            STR_MonsterBasicInfo* t_monster = &(it->second.monster);
//            cout << t_monster->MonsterID << "移动了" << endl;
            if(it->second.hatredRoleid == 0) //怪物没有追击目标
            {
                if(it->second.monsterStatus)
                {
                    it->second.ChangeMonsterStatus();
                }

                NewMovePosition(&it->second, t_monsterSpawns); //自由活动
                SendMonsterToViewRole(&it->second.monster);
                continue;
            }

            printf("t_monsterPos:%f,%f\n",t_monster->Current_x, t_monster->Current_z);
            //计算怪物是否超过追击距离
            hf_float t_startDis = CalculationPursuitDistance(&it->second);
            printf("离开追击点的距离：%lf\n", t_startDis);
            if(t_startDis >= MonsterPursuitDis) //超过追击距离，返回起始点
            {
                it->second.MoveToStartPos(currentTime, t_startDis);
                SendMonsterToViewRole(&it->second.monster);
//                cout << it->second.hatredRoleid << endl;
                continue;
            }

//            cout << "hatredRoleid::" << it->second.hatredRoleid << endl;
            t_monster->Current_x = t_monster->Target_x;
            t_monster->Current_z = t_monster->Target_z;
//            cout << "t_sock_size:" << t_roleSock->size() << endl;
            _umap_roleSock::iterator role_it = t_roleSock->find(it->second.hatredRoleid);
            if(role_it == t_roleSock->end()) //确定新目标 玩家已经退出游戏,暂时让其自由活动
            {
                it->second.ChangeHatredRoleid(0);
//                t_monster->ActID = Action_Walk;
//                t_monster->MoveRate /= 2;
            }
            else
            {
                //计算与玩家之间的距离
                STR_PackPlayerPosition*  t_playerPos = &(*smap)[role_it->second].m_position;
                hf_float dx = t_playerPos->Pos_x - t_monster->Current_x;
                hf_float dz = t_playerPos->Pos_z - t_monster->Current_z;
                hf_float dis = sqrt(dx*dx + dz*dz);
                cout << "怪物与玩家的距离:" << dis << endl << endl;
                if(dis > MonsterAttackView)
                {
                    it->second.ChangeMonsterPos(currentTime, dis, dx, dz);
                }

//                printf("roleid = :%d\n",(*smap)[role_it->second].m_roleid);
//                printf("t_playerPos:%f,%f\n",t_playerPos->Pos_x, t_playerPos->Pos_z);
//                printf("t_monsterPos:%f,%f\n",t_monster->Current_x, t_monster->Current_z);
//                printf("dx:%f,dz:%f,   dis:%f\n",dx,dz,dis);

//                t_monster->Direct = CalculationDirect(dx, dz);
//                t_monster->ActID = Action_Run;
////                cout << "monster moverate:" << t_monster->MoveRate << endl;
//                if(dis >= PursuitFarDistance*4) //距离较远
//                {
//                    t_monster->Target_x += 2*PursuitFarDistance/dis*dx;
//                    t_monster->Target_z += 2*PursuitFarDistance/dis*dz;

//                    it->second.aimTime = currentTime + (2*PursuitFarDistance/((hf_double)t_monster->MoveRate/100 * MonsterMoveDistance))/2;
//                }
//                else if(dis < PursuitFarDistance*4 && dis > PursuitFarDistance*2)
//                {
//                    t_monster->Target_x += PursuitFarDistance/dis*dx;
//                    t_monster->Target_z += PursuitFarDistance/dis*dz;
//                    it->second.aimTime = currentTime + (PursuitFarDistance/((hf_double)t_monster->MoveRate/100 * MonsterMoveDistance))/2;
//                }
//                else if(dis < PursuitFarDistance*2 && dis > PursuitNearlyDistance)
//                {
//                    t_monster->Target_x += PursuitNearlyDistance/dis*dx;
//                    t_monster->Target_z += PursuitNearlyDistance/dis*dz;
//                    it->second.aimTime = currentTime + (PursuitNearlyDistance/((hf_double)t_monster->MoveRate/100 * MonsterMoveDistance))/2;
//                }
                else //攻击玩家
                {
                    if(((*smap)[role_it->second].m_roleInfo.HP) == 0)
                    {
                        it->second.ChangeHatredRoleid(0);
//                        t_monster->ActID = Action_Walk;
//                        t_monster->MoveRate /= 2;
                        cout << "玩家死亡，怪物仇恨目标为0：" << it->second.hatredRoleid << endl;
                        it->second.ChangeMonsterAimTime(currentTime);
//                        it->second.aimTime = currentTime;
                        continue;
                    }
                    hf_uint32 roleid = (*smap)[role_it->second].m_roleid;
                    hf_uint32 hp = (*m_monsterAttack)[t_monster->MonsterTypeID].PhysicalAttack;

                    t_monster->ActID = Action_Attack;

                    STR_PackDamageData damage(roleid, t_monster->MonsterID, hp, 1, HIT);
                    role_it->second->Write_all(&damage, sizeof(STR_PackDamageData));

                    if((*smap)[role_it->second].m_roleInfo.HP >= hp)
                    {
                        (*smap)[role_it->second].m_roleInfo.HP -= hp;
                    }
                    else
                    {
                        (*smap)[role_it->second].m_roleInfo.HP = 0;
                        (*monsterViewRole)[it->first].erase(it->second.hatredRoleid);

//                        t_monster->ActID = Action_Walk;
//                        t_monster->MoveRate /= 2;

                        it->second.ChangeHatredRoleid(0);
//                        cout << "玩家死亡，怪物仇恨目标为0：" << it->second.hatredRoleid << endl;
                    }
                    hf_uint32 playerHP = (*smap)[role_it->second].m_roleInfo.HP;
                    STR_RoleAttribute t_roleAttr(roleid, playerHP);

                    role_it->second->Write_all(&t_roleAttr, sizeof(STR_RoleAttribute));
                    Server::GetInstance()->GetGameAttack()->SendRoleHpToViewRole(role_it->second, &t_roleAttr);


                    it->second.ChangeMonsterAimTime(currentTime+MonsterAttackSpeed);
//                    it->second.aimTime = currentTime + MonsterAttackSpeed;
                    continue;
                }
//                cout << "怪物移动后：" << t_monster->MonsterID << ",x:" << t_monster->Current_x << ",y:" << t_monster->Current_y << ",z:" << t_monster->Current_z << endl;
            }
//            printf("%lf,%lf\n", currentTime, it->second.aimTime);
//            cout << it->second.monster.Current_x << it->second.monster.Current_z << endl;
            SendMonsterToViewRole(&it->second.monster);
            usleep(1000);
        }
    }
    Server::GetInstance()->free(buff);
}

//生成怪物
void Monster::MonsterSpawns(STR_MonsterInfo* monsterInfo, STR_MonsterSpawns* monsterSpawns, hf_double currentTime)
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

            monsterInfo->monster.ActID = Action_Walk;
            monsterInfo->spawnsPos = monsterSpawns->SpawnsPosID;
            monsterInfo->monster.HP = monsterInfo->monster.MaxHP;


            hf_uint32 boundary = monsterSpawns->Boundary/monsterSpawns->Amount;
            monsterInfo->monster.Target_x = monsterSpawns->Pos_x - boundary + (float)rand()/(float)RAND_MAX * boundary*2;
            monsterInfo->monster.Target_y = monsterInfo->monster.Current_y;
            monsterInfo->monster.Target_z = monsterSpawns->Pos_z - boundary + (float)rand()/RAND_MAX * boundary*2;

            dx = monsterInfo->monster.Target_x - monsterInfo->monster.Current_x;
            dz = monsterInfo->monster.Target_z - monsterInfo->monster.Current_z;
            monsterInfo->monster.Direct = CalculationDirect(dx,dz);

            monsterInfo->aimTime = currentTime + sqrt(dx*dx + dz*dz) / ((hf_double)monsterInfo->monster.MoveRate/100 * MonsterMoveDistance);
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
    monsterInfo->aimTime += (sqrt(dx*dx + dz*dz) / ((hf_double)monsterInfo->monster.MoveRate/100 * MonsterMoveDistance)/* + MonsterStopTime*/);
}

//将变化的怪物信息发送给怪物可视范围内的玩家
void Monster::SendMonsterToViewRole(STR_MonsterBasicInfo* monster)
{
    _umap_viewRole* t_viewRole = &(*m_monsterViewRole)[monster->MonsterID];
    umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
    STR_PackMonsterBasicInfo t_monster(monster);
//    cout << "发送给客户端的怪物位置：x:" << t_monster.monster.Current_x << ",y:" << t_monster.monster.Current_y << ",z:" << t_monster.monster.Current_z << endl;
//    if(t_monster.monster.ActID == Action_Run)
//    {
//        t_monster.monster.MoveRate *= 2;
//        cout << "发送给客户端的怪物位置：x:" << t_monster.monster.Current_x << ",y:" << t_monster.monster.Current_y << ",z:" << t_monster.monster.Current_z << endl;
//    }
    for(_umap_viewRole::iterator it = t_viewRole->begin(); it != t_viewRole->end(); it++)
    {
        _umap_roleSock::iterator iter = t_roleSock->find(it->first);
        if(iter != t_roleSock->end())
        {
            iter->second->Write_all(&t_monster, sizeof(STR_PackMonsterBasicInfo));
        }
    }
}

//发送伤害给可视范围内的玩家
void Monster::SendMonsterHPToViewRole(STR_PackMonsterAttrbt* monsterBt)
{
    _umap_viewRole* t_viewRole = &(*m_monsterViewRole)[monsterBt->MonsterID];

    umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
    for(_umap_viewRole::iterator it = t_viewRole->begin(); it != t_viewRole->end(); it++)
    {
        _umap_roleSock::iterator iter = t_roleSock->find(it->first);
        if(iter != t_roleSock->end())
        {
            iter->second->Write_all(monsterBt, sizeof(STR_PackMonsterAttrbt));
        }
    }
}

//计算方向
hf_float Monster::CalculationDirect(hf_float dx, hf_float dz)
{
    if(dx == 0)
    {
        if(dz > 0)
            return PI;
        else
            return PI*3/2;
    }
    if(dz > 0)
    {
        if(dx > 0)
            return atan2(dz, dx);          //1
        else
            return atan2(dz, dx) + PI;     //2
    }
    else
    {
        if(dx > 0)
            return atan2(dz, dx) + 2*PI;   //4
        else
            return atan2(dz, dx) + PI;     //3
    }
}

//计算怪物与追击点之间的距离
hf_float Monster::CalculationPursuitDistance(STR_MonsterInfo* monsterInfo)
{
    hf_float dx = monsterInfo->monster.Current_x - monsterInfo->pursuitPos.Come_x;
    hf_float dz = monsterInfo->monster.Current_z - monsterInfo->pursuitPos.Come_z;
    return (hf_float)sqrt(dx*dx + dz*dz);
}
