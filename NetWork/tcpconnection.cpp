#include "./../server.h"
#include "./../Game/postgresqlstruct.h"
#include "./../PlayerLogin/playerlogin.h"
#include "./../Game/cmdparse.h"
#include "./../Game/session.hpp"

#include "tcpconnection.h"

TCPConnection::TCPConnection(boost::asio::io_service &io)
    :m_socket(io),m_dataPos(0),m_LoginStatus(PlayerNotLoginUser)
{

}

TCPConnection::~TCPConnection()
{
    Logger::GetLogger()->Info("Release Memory of TCPConnection");
}


void TCPConnection::Start()
{
    ReadSome();
}

int TCPConnection::Write_all(void *buff, int size)
{
    if(size >= CHUNK_SIZE)  //test
    {
        STR_PackHead t_packHead;
        memcpy(&t_packHead, buff, sizeof(STR_PackHead));
        Logger::GetLogger()->Error("send data :%u,flag:%u,len:%u\n",size, t_packHead.Flag, t_packHead.Len);
        return 0;
    }
//    STR_PackHead t_packHead;
//    memcpy(&t_packHead, buff, sizeof(STR_PackHead));
//    Logger::GetLogger()->Error("send data :%u,flag:%u,len:%u\n",size, t_packHead.Flag, t_packHead.Len);

    m_write_lock.lock();

    //将要发送的数据拷贝至发送缓冲区，防止数据丢失
    memcpy(m_send_buf,buff,size);
    boost::asio::async_write( m_socket,boost::asio::buffer((char*)buff,size),boost::asio::transfer_all(),boost::bind(
                              &TCPConnection::CallBack_Write,shared_from_this(),boost::asio::placeholders::error(),
                              boost::asio::placeholders::bytes_transferred()
                              )
                            );
    return 0;
}

void TCPConnection::ReadSome()
{
        m_socket.async_read_some(boost::asio::buffer(m_buf + m_dataPos,TCP_BUFFER_SIZE - m_dataPos),
                                boost::bind(&TCPConnection::CallBack_Read_Some,
                                            shared_from_this(),
                                            boost::asio::placeholders::error(),
                                            boost::asio::placeholders::bytes_transferred()));
}

void TCPConnection::CallBack_Read_Some(const boost::system::error_code &ec, hf_uint16 size)
{
    if ( !ec )
    {
//        Logger::GetLogger()->Debug("size:%u",size);

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
            if(head.Len > 100)
            {
                Logger::GetLogger()->Debug("Client head Disconnected");
                Server::GetInstance()->GetPlayerLogin()->SavePlayerOfflineData(shared_from_this() );
                SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
                SessionMgr::Instance()->NameSockErase(&(*smap)[shared_from_this()].m_usrid[0]);
                SessionMgr::Instance()->SessionsErase(shared_from_this());
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
                Logger::GetLogger()->Debug("you wei jie xi de bao currentPos:%u,m_dataPos:%u,head.len:%u,head.flag:%u",currentPos,m_dataPos,head.Len, head.Flag);
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
        Logger::GetLogger()->Debug("Client head Disconnected");
        Server::GetInstance()->GetPlayerLogin()->SavePlayerOfflineData(shared_from_this() );
        SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
        SessionMgr::Instance()->NameSockErase(&(*smap)[shared_from_this()].m_usrid[0]);
        SessionMgr::Instance()->SessionsErase(shared_from_this());
    }
}


void TCPConnection::CallBack_Write(const boost::system::error_code &code, hf_uint16 transfferd)
{
    m_write_lock.unlock();
    if ( code ||  transfferd == 0 )
      {
        Logger::GetLogger()->Debug("Send Data to Player Error");
        return;
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


