#ifndef OPERATIONPOSTGRES_H
#define OPERATIONPOSTGRES_H

#include "Game/postgresqlstruct.h"
#include <boost/lockfree/queue.hpp>

class OperationPostgres
{
public:
    OperationPostgres();
    ~OperationPostgres();

    //该函数负责实时将玩家数据写入数据库
    void UpdatePlayerData();

    void PushUpdateMoney(hf_uint32 roleid, STR_PlayerMoney* money)
    {
        UpdateMoney upMoney(roleid, money);
        m_UpdateMoney->push(upMoney);
    }

    void PushUpdateLevel(hf_uint32 roleid, hf_uint8 level)
    {
        UpdateLevel upLevel(roleid, level);
        m_UpdateLevel->push(upLevel);
    }

    void PushUpdateExp(hf_uint32 roleid, hf_uint32 exp)
    {
        UpdateExp upExp(roleid, exp);
        m_UpdateExp->push(upExp);
    }

    void PushUpdateGoods(hf_uint32 roleid, STR_Goods* goods, hf_uint8 operate)
    {
        UpdateGoods upGoods(roleid, goods, operate);
        m_UpdateGoods->push(upGoods);
    }

    void PushUpdateEquAttr(hf_uint32 roleid, STR_Equipment* equ, hf_uint8 operate)
    {
        UpdateEquAttr upEpq(roleid, equ, operate);
        m_UpdateEquAttr->push(upEpq);
    }

    void PushUpdateTask(hf_uint32 roleid, STR_TaskProcess* task, hf_uint8 operate)
    {
        UpdateTask upTask(roleid, task, operate);
        m_UpdateTask->push(upTask);
    }

private:
    boost::lockfree::queue<UpdateMoney>     *m_UpdateMoney;   //更新金钱
    boost::lockfree::queue<UpdateLevel>     *m_UpdateLevel;   //更新等级
    boost::lockfree::queue<UpdateExp>       *m_UpdateExp;     //更新经验
    boost::lockfree::queue<UpdateGoods>     *m_UpdateGoods;   //更新背包物品
    boost::lockfree::queue<UpdateEquAttr>   *m_UpdateEquAttr; //更新装备属性
    boost::lockfree::queue<UpdateTask>      *m_UpdateTask;    //更新任务进度

};

#endif // OPERATIONPOSTGRES_H
