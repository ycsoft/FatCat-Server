
#include<iostream>

#include "tcpserver.h"
#include "hf_types.h"
#include "server.h"

#include "Game/log.hpp"
#include "GameAttack/gameattack.h"
#include "Monster/monster.h"
#include "OperationPostgres/operationpostgres.h"

using namespace std;

#define  SRV_PORT_DEFAULT  7001

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

        //set nodelay option
        boost::asio::ip::tcp::no_delay  nodelay(true);
        conn->socket().set_option(nodelay);

        Server::GetInstance()->RunTask(boost::bind(&TCPConnection::Start,conn));
    }
    StartListen();
}
