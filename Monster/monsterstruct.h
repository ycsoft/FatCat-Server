#ifndef MONSTERSTRUCT_H
#define MONSTERSTRUCT_H

#include "Game/postgresqlstruct.h"
#include <boost/unordered_map.hpp>

typedef struct _STR_MonsterInfo
{
private:
    boost::mutex    m_mtx;
public:
    void lockMonster()
    {
       m_mtx.lock();
    }
    void unlockMonster()
    {
        m_mtx.unlock();
    }

    hf_uint32 ReduceHp(hf_uint32 hp)
    {
        m_mtx.lock();
        if(monster.HP > hp)
            monster.HP -= hp;
        else
        {
            monster.HP = 0;
            struct timeval start;
            gettimeofday( &start, NULL);
            aimTime = (hf_double)start.tv_sec + MonsterDeathTime + (hf_double)start.tv_usec / 1000000;
        }

        m_mtx.unlock();
        return monster.HP;
    }

    _STR_MonsterInfo& operator=(_STR_MonsterInfo& mon)
    {
        if(this == &mon)
        {
            return *this;
        }
        monster = mon.monster;
        pos = mon.pos;
        spawnsPos = mon.spawnsPos;
        aimTime = mon.aimTime;
        return *this;
    }

    STR_MonsterBasicInfo monster;   //怪物基本信息
    STR_Position    pos;            //怪物刷出坐标点,怪物自由活动用
    hf_double       aimTime;        //当monster.HP等于0时，怪物死亡计算复活时间，当monster.HP不等于0时，表示怪物到运动到下一个坐标点的时间
    hf_uint32       spawnsPos;      //怪物刷怪点
    hf_uint32       roleid;         //第一个攻击此怪物的玩家

}STR_MonsterInfo;

typedef boost::unordered_map<hf_uint32, STR_MonsterInfo> _umap_monsterInfo;
typedef boost::shared_ptr<_umap_monsterInfo> umap_monsterInfo;
#endif // MONSTERSTRUCT_H

