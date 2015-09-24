#ifndef CMDPARSE_H
#define CMDPARSE_H

//#include <list>
#include <queue>

#include "NetWork/tcpconnection.h"
#include "Game/postgresqlstruct.h"


using namespace std;


typedef struct _Queue_AskTask
{
    _Queue_AskTask(TCPConnection::Pointer _conn, hf_uint32 _taskID)
        :conn(_conn),taskID(_taskID)
    {

    }
    TCPConnection::Pointer conn;
    hf_uint32  taskID;
}Queue_AskTask;


typedef struct _Queue_QuitTask
{
    _Queue_QuitTask(TCPConnection::Pointer _conn, hf_uint32 _taskID)
        :conn(_conn),taskID(_taskID)
    {

    }
    TCPConnection::Pointer conn;
    hf_uint32  taskID;
}Queue_QuitTask;

typedef struct _Queue_AskFinishTask
{
    _Queue_AskFinishTask(TCPConnection::Pointer _conn, STR_FinishTask* _finishTask)
        :conn(_conn)
    {
        finishTask.TaskID = _finishTask->TaskID;
        finishTask.SelectGoodsID = _finishTask->SelectGoodsID;
    }
    TCPConnection::Pointer conn;
    STR_FinishTask finishTask;
}Queue_AskFinishTask;

typedef struct _Queue_AskTaskData
{
    _Queue_AskTaskData(TCPConnection::Pointer _conn, hf_uint32 _taskID, hf_uint16 _flag)
        :conn(_conn),taskID(_taskID),flag(_flag)
    {

    }

    TCPConnection::Pointer conn;
    hf_uint32 taskID;
    hf_uint16 flag;
}Queue_AskTaskData;

typedef struct _Queue_AddFriend
{
    _Queue_AddFriend(TCPConnection::Pointer _conn, STR_PackAddFriend* _addFriend)
        :conn(_conn)
    {
        memcpy(&addFriend, _addFriend, sizeof(STR_PackAddFriend));
    }

    TCPConnection::Pointer conn;
    STR_PackAddFriend addFriend;
}Queue_AddFriend;

typedef struct _Queue_DeleteFriend
{
    _Queue_DeleteFriend(TCPConnection::Pointer _conn, hf_uint32 _roleid)
        :conn(_conn), roleid(_roleid)
    {

    }

    TCPConnection::Pointer conn;
    hf_uint32 roleid;
}Queue_DeleteFriend;

typedef struct _Queue_AddFriendReturn
{
    _Queue_AddFriendReturn(TCPConnection::Pointer _conn, STR_PackAddFriendReturn* _addFriendReturn)
        :conn(_conn)
    {
        memcpy(&addFriendReturn, _addFriendReturn, sizeof(STR_PackAddFriendReturn));
    }

    TCPConnection::Pointer conn;
    STR_PackAddFriendReturn addFriendReturn;
}Queue_AddFriendReturn;

typedef struct _Queue_PickGoods
{
    _Queue_PickGoods(TCPConnection::Pointer _conn, hf_uint16 _len, STR_PickGoods* _pickGoods)
        :conn(_conn), len(_len)
    {
        memcpy(pickGoods, _pickGoods, len);
    }
    TCPConnection::Pointer conn;
    hf_uint16 len;
    STR_PickGoods pickGoods[5];
}Queue_PickGoods;

typedef struct _Queue_RemoveGoods
{
    _Queue_RemoveGoods(TCPConnection::Pointer _conn, STR_RemoveBagGoods* _removeGoods)
        :conn(_conn)
    {
        memcpy(&removeGoods, _removeGoods, sizeof(STR_RemoveBagGoods));
    }

    TCPConnection::Pointer conn;
    STR_RemoveBagGoods removeGoods;
}Queue_RemoveGoods;

typedef struct _Queue_MoveGoods
{
    _Queue_MoveGoods(TCPConnection::Pointer _conn, STR_MoveBagGoods* _moveGoods)
        :conn(_conn)
    {
        memcpy(&moveGoods, _moveGoods, sizeof(STR_MoveBagGoods));
    }
    TCPConnection::Pointer conn;
    STR_MoveBagGoods moveGoods;
}Queue_MoveGoods;

typedef struct _Queue_BuyGoods
{
    _Queue_BuyGoods(TCPConnection::Pointer _conn, STR_BuyGoods* _buyGoods)
        :conn(_conn)
    {
        memcpy(&buyGoods, _buyGoods, sizeof(STR_BuyGoods));
    }
    TCPConnection::Pointer conn;
    STR_BuyGoods buyGoods;
}Queue_BuyGoods;

typedef struct _Queue_SellGoods
{
    _Queue_SellGoods(TCPConnection::Pointer _conn, STR_SellGoods* _sellGoods)
        :conn(_conn)
    {
        memcpy(&sellGoods, _sellGoods, sizeof(STR_SellGoods));
    }
    TCPConnection::Pointer conn;
    STR_SellGoods sellGoods;
}Queue_SellGoods;

typedef struct _Queue_WearBodyEqu
{
    _Queue_WearBodyEqu(TCPConnection::Pointer _conn, STR_WearEqu* _wearEqu)
        :conn(_conn)
    {
        memcpy(&wearEqu, _wearEqu, sizeof(STR_WearEqu));
    }
    TCPConnection::Pointer conn;
    STR_WearEqu wearEqu;
}Queue_WearBodyEqu;

