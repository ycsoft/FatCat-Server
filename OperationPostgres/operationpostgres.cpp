#include "operationpostgres.h"
#include "PlayerLogin/playerlogin.h"

OperationPostgres::OperationPostgres():
    m_UpdateMoney(new boost::lockfree::queue<UpdateMoney>(100)),
    m_UpdateLevel(new boost::lockfree::queue<UpdateLevel>(100)),
    m_UpdateExp(new boost::lockfree::queue<UpdateExp>(100)),
    m_UpdateGoods(new boost::lockfree::queue<UpdateGoods>(100)),
    m_UpdateEquAttr(new boost::lockfree::queue<UpdateEquAttr>(100)),
    m_UpdateTask(new boost::lockfree::queue<UpdateTask>(100)),
    m_UpdateCompleteTask(new boost::lockfree::queue<UpdateCompleteTask>(100))
{

}

OperationPostgres::~OperationPostgres()
{
    delete m_UpdateMoney;
    delete m_UpdateLevel;
    delete m_UpdateExp;
    delete m_UpdateGoods;
    delete m_UpdateEquAttr;
    delete m_UpdateTask;
    delete m_UpdateCompleteTask;
}

//该函数负责实时将玩家数据写入数据库
//void OperationPostgres::UpdatePlayerData()
//{
//    while(1)
//    {

//    }
//}

void OperationPostgres::PopUpdateMoney()
{
    UpdateMoney t_updateMoney;
    while(1)
    {
        if(m_UpdateMoney->pop(t_updateMoney))
        {
            PlayerLogin::UpdatePlayerMoney(&t_updateMoney);
        }
        else
        {
           usleep(1000);
        }
    }
}

void OperationPostgres::PopUpdateLevel()
{
    UpdateLevel t_updateLevel;
    while(1)
    {
        if(m_UpdateLevel->pop(t_updateLevel))
        {
            PlayerLogin::UpdatePlayerLevel(&t_updateLevel);
        }
        else
        {
            usleep(1000);
        }
    }
}

void OperationPostgres::PopUpdateExp()
{
    UpdateExp t_updateExp;
    while(1)
    {
        if(m_UpdateExp->pop(t_updateExp))
        {
            PlayerLogin::UpdatePlayerExp(&t_updateExp);
        }
        else
        {
            usleep(1000);
        }
    }
}


void OperationPostgres::PopUpdateGoods()
{
    UpdateGoods t_updateGoods;
    while(1)
    {
        if(m_UpdateGoods->pop(t_updateGoods))
        {
            cout << t_updateGoods.Goods.GoodsID << endl;
            if(t_updateGoods.Operate == PostUpdate)
            {
                PlayerLogin::UpdatePlayerGoods(t_updateGoods.RoleID, &t_updateGoods.Goods);
            }
            else if(t_updateGoods.Operate == PostInsert)
            {
                PlayerLogin::InsertPlayerGoods(t_updateGoods.RoleID, &t_updateGoods.Goods);
            }
            else if(t_updateGoods.Operate == PostDelete)
            {
                PlayerLogin::DeletePlayerGoods(t_updateGoods.RoleID, t_updateGoods.Goods.Position);
            }
        }
        else
        {
            usleep(1000);
        }
    }
}

void OperationPostgres::PopUpdateEquAttr()
{
    UpdateEquAttr t_updateEquAttr;
    while(1)
    {
        if(m_UpdateEquAttr->pop(t_updateEquAttr))
        {
            if(t_updateEquAttr.Operate == PostUpdate)
            {
                PlayerLogin::UpdatePlayerEquAttr(t_updateEquAttr.RoleID, &t_updateEquAttr.EquAttr);
            }
            else if(t_updateEquAttr.Operate == PostInsert)
            {
                PlayerLogin::InsertPlayerEquAttr(t_updateEquAttr.RoleID, &t_updateEquAttr.EquAttr);
            }
            else if(t_updateEquAttr.Operate == PostDelete)
            {
                PlayerLogin::DeletePlayerEquAttr(t_updateEquAttr.RoleID, t_updateEquAttr.EquAttr.EquID);
            }
        }
        else
        {
            usleep(1000);
        }
    }
}

void OperationPostgres::PopUpdateTask()
{
    UpdateTask t_updateTask;
    while(1)
    {
        if(m_UpdateTask->pop(t_updateTask))
        {
            if(t_updateTask.Operate == PostUpdate)
            {
                PlayerLogin::UpdatePlayerTask(t_updateTask.RoleID, &t_updateTask.TaskProcess);
            }
            else if(t_updateTask.Operate == PostInsert)
            {
                PlayerLogin::InsertPlayerTask(t_updateTask.RoleID, &t_updateTask.TaskProcess);
            }
            else if(t_updateTask.Operate == PostDelete)
            {
                PlayerLogin::DeletePlayerTask(t_updateTask.RoleID, t_updateTask.TaskProcess.TaskID);
            }
        }
        else
        {
            usleep(1000);
        }
    }
}


void OperationPostgres::PopUpdateCompleteTask()
{
    UpdateCompleteTask t_comTask;
    while(1)
    {
        if(m_UpdateCompleteTask->pop(t_comTask))
            PlayerLogin::InsertPlayerCompleteTask(t_comTask.RoleID, t_comTask.TaskID);
        else
            usleep(1000);
    }
}
