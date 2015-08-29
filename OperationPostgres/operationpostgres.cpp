#include "operationpostgres.h"
#include "PlayerLogin/playerlogin.h"


OperationPostgres::OperationPostgres():
    m_UpdateMoney(new list<UpdateMoney>),
    m_UpdateLevel(new list<UpdateLevel>),
    m_UpdateExp(new list<UpdateExp>),
    m_UpdateGoods(new list<UpdateGoods>),
    m_UpdateEquAttr(new list<UpdateEquAttr>),
    m_UpdateTask(new list<UpdateTask>)
{
    m_upEquFlag = 0;
    m_upLevelFlag = 0;
    m_upExpFlag = 0;
    m_upGoodsFlag = 0;
    m_upEquFlag = 0;
    m_upTaskFlag = 0;
}

OperationPostgres::~OperationPostgres()
{

}

//该函数负责实时将玩家数据写入数据库
void OperationPostgres::UpdatePlayerData()
{
    while(1)
    {
        m_mtxUpMoney.lock();
        for(list<UpdateMoney>::iterator it = m_UpdateMoney->begin(); it != m_UpdateMoney->end();)
        {
            if(m_upMoneyFlag == 1)  //需要向list中插入数据，跳出执行下一个list
                break;
            list<UpdateMoney>::iterator _it = it;
            it++;
            PlayerLogin::UpdatePlayerMoney(&(*_it));
            m_UpdateMoney->erase(_it);
        }
        m_mtxUpMoney.unlock();

        m_mtxUpLevel.lock();
        for(list<UpdateLevel>::iterator it = m_UpdateLevel->begin(); it != m_UpdateLevel->end();)
        {
            if(m_upLevelFlag == 1)
                break;
            list<UpdateLevel>::iterator _it = it;
            it++;
            PlayerLogin::UpdatePlayerLevel(&(*_it));
            m_UpdateLevel->erase(_it);
        }
        m_mtxUpLevel.unlock();

        m_mtxUpExp.lock();
        for(list<UpdateExp>::iterator it = m_UpdateExp->begin(); it != m_UpdateExp->end();)
        {
            if(m_upExpFlag == 1)
                break;
            list<UpdateExp>::iterator _it = it;
            it++;
            PlayerLogin::UpdatePlayerExp(&(*_it));
            m_UpdateExp->erase(_it);
        }
        m_mtxUpExp.unlock();


        m_mtxUpGoods.lock();
        for(list<UpdateGoods>::iterator it = m_UpdateGoods->begin(); it != m_UpdateGoods->end();)
        {
            if(m_upGoodsFlag == 1)
                break;
            list<UpdateGoods>::iterator _it = it;
            it++;
            if(_it->Operate == PostUpdate)      //更新操作
            {
                PlayerLogin::UpdatePlayerGoods(_it->RoleID,&(_it->Goods));
            }
            else if(_it->Operate == PostInsert) //插入操作
            {
                PlayerLogin::InsertPlayerGoods(_it->RoleID,&(_it->Goods));
            }
            else if(_it->Operate == PostDelete) //删除操作
            {
                PlayerLogin::DeletePlayerGoods(_it->RoleID, _it->Goods.Position);
            }
            m_UpdateGoods->erase(_it);
        }
        m_mtxUpGoods.unlock();

        m_mtxUpEqu.lock();
        for(list<UpdateEquAttr>::iterator it = m_UpdateEquAttr->begin(); it != m_UpdateEquAttr->end();)
        {
            if(m_upEquFlag == 1)
                break;
            list<UpdateEquAttr>::iterator _it = it;
            it++;
            if(_it->Operate == PostUpdate)
            {
                PlayerLogin::UpdatePlayerEquAttr(_it->RoleID, &(_it->EquAttr));
            }
            else if(_it->Operate == PostInsert)
            {
                PlayerLogin::InsertPlayerEquAttr(_it->RoleID, &(_it->EquAttr));
            }
            else if(_it->Operate == PostDelete)
            {
                PlayerLogin::DeletePlayerEquAttr(_it->RoleID, _it->EquAttr.EquID);
            }
            m_UpdateEquAttr->erase(_it);
        }
        m_mtxUpEqu.unlock();


        m_mtxUpTask.lock();
        for(list<UpdateTask>::iterator it = m_UpdateTask->begin(); it != m_UpdateTask->end();)
        {
            if(m_upTaskFlag == 1)
                break;
            list<UpdateTask>::iterator _it = it;
            it++;
            if(_it->Operate == PostUpdate)
            {
                PlayerLogin::UpdatePlayerTask(_it->RoleID, &(_it->TaskProcess));
            }
            else if(_it->Operate == PostInsert)
            {
                PlayerLogin::InsertPlayerTask(_it->RoleID, &(_it->TaskProcess));
            }
            else if(_it->Operate == PostDelete)
            {
                PlayerLogin::DeletePlayerTask(_it->RoleID, _it->TaskProcess.TaskID);
            }
            m_UpdateTask->erase(_it);
        }
        m_mtxUpTask.unlock();       
    }
}
