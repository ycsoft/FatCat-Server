#include "cmdparse.h"
#include "GameTask/gametask.h"
#include "TeamFriend/teamfriend.h"
#include "OperationGoods/operationgoods.h"
#include "Game/userposition.hpp"
#include "server.h"

CmdParse::CmdParse():
    m_AskTask(new queue<Queue_AskTask>),
    m_QuitTask(new queue<Queue_QuitTask>),
    m_AskFinishTask(new queue<Queue_AskFinishTask>),
    m_AskTaskData(new queue<Queue_AskTaskData>),
    m_AddFriend(new queue<Queue_AddFriend>),
    m_DeleteFriend(new queue<Queue_DeleteFriend>),
    m_AddFriendReturn(new queue<Queue_AddFriendReturn>),    
    m_PickGoods(new queue<Queue_PickGoods>),
    m_RemoveGoods(new queue<Queue_RemoveGoods>),
    m_MoveGoods(new queue<Queue_MoveGoods>),
    m_BuyGoods(new queue<Queue_BuyGoods>),
    m_SellGoods(new queue<Queue_SellGoods>),
    m_PlayerMove(new queue<Queue_PlayerMove>),
    m_UserAttackAim(new queue<Queue_UserAttackAim>),
    m_UserAttackPoint(new queue<Queue_UserAttackPoint>)
{

}

void CmdParse::PushAskTask(TCPConnection::Pointer conn, hf_uint32 taskID)
{
    Queue_AskTask t_task(conn, taskID);
    m_AskTask->push(t_task);
}

void CmdParse::PopAskTask()
{
    GameTask* gameTask = Server::GetInstance()->GetGameTask();
    while(1)
    {
        for(hf_uint32 i = 0; i < m_AskTask->size(); i++)
        {
            Queue_AskTask t_task = m_AskTask->front();
            gameTask->AskTask(t_task.conn, t_task.taskID);
            m_AskTask->pop();
        }
    }
}


void CmdParse::PushQuitTask(TCPConnection::Pointer conn, hf_uint32 taskID)
{
    Queue_QuitTask t_task(conn, taskID);
    m_QuitTask->push(t_task);
}

void CmdParse::PopQuitTask()
{
    GameTask* gameTask = Server::GetInstance()->GetGameTask();
    while(1)
    {
        for(hf_uint32 i = 0; i < m_QuitTask->size(); i++)
        {
            Queue_QuitTask t_task = m_QuitTask->front();
            gameTask->QuitTask(t_task.conn, t_task.taskID);
            m_QuitTask->pop();
        }
    }
}

void CmdParse::PushAskFinishTask(TCPConnection::Pointer conn, STR_FinishTask* finishTask)
{
    Queue_AskFinishTask t_task(conn, finishTask);
    m_AskFinishTask->push(t_task);
}

void CmdParse::PopAskFinishTask()
{
    GameTask* gameTask = Server::GetInstance()->GetGameTask();
    while(1)
    {
        for(hf_uint32 i = 0; i < m_AskFinishTask->size(); i++)
        {
            Queue_AskFinishTask t_task = m_AskFinishTask->front();
            gameTask->AskFinishTask(t_task.conn, &t_task.finishTask);
            m_AskFinishTask->pop();
        }
    }
}

void CmdParse::PushAskTaskData(TCPConnection::Pointer conn, hf_uint32 taskID, hf_uint16 flag)
{
    Queue_AskTaskData t_task(conn, taskID, flag);
    m_AskTaskData->push(t_task);
}

void CmdParse::PopAskTaskData()
{
    GameTask* gameTask = Server::GetInstance()->GetGameTask();
    while(1)
    {
        for(hf_uint32 i = 0; i < m_AskTaskData->size(); i++)
        {
            Queue_AskTaskData t_task = m_AskTaskData->front();
            switch(t_task.flag)
            {
            case FLAG_StartTaskDlg:
            {
                gameTask->StartTaskDlg(t_task.conn, t_task.taskID);
                break;
            }
            case FLAG_FinishTaskDlg:
            {
                gameTask->FinishTaskDlg(t_task.conn, t_task.taskID);
                break;
            }
            case FLAG_TaskDescription:
            {
                gameTask->TaskDescription(t_task.conn, t_task.taskID);
                break;
            }
            case FLAG_TaskAim:
            {
                gameTask->TaskAim(t_task.conn, t_task.taskID);
                break;
            }
            case FLAG_TaskReward:
            {
                gameTask->TaskReward(t_task.conn, t_task.taskID);
                break;
            }
            default:
                break;
            }
            m_AskTaskData->pop();
        }
    }
}

void CmdParse::PushAddFriend(TCPConnection::Pointer conn, STR_PackAddFriend* addFriend)
{
    Queue_AddFriend t_friend(conn, addFriend);
    m_AddFriend->push(t_friend);
}

void CmdParse::PopAddFriend()
{
    TeamFriend* teamFriend = Server::GetInstance()->GetTeamFriend();
    while(1)
    {
        for(hf_uint32 i = 0; i < m_AddFriend->size(); i++)
        {
            Queue_AddFriend t_friend = m_AddFriend->front();
            teamFriend->addFriend(t_friend.conn, &t_friend.addFriend);
            m_AddFriend->pop();
        }
    }
}

void CmdParse::PushDeleteFriend(TCPConnection::Pointer conn, hf_uint32 roleid)
{
    Queue_DeleteFriend t_friend(conn, roleid);
    m_DeleteFriend->push(t_friend);
}

