#include "cmdparse.h"
#include "GameTask/gametask.h"
#include "TeamFriend/teamfriend.h"
#include "OperationGoods/operationgoods.h"
#include "Game/userposition.hpp"
#include "server.h"

CmdParse::CmdParse():
    m_AskTask(new boost::lockfree::queue<Queue_AskTask>(100)),
    m_QuitTask(new boost::lockfree::queue<Queue_QuitTask>(100)),
    m_AskFinishTask(new boost::lockfree::queue<Queue_AskFinishTask>(100)),
    m_AskTaskExeDlg(new boost::lockfree::queue<Queue_AskTaskExeDlg>(100)),
    m_AskTaskExeDlgFinish(new boost::lockfree::queue<Queue_AskTaskExeDlg>(100)),
    m_AskTaskData(new boost::lockfree::queue<Queue_AskTaskData>(100)),
    m_AddFriend(new boost::lockfree::queue<Queue_AddFriend>(100)),
    m_DeleteFriend(new boost::lockfree::queue<Queue_DeleteFriend>(100)),
    m_AddFriendReturn(new boost::lockfree::queue<Queue_AddFriendReturn>(100)),
    m_PickGoods(new boost::lockfree::queue<Queue_PickGoods>(100)),
    m_RemoveGoods(new boost::lockfree::queue<Queue_RemoveGoods>(100)),
    m_MoveGoods(new boost::lockfree::queue<Queue_MoveGoods>(100)),
    m_BuyGoods(new boost::lockfree::queue<Queue_BuyGoods>(100)),
    m_SellGoods(new boost::lockfree::queue<Queue_SellGoods>(100)),
    m_WearBodyEqu(new boost::lockfree::queue<Queue_WearBodyEqu>(100)),
    m_TakeOffBodyEqu(new boost::lockfree::queue<Queue_TakeOffBodyEqu>(100)),
    m_PlayerDirectChange(new boost::lockfree::queue<Queue_PlayerDirectChange>(100)),
    m_PlayerActionChange(new boost::lockfree::queue<Queue_PlayerActionChange>(100)),
    m_PlayerMove(new boost::lockfree::queue<Queue_PlayerMove>(100)),
    m_AttackAim(new boost::lockfree::queue<Queue_AttackAim>(100)),
    m_AttackPoint(new boost::lockfree::queue<Queue_AttackPoint>(100))
{

}

CmdParse::~CmdParse()
{
    delete m_AskTask;
    delete m_QuitTask;
    delete m_AskFinishTask;
    delete m_AskTaskExeDlg;
    delete m_AskTaskExeDlgFinish;
    delete m_AskTaskData;
    delete m_AddFriend;
    delete m_DeleteFriend;
    delete m_AddFriendReturn;
    delete m_PickGoods;
    delete m_RemoveGoods;
    delete m_MoveGoods;
    delete m_BuyGoods;
    delete m_SellGoods;
    delete m_WearBodyEqu;
    delete m_TakeOffBodyEqu;
    delete m_PlayerDirectChange;
    delete m_PlayerActionChange;
    delete m_PlayerMove;
    delete m_AttackAim;
    delete m_AttackPoint;
}

void CmdParse::PushAskTask(TCPConnection::Pointer conn, hf_uint32 taskID)
{
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    hf_uint32 roleid = (*smap)[conn].m_roleid;
    Queue_AskTask t_task(roleid, taskID);
    while(!m_AskTask->push(t_task));
}

void CmdParse::PopAskTask()
{
    GameTask* gameTask = Server::GetInstance()->GetGameTask();
    umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
    Queue_AskTask t_task;
    while(1)
    {       
//        umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
        if(m_AskTask->pop(t_task))
        {
            _umap_roleSock::iterator it = t_roleSock->find(t_task.roleid);
            if(it != t_roleSock->end())
            {
                gameTask->AskTask(it->second, t_task.taskID);
            }
        }
        else
        {
            usleep(1000);
        }

//        for(hf_uint32 i = 0; i < m_AskTask->size(); i++)
//        {
//            Queue_AskTask t_task = m_AskTask->front();
//            gameTask->AskTask(t_task.conn, t_task.taskID);
//            m_AskTask->pop();
//        }
    }
}



void CmdParse::PushQuitTask(TCPConnection::Pointer conn, hf_uint32 taskID)
{
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    hf_uint32 roleid = (*smap)[conn].m_roleid;

    Queue_QuitTask t_task(roleid, taskID);
    while(!m_QuitTask->push(t_task));
}

