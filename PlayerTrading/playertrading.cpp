#include "./../Game/session.hpp"
#include "./../server.h"



#include "playertrading.h"

PlayerTrading::PlayerTrading()
{

}

PlayerTrading::~PlayerTrading()
{

}

//请求交易
void PlayerTrading::RequestTrade(TCPConnection::Pointer conn, STR_PackRequestOper* oper)
{
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    if((*smap)[conn].m_tradeStatus != NotTrading)
    {
        Server::GetInstance()->free(oper);
        return;
    }

    umap_roleSock t_roleSock =  SessionMgr::Instance()->GetRoleSock();
    TCPConnection::Pointer t_conn = (*t_roleSock)[oper->RoleID];
    if(t_conn == NULL)
    {
        Server::GetInstance()->free(oper);
        return;
    }
    if((*smap)[t_conn].m_tradeStatus != NotTrading)
    {
//        STR_PackRequestReply reply((*smap)[conn].m_roleid,);
//        reply.
    }
}

//请求答复
void PlayerTrading::RequestReply(TCPConnection::Pointer conn, STR_PackRequestReply* oper)
{

}

//交易物品  增加或取消
void PlayerTrading::TradeGoods(TCPConnection::Pointer conn)
{

}

//交易金钱  增加或取消
void PlayerTrading::TradeMoney(TCPConnection::Pointer conn)
{

}

//锁定交易
void PlayerTrading::LockTrade(TCPConnection::Pointer conn)
{

}

//取消锁定
void PlayerTrading::CancelLockTrade(TCPConnection::Pointer conn)
{

}

//交易
void PlayerTrading::Trade(TCPConnection::Pointer conn)
{

}

//取消交易
void PlayerTrading::CancelTrade(TCPConnection::Pointer conn)
{

}
