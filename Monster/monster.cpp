#include <cmath>
#include "memManage/diskdbmanager.h"
#include "utils/stringbuilder.hpp"
#include "Game/session.hpp"
#include "Game/log.hpp"
#include "monster.h"
#include "server.h"
#include "GameAttack/gameattack.h"
#include "Game/getdefinevalue.h"


hf_uint8 Monster::JudgeDisAndDirect(STR_PackPlayerPosition *usr,  STR_MonsterInfo *monster, hf_double currentTime, STR_PosDis* t_posDis)
{
    STR_MonsterBasicInfo t_monster;
    memcpy(&t_monster, &monster->monster, sizeof(STR_MonsterBasicInfo));

//    printf("\n\n%u,怪物起始坐标点：x:%f y:%f z:%f\n", t_monster.MonsterID, t_monster.Current_x, t_monster.Current_y, t_monster.Current_z);
//    printf("怪物目标坐标点：x:%f y:%f z:%f\n", t_monster.Target_x, t_monster.Target_y, t_monster.Target_z);

    hf_float dx = t_monster.Target_x - t_monster.Current_x;
    hf_float dz = t_monster.Target_z - t_monster.Current_z;
    hf_float dis = sqrt(dx*dx + dz*dz);
    hf_double userTime = dis/((hf_double)t_monster.MoveRate/100*MonsterMoveDistance);
//    printf("计算要用的时间:%f\n", userTime);
//    hf_double starttime = monster->aimTime - userTime;

//    t_monster.Current_x += (currentTime - starttime)/userTime * dx;
//    t_monster.Current_z += (currentTime - starttime)/userTime * dz;
//    t_monster.Current_x += (currentTime - monster->startTime)/userTime * dx;
//    t_monster.Current_z += (currentTime - monster->startTime)/userTime * dz;

//    printf("怪物ID:%d,移动速度:%d,距离：%f,目标时间：%f = %f + %f\n", monster->monster.MonsterID, monster->monster.MoveRate, dis, monster->aimTime, starttime, dis / ((hf_double)monster->monster.MoveRate/100 * MonsterMoveDistance));

//    printf("起始时间:%f\n", monster->startTime);
//    printf("计算的起始时间:%lf，当前时间:%lf，目标时间:%lf\n", starttime, currentTime, monster->aimTime);
//    printf("怪物当前坐标点:x:%f y:%f z:%f\n\n\n", t_monster.Current_x, t_monster.Current_y, t_monster.Current_z);
    t_monster.Current_x = t_monster.Target_x - (monster->aimTime - currentTime)/userTime * dx;
    t_monster.Current_z = t_monster.Target_z - (monster->aimTime - currentTime)/userTime * dz;

//    Server::GetInstance()->GetMonster()->SendMonsterToViewRole(&t_monster);
    dx = t_monster.Current_x - usr->Pos_x;
    dz = t_monster.Current_z - usr->Pos_z;
    dis = sqrt(dx*dx + dz *dz);
    if(dis > PlayerAttackView)
        return 1;
    if(dx*cos(usr->Direct) + dz*sin(usr->Direct) < 0)
        return 2;
    else
    {
        t_posDis->dis = dis;
        t_posDis->dx = 0 - dx;
        t_posDis->dz = 0 - dz;
        t_posDis->Current_x = t_monster.Current_x;
        t_posDis->Current_z = t_monster.Current_z;

//        printf("怪物与玩家之间的距离：%f,%f,%f,%f,%f\n", t_posDis->dis, t_posDis->dx, t_posDis->dz, t_posDis->Current_x, t_posDis->Current_z);
        return 0;
    }
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

    printf("生成怪物时间:%f\n", currentTime);
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
            t_monsterInfo.MonsterSpawns(&it->second, currentTime);
//            MonsterSpawns(&t_monsterInfo, &it->second, currentTime);

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

    STR_PackDamageData t_damageData;
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
                it->second.MonsterSpawns(t_monsterSpawns, currentTime);
//                MonsterSpawns(&it->second, t_monsterSpawns, currentTime); //怪物复活
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
            if(it->second.hatredRoleid == 0) //怪物没有追击目标
            {
                if(it->second.monsterStatus)
                {
                    it->second.ChangeMonsterStatus();
                }

//                printf("怪物自由活动,%d,%f,%f\n",it->second.monster.MonsterID, currentTime, GetCurrentTime ());
//                STR_Position newPos;
                it->second.NewMovePosition(t_monsterSpawns, currentTime);
//                NewMovePosition(t_monserSpawns, &newPos);
//                NewMovePosition(&it->second, t_monsterSpawns, currentTime); //自由活动
//                it->second.newMovePosition(&newPos, currentTime);
                SendMonsterToViewRole(&it->second.monster);
                continue;
            }

//            printf("t_monsterPos:%f,%f,%f\n",t_monster->Current_x, t_monster->Current_y, t_monster->Current_z);
            //计算怪物是否超过追击距离
            hf_float t_startDis = CalculationPursuitDistance(&it->second);
//            printf("离开追击点的距离：%lf\n", t_startDis);
            if(t_startDis >= MonsterPursuitDis) //超过追击距离，返回起始点
            {
                it->second.ChangeHatredRoleid(0);
                it->second.MoveToStartPos(currentTime, t_startDis);
                SendMonsterToViewRole(&it->second.monster);
                continue;
            }

            _umap_roleSock::iterator role_it = t_roleSock->find(it->second.hatredRoleid);
            if(role_it == t_roleSock->end()) //确定新目标 玩家已经退出游戏,暂时让其自由活动
            {
                it->second.ChangeHatredRoleid(0);
                it->second.MoveToStartPos(currentTime, t_startDis);
//                printf("怪物自由活动；monsterID %d,%f, %f\n", it->second.monster.MonsterID, currentTime, GetCurrentTime ());
                SendMonsterToViewRole(&it->second.monster);
                continue;
            }
            else
            {
                //如果玩家已经死亡
                if(((*smap)[role_it->second].m_roleInfo.HP) == 0)
                {
                    it->second.ChangeHatredRoleid(0);
                    cout << "玩家死亡，怪物仇恨目标为0：" << it->second.hatredRoleid << endl;
                    it->second.MoveToStartPos(currentTime, t_startDis);
                    SendMonsterToViewRole(&it->second.monster);
                    continue;
                }

                //计算与玩家之间的距离
                STR_PackPlayerPosition*  t_playerPos = &(*smap)[role_it->second].m_position;
                hf_float dx = t_playerPos->Pos_x - t_monster->Current_x;
                hf_float dz = t_playerPos->Pos_z - t_monster->Current_z;
                hf_float dis = sqrt(dx*dx + dz*dz);
//                cout << "怪物与玩家的距离:" << dis << endl << endl;
                if(dis > MonsterAttackView)
                {
                    it->second.ChangeMonsterPos(currentTime, dis, dx, dz);
                    SendMonsterToViewRole(&it->second.monster);
                    continue;
                }

                hf_float cosDirect = (dx*cos(it->second.monster.Direct) + dz*sin(it->second.monster.Direct))/sqrt(dx*dx + dz*dz);
                printf("角度计算值:%f\n", cosDirect);
                if((dx*cos(it->second.monster.Direct) + dz*sin(it->second.monster.Direct)/sqrt(dx*dx + dz*dz) < SQRT3DIV2))  //夹角大于30度
                {
                    cout << "1玩家方向:" << t_playerPos->Direct << endl;
//                    it->second.ChangeMonsterDirect(0 - t_playerPos->Direct);
                    it->second.ChangeMonsterDirect(dx, dz);
                    cout << "1改变怪物方向，改变后:" << it->second.monster.Direct << endl << endl;
                    SendMonsterToViewRole(&it->second.monster);
                }

                //攻击玩家
                hf_uint32 roleid = (*smap)[role_it->second].m_roleid;                
                t_damageData.AimID = roleid;
                t_damageData.AttackID = t_monster->MonsterID;

                STR_RoleInfo* t_AimInfo = &(*smap)[role_it->second].m_roleInfo;
                if(t_AimInfo->Dodge_Rate*100 >= rand()%100) //闪避
                {
                    t_damageData.Damage = 0;
                    t_damageData.TypeID = 0;
                    t_damageData.Flag = Dodge;
                }
                else
                {
                    hf_uint32 monsterPhyAttack = (*m_monsterAttack)[t_monster->MonsterTypeID].PhysicalAttack;

//                    hf_uint8 t_level = (*smap)[role_it->second].m_RoleBaseInfo.Level;
//                    t_damageData.Damage = monsterPhyAttack* GetDamage_reduction(t_level)/t_AimInfo->PhysicalDefense;

                    t_damageData.Damage = monsterPhyAttack; //test
                    t_damageData.TypeID = PhyAttackSkillID;
                    t_damageData.Flag = HIT;
                }

                //发送怪物产生的伤害给玩家
                role_it->second->Write_all(&t_damageData, sizeof(STR_PackDamageData));

                if((*smap)[role_it->second].m_roleInfo.HP >= t_damageData.Damage)
                {
                    (*smap)[role_it->second].m_roleInfo.HP -= t_damageData.Damage;
                    hf_uint32 playerHP = (*smap)[role_it->second].m_roleInfo.HP;
                    STR_RoleAttribute t_roleAttr(roleid, playerHP);

                    role_it->second->Write_all(&t_roleAttr, sizeof(STR_RoleAttribute));
                    Server::GetInstance()->GetGameAttack()->SendRoleHpToViewRole(role_it->second, &t_roleAttr);

                    it->second.ChangeMonsterAimTime(currentTime+MonsterAttackSpeed);
                }
                else
                {
                    (*smap)[role_it->second].m_roleInfo.HP = 0;
                    STR_RoleAttribute t_roleAttr(roleid, 0);

                    role_it->second->Write_all(&t_roleAttr, sizeof(STR_RoleAttribute));
                    Server::GetInstance()->GetGameAttack()->SendRoleHpToViewRole(role_it->second, &t_roleAttr);

                    it->second.ChangeHatredRoleid(0);
                    it->second.MoveToStartPos(currentTime, t_startDis);
                    SendMonsterToViewRole(&it->second.monster);
                }
            }           
        }
        usleep(1000);
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

