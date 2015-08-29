#ifndef NETCLIENT_H
#define NETCLIENT_H

#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>

#include "tcpconnection.h"

class NetClient:public boost::enable_shared_from_this<NetClient>
{
public:

  typedef boost::shared_ptr<NetClient>    Pointer;

  ~NetClient();

  ///
  /// \brief Create 创建对象的智能指针
  /// \param io
  /// \return
  ///
  Pointer    Create( boost::asio::io_service &io);

  ///
  /// \brief Send_All 发送所有需要发送的数据
  /// \param buf
  /// \param size
  /// \return
  ///
  int         Send_All( void *buf, int size );

private:

  NetClient( boost::asio::io_service &io);
  boost::mutex                        m_read_lock;
  boost::mutex                        m_write_lock;
  TCPConnection::Pointer      m_conn;

};

#endif // NETCLIENT_H
