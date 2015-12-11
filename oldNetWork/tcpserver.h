#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <ctime>
#include <iostream>
#include <string>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

#include "tcpconnection.h"

using boost::asio::ip::tcp;

class TCPServer
{
public:
    TCPServer( boost::asio::io_service & io );
    ~TCPServer();

    void    Start();
    void    StartListen();
private:

    void        CallBack_Accept( TCPConnection::Pointer conn, const boost::system::error_code &ec);

    tcp::acceptor               m_acceptor;
    //boost::asio::io_service::strand     m_strand;
};

#endif // TCPSERVER_H
