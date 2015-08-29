#ifndef OPERATIONPOSTGRES_H
#define OPERATIONPOSTGRES_H

#include <list>
#include <boost/thread/mutex.hpp>
#include "Game/postgresqlstruct.h"

class OperationPostgres
{
public:
    OperationPostgres();
    ~OperationPostgres();

    //该函数负责实时将玩家数据写入数据库
    void UpdatePlayerData();

    void PushUpdateMoney(hf_uint32 roleid, STR_PlayerMoney* money)
    {
        m_upMoneyFlag = 1;
        UpdateMoney upMoney(roleid, money);
        m_mtxUpMoney.lock();
        m_UpdateMoney->push_back(upMoney);
        m_mtxUpMoney.unlock();
        m_upMoneyFlag = 0;
    }

    void PushUpdateLevel(hf_uint32 roleid, hf_uint8 level)
    {
        m_upLevelFlag = 1;
        UpdateLevel upLevel(roleid, level);
        m_mtxUpLevel.lock();
        m_UpdateLevel->push_back(upLevel);
        m_mtxUpLevel.unlock();
        m_upLevelFlag = 0;
    }

    void PushUpdateExp(hf_uint32 roleid, hf_uint32 exp)
    {
        m_upExpFlag = 1;
        UpdateExp upExp(roleid, exp);
        m_mtxUpExp.lock();
        m_UpdateExp->push_back(upExp);
        m_mtxUpExp.unlock();
        m_upExpFlag = 0;
    }

    void PushUpdateGoods(hf_uint32 roleid, STR_Goods* goods, hf_uint8 operate)
    {

        m_upGoodsFlag = 1;
        UpdateGoods upGoods(roleid, goods, operate);
        m_mtxUpGoods.lock();
        m_UpdateGoods->push_back(upGoods);
        m_mtxUpGoods.unlock();
        m_upGoodsFlag = 0;
    }

    void PushUpdateEquAttr(hf_uint32 roleid, STR_Equipment* equ, hf_uint8 operate)
    {
        m_upEquFlag = 1;
        UpdateEquAttr upEpq(roleid, equ, operate);
        m_mtxUpEqu.lock();
        m_UpdateEquAttr->push_back(upEpq);
        m_mtxUpEqu.unlock();
        m_upEquFlag = 0;
    }

    void PushUpdateTask(hf_uint32 roleid, STR_TaskProcess* task, hf_uint8 operate)
    {
        m_upTaskFlag = 1;
        UpdateTask upTask(roleid, task, operate);
        m_mtxUpTask.lock();
        m_UpdateTask->push_back(upTask);
        m_mtxUpTask.unlock();
        m_upTaskFlag = 0;
    }

private:
    list<UpdateMoney>   *m_UpdateMoney;   //更新金钱
    list<UpdateLevel>   *m_UpdateLevel;   //更新等级
    list<UpdateExp>     *m_UpdateExp;     //更新经验经验
    list<UpdateGoods>   *m_UpdateGoods;   //更新背包物品
    list<UpdateEquAttr> *m_UpdateEquAttr; //更新装备属性
    list<UpdateTask>    *m_UpdateTask;    //更新任务进度


    boost::mutex         m_mtxUpMoney;    //更新金钱锁
    boost::mutex         m_mtxUpLevel;    //更新等级锁
    boost::mutex         m_mtxUpExp;      //更新经验经验锁
    boost::mutex         m_mtxUpGoods;    //更新背包物品锁
    boost::mutex         m_mtxUpEqu;      //更新装备属性锁
    boost::mutex         m_mtxUpTask;     //更新任务进度锁

    //下面这些标记主要为在玩家需要往list中插入数据时。保证快速得到锁。
    //在list中数据比较多的情况下，每次需要向list中插入数据时将标志置为1，插入完成后将标志置为0.
    hf_uint8             m_upMoneyFlag;   //是否在操作money list的标志
    hf_uint8             m_upLevelFlag;   //是否在操作Level list的标志
    hf_uint8             m_upExpFlag;     //是否在操作Exp list的标志
    hf_uint8             m_upGoodsFlag;   //是否在操作Goods list的标志
    hf_uint8             m_upEquFlag;     //是否在操作Equ list的标志
    hf_uint8             m_upTaskFlag;    //是否在操作Task list的标志
};

#endif // OPERATIONPOSTGRES_H
