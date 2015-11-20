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
//            if(monster.ActID == Action_Run)
//            {
//                monster.ActID = Action_Walk;
//                monster.MoveRate /= 2;
//            }
//            else
//            {
//                monster.ActID = Action_Walk;
//            }
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

    void ChangeAimTimeAndPos(hf_uint32 _hatredRoleid, hf_double currentTime, STR_PosDis* posDis)
    {
        m_mtx.lock();
        hatredRoleid = _hatredRoleid;
//        cout << "修改前怪物当前坐标点" << monster.Current_x << "," << monster.Current_z << endl;
//        cout << "修改前怪物要走到的目标点:" << monster.Target_x << "," << monster.Target_z << endl;
//        cout << "移动速度" << monster.MoveRate << endl;

//        hf_float dx = monster.Target_x - monster.Current_x;
//        hf_float dz = monster.Target_z - monster.Current_z;
//        hf_float dis = sqrt(dx*dx + dz*dz);
//        hf_double userTime = dis/(hf_double)(monster.MoveRate/100*MonsterMoveDistance);

//        printf("怪物受到攻击时的时间timep:%lf\n", timep);
//        printf("怪物目标时间aimTime:%lf\n",aimTime);
//        cout << "要用的时间" << userTime << endl;

//        monster.Current_x = monster.Target_x - (aimTime - currentTime)/userTime * dx;
//        monster.Current_z = monster.Target_z - (aimTime - currentTime)/userTime * dz;

        monster.Current_x = posDis->Current_x;
        monster.Current_z = posDis->Current_z;

        pursuitPos.Come_x = monster.Current_x;
        pursuitPos.Come_y = monster.Current_y;
        pursuitPos.Come_z = monster.Current_z;

        if(monster.ActID != Action_Run)
        {
            monster.ActID = Action_Run;
            monster.MoveRate *= 2;
        }


        monster.Direct = CalculationDirect(posDis->dx, posDis->dz);


        if(posDis->dis > MonsterAttackView)
        {
            monster.Target_x = monster.Current_x + PursuitNearlyDistance/posDis->dis*posDis->dx;
            monster.Target_z = monster.Current_z + PursuitNearlyDistance/posDis->dis*posDis->dz;
            startTime = currentTime;//test
            aimTime = currentTime + PursuitNearlyDistance/((hf_double)monster.MoveRate/100 * MonsterMoveDistance);
        }
        else
        {
            monster.Target_x = monster.Current_x;
            monster.Target_z = monster.Current_z;
            startTime = currentTime;//test
            aimTime = currentTime;
        }

//        cout << "修改后怪物当前坐标点" << monster.Current_x << "," << monster.Current_z << endl;
//        cout << "修改后怪物要走到的目标点:" << monster.Target_x << "," << monster.Target_z << endl;
//        cout << "移动速度" << monster.MoveRate << endl;
        m_mtx.unlock();
    }

    void MoveToStartPos(hf_double currentTime, hf_float dis)
    {
        m_mtx.lock();
        hatredRoleid = 0;
        monster.Target_x = pursuitPos.Come_x;
        monster.Target_y = pursuitPos.Come_y;
        monster.Target_z = pursuitPos.Come_z;

        startTime = currentTime;//test
        aimTime = currentTime + dis/(hf_double)(monster.MoveRate/100*MonsterMoveDistance);
        monsterStatus = true;
        m_mtx.unlock();
    }

    void ChangeMonsterStatus()
    {
        m_mtx.lock();
        monsterStatus = !monsterStatus;
        monster.HP = monster.MaxHP;
        cout << "怪物回到起始追击点，血量加满:" << monster.HP << endl;
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
    void NewMovePosition(STR_MonsterSpawns* monsterSpawns, hf_double currentTime)
    {
        m_mtx.lock();
        monster.Current_x = monster.Target_x;
        monster.Current_y = monster.Target_y;
        monster.Current_z = monster.Target_z;

        hf_uint32 boundary = monsterSpawns->Boundary/*/monsterSpawns->Amount*/;
        monster.Target_x = monsterSpawns->Pos_x - boundary + (float)rand()/(float)RAND_MAX * boundary*2;
        monster.Target_y = monster.Target_y;
        monster.Target_z = monsterSpawns->Pos_z - boundary + (float)rand()/RAND_MAX * boundary*2;

        hf_float dx = monster.Target_x - monster.Current_x;
        hf_float dz = monster.Target_z - monster.Current_z;

        monster.Direct = CalculationDirect(dx, dz);
        startTime = currentTime;
//        hf_float dis = sqrt(dx*dx + dz*dz);
        aimTime = currentTime + (sqrt(dx*dx + dz*dz) / ((hf_double)monster.MoveRate/100 * MonsterMoveDistance)/* + MonsterStopTime*/);

//        printf("怪物ID:%d,移动速度:%d,距离：%f,目标时间：%f = %f + %f\n", monster.MonsterID, monster.MoveRate, dis, aimTime, currentTime, dis / ((hf_double)monster.MoveRate/100 * MonsterMoveDistance));

        m_mtx.unlock();
    }

    void MonsterSpawns(STR_MonsterSpawns* monsterSpawns, hf_double currentTime)
    {
        hf_uint8 Flag = 1;
        while(Flag)
        {
            monster.Current_x = (monsterSpawns->Pos_x - monsterSpawns->Boundary) + (float)rand()/(float)RAND_MAX * (monsterSpawns->Boundary*2);
            monster.Current_y = monsterSpawns->Pos_y;
            monster.Current_z = (monsterSpawns->Pos_z - monsterSpawns->Boundary) + (float)rand()/RAND_MAX * (monsterSpawns->Boundary*2);
            hf_float dx = monster.Current_x - monsterSpawns->Pos_x;
            hf_float dz = monster.Current_z - monsterSpawns->Pos_z;
            hf_float dis = sqrt(dx*dx + dz*dz);
            if(dis <= monsterSpawns->Boundary)
            {
                Flag = 0;
                pos.Come_x = monster.Current_x;
                pos.Come_y = monster.Current_y;
                pos.Come_z = monster.Current_z;

                if(monster.ActID == Action_Run)
                {
                    monster.MoveRate /= 2;
                    monster.ActID = Action_Walk;
                }
                else
                {
                    monster.ActID = Action_Walk;
                }
                spawnsPos = monsterSpawns->SpawnsPosID;
                monster.HP = monster.MaxHP;


                hf_uint32 boundary = monsterSpawns->Boundary/monsterSpawns->Amount;
                monster.Target_x = monsterSpawns->Pos_x - boundary + (float)rand()/(float)RAND_MAX * boundary*2;
                monster.Target_y = monster.Current_y;
                monster.Target_z = monsterSpawns->Pos_z - boundary + (float)rand()/RAND_MAX * boundary*2;

                dx = monster.Target_x - monster.Current_x;
                dz = monster.Target_z - monster.Current_z;
                monster.Direct = CalculationDirect(dx,dz);

                startTime = currentTime;
                dis = sqrt(dx*dx + dz*dz);
                aimTime = currentTime + dis / ((hf_double)monster.MoveRate/100 * MonsterMoveDistance);
//                printf("怪物ID:%d,移动速度:%d,距离：%f,目标时间：%f = %f + %f\n", monster.MonsterID, monster.MoveRate, dis, aimTime, currentTime, dis / ((hf_double)monster.MoveRate/100 * MonsterMoveDistance));
            }
        }
    }

    void ChangeMonsterAimTime(hf_double _aimTime)
    {
        m_mtx.lock();
        aimTime = _aimTime;
        m_mtx.unlock();
    }

    hf_float CalculationDirect(hf_float dx, hf_float dz)
    {

        hf_float cosDirect = dx/sqrt(dx*dx + dz*dz);
        if(dx > 0)
        {
            if(dz > 0)
                return acos(cosDirect);          //1
            else
                return 2*PI - acos(cosDirect);     //4
        }
        else
        {
            if(dz > 0)
                return PI - acos(0 - cosDirect);   //2
            else
                return PI + acos(0 - cosDirect);     //3
        }
    }

//    hf_float CalculationDirect(hf_float dx, hf_float dz)
//    {
//        if(dx == 0)
//        {
//            if(dz > 0)
//                return PI/2;
//            else
//                return PI*3/2;
//        }
//        if(dz > 0)
//        {
//            if(dx > 0)
//                return atan2(dz, dx);          //1
//            else
//                return PI - atan2(dz, 0 - dx);     //2
//        }
//        else
//        {
//            if(dx > 0)
//                return 2*PI - atan2(dz, 0 - dx);   //4
//            else
//                return PI + atan2(dz, dx);     //3
//        }
//    }

    void ChangeMonsterDirect(hf_float dx, hf_float dz)
    {
        m_mtx.lock ();
        cout << "改变前怪物方向:" << monster.Direct << endl;
        monster.Direct = CalculationDirect (dx, dz);
        monster.Target_x = monster.Current_x;
        monster.Target_y = monster.Current_y;
        monster.Target_z = monster.Current_z;
        m_mtx.unlock ();
    }
//    void ChangeMonsterDirect(hf_float direct)
//    {
//        m_mtx.lock ();
//        cout << "改变前怪物方向:" << monster.Direct << endl;
//        monster.Direct = direct;
//        monster.Target_x = monster.Current_x;
//        monster.Target_y = monster.Current_y;
//        monster.Target_z = monster.Current_z;
//        m_mtx.unlock ();
//    }

    void ChangeMonsterPos(hf_double currentTime, hf_float dis, hf_float dx, hf_float dz)
    {
        m_mtx.lock();
        monster.Current_x = monster.Target_x;
        monster.Current_z = monster.Target_z;

        monster.Direct = CalculationDirect(dx, dz);
        monster.ActID = Action_Run;

        monster.Target_x += PursuitNearlyDistance/dis*dx;
        monster.Target_z += PursuitNearlyDistance/dis*dz;
        startTime = currentTime;//test
        aimTime = currentTime + PursuitNearlyDistance/((hf_double)monster.MoveRate/100 * MonsterMoveDistance);
//            printf("怪物ID:%d,距离：%f,目标时间：%f = %f + %f\n", monster.MonsterID, dis, aimTime, currentTime, PursuitNearlyDistance / ((hf_double)monster.MoveRate/100 * MonsterMoveDistance));
//        }

        m_mtx.unlock();
    }

    //怪物向做移动一格
    void MonsterMoveLeft(hf_double currentTime)
    {
        m_mtx.lock();
        monster.Current_x = monster.Target_x;
        monster.Current_z = monster.Target_z;

        if(monster.ActID != Action_Run)
        {
            monster.ActID = Action_Run;
            monster.MoveRate *= 2;
        }

        monster.Target_x -= PursuitNearlyDistance;
        startTime = currentTime;//test
        aimTime = currentTime + PursuitNearlyDistance/((hf_double)monster.MoveRate/100 * MonsterMoveDistance);
        m_mtx.lock();
    }

    //怪物向右移动一格
    void MonsterMoveRight(hf_double currentTime)
    {
        m_mtx.lock();
        monster.Current_x = monster.Target_x;
        monster.Current_z = monster.Target_z;

        if(monster.ActID != Action_Run)
        {
            monster.ActID = Action_Run;
            monster.MoveRate *= 2;
        }

        monster.Target_x += PursuitNearlyDistance;
        startTime = currentTime;//test
        aimTime = currentTime + PursuitNearlyDistance/((hf_double)monster.MoveRate/100 * MonsterMoveDistance);
        m_mtx.lock();
    }

    //怪物向上移动一格
    void MonsterMoveUp(hf_double currentTime)
    {
        m_mtx.lock();
        monster.Current_x = monster.Target_x;
        monster.Current_z = monster.Target_z;

        if(monster.ActID != Action_Run)
        {
            monster.ActID = Action_Run;
            monster.MoveRate *= 2;
        }

        monster.Target_z -= PursuitNearlyDistance;
        startTime = currentTime;//test
        aimTime = currentTime + PursuitNearlyDistance/((hf_double)monster.MoveRate/100 * MonsterMoveDistance);
        m_mtx.lock();
    }

    //怪物向下移动一格
    void MonsterMoveDown(hf_double currentTime)
    {
        m_mtx.lock();
        monster.Current_x = monster.Target_x;
        monster.Current_z = monster.Target_z;

        if(monster.ActID != Action_Run)
        {
            monster.ActID = Action_Run;
            monster.MoveRate *= 2;
        }

        monster.Target_z += PursuitNearlyDistance;
        startTime = currentTime;//test
        aimTime = currentTime + PursuitNearlyDistance/((hf_double)monster.MoveRate/100 * MonsterMoveDistance);
        m_mtx.lock();
    }


    STR_MonsterBasicInfo monster;   //怪物基本信息
    STR_Position    pos;            //怪物刷出坐标点,怪物自由活动用
    STR_Position    pursuitPos;     //以这个点为起点追击
    hf_double       aimTime;        //当monster.HP等于0时，怪物死亡计算复活时间，当monster.HP不等于0时，表示怪物到运动到下一个坐标点的时间
    hf_double       startTime;      //test 怪物起始时间
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