////怪物新的移动点
//void Monster::NewMovePosition(STR_MonsterSpawns* monsterSpawns, STR_Position* pos)
//{
//    hf_uint32 boundary = monsterSpawns->Boundary/*/monsterSpawns->Amount*/;
//    pos->Come_x = monsterSpawns->Pos_x - boundary + (float)rand()/(float)RAND_MAX * boundary*2;
//    pos->Come_y = 1000;
//    pos->Come_z = monsterSpawns->Pos_z - boundary + (float)rand()/RAND_MAX * boundary*2;
//}

//void Monster::NewMovePosition(STR_MonsterInfo* monsterInfo, STR_MonsterSpawns* monsterSpawns, hf_double currentTime)
//{
//    monsterInfo->monster.Current_x = monsterInfo->monster.Target_x;
//    monsterInfo->monster.Current_y = monsterInfo->monster.Target_y;
//    monsterInfo->monster.Current_z = monsterInfo->monster.Target_z;

//    hf_uint32 boundary = monsterSpawns->Boundary/*/monsterSpawns->Amount*/;
//    monsterInfo->monster.Target_x = monsterSpawns->Pos_x - boundary + (float)rand()/(float)RAND_MAX * boundary*2;
//    monsterInfo->monster.Target_y = monsterInfo->monster.Target_y;
//    monsterInfo->monster.Target_z = monsterSpawns->Pos_z - boundary + (float)rand()/RAND_MAX * boundary*2;

