#ifndef GAMEINTERCHANGE_H
#define GAMEINTERCHANGE_H
#include "Game/postgresqlstruct.h"
#include "NetWork/tcpconnection.h"
#include "Game/cmdtypes.h"
#include "Game/session.hpp"

#define Interchange_Change 1
#define Interchange_Lock 2
#define Interchange_Unlock 3
#define Interchange_CancelChange 4
#define Interchange_CancelRequest 7


class GameInterchange
{
public:
    GameInterchange();
    ~GameInterchange();
    void operRequest(TCPConnection::Pointer conn,  operationRequest*  operReq);
    void operResponse(TCPConnection::Pointer conn,  operationRequestResult*  operReq);
    void operChanges(TCPConnection::Pointer conn,  interchangeOperGoods*  oper);
    void operProCheckChange(TCPConnection::Pointer conn,  interchangeOperPro*  oper);  //交易
    void operProCancelChange(TCPConnection::Pointer conn,  interchangeOperPro*  oper);  //取消交易
    void operProCancelRequest(TCPConnection::Pointer conn,  interchangeOperPro*  oper);  //取消交易
    void operProLock(TCPConnection::Pointer conn,  interchangeOperPro*  oper);   //锁定
    void operProUnlock(TCPConnection::Pointer conn, interchangeOperPro*  oper);  //取消锁定
    void operDoChange(TCPConnection::Pointer conn); //完成交易
    void operMoneyChanges(TCPConnection::Pointer conn, interchangeMoney*  oper); //交易金钱变动
    void operReport(TCPConnection::Pointer conn);//交易报告，交易完成后发送双方物品金钱变动
};

#endif // GAMEINTERCHANGE_H