void CmdParse::PopQuitTask()
{
    GameTask* gameTask = Server::GetInstance()->GetGameTask();
    umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
    while(1)
    {
//        umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
        Queue_QuitTask t_task;
        if(m_QuitTask->pop(t_task))
        {
            _umap_roleSock::iterator it = t_roleSock->find(t_task.roleid);
            if(it != t_roleSock->end())
            {
                gameTask->QuitTask(it->second, t_task.taskID);
            }
        }
        else
        {
            usleep(10000);
        }
    }
}

void CmdParse::PushAskFinishTask(TCPConnection::Pointer conn, STR_FinishTask* finishTask)
{
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    hf_uint32 roleid = (*smap)[conn].m_roleid;

    Queue_AskFinishTask t_task(roleid, finishTask);
    m_AskFinishTask->push(t_task);
}

void CmdParse::PopAskFinishTask()
{
    GameTask* gameTask = Server::GetInstance()->GetGameTask();
    umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
    Queue_AskFinishTask t_task;
    while(1)
    {
//        umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
        if(m_AskFinishTask->pop(t_task))
        {
            _umap_roleSock::iterator it = t_roleSock->find(t_task.roleid);
            if(it != t_roleSock->end())
            {
                gameTask->AskFinishTask(it->second, &t_task.finishTask);
            }
        }
        else
        {
            usleep(10000);
        }
    }
}

//请求任务执行对话
void CmdParse::PushAskTaskExeDlg(TCPConnection::Pointer conn, STR_AskTaskExeDlg* exeDlg)
{
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    hf_uint32 roleid = (*smap)[conn].m_roleid;

    Queue_AskTaskExeDlg t_task(roleid, exeDlg);
    m_AskTaskExeDlg->push(t_task);
}

void CmdParse::PopAskTaskExeDlg()
{
    GameTask* gameTask = Server::GetInstance()->GetGameTask();
    umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
    Queue_AskTaskExeDlg t_task;
    while(1)
    {
//        umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
        if(m_AskTaskExeDlg->pop(t_task))
        {
            _umap_roleSock::iterator it = t_roleSock->find(t_task.roleid);
            if(it != t_roleSock->end())
            {
                gameTask->AskTaskExeDialog(it->second, &t_task.exeDlg);
            }
        }
        else
        {
            usleep(10000);
        }
    }
}

//请求任务执行对话完成
void CmdParse::PushAskTaskExeDlgFinish(TCPConnection::Pointer conn, STR_AskTaskExeDlg* exeDlg)
{
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    hf_uint32 roleid = (*smap)[conn].m_roleid;

    Queue_AskTaskExeDlg t_task(roleid, exeDlg);
    m_AskTaskExeDlgFinish->push(t_task);
}

void CmdParse::PopAskTaskExeDlgFinish()
{
    GameTask* gameTask = Server::GetInstance()->GetGameTask();
    umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
    Queue_AskTaskExeDlg t_task;
    while(1)
    {
//        umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
        if(m_AskTaskExeDlgFinish->pop(t_task))
        {
            _umap_roleSock::iterator it = t_roleSock->find(t_task.roleid);
            if(it != t_roleSock->end())
            {
                gameTask->TaskExeDialogFinish(it->second, &t_task.exeDlg);
            }
        }
        else
        {
            usleep(10000);
        }
    }
}

void CmdParse::PushAskTaskData(TCPConnection::Pointer conn, hf_uint32 taskID, hf_uint16 flag)
{
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    hf_uint32 roleid = (*smap)[conn].m_roleid;

    Queue_AskTaskData t_task(roleid, taskID, flag);
    m_AskTaskData->push(t_task);
}

void CmdParse::PopAskTaskData()
{
    GameTask* gameTask = Server::GetInstance()->GetGameTask();
    umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
    Queue_AskTaskData t_task;
    while(1)
    {
//        umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
        if(m_AskTaskData->pop(t_task))
        {
            _umap_roleSock::iterator it = t_roleSock->find(t_task.roleid);
            if(it != t_roleSock->end())
            {
                switch(t_task.flag)
                {
                case FLAG_TaskStartDlg:
                {
                    gameTask->StartTaskDlg(it->second, t_task.taskID);
                    break;
                }
                case FLAG_TaskFinishDlg:
                {
                    gameTask->FinishTaskDlg(it->second, t_task.taskID);
                    break;
                }
                case FLAG_TaskDescription:
                {
                    gameTask->TaskDescription(it->second, t_task.taskID);
                    break;
                }
                case FLAG_TaskAim:
                {
                    gameTask->TaskAim(it->second, t_task.taskID);
                    break;
                }
                case FLAG_TaskReward:
                {
                    gameTask->TaskReward(it->second, t_task.taskID);
                    break;
                }
                default:
                    break;
                }
            }
        }
        else
        {
            usleep(10000);
        }
    }
}

