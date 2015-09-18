

#include "server.h"
#include "tcpconnection.h"
#include "Game/postgresqlstruct.h"

#include "Game/cmdparse.hpp"

#include "Game/session.hpp"

#include "Game/log.hpp"


//size_t hash_value(  TCPConnection &conn)
//{
//    size_t seed = 0;
//    boost::hash_combine(seed, conn.socket().native_handle());
//    return seed;
//}
//size_t hash_value(  TCPConnection::Pointer conn)
//{
//    size_t seed = 0;
//    boost::hash_combine(seed, conn->socket().native_handle());
//    return seed;
//}
//bool operator ==( TCPConnection::Pointer p1, TCPConnection::Pointer p2)
//{
//    return p1->socket().native_handle() == p2->socket().native_handle();
//}

//bool operator ==( TCPConnection &p1, TCPConnection &p2)
//{
//    return p1.socket().native_handle() == p2.socket().native_handle();
//}

TCPConnection::TCPConnection(boost::asio::io_service &io)
    :m_socket(io)
{

}

TCPConnection::~TCPConnection()
{
    Logger::GetLogger()->Info("Release Memory of TCPConnection");
}


void TCPConnection::Start()
{
    ReadHead();
}

int TCPConnection::Write_all(void *buff, int size)
{
    if(size >= CHUNK_SIZE)  //test
    {
        STR_PackHead t_packHead;
        memcpy(&t_packHead, buff, sizeof(STR_PackHead));
        cout << "发送数据大于1024,长度为:" << size << "Flag:" << t_packHead.Flag << ",Len" << t_packHead.Len << endl;
        return 0;
    }
    m_write_lock.lock();

    //将要发送的数据拷贝至发送缓冲区，防止数据丢失
    memcpy(m_send_buf,buff,size);
    boost::asio::async_write( m_socket,boost::asio::buffer((char*)buff,size),boost::asio::transfer_all(),boost::bind(
                              &TCPConnection::CallBack_Write,shared_from_this(),boost::asio::placeholders::error(),
                              boost::asio::placeholders::bytes_transferred()
                              )
                            );
}

void TCPConnection::ReadHead()
{

        m_read_lock.lock();

        boost::asio::async_read(m_socket,boost::asio::buffer(m_buf,sizeof(STR_PackHead)),
                                boost::bind(&TCPConnection::CallBack_Read_Head,
                                            shared_from_this(),
                                            boost::asio::placeholders::error(),
                                            boost::asio::placeholders::bytes_transferred()));



}
void TCPConnection::ReadBody()
{
    STR_PackHead *pack = (STR_PackHead*)m_buf;
    hf_uint16           len = /*ntohs*/(pack->Len);
//    hf_uint16           flag = /*ntohs*/(pack->Flag);
    if(len >= 1024)
    {
        m_read_lock.unlock();
        SessionMgr::Instance()->RemoveSession(shared_from_this());
        return;
    }
    if(len == 0)
    {
        cout << len << endl;
        return;
    }

    boost::asio::async_read(m_socket,boost::asio::buffer(m_buf+sizeof(STR_PackHead),len),
                            boost::bind(&TCPConnection::CallBack_Read_Body,
                                        shared_from_this(),
                                        boost::asio::placeholders::error(),
                                        boost::asio::placeholders::bytes_transferred()));
}

void TCPConnection::CallBack_Read_Head(const boost::system::error_code &ec,size_t size)
{
    if ( !ec )
     {
        ReadBody();
      }
    else if ( size == 0 || ec == boost::asio::error::eof || ec == boost::asio::error::shut_down)
    {
        Logger::GetLogger()->Debug("Client head Disconnected");
        Server::GetInstance()->GetPlayerLogin()->SavePlayerOfflineData(shared_from_this() );
        Server::GetInstance()->GetPlayerLogin()->DeleteNameSock(shared_from_this() );
        SessionMgr::Instance()->RemoveSession(shared_from_this());

        m_read_lock.unlock();
    }
}
void TCPConnection::CallBack_Read_Body(const boost::system::error_code &ec, size_t size)
{
    if ( ! ec )
    {
        Server  *srv = Server::GetInstance();
        char *buf = (char*)srv->malloc();
        memcpy(buf,m_buf,size + sizeof(STR_PackHead));
        //srv->RunTask(boost::bind(&CommandParse,&m_socket,buf));
        CommandParse(shared_from_this(),buf);
        srv->free(buf);

        //至此，已处理完一个完整数据包
        m_read_lock.unlock();
        ReadHead();
    }
    else if ( size == 0 || ec == boost::asio::error::eof || ec == boost::asio::error::shut_down)
    {
        Logger::GetLogger()->Debug("Client body Disconnected");
//        int fd = m_socket.native_handle();
        Server::GetInstance()->GetPlayerLogin()->SavePlayerOfflineData(shared_from_this() );
        Server::GetInstance()->GetPlayerLogin()->DeleteNameSock(shared_from_this() );
        SessionMgr::Instance()->RemoveSession(shared_from_this());
        m_read_lock.unlock();
    }
}

void TCPConnection::CallBack_Write(const boost::system::error_code &code, size_t transfferd)
{
    m_write_lock.unlock();
    if ( code ||  transfferd == 0 )
      {
        Logger::GetLogger()->Debug("Send Data to Player Error");
        return;
      }
}