void CmdParse::PopDeleteFriend()
{
    TeamFriend* teamFriend = Server::GetInstance()->GetTeamFriend();
    while(1)
    {
        for(hf_uint32 i = 0; i < m_DeleteFriend->size(); i++)
        {
            Queue_DeleteFriend t_friend = m_DeleteFriend->front();
            teamFriend->deleteFriend(t_friend.conn, t_friend.roleid);
            m_DeleteFriend->pop();
        }
    }
}

void CmdParse::PushAddFriendReturn(TCPConnection::Pointer conn, STR_PackAddFriendReturn* addFriendReturn)
{
    Queue_AddFriendReturn t_friend(conn, addFriendReturn);
    m_AddFriendReturn->push(t_friend);
}

void CmdParse::PopAddFriendReturn()
{
    TeamFriend* teamFriend = Server::GetInstance()->GetTeamFriend();
    while(1)
    {
        for(hf_uint32 i = 0; i < m_AddFriendReturn->size(); i++)
        {
            Queue_AddFriendReturn t_friend = m_AddFriendReturn->front();
            teamFriend->ReciveAddFriend(t_friend.conn, &t_friend.addFriendReturn);
            m_AddFriendReturn->pop();
        }
    }
}


void CmdParse::PushPickGoods(TCPConnection::Pointer conn, hf_uint16 len, STR_PickGoods* pickGoods)
{
    Queue_PickGoods t_goods(conn, len, pickGoods);
    m_PickGoods->push(t_goods);
}

void CmdParse::PopPickGoods()
{
    OperationGoods* optGoods = Server::GetInstance()->GetOperationGoods();
    while(1)
    {
        for(hf_uint32 i = 0; i < m_PickGoods->size(); i++)
        {
            Queue_PickGoods t_goods = m_PickGoods->front();
            optGoods->PickUpGoods(t_goods.conn, t_goods.len, t_goods.pickGoods);
            m_PickGoods->pop();
        }
    }
}

void CmdParse::PushRemoveGoods(TCPConnection::Pointer conn, STR_RemoveBagGoods* removeGoods)
{
    Queue_RemoveGoods t_goods(conn, removeGoods);
    m_RemoveGoods->push(t_goods);
}

void CmdParse::PopRemoveGoods()
{
    OperationGoods* optGoods = Server::GetInstance()->GetOperationGoods();
    while(1)
    {
        for(hf_uint32 i = 0; i < m_RemoveGoods->size(); i++)
        {
            Queue_RemoveGoods t_goods = m_RemoveGoods->front();
            optGoods->RemoveBagGoods(t_goods.conn, &t_goods.removeGoods);
            m_RemoveGoods->pop();
        }
    }
}

void CmdParse::PushMoveGoods(TCPConnection::Pointer conn, STR_MoveBagGoods* moveGoods)
{
    Queue_MoveGoods t_goods(conn, moveGoods);
    m_MoveGoods->push(t_goods);
}

void CmdParse::PopMoveGoods()
{
    OperationGoods* optGoods = Server::GetInstance()->GetOperationGoods();
    while(1)
    {
        for(hf_uint32 i = 0; i < m_MoveGoods->size(); i++)
        {
            Queue_MoveGoods t_goods = m_MoveGoods->front();
            optGoods->MoveBagGoods(t_goods.conn, &t_goods.moveGoods);
            m_MoveGoods->pop();
        }
    }
}

void CmdParse::PushBuyGoods(TCPConnection::Pointer conn, STR_BuyGoods* buyGoods)
{
    Queue_BuyGoods t_goods(conn, buyGoods);
    m_BuyGoods->push(t_goods);
}

void CmdParse::PopBuyGoods()
{
    OperationGoods* optGoods = Server::GetInstance()->GetOperationGoods();
    while(1)
    {
        for(hf_uint32 i = 0; i < m_BuyGoods->size(); i++)
        {
            Queue_BuyGoods t_goods = m_BuyGoods->front();
            optGoods->BuyGoods(t_goods.conn, &t_goods.buyGoods);
            m_BuyGoods->pop();
        }
    }
}

void CmdParse::PushSellGoods(TCPConnection::Pointer conn, STR_SellGoods* sellGoods)
{
    Queue_SellGoods t_goods(conn, sellGoods);
    m_SellGoods->push(t_goods);
}

void CmdParse::PopSellGoods()
{
    OperationGoods* optGoods = Server::GetInstance()->GetOperationGoods();
    while(1)
    {
        for(hf_uint32 i = 0; i < m_SellGoods->size(); i++)
        {
            Queue_SellGoods t_goods = m_SellGoods->front();
            optGoods->SellGoods(t_goods.conn, &t_goods.sellGoods);
            m_SellGoods->pop();
        }
    }
}

void CmdParse::PushPlayerMove(TCPConnection::Pointer conn, STR_PlayerMove* playerMove)
{
    Queue_PlayerMove t_move(conn, playerMove);
    m_PlayerMove->push(t_move);
}

void CmdParse::PopPlayerMove()
{
    while(1)
    {
        for(hf_uint32 i = 0; i < m_PlayerMove->size(); i++)
        {
            Queue_PlayerMove t_move = m_PlayerMove->front();
            UserPosition::PlayerPositionMove(t_move.conn, &t_move.playerMove);
            m_PlayerMove->pop();
        }
    }
}