//    hf_float dx = monsterInfo->monster.Target_x - monsterInfo->monster.Current_x;
//    hf_float dz = monsterInfo->monster.Target_z - monsterInfo->monster.Current_z;

//    monsterInfo->monster.Direct = CalculationDirect(dx, dz);
//    monsterInfo->startTime = currentTime;
//    monsterInfo->aimTime = currentTime + (sqrt(dx*dx + dz*dz) / ((hf_double)monsterInfo->monster.MoveRate/100 * MonsterMoveDistance)/* + MonsterStopTime*/);
//}

//将变化的怪物信息发送给怪物可视范围内的玩家
void Monster::SendMonsterToViewRole(STR_MonsterBasicInfo* monster)
{
    _umap_viewRole* t_viewRole = &(*m_monsterViewRole)[monster->MonsterID];
    umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
    STR_PackMonsterBasicInfo t_monster(monster);
//    cout << t_monster.monster.MonsterID << " 发送给客户端的怪物位置：x:" << t_monster.monster.Current_x << ",y:" << t_monster.monster.Current_y << ",z:" << t_monster.monster.Current_z << ",t_x:" << t_monster.monster.Target_x << ",t_y:" << t_monster.monster.Target_y << ",t_z:" << t_monster.monster.Target_z << endl;
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
