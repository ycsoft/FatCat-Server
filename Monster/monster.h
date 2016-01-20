#ifndef MONSTER_H
#define MONSTER_H

#include <boost/thread/mutex.hpp>

#include "./../Game/cmdtypes.h"
#include "./../Game/postgresqlstruct.h"
#include "./../NetWork/tcpconnection.h"
#include "./../server.h"
#include "monsterstruct.h"

#define   KB           1024
#define   MB          1048576

class Monster
{
public:
    Monster()
        :m_monsterBasic(new _umap_monsterInfo)
        ,m_monsterAttack(new umap_monsterAttackInfo)
        ,m_monsterSpawns(new umap_monsterSpawns)
        ,m_monsterSpawnsPos(new umap_monsterSpawnsPos)
        ,m_monsterType(new umap_monsterType)
        ,m_npcInfo(new umap_npcInfo)
        ,m_monsterLoot(new umap_monsterLoot)
        ,m_monsterViewRole(new _umap_monsterViewRole)
    {

    }

    ~Monster()
    {
        if(m_monsterAttack)
        {
            delete m_monsterAttack;
            m_monsterAttack = NULL;
        }
        if(m_monsterSpawns)
        {
            delete m_monsterSpawns;
            m_monsterSpawns = NULL;
        }
        if(m_monsterSpawnsPos)
        {
            delete m_monsterSpawns;
            m_monsterSpawns = NULL;
        }
        if(m_monsterType)
        {
            delete m_monsterType;
            m_monsterType = NULL;
        }

        if(m_npcInfo)
        {
            delete m_npcInfo;
            m_npcInfo = NULL;
        }
        if(m_monsterLoot)
        {
            delete m_monsterLoot;
            m_monsterLoot = NULL;
        }
    }

    hf_uint8 JudgeDisAndDirect(STR_PackPlayerPosition *usr,  STR_MonsterInfo *monster, hf_double currentTime, STR_PosDis* t_posDis);

    hf_uint8 JudgeSkillDisAndDirect(STR_PackPlayerPosition *usr,  STR_MonsterInfo *monster, hf_double currentTime, STR_PosDis* t_posDis, STR_PackSkillInfo* skillInfo);

    hf_float caculateDistanceWithRole(STR_PackPlayerPosition *usr,  STR_MonsterBasicInfo *monster);

    //计算玩家与怪物之间的距离
    hf_float    caculateDistanceWithMonster( STR_PackPlayerPosition *usr,  STR_MonsterInfo *monster, hf_double currentTime, STR_MonsterBasicInfo *monsterBasicInfo);

    //从数据库中查询生成怪物所需要的信息
     void QueryMonstersAll();
    //生成怪物
    void CreateMonster();


    //玩家上线时推送所有的怪物数据
    void PushViewMonsters( TCPConnection::Pointer conn);

    //查询NPC信息
    void QueryNpcInfo();
    //查询怪物掉落物品
    void QueryMonsterLoot();

    //怪物活动
    void Monsteractivity();

    //怪物复活
    void MonsterSpawns(STR_MonsterInfo* monsterInfo, STR_MonsterSpawns* monsterSpawns, hf_double currentTime);

    //怪物新的移动点
//    void NewMovePosition(STR_MonsterSpawns* monsterSpawns, STR_Position* pos);

//    void NewMovePosition(STR_MonsterInfo* monsterInfo, STR_MonsterSpawns* monsterSpawns, hf_double currentTime);

    //将变化的怪物信息发送给怪物可视范围内的玩家
    void SendMonsterToViewRole(STR_MonsterBasicInfo* monster);

    //将变化的怪物方向发送给怪物可视范围内的玩家
    void SendMonsterDirectToViewRole(STR_PackMonsterDirect* monster);

    //发送伤害给可是范围内的玩家
    void SendMonsterHPToViewRole(STR_PackMonsterAttrbt* monsterBt);

    //发送变化的怪物动作给可视范围内的玩家
    void SendMonsterActionToViewRole(STR_PackMonsterAction* monster);
    //发送施法效果给周围玩家
    void SendSkillEffectToMonsterViewRole(STR_PackSkillAimEffect* skillEffect);

    //计算方向
    hf_float CalculationDirect(hf_float dx, hf_float dz);

    //计算怪物与追击点之间的距离
    hf_float CalculationPursuitDistance(STR_MonsterInfo* monsterInfo);

    //确定新的追击目标，如果仇恨值都为0，则返回起始追击点
    void  SearchNewAim(STR_MonsterInfo* monster, hf_double currentTime, hf_float startDis, hf_uint32 hatredID);
    void DeleteOldSearchNewAim(STR_MonsterInfo* monster, hf_double currentTime, hf_float startDis, hf_uint32 hatredID);

    //返回当前时间
    hf_double GetCurrentTime()
    {
        struct timeval start;
        gettimeofday( &start, NULL );
        return (hf_double)start.tv_sec + (hf_double)start.tv_usec/1000000;
    }

    umap_monsterInfo GetMonsterBasic()
    {
        return m_monsterBasic;
    }

    umap_monsterSpawns* GetMonsterSpawns()
    {
        return m_monsterSpawns;
    }

    umap_monsterSpawnsPos* GetMonsterSpawnsPos()
    {
        return m_monsterSpawnsPos;
    }

    umap_monsterType* GetMonsterType()
    {
        return m_monsterType;
    }

    umap_monsterAttackInfo* GetMonsterAttack()
    {
        return m_monsterAttack;
    }

    umap_npcInfo* GetNpcInfo()
    {
        return m_npcInfo;
    }

    umap_monsterLoot* GetMonsterLoot()
    {
        return m_monsterLoot;
    }

    umap_monsterViewRole GetMonsterViewRole()
    {
        return m_monsterViewRole;
    }

    void MonsterViewAddRole(hf_uint32 monsterID, hf_uint32 roleid)
    {
        m_monsterViewMtx.lock();
        ((*m_monsterViewRole)[monsterID])[roleid] = 0;
        m_monsterViewMtx.unlock();
    }

    void MonsterViewDeleteRole(hf_uint32 monsterID, hf_uint32 roleid)
    {
        m_monsterViewMtx.lock();
        (*m_monsterViewRole)[monsterID].erase(roleid);
        m_monsterViewMtx.unlock();
    }

private:
    umap_monsterInfo                m_monsterBasic;     //怪物基本信息
    umap_monsterAttackInfo*         m_monsterAttack;    //怪物攻击信息
    umap_monsterSpawns*             m_monsterSpawns;    //怪物刷新信息
    umap_monsterSpawnsPos*          m_monsterSpawnsPos; //怪物刷拐点
    umap_monsterType*               m_monsterType;      //怪物类型信息
    umap_npcInfo*                   m_npcInfo;          //NPC信息
    umap_monsterLoot*               m_monsterLoot;      //怪物掉落物品

    umap_monsterViewRole            m_monsterViewRole;  //怪物可视范围内的玩家
    boost::mutex                    m_monsterViewMtx;   //操作怪物可视范围内的玩家锁
};




#endif // MONSTER_H

