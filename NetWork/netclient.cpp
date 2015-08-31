#include "netclient.h"

using boost::asio::ip::tcp;

NetClient::NetClient(boost::asio::io_service &io)
{
    m_conn = TCPConnection::Create(io);
}

NetClient::~NetClient()
{

}

NetClient::Pointer NetClient::Create(boost::asio::io_service &io)
{
    return NetClient::Pointer( new NetClient(io) );
}


// < size, error, 0  close ,   < 0 fatal error
int NetClient::Send_All(void *buf, int size)
{

      char cbuf[1024] = {0};

      tcp::socket &sock = m_conn->socket();
      memcpy(cbuf,buf,size);
      m_write_lock.lock();
      int st =  boost::asio::write(m_conn->socket(),boost::asio::buffer(cbuf,size),boost::asio::transfer_all());
      if ( 0 == st )
      {
          m_conn->socket().close();
      }
      m_write_lock.unlock();
}
