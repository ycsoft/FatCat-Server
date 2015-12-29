#include<iostream>

#include "./../hf_types.h"
#include "./../server.h"
#include "./../Game/log.h"
#include "./../GameAttack/gameattack.h"
#include "./../Monster/monster.h"
#include "./../OperationPostgres/operationpostgres.h"

#include "tcpserver.h"

using namespace std;

#define  SRV_PORT_DEFAULT  7000

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
//        int fd = conn->socket().native_handle();
//        if(setSockKeepAlive(fd))
//        {
//            Logger::GetLogger()->Debug("set xintiao success,%d",fd);
//        }
//        else
//        {
//            Logger::GetLogger()->Debug("set xintiao error,%d",fd);
//        }

        //set nodelay option
        boost::asio::ip::tcp::no_delay  nodelay(true);
        conn->socket().set_option(nodelay);

        conn->Start();
//        Server::GetInstance()->RunTask(boost::bind(&TCPConnection::Start,conn));
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
