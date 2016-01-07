#ifndef CMDPARSE_H
#define CMDPARSE_H

#include <queue>
#include <boost/lockfree/queue.hpp>

#include "NetWork/tcpconnection.h"
#include "Game/postgresqlstruct.h"


using namespace std;


typedef struct _Queue_AskTask
{
    _Queue_AskTask(hf_uint32 _roleid, hf_uint32 _taskID)
        :roleid(_roleid),taskID(_taskID)
    {

    }
    _Queue_AskTask()
    {

    }

    hf_uint32  roleid;
    hf_uint32  taskID;
}Queue_AskTask;


typedef struct _Queue_QuitTask
{
    _Queue_QuitTask(hf_uint32 _roleid, hf_uint32 _taskID)
        :roleid(_roleid),taskID(_taskID)
    {

    }
    _Queue_QuitTask()
    {

    }

    hf_uint32 roleid;
    hf_uint32  taskID;
}Queue_QuitTask;

typedef struct _Queue_AskFinishTask
{
    _Queue_AskFinishTask(hf_uint32 _roleid, STR_FinishTask* _finishTask)
        :roleid(_roleid)
    {
        finishTask.TaskID = _finishTask->TaskID;
        finishTask.SelectGoodsID = _finishTask->SelectGoodsID;
    }
    _Queue_AskFinishTask()
    {

    }

    hf_uint32 roleid;
    STR_FinishTask finishTask;
}Queue_AskFinishTask;

typedef struct _Queue_AskTaskExeDlg
{
    _Queue_AskTaskExeDlg(hf_uint32 _roleid, STR_AskTaskExeDlg* _exeDlg)
        :roleid(_roleid)
    {
        exeDlg.TaskID = _exeDlg->TaskID;
        exeDlg.AimID = _exeDlg->AimID;
    }
    _Queue_AskTaskExeDlg()
    {

    }

    hf_uint32 roleid;
    STR_AskTaskExeDlg exeDlg;
}Queue_AskTaskExeDlg;

typedef struct _Queue_AskTaskData
{
    _Queue_AskTaskData(hf_uint32 _roleid, hf_uint32 _taskID, hf_uint16 _flag)
        :roleid(_roleid),taskID(_taskID),flag(_flag)
    {

    }
    _Queue_AskTaskData()
    {

    }

    hf_uint32 roleid;
    hf_uint32 taskID;
    hf_uint16 flag;
}Queue_AskTaskData;

typedef struct _Queue_AddFriend
{
    _Queue_AddFriend(hf_uint32 _roleid, STR_PackAddFriend* _addFriend)
        :roleid(_roleid)
    {
        memcpy(&addFriend, _addFriend, sizeof(STR_PackAddFriend));
    }
    _Queue_AddFriend()
    {

    }

    hf_uint32 roleid;
    STR_PackAddFriend addFriend;
}Queue_AddFriend;

typedef struct _Queue_DeleteFriend
{
    _Queue_DeleteFriend(hf_uint32 _roleid, hf_uint32 _deleteRoleid)
        :roleid(_roleid), deleteRoleid(_deleteRoleid)
    {

    }
    _Queue_DeleteFriend()
    {

    }

    hf_uint32 roleid;
    hf_uint32 deleteRoleid;
}Queue_DeleteFriend;

typedef struct _Queue_AddFriendReturn
{
    _Queue_AddFriendReturn(hf_uint32 _roleid, STR_PackAddFriendReturn* _addFriendReturn)
        :roleid(_roleid)
    {
        memcpy(&addFriendReturn, _addFriendReturn, sizeof(STR_PackAddFriendReturn));
    }
    _Queue_AddFriendReturn()
    {

    }

    hf_uint32 roleid;
    STR_PackAddFriendReturn addFriendReturn;
}Queue_AddFriendReturn;

typedef struct _Queue_PickGoods
{
    _Queue_PickGoods(hf_uint32 _roleid, STR_PickGoods* _pickGoods)
        :roleid(_roleid)
    {
        memcpy(&pickGoods, _pickGoods, sizeof(STR_PickGoods));
    }
    _Queue_PickGoods()
    {

    }

    hf_uint32 roleid;
    STR_PickGoods pickGoods;
}Queue_PickGoods;