void CmdParse::PushAddFriend(TCPConnection::Pointer conn, STR_PackAddFriend* addFriend)
{
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    hf_uint32 roleid = (*smap)[conn].m_roleid;

    Queue_AddFriend t_friend(roleid, addFriend);
    m_AddFriend->push(t_friend);
}

void CmdParse::PopAddFriend()
{
    TeamFriend* teamFriend = Server::GetInstance()->GetTeamFriend();
    umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
    Queue_AddFriend t_friend;
    while(1)
    {
//        umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
        if(m_AddFriend->pop(t_friend))
        {
            _umap_roleSock::iterator it = t_roleSock->find(t_friend.roleid);
            if(it != t_roleSock->end())
            {
               teamFriend->addFriend(it->second, &t_friend.addFriend);
            }
        }
        else
        {
            usleep(10000);
        }
    }
}

void CmdParse::PushDeleteFriend(TCPConnection::Pointer conn, hf_uint32 deleteRoleid)
{
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    hf_uint32 roleid = (*smap)[conn].m_roleid;

    Queue_DeleteFriend t_friend(roleid, deleteRoleid);
    m_DeleteFriend->push(t_friend);
}

void CmdParse::PopDeleteFriend()
{
    TeamFriend* teamFriend = Server::GetInstance()->GetTeamFriend();
    umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
    Queue_DeleteFriend t_friend;
    while(1)
    {
//        umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
        if(m_DeleteFriend->pop(t_friend))
        {
            _umap_roleSock::iterator it = t_roleSock->find(t_friend.roleid);
            if(it != t_roleSock->end())
            {
               teamFriend->deleteFriend(it->second, t_friend.roleid);
            }
        }
        else
        {
            usleep(10000);
        }
    }
}

void CmdParse::PushAddFriendReturn(TCPConnection::Pointer conn, STR_PackAddFriendReturn* addFriendReturn)
{
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    hf_uint32 roleid = (*smap)[conn].m_roleid;

    Queue_AddFriendReturn t_friend(roleid, addFriendReturn);
    m_AddFriendReturn->push(t_friend);
}

void CmdParse::PopAddFriendReturn()
{
    TeamFriend* teamFriend = Server::GetInstance()->GetTeamFriend();
    umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
    Queue_AddFriendReturn t_friend;
    while(1)
    {
//        umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
        if(m_AddFriendReturn->pop(t_friend))
        {
            _umap_roleSock::iterator it = t_roleSock->find(t_friend.roleid);
            if(it != t_roleSock->end())
            {
               teamFriend->ReciveAddFriend(it->second, &t_friend.addFriendReturn);
            }
        }
        else
        {
            usleep(10000);
        }
    }
}


void CmdParse::PushPickGoods(TCPConnection::Pointer conn, STR_PickGoods* pickGoods)
{
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    hf_uint32 roleid = (*smap)[conn].m_roleid;

    Queue_PickGoods t_goods(roleid, pickGoods);
    m_PickGoods->push(t_goods);
}

void CmdParse::PopPickGoods()
{
    OperationGoods* optGoods = Server::GetInstance()->GetOperationGoods();
//    umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
    Queue_PickGoods t_goods;
    while(1)
    {
        umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
        if(m_PickGoods->pop(t_goods))
        {
            _umap_roleSock::iterator it = t_roleSock->find(t_goods.roleid);
            if(it != t_roleSock->end())
            {
               cout << "捡东西:" << t_goods.roleid << ",掉落者：" << t_goods.pickGoods.GoodsFlag << ",物品ID：" << t_goods.pickGoods.LootGoodsID << endl;
               optGoods->PickUpGoods(it->second, &t_goods.pickGoods);
            }
        }
        else
        {
            usleep(10000);
        }
    }
}

void CmdParse::PushRemoveGoods(TCPConnection::Pointer conn, STR_RemoveBagGoods* removeGoods)
{
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    hf_uint32 roleid = (*smap)[conn].m_roleid;

    Queue_RemoveGoods t_goods(roleid, removeGoods);
    m_RemoveGoods->push(t_goods);
}

