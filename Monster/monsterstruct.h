#ifndef MONSTERSTRUCT_H
#define MONSTERSTRUCT_H

#include "Game/postgresqlstruct.h"
//#include <boost/thread/mutex.hpp>
#include <boost/unordered_map.hpp>

typedef struct _STR_MonsterInfo
{
private:
    boost::mutex    m_mtx;
public:
    void lockMonster()
    {
        cout << "lock" << endl;
       m_mtx.lock();
    }
    void unlockMonster()
    {
        m_mtx.unlock();
        cout << "unlock" << endl;
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
    hf_double       aimTime;     //当monster.HP等于0时，怪物死亡计算复活时间，当monster.HP不等于0时，表示怪物到运动到下一个坐标点的时间
    hf_uint32       spawnsPos;      //怪物刷怪点

}STR_MonsterInfo;


typedef struct monstter
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
}monstter44;

typedef boost::unordered_map<hf_uint32, monstter44> _umap_rInfo;
//typedef boost::shared_ptr<_umap_monsterInfo> umap_moerInfo;

typedef boost::unordered_map<hf_uint32, STR_MonsterInfo> _umap_monsterInfo;
typedef boost::shared_ptr<_umap_monsterInfo> umap_monsterInfo;
#endif // MONSTERSTRUCT_H
