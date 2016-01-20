#include "./../server.h"
#include "./../Game/postgresqlstruct.h"
#include "./../PlayerLogin/playerlogin.h"
#include "./../Game/cmdparse.h"
#include "./../Game/session.hpp"

#include "tcpconnection.h"

TCPConnection::TCPConnection(boost::asio::io_service &io)
    :m_socket(io),m_dataPos(0),m_LoginStatus(PlayerNotLoginUser),m_strand(io),
      m_queue(1024)
{
    Logger::GetLogger()->Debug("Create New Client TCPConnection");
    _isWriting = false;
    _isClosed = false;
    _isSaved = false;
    _push_times = 0;
    _pop_times = 0;
}

TCPConnection::~TCPConnection()
{
    Logger::GetLogger()->Info("Release Memory of TCPConnection");
}


void TCPConnection::Start()
{
    ReadSome();
}

void TCPConnection::_clear_queue(boost::lockfree::queue<_Buffer> &queue)
{
    _Buffer buf;
    while( queue.pop(buf) );
}

void TCPConnection::_push(void *buff, int size)
{
    size_t  tms = 0;
    _Buffer _buf;
    memcpy(_buf.buf,buff,size);
    _buf.sz = size;

    while ( getSendQueueLength() > 100 )
    {
        usleep(1000);
        tms++;
        if ( tms >= 1000)
        {
            break;
        }
    }

    m_queue.push(_buf);
    _push_times+=1;

    Logger::GetLogger()->Debug("Send Queue Length:%d",getSendQueueLength());
    if(!_isWriting)
    {
        _write();
    }
}
void TCPConnection::_write()
{
    if (m_queue.empty()){
        _isWriting = false;
        _push_times = 0;
        _pop_times = 0;
        return;
    }

    _Buffer buf;

    m_queue.pop(buf);
    _pop_times += 1;

    _isWriting = true;

    boost::asio::async_write(m_socket,boost::asio::buffer((char*)buf.buf,buf.sz),
                             boost::asio::transfer_exactly(buf.sz),
                             m_strand.wrap(
                             boost::bind(
                             &TCPConnection::CallBack_Write,shared_from_this(),boost::asio::placeholders::error(),
                             boost::asio::placeholders::bytes_transferred()
                             ))
                             );

}
void TCPConnection::CallBack_Write(const boost::system::error_code &code, hf_uint16 transfferd)
{
    if ( code ||  transfferd == 0 )
      {
        Logger::GetLogger()->Debug("Send Data to Player Error,Maybe socket closed!");
        _isWriting = false;
        return;
      }
    _Buffer _buf;
    if ( m_queue.empty() )
    {
        _isWriting = false;
        _push_times = 0;
        _pop_times = 0;
        return;
    }
    if (!m_queue.pop(_buf))
    {
        _isWriting = false;
        return;
    }
    _pop_times += 1;

        memcpy(m_send_buf,_buf.buf,_buf.sz);

        boost::asio::async_write( m_socket,boost::asio::buffer((char*)m_send_buf,_buf.sz),
                                  boost::asio::transfer_exactly(_buf.sz),
                                  m_strand.wrap(
                                  boost::bind(
                                  &TCPConnection::CallBack_Write,shared_from_this(),boost::asio::placeholders::error(),
                                  boost::asio::placeholders::bytes_transferred()
                                  ))
                                );
}
int TCPConnection::Write_all(void *buff, int size)
{
    //if socket is closed , stop send data
    if ( _isClosed )
    {
        //clear the queue
        _clear_queue( m_queue );
        _push_times = 0;
        _pop_times = 0;
        return 0;
    }
    if(size >= CHUNK_SIZE)  //test
    {
        STR_PackHead t_packHead;
        memcpy(&t_packHead, buff, sizeof(STR_PackHead));
        Logger::GetLogger()->Error("send data :%u,flag:%u,len:%u\n",size, t_packHead.Flag, t_packHead.Len);
        return 0;
    }
    _push(buff,size);

    return 0;
}