void CmdParse::PopRemoveGoods()
{
    OperationGoods* optGoods = Server::GetInstance()->GetOperationGoods();
//    umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
    Queue_RemoveGoods t_goods;
    while(1)
    {       
        umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
        if(m_RemoveGoods->pop(t_goods))
        {
            _umap_roleSock::iterator it = t_roleSock->find(t_goods.roleid);
            if(it != t_roleSock->end())
            {
               optGoods->RemoveBagGoods(it->second, &t_goods.removeGoods);
            }
        }
        else
        {
            usleep(10000);
        }
    }
}

void CmdParse::PushMoveGoods(TCPConnection::Pointer conn, STR_MoveBagGoods* moveGoods)
{
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    hf_uint32 roleid = (*smap)[conn].m_roleid;

    Queue_MoveGoods t_goods(roleid, moveGoods);
    m_MoveGoods->push(t_goods);
}

void CmdParse::PopMoveGoods()
{
    OperationGoods* optGoods = Server::GetInstance()->GetOperationGoods();
//    umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
    Queue_MoveGoods t_goods;
    while(1)
    {
        umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
        if(m_MoveGoods->pop(t_goods))
        {
            _umap_roleSock::iterator it = t_roleSock->find(t_goods.roleid);
            if(it != t_roleSock->end())
            {
               optGoods->MoveBagGoods(it->second, &t_goods.moveGoods);
            }
        }
        else
        {
            usleep(10000);
        }
    }
}

void CmdParse::PushBuyGoods(TCPConnection::Pointer conn, STR_BuyGoods* buyGoods)
{
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    hf_uint32 roleid = (*smap)[conn].m_roleid;

    Queue_BuyGoods t_goods(roleid, buyGoods);
    m_BuyGoods->push(t_goods);
}

void CmdParse::PopBuyGoods()
{
    OperationGoods* optGoods = Server::GetInstance()->GetOperationGoods();
//    umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
    Queue_BuyGoods t_goods;
    while(1)
    {
        umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
        if(m_BuyGoods->pop(t_goods))
        {
            _umap_roleSock::iterator it = t_roleSock->find(t_goods.roleid);
            if(it != t_roleSock->end())
            {
               optGoods->BuyGoods(it->second, &t_goods.buyGoods);
            }
        }
        else
        {
            usleep(10000);
        }
    }
}

void CmdParse::PushSellGoods(TCPConnection::Pointer conn, STR_SellGoods* sellGoods)
{
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    hf_uint32 roleid = (*smap)[conn].m_roleid;

    Queue_SellGoods t_goods(roleid, sellGoods);
    m_SellGoods->push(t_goods);
}

void CmdParse::PopSellGoods()
{
    OperationGoods* optGoods = Server::GetInstance()->GetOperationGoods();
//    umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
    Queue_SellGoods t_goods;
    while(1)
    {
        umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
        if(m_SellGoods->pop(t_goods))
        {
            _umap_roleSock::iterator it = t_roleSock->find(t_goods.roleid);
            if(it != t_roleSock->end())
            {
                optGoods->SellGoods(it->second, &t_goods.sellGoods);
            }
        }
        else
        {
            usleep(10000);
        }
    }
}

//穿装备
void CmdParse::PushWearBodyEqu(TCPConnection::Pointer conn, STR_WearEqu* wearEqu)
{
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    hf_uint32 roleid = (*smap)[conn].m_roleid;

    Queue_WearBodyEqu t_bodyEqu(roleid, wearEqu);
    m_WearBodyEqu->push(t_bodyEqu);
}

void CmdParse::PopWearBodyEqu()
{
    OperationGoods* optGoods = Server::GetInstance()->GetOperationGoods();
//    umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
    Queue_WearBodyEqu t_bodyEqu;
    while(1)
    {
        umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
        if(m_WearBodyEqu->pop(t_bodyEqu))
        {
            _umap_roleSock::iterator it = t_roleSock->find(t_bodyEqu.roleid);
            if(it != t_roleSock->end())
            {
                optGoods->WearBodyEqu(it->second, t_bodyEqu.wearEqu.equid, t_bodyEqu.wearEqu.pos);
            }
        }
        else
        {
            usleep(10000);
        }
    }
}

//脱装备
void CmdParse::PushTakeOffBodyEqu(TCPConnection::Pointer conn, hf_uint32 equid)
{
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    hf_uint32 roleid = (*smap)[conn].m_roleid;

    Queue_TakeOffBodyEqu t_bodyEqu(roleid, equid);
    m_TakeOffBodyEqu->push(t_bodyEqu);
}

