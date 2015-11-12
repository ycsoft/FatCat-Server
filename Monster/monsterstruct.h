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
        monsterStatus = mon.monsterStatus;
        return *this;
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
            aimTime = (hf_double)start.tv_sec + (hf_double)start.tv_usec/1000000;
        }

        m_mtx.unlock();
        return monster.HP;
    }

    void ChangeHatredRoleid(hf_uint32 _hatredRoleid)
    {
        m_mtx.lock();
        if(_hatredRoleid == 0)
        {
            if(monster.ActID == Action_Run)
            {
                monster.ActID = Action_Walk;
                monster.MoveRate /= 2;
            }
            else
            {
                monster.ActID = Action_Walk;
            }
        }
        else
        {
            if(monster.ActID != Action_Run)
            {
                monster.ActID = Action_Run;
                monster.MoveRate *= 2;
            }
        }
        hatredRoleid = _hatredRoleid;
        m_mtx.unlock();
    }

//    void ChangeActID(hf_uint8 actID)
//    {
//        m_mtx.lock();
//        monster.ActID = actID;
//        m_mtx.unlock();
//    }

    void ChangeAimTimeAndPos(hf_uint32 _hatredRoleid, hf_double timep)
    {
        m_mtx.lock();
        hatredRoleid = _hatredRoleid;
        cout << "修改前怪物当前坐标点" << monster.Current_x << "," << monster.Current_z << endl;
        cout << "修改前怪物要走到的目标点:" << monster.Target_x << "," << monster.Target_z << endl;
        cout << "移动速度" << monster.MoveRate << endl;

        hf_float dx = monster.Target_x - monster.Current_x;
        hf_float dz = monster.Target_z - monster.Current_z;
        hf_float dis = sqrt(dx*dx + dz*dz);
        hf_double userTime = dis/(hf_double)(monster.MoveRate/100*MonsterMoveDistance);

        printf("怪物受到攻击时的时间timep:%lf\n", timep);
        printf("怪物目标时间aimTime:%lf\n",aimTime);
        cout << "要用的时间" << userTime << endl;

        monster.Current_x = monster.Target_x - (aimTime - timep)/userTime * dx;
        monster.Current_z = monster.Target_z - (aimTime - timep)/userTime * dz;
        pursuitPos.Come_x = monster.Current_x;
        pursuitPos.Come_z = monster.Current_z;

        monster.Target_x = monster.Current_x;
        monster.Target_z = monster.Current_z;
        if(monster.ActID != Action_Run)
        {
            monster.ActID = Action_Run;
            monster.MoveRate *= 2;
        }

        cout << "修改后怪物当前坐标点" << monster.Current_x << "," << monster.Current_z << endl;
        cout << "修改后怪物要走到的目标点:" << monster.Target_x << "," << monster.Target_z << endl;
        aimTime = 0;
        m_mtx.unlock();
    }

    void MoveToStartPos(hf_double timep, hf_float dis)
    {
        m_mtx.lock();
        hatredRoleid = 0;
        monster.Target_x = pursuitPos.Come_x;
        monster.Target_y = pursuitPos.Come_y;
        monster.Target_z = pursuitPos.Come_z;

        pursuitPos.Come_x = 0;
        pursuitPos.Come_y = 0;
        pursuitPos.Come_z = 0;
        aimTime = timep + dis/(hf_double)(monster.MoveRate/100*MonsterMoveDistance);
        monsterStatus = true;
        m_mtx.unlock();
    }

    void ChangeMonsterStatus()
    {
        m_mtx.lock();
        monsterStatus = !monsterStatus;
        if(monster.ActID == Action_Run)
        {
            monster.MoveRate /= 2;
            monster.ActID = Action_Walk;
        }
        else
        {
            monster.ActID = Action_Walk;
        }
        m_mtx.unlock();
    }
    void ChangeMonsterAimTime(hf_double _aimTime)
    {
        m_mtx.lock();
        aimTime = _aimTime;
        m_mtx.unlock();
    }

    hf_float CalculationDirect(hf_float dx, hf_float dz)
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

    void ChangeMonsterPos(hf_double currentTime, hf_float dis, hf_float dx, hf_float dz)
    {
        m_mtx.lock();
        monster.Current_x = monster.Target_x;
        monster.Current_z = monster.Target_z;

        monster.Direct = CalculationDirect(dx, dz);
        monster.ActID = Action_Run;

        if(dis >= PursuitFarDistance*4) //距离较远
        {
            monster.Target_x += 2*PursuitFarDistance/dis*dx;
            monster.Target_z += 2*PursuitFarDistance/dis*dz;

            aimTime = currentTime + (2*PursuitFarDistance/((hf_double)monster.MoveRate/100 * MonsterMoveDistance))/2;
        }
        else if(dis < PursuitFarDistance*4 && dis > PursuitFarDistance*2)
        {
            monster.Target_x += PursuitFarDistance/dis*dx;
            monster.Target_z += PursuitFarDistance/dis*dz;
            aimTime = currentTime + (PursuitFarDistance/((hf_double)monster.MoveRate/100 * MonsterMoveDistance))/2;
        }
        else if(dis < PursuitFarDistance*2 && dis > MonsterAttackView)
        {
            monster.Target_x += PursuitNearlyDistance/dis*dx;
            monster.Target_z += PursuitNearlyDistance/dis*dz;
            aimTime = currentTime + (PursuitNearlyDistance/((hf_double)monster.MoveRate/100 * MonsterMoveDistance))/2;
        }
        m_mtx.unlock();
    }


    STR_MonsterBasicInfo monster;   //怪物基本信息
    STR_Position    pos;            //怪物刷出坐标点,怪物自由活动用
    STR_Position    pursuitPos;     //以这个点为起点追击
    hf_double       aimTime;        //当monster.HP等于0时，怪物死亡计算复活时间，当monster.HP不等于0时，表示怪物到运动到下一个坐标点的时间
    hf_uint32       spawnsPos;      //怪物刷怪点
    hf_uint32       roleid;         //第一个攻击此怪物的玩家
    hf_uint32       hatredRoleid;   //仇恨值最大的玩家
    bool            monsterStatus;  //怪物是否处于返回起始追击点，true表示返回之中

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