typedef struct _Queue_TakeOffBodyEqu
{
    _Queue_TakeOffBodyEqu(TCPConnection::Pointer _conn, hf_uint32 _equid)
        :conn(_conn),equid(_equid)
    {

    }
    TCPConnection::Pointer conn;
    hf_uint32 equid;
}Queue_TakeOffBodyEqu;



typedef struct _Queue_PlayerMove
{
    _Queue_PlayerMove(TCPConnection::Pointer _conn, STR_PlayerMove* _playerMove)
        :conn(_conn)
    {
        memcpy(&playerMove, _playerMove, sizeof(STR_PlayerMove));
    }

    TCPConnection::Pointer conn;
    STR_PlayerMove playerMove;
}Queue_PlayerMove;

typedef struct _Queue_AttackAim
{
    _Queue_AttackAim(TCPConnection::Pointer _conn, STR_PackUserAttackAim* _attackAim)
        :conn(_conn)
    {
        memcpy(&attackAim, _attackAim, sizeof(STR_PackUserAttackAim));
    }
    TCPConnection::Pointer conn;
    STR_PackUserAttackAim attackAim;
}Queue_AttackAim;

typedef struct _Queue_AttackPoint
{
    _Queue_AttackPoint(TCPConnection::Pointer _conn, STR_PackUserAttackPoint* _attackPoint)
        :conn(_conn)
    {
        memcpy(&attackPoint, _attackPoint, sizeof(STR_PackUserAttackPoint));
    }
    TCPConnection::Pointer conn;
    STR_PackUserAttackPoint attackPoint;
}Queue_AttackPoint;


class CmdParse
{
public:
    CmdParse();

    //Task
    void PushAskTask(TCPConnection::Pointer conn, hf_uint32 taskID);   //请求任务
    void PopAskTask();

    void PushQuitTask(TCPConnection::Pointer conn, hf_uint32 taskID);  //放弃任务
    void PopQuitTask();

    void PushAskFinishTask(TCPConnection::Pointer conn, STR_FinishTask* finishTask); //请求完成任务
    void PopAskFinishTask();

    void PushAskTaskData(TCPConnection::Pointer conn, hf_uint32 taskID, hf_uint16 flag); //请求任务数据
    void PopAskTaskData();


    //friend
    void PushAddFriend(TCPConnection::Pointer conn, STR_PackAddFriend* addFriend); //添加好友
    void PopAddFriend();

    void PushDeleteFriend(TCPConnection::Pointer conn, hf_uint32 roleid); //删除好友
    void PopDeleteFriend();

    void PushAddFriendReturn(TCPConnection::Pointer conn, STR_PackAddFriendReturn* addFriendReturn); //添加好友返回
    void PopAddFriendReturn();

    //goods
    void PushPickGoods(TCPConnection::Pointer conn, hf_uint16 len, STR_PickGoods* pickGoods); //拾取物品
    void PopPickGoods();

    void PushRemoveGoods(TCPConnection::Pointer conn, STR_RemoveBagGoods* removeGoods);  //丢弃物品
    void PopRemoveGoods();

    void PushMoveGoods(TCPConnection::Pointer conn, STR_MoveBagGoods* moveGoods); //移动物品
    void PopMoveGoods();

    void PushBuyGoods(TCPConnection::Pointer conn, STR_BuyGoods* buyGoods); //购买物品
    void PopBuyGoods();

    void PushSellGoods(TCPConnection::Pointer conn, STR_SellGoods* sellGoods); //出售物品
    void PopSellGoods();

    //穿装备
    void PushWearBodyEqu(TCPConnection::Pointer conn, STR_WearEqu* wearEqu);
    void PopWearBodyEqu();
    //脱装备
    void PushTakeOffBodyEqu(TCPConnection::Pointer conn, hf_uint32 equid);
    void PopTakeOffBodyEqu();

    //player move
    void PushPlayerMove(TCPConnection::Pointer conn, STR_PlayerMove* playerMove);
    void PopPlayerMove();

    //player Attack
    void PushAttackAim(TCPConnection::Pointer conn, STR_PackUserAttackAim* attackAim);
    void PopAttackAim();

    void PushAttackPoint(TCPConnection::Pointer conn, STR_PackUserAttackPoint* attackPoint);
    void PopAttackPoint();



private:
    queue<Queue_AskTask>           *m_AskTask;
    queue<Queue_QuitTask>          *m_QuitTask;
    queue<Queue_AskFinishTask>     *m_AskFinishTask;
    queue<Queue_AskTaskData>       *m_AskTaskData;
    queue<Queue_AddFriend>         *m_AddFriend;
    queue<Queue_DeleteFriend>      *m_DeleteFriend;
    queue<Queue_AddFriendReturn>   *m_AddFriendReturn;
    queue<Queue_PickGoods>         *m_PickGoods;
    queue<Queue_RemoveGoods>       *m_RemoveGoods;
    queue<Queue_MoveGoods>         *m_MoveGoods;
    queue<Queue_BuyGoods>          *m_BuyGoods;
    queue<Queue_SellGoods>         *m_SellGoods;
    queue<Queue_WearBodyEqu>       *m_WearBodyEqu;
    queue<Queue_TakeOffBodyEqu>    *m_TakeOffBodyEqu;
    queue<Queue_PlayerMove>        *m_PlayerMove;
    queue<Queue_AttackAim>         *m_AttackAim;
    queue<Queue_AttackPoint>       *m_AttackPoint;
//    queue<> m_;
};




#endif // CMDPARSE_H