void TCPConnection::ReadSome()
{
        m_socket.async_read_some(boost::asio::buffer(m_buf + m_dataPos,TCP_BUFFER_SIZE - m_dataPos),

                                 m_strand.wrap(
                                 boost::bind(&TCPConnection::CallBack_Read_Some,
                                            shared_from_this(),
                                            boost::asio::placeholders::error(),
                                            boost::asio::placeholders::bytes_transferred()))
                                 );
}

void TCPConnection::CallBack_Read_Some(const boost::system::error_code &ec, hf_uint16 size)
{
    if ( !ec )
    {
        m_dataPos += size;
        hf_uint16 currentPos = 0;
        STR_PackHead head;
        SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
        while(currentPos < m_dataPos)
        {
            if(m_dataPos < sizeof(STR_PackHead))
            {
                Logger::GetLogger()->Debug("datalen:%u",m_dataPos);
                break;
            }
            memcpy(&head, m_buf + currentPos, sizeof(STR_PackHead));
            if(head.Len > 512)
            {
                Logger::GetLogger()->Debug("Client head.len > 512 Disconnected");
                Server::GetInstance()->GetPlayerLogin()->SavePlayerOfflineData(shared_from_this() );
            }
            if(currentPos + sizeof(STR_PackHead) + head.Len == m_dataPos) //最后一个包
            {
                memcpy(m_pack.data, m_buf + currentPos, sizeof(STR_PackHead) + head.Len);
                hf_uint8 value = JudgePlayerLogin(head.Flag);
                if(value == 2)
                {
                   m_pack.roleid = (*smap)[shared_from_this()].m_roleid;
                   Server::GetInstance()->PushPackage(m_pack);
                }
                else if(value == 1)//未登录角色
                {
                   CommandParseLogin(shared_from_this(), m_pack.data);
                }
                m_dataPos = 0;
                break;
            }
            else if(currentPos + sizeof(STR_PackHead) + head.Len < m_dataPos)  //未处理的数据长度多于一个包
            {
                memcpy(m_pack.data, m_buf + currentPos, sizeof(STR_PackHead) + head.Len);
                hf_uint8 value = JudgePlayerLogin(head.Flag);
                if(value == 2)
                {
                   m_pack.roleid = (*smap)[shared_from_this()].m_roleid;
                   Server::GetInstance()->PushPackage(m_pack);
                }
                else if(value == 1)//未登录角色
                {
                   CommandParseLogin(shared_from_this(), m_pack.data);
                }
                currentPos += sizeof(STR_PackHead) + head.Len;
                continue;
            }
            else //不够一个包的长度
            {
//                Logger::GetLogger()->Debug("you wei jie xi de bao currentPos:%u,m_dataPos:%u,head.len:%u,head.flag:%u",currentPos,m_dataPos,head.Len, head.Flag);
                hf_char buff[TCP_BUFFER_SIZE] = { 0 };
                memcpy(buff, m_buf + currentPos, m_dataPos - currentPos);
                memcpy(m_buf, buff, TCP_BUFFER_SIZE);
                m_dataPos -= currentPos;

                break;
            }
        }
        ReadSome();
      }
    else if ( size == 0 || ec == boost::asio::error::eof || ec == boost::asio::error::shut_down)
    {
        _isClosed = true;
        Logger::GetLogger()->Debug("Client Disconnected,Socket_t:%d",m_socket.native());
        Server::GetInstance()->GetPlayerLogin()->SavePlayerOfflineData(shared_from_this());
        m_socket.close();
    }
}




//判断玩家是否登录角色
hf_uint8 TCPConnection::JudgePlayerLogin(hf_uint8 flag)
{
    if(m_LoginStatus == PlayerNotLoginUser) //未登录用户名，只能登录或注册用户
    {
        if(flag == FLAG_PlayerLoginUserId || flag == FLAG_PlayerRegisterUserId)
            return 1;
        else
            return 0;
    }
    else if(m_LoginStatus == PlayerLoginUser)//未登录角色，只能登录或注册或删除角色
    {
        if(flag == FLAG_PlayerLoginRole || flag == FLAG_PlayerRegisterRole || flag == FLAG_UserDeleteRole)
            return 1;
        else
            return 0;
    }
    else
    {
        return 2;
    }
}