typedef struct _Queue_RemoveGoods
{
    _Queue_RemoveGoods(hf_uint32 _roleid, STR_RemoveBagGoods* _removeGoods)
        :roleid(_roleid)
    {
        memcpy(&removeGoods, _removeGoods, sizeof(STR_RemoveBagGoods));
    }
    _Queue_RemoveGoods()
    {

    }

    hf_uint32 roleid;
    STR_RemoveBagGoods removeGoods;
}Queue_RemoveGoods;

typedef struct _Queue_MoveGoods
{
    _Queue_MoveGoods(hf_uint32 _roleid, STR_MoveBagGoods* _moveGoods)
        :roleid(_roleid)
    {
        memcpy(&moveGoods, _moveGoods, sizeof(STR_MoveBagGoods));
    }
    _Queue_MoveGoods()
    {

    }

    hf_uint32 roleid;
    STR_MoveBagGoods moveGoods;
}Queue_MoveGoods;

typedef struct _Queue_BuyGoods
{
    _Queue_BuyGoods(hf_uint32 _roleid, STR_BuyGoods* _buyGoods)
        :roleid(_roleid)
    {
        memcpy(&buyGoods, _buyGoods, sizeof(STR_BuyGoods));
    }
    _Queue_BuyGoods()
    {

    }

    hf_uint32 roleid;
    STR_BuyGoods buyGoods;
}Queue_BuyGoods;

typedef struct _Queue_SellGoods
{
    _Queue_SellGoods(hf_uint32 _roleid, STR_SellGoods* _sellGoods)
        :roleid(_roleid)
    {
        memcpy(&sellGoods, _sellGoods, sizeof(STR_SellGoods));
    }
    _Queue_SellGoods()
    {

    }

    hf_uint32 roleid;
    STR_SellGoods sellGoods;
}Queue_SellGoods;

typedef struct _Queue_WearBodyEqu
{
    _Queue_WearBodyEqu(hf_uint32 _roleid, STR_WearEqu* _wearEqu)
        :roleid(_roleid)
    {
        memcpy(&wearEqu, _wearEqu, sizeof(STR_WearEqu));
    }
    _Queue_WearBodyEqu()
    {

    }

    hf_uint32 roleid;
    STR_WearEqu wearEqu;
}Queue_WearBodyEqu;

typedef struct _Queue_TakeOffBodyEqu
{
    _Queue_TakeOffBodyEqu(hf_uint32 _roleid, hf_uint32 _equid)
        :roleid(_roleid),equid(_equid)
    {

    }
    _Queue_TakeOffBodyEqu()
    {

    }

    hf_uint32 roleid;
    hf_uint32 equid;
}Queue_TakeOffBodyEqu;

typedef struct _Queue_PlayerDirectChange
{
    _Queue_PlayerDirectChange(hf_uint32 _roleid, hf_float _direct)
        :roleid(_roleid), direct(_direct)
    {

    }

    _Queue_PlayerDirectChange()
    {

    }

    hf_uint32 roleid;
    hf_float direct;
}Queue_PlayerDirectChange;

typedef struct _Queue_PlayerActionChange
{
    _Queue_PlayerActionChange(hf_uint32 _roleid, hf_uint8 _action)
        :roleid(_roleid), action(_action)
    {

    }

    _Queue_PlayerActionChange()
    {

    }

    hf_uint32 roleid;
    hf_uint8 action;
}Queue_PlayerActionChange;

typedef struct _Queue_PlayerMove
{
    _Queue_PlayerMove(hf_uint32 _roleid, STR_PlayerMove* _playerMove)
        :roleid(_roleid)
    {
        memcpy(&playerMove, _playerMove, sizeof(STR_PlayerMove));
    }

    _Queue_PlayerMove()
    {

    }

    hf_uint32 roleid;
    STR_PlayerMove playerMove;
}Queue_PlayerMove;

typedef struct _Queue_AttackAim
{
    _Queue_AttackAim(hf_uint32 _roleid, STR_PackUserAttackAim* _attackAim)
        :roleid(_roleid)
    {
        memcpy(&attackAim, _attackAim, sizeof(STR_PackUserAttackAim));
    }
    _Queue_AttackAim()
    {

    }

    hf_uint32 roleid;
    STR_PackUserAttackAim attackAim;
}Queue_AttackAim;

