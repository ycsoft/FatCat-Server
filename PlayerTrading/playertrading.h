#ifndef PLAYERTRADING_H
#define PLAYERTRADING_H

#include "./../NetWork/tcpconnection.h"
#include "./../Game/postgresqlstruct.h"

class PlayerTrading
{
public:
    PlayerTrading();
    ~PlayerTrading();

    void RequestTrade(TCPConnection::Pointer conn, STR_PackRequestOper* oper);     //请求交易
    void RequestReply(TCPConnection::Pointer conn, STR_PackRequestReply* oper);     //请求答复

    void TradeGoods(TCPConnection::Pointer conn);       //交易物品  增加或取消
    void TradeMoney(TCPConnection::Pointer conn);       //交易金钱  增加或取消

    void LockTrade(TCPConnection::Pointer conn);        //锁定交易
    void CancelLockTrade(TCPConnection::Pointer conn);  //取消锁定

    void Trade(TCPConnection::Pointer conn);            //交易
    void CancelTrade(TCPConnection::Pointer conn);      //取消交易
};

#endif // PLAYERTRADING_H