void CmdParse::PopTakeOffBodyEqu()
{
    OperationGoods* optGoods = Server::GetInstance()->GetOperationGoods();
//    umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
    Queue_TakeOffBodyEqu t_bodyEqu;
    while(1)
    {
        umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
        if(m_TakeOffBodyEqu->pop(t_bodyEqu))
        {
            _umap_roleSock::iterator it = t_roleSock->find(t_bodyEqu.roleid);
            if(it != t_roleSock->end())
            {
                optGoods->TakeOffBodyEqu(it->second, t_bodyEqu.equid);
            }
        }
        else
        {
            usleep(10000);
        }
    }
}

void CmdParse::PushPlayerDirectChange(TCPConnection::Pointer conn, hf_float direct)
{
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    hf_uint32 roleid = (*smap)[conn].m_roleid;
    Queue_PlayerDirectChange t_direct(roleid, direct);
    m_PlayerDirectChange->push(t_direct);
}

void CmdParse::PopPlayerDirectChange()
{
    Queue_PlayerDirectChange t_direct;
    while(1)
    {
        umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
        if(m_PlayerDirectChange->pop(t_direct))
        {
            _umap_roleSock::iterator it = t_roleSock->find(t_direct.roleid);
            if(it != t_roleSock->end())
            {
                UserPosition::PlayerDirectChange(it->second, t_direct.direct);
            }
        }
        else
        {
            usleep(1000);
        }
    }
}

void CmdParse::PushPlayerActionChange(TCPConnection::Pointer conn, hf_uint8 action)
{
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    hf_uint32 roleid = (*smap)[conn].m_roleid;
    Queue_PlayerActionChange t_action(roleid, action);
    m_PlayerActionChange->push(t_action);
}

void CmdParse::PopPlayerActionChange()
{
    Queue_PlayerActionChange t_action;
    while(1)
    {
        umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
        if(m_PlayerActionChange->pop(t_action))
        {
            _umap_roleSock::iterator it = t_roleSock->find(t_action.roleid);
            if(it != t_roleSock->end())
            {
                UserPosition::PlayerActionChange(it->second, t_action.action);
            }
        }
        else
        {
            usleep(1000);
        }
    }
}

void CmdParse::PushPlayerMove(TCPConnection::Pointer conn, STR_PlayerMove* playerMove)
{
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    hf_uint32 roleid = (*smap)[conn].m_roleid;

    Queue_PlayerMove t_move(roleid, playerMove);
    m_PlayerMove->push(t_move);
}

void CmdParse::PopPlayerMove()
{
//    umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
    Queue_PlayerMove t_move;
    while(1)
    {
        umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
        if(m_PlayerMove->pop(t_move))
        {
            _umap_roleSock::iterator it = t_roleSock->find(t_move.roleid);
            if(it != t_roleSock->end())
            {
                UserPosition::PlayerPositionMove(it->second, &t_move.playerMove);
            }
        }
        else
        {
            usleep(1000);
        }
    }
}

void CmdParse::PushAttackAim(TCPConnection::Pointer conn, STR_PackUserAttackAim* attackAim)
{
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    hf_uint32 roleid = (*smap)[conn].m_roleid;

    Queue_AttackAim t_attack(roleid, attackAim);
    m_AttackAim->push(t_attack);
}

void CmdParse::PopAttackAim()
{
    GameAttack* gameAttack = Server::GetInstance()->GetGameAttack();
//    umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
    Queue_AttackAim t_attack;
    while(1)
    {
        umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
        if(m_AttackAim->pop(t_attack))
        {
            _umap_roleSock::iterator it = t_roleSock->find(t_attack.roleid);
            if(it != t_roleSock->end())
            {
                gameAttack->AttackAim(it->second, &t_attack.attackAim);
            }
        }
        else
        {
            usleep(1000);
        }
    }
}

void CmdParse::PushAttackPoint(TCPConnection::Pointer conn, STR_PackUserAttackPoint* attackPoint)
{
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    hf_uint32 roleid = (*smap)[conn].m_roleid;

    Queue_AttackPoint t_attack(roleid, attackPoint);
    m_AttackPoint->push(t_attack);
}

void CmdParse::PopAttackPoint()
{
    GameAttack* gameAttack = Server::GetInstance()->GetGameAttack();
//    umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
    Queue_AttackPoint t_attack;
    while(1)
    {
        umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
        if(m_AttackPoint->pop(t_attack))
        {
            _umap_roleSock::iterator it = t_roleSock->find(t_attack.roleid);
            if(it != t_roleSock->end())
            {
                gameAttack->AttackPoint(it->second, &t_attack.attackPoint);
            }
        }
        else
        {
            usleep(1000);
        }
    }
}
