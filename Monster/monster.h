#ifndef MONSTER_HPP
#define MONSTER_HPP

#include "Game/cmdtypes.h"
#include "Game/postgresqlstruct.h"
#include "NetWork/tcpconnection.h"

#define   KB           1024
#define   MB          1048576

class Monster
{
public:
    Monster()
        :m_monsterBasic(new _umap_monsterBasicInfo)
        ,m_monsterDeath(new _umap_monsterDeath)
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
    //计算玩家与怪物之间的距离
    hf_float    caculateDistanceWithMonster( STR_PackPlayerPosition *usr,  STR_MonsterBasicInfo *monster);

    //从数据库中查询生成怪物所需要的信息
     void QueryMonstersAll();
    //生成怪物
    void CreateMonster();


    //玩家上线时推送所有的怪物数据
    void PushViewMonsters( TCPConnection::Pointer conn);

    //保存死亡的怪
    void SaveDeathMonster(hf_uint32 monsterID);
    //怪物复活
    void MonsterSpawns();

    //查询NPC信息
    void QueryNpcInfo();
    //查询怪物掉落物品
    void QueryMonsterLoot();

    //生成怪物有效位置
    void CreateEffectivePos(STR_MonsterBasicInfo* monsterInfo, STR_MonsterSpawns* monsterSpawns);

    umap_monsterBasicInfo GetMonsterBasic()
    {
        return m_monsterBasic;
    }
    umap_monsterDeath GetMonsterDeath()
    {
        return m_monsterDeath;
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

private:
    umap_monsterBasicInfo           m_monsterBasic;     //怪物基本信息
    umap_monsterDeath               m_monsterDeath;     //死亡的怪
    umap_monsterAttackInfo*         m_monsterAttack;    //怪物攻击信息
    umap_monsterSpawns*             m_monsterSpawns;    //怪物刷新信息
    umap_monsterSpawnsPos*          m_monsterSpawnsPos; //怪物刷拐点
    umap_monsterType*               m_monsterType;      //怪物类型信息
    umap_npcInfo*                   m_npcInfo;          //NPC信息
    umap_monsterLoot*               m_monsterLoot;      //怪物掉落物品

    umap_monsterViewRole            m_monsterViewRole;  //怪物可视范围内的玩家
};

#endif // MONSTER_HPP

