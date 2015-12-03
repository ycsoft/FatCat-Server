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
//    void UpdatePlayerData();

    void PushUpdateMoney(hf_uint32 roleid, STR_PlayerMoney* money)
    {
        UpdateMoney upMoney(roleid, money);
        m_UpdateMoney->push(upMoney);
    }
    void PopUpdateMoney();

    void PushUpdateLevel(hf_uint32 roleid, hf_uint8 level)
    {
        UpdateLevel upLevel(roleid, level);
        m_UpdateLevel->push(upLevel);
    }
    void PopUpdateLevel();

    void PushUpdateExp(hf_uint32 roleid, hf_uint32 exp)
    {
        UpdateExp upExp(roleid, exp);
        m_UpdateExp->push(upExp);
    }
    void PopUpdateExp();

    void PushUpdateGoods(hf_uint32 roleid, STR_Goods* goods, hf_uint8 operate)
    {
        UpdateGoods upGoods(roleid, goods, operate);
        m_UpdateGoods->push(upGoods);
    }
    void PopUpdateGoods();

    void PushUpdateEquAttr(hf_uint32 roleid, STR_EquipmentAttr* equ, hf_uint8 operate)
    {
        UpdateEquAttr upEpq(roleid, equ, operate);
        m_UpdateEquAttr->push(upEpq);
    }
    void PopUpdateEquAttr();

    void PushUpdateTask(hf_uint32 roleid, STR_TaskProcess* task, hf_uint8 operate)
    {
        UpdateTask upTask(roleid, task, operate);
        m_UpdateTask->push(upTask);
    }
    void PopUpdateTask();

    void PushUpdateCompleteTask(hf_uint32 roleid, hf_uint32 taskid)
    {
        UpdateCompleteTask upTask(roleid, taskid);
        m_UpdateCompleteTask->push(upTask);
    }

    void PopUpdateCompleteTask();

private:
    boost::lockfree::queue<UpdateMoney>     *m_UpdateMoney;   //更新金钱
    boost::lockfree::queue<UpdateLevel>     *m_UpdateLevel;   //更新等级
    boost::lockfree::queue<UpdateExp>       *m_UpdateExp;     //更新经验
    boost::lockfree::queue<UpdateGoods>     *m_UpdateGoods;   //更新背包物品
    boost::lockfree::queue<UpdateEquAttr>   *m_UpdateEquAttr; //更新装备属性
    boost::lockfree::queue<UpdateTask>      *m_UpdateTask;    //更新任务进度
    boost::lockfree::queue<UpdateCompleteTask> *m_UpdateCompleteTask; //更新已完成的任务

};

#endif // OPERATIONPOSTGRES_H