typedef struct _Queue_AttackPoint
{
    _Queue_AttackPoint(hf_uint32 _roleid, STR_PackUserAttackPoint* _attackPoint)
        :roleid(_roleid)
    {
        memcpy(&attackPoint, _attackPoint, sizeof(STR_PackUserAttackPoint));
    }
    _Queue_AttackPoint()
    {

    }

    hf_uint32 roleid;
    STR_PackUserAttackPoint attackPoint;
}Queue_AttackPoint;


class CmdParse
{
public:
    CmdParse();
    ~CmdParse();

    //Task
    void PushAskTask(TCPConnection::Pointer conn, hf_uint32 taskID);   //请求任务
    void PopAskTask();

    void PushQuitTask(TCPConnection::Pointer conn, hf_uint32 taskID);  //放弃任务
    void PopQuitTask();

    void PushAskFinishTask(TCPConnection::Pointer conn, STR_FinishTask* finishTask); //请求完成任务
    void PopAskFinishTask();

    void PushAskTaskExeDlg(TCPConnection::Pointer conn, STR_AskTaskExeDlg* exeDlg); //请求任务执行对话
    void PopAskTaskExeDlg();

    void PushAskTaskExeDlgFinish(TCPConnection::Pointer conn, STR_AskTaskExeDlg* exeDlg); //请求任务执行对话完成
    void PopAskTaskExeDlgFinish();

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
    void PushPickGoods(TCPConnection::Pointer conn, STR_PickGoods* pickGoods); //拾取物品
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

    //player direct change
    void PushPlayerDirectChange(TCPConnection::Pointer conn, hf_float direct);
    void PopPlayerDirectChange();

    //player Action change
    void PushPlayerActionChange(TCPConnection::Pointer conn, hf_uint8 action);
    void PopPlayerActionChange();

    //player move
    void PushPlayerMove(TCPConnection::Pointer conn, STR_PlayerMove* playerMove);
    void PopPlayerMove();

    //player Attack
    void PushAttackAim(TCPConnection::Pointer conn, STR_PackUserAttackAim* attackAim);
    void PopAttackAim();

    void PushAttackPoint(TCPConnection::Pointer conn, STR_PackUserAttackPoint* attackPoint);
    void PopAttackPoint();



private:
    boost::lockfree::queue<Queue_AskTask>            *m_AskTask;
    boost::lockfree::queue<Queue_QuitTask>           *m_QuitTask;
    boost::lockfree::queue<Queue_AskFinishTask>      *m_AskFinishTask;
    boost::lockfree::queue<Queue_AskTaskExeDlg>      *m_AskTaskExeDlg;
    boost::lockfree::queue<Queue_AskTaskExeDlg>      *m_AskTaskExeDlgFinish;
    boost::lockfree::queue<Queue_AskTaskData>        *m_AskTaskData;
    boost::lockfree::queue<Queue_AddFriend>          *m_AddFriend;
    boost::lockfree::queue<Queue_DeleteFriend>       *m_DeleteFriend;
    boost::lockfree::queue<Queue_AddFriendReturn>    *m_AddFriendReturn;
    boost::lockfree::queue<Queue_PickGoods>          *m_PickGoods;
    boost::lockfree::queue<Queue_RemoveGoods>        *m_RemoveGoods;
    boost::lockfree::queue<Queue_MoveGoods>          *m_MoveGoods;
    boost::lockfree::queue<Queue_BuyGoods>           *m_BuyGoods;
    boost::lockfree::queue<Queue_SellGoods>          *m_SellGoods;
    boost::lockfree::queue<Queue_WearBodyEqu>        *m_WearBodyEqu;
    boost::lockfree::queue<Queue_TakeOffBodyEqu>     *m_TakeOffBodyEqu;
    boost::lockfree::queue<Queue_PlayerDirectChange> *m_PlayerDirectChange;
    boost::lockfree::queue<Queue_PlayerActionChange> *m_PlayerActionChange;
    boost::lockfree::queue<Queue_PlayerMove>         *m_PlayerMove;
    boost::lockfree::queue<Queue_AttackAim>          *m_AttackAim;
    boost::lockfree::queue<Queue_AttackPoint>        *m_AttackPoint;
};




#endif // CMDPARSE_H

