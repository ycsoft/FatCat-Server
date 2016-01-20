#include<iostream>

#include "./../hf_types.h"
#include "./../server.h"
#include "./../Game/log.h"
#include "./../GameAttack/gameattack.h"
#include "./../Monster/monster.h"
#include "./../OperationPostgres/operationpostgres.h"

#include "tcpserver.h"

using namespace std;

#define  SRV_PORT_DEFAULT  8000

TCPServer::TCPServer( boost::asio::io_service & io  )
    :m_acceptor(io,tcp::endpoint(tcp::v4(),SRV_PORT_DEFAULT))
{

}

TCPServer::~TCPServer()
{

}

void TCPServer::Start()
{
    StartListen();
    Logger::GetLogger()->Debug("Ready to Server Players");
}

void TCPServer::StartListen()
{

    TCPConnection ::Pointer  conn = TCPConnection::Create(m_acceptor.get_io_service());
    m_acceptor.async_accept(conn->socket(),  boost::bind(&TCPServer::CallBack_Accept,
                                                       this,conn,boost::asio::placeholders::error()) );
}

void TCPServer::CallBack_Accept(TCPConnection::Pointer conn, const boost::system::error_code &ec)
{
    if ( ! ec)
    {
        Logger::GetLogger()->Debug("Client Connected");
        boost::asio::ip::tcp::no_delay  nodelay(true);
        boost::asio::socket_base::non_blocking_io none_block(true);
        conn->socket().io_control(none_block);
        if ( conn->socket().non_blocking() ){
            Logger::GetLogger()->Debug("None_Block");
        }else{
            Logger::GetLogger()->Debug("Block Socket");
        }
        conn->socket().set_option(nodelay);

        conn->Start();
//        Server::GetInstance()->RunTask(boost::bind(&TCPConnection::Start,conn));
    }
    else
    {
        Logger::GetLogger()->Error("error:%d",ec.value());
    }
    StartListen();
}

bool TCPServer::setSockKeepAlive(int Sock)
{
    int t_sockVal = 1;
    if(setsockopt(Sock, SOL_SOCKET, SO_KEEPALIVE, &t_sockVal, (socklen_t)sizeof(t_sockVal)))
        return false;

    t_sockVal = 10;
    if(setsockopt(Sock, SOL_TCP, TCP_KEEPIDLE, &t_sockVal, (socklen_t)sizeof(t_sockVal)))
        return false;

    t_sockVal = 10;
    if(setsockopt(Sock, SOL_TCP, TCP_KEEPINTVL, &t_sockVal, (socklen_t)sizeof(t_sockVal)))
        return false;

    t_sockVal = 3;
    if(setsockopt(Sock, SOL_TCP, TCP_KEEPCNT, &t_sockVal, (socklen_t)sizeof(t_sockVal)))
        return false;

    return true;
}
