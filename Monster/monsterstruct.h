#ifndef MONSTERSTRUCT_H
#define MONSTERSTRUCT_H

#include "Game/postgresqlstruct.h"
#include <boost/unordered_map.hpp>
#include "NetWork/tcpconnection.h"

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

    hf_uint32 ReduceHp(hf_uint32 _roleid, hf_uint32 hp)
    {
        m_mtx.lock();
        if(monster.HP == monster.MaxHP)
            roleid = _roleid;
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

    void ChangeHatredRoleid(hf_uint32 _hatredRoleid)
    {
        m_mtx.lock();
        hatredRoleid = _hatredRoleid;
        m_mtx.unlock();
    }

    void ChangeAimTimeAndPos(hf_double timep)
    {
        m_mtx.lock();
        aimTime = timep;

        hf_float dx = monster.Target_x - monster.Current_x;
        hf_float dz = monster.Target_z - monster.Current_z;
        hf_float dis = sqrt(dx*dx + dz*dz);
        hf_double userTime = dis/(hf_double)(monster.MoveRate/100*MonsterMoveDistance);

        monster.Current_x = monster.Target_x - (aimTime - timep)/userTime * dx;
        monster.Current_z = monster.Target_x - (aimTime - timep)/userTime * dx;
        monster.Target_x = monster.Current_x;
        monster.Target_z = monster.Current_z;
        m_mtx.unlock();
    }

    _STR_MonsterInfo& operator=(_STR_MonsterInfo& mon)
    {
        if(this == &mon)
        {
            return *this;
        }
        monster = mon.monster;
        pos = mon.pos;
        pursuitPos = mon.pursuitPos;
        spawnsPos = mon.spawnsPos;
        aimTime = mon.aimTime;
        hatredRoleid = mon.hatredRoleid;
        return *this;
    }

    STR_MonsterBasicInfo monster;   //怪物基本信息
    STR_Position    pos;            //怪物刷出坐标点,怪物自由活动用
    STR_Position    pursuitPos;     //以这个点为起点追击
    hf_double       aimTime;        //当monster.HP等于0时，怪物死亡计算复活时间，当monster.HP不等于0时，表示怪物到运动到下一个坐标点的时间
    hf_uint32       spawnsPos;      //怪物刷怪点
    hf_uint32       roleid;         //第一个攻击此怪物的玩家
    hf_uint32       hatredRoleid;   //仇恨值最大的玩家

}STR_MonsterInfo;


//角色ID，仇恨值
typedef boost::unordered_map<hf_uint32, hf_uint32> _umap_viewRole;
typedef boost::shared_ptr<_umap_viewRole> umap_viewRole;

//<怪物ID,_umap_roleSock>
typedef boost::unordered_map<hf_uint32, _umap_viewRole> _umap_monsterViewRole;
typedef boost::shared_ptr<_umap_monsterViewRole> umap_monsterViewRole;

typedef boost::unordered_map<hf_uint32, STR_MonsterInfo> _umap_monsterInfo;
typedef boost::shared_ptr<_umap_monsterInfo> umap_monsterInfo;

#endif // MONSTERSTRUCT_H

