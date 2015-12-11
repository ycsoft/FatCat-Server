

#include "server.h"
#include "tcpconnection.h"
#include "Game/postgresqlstruct.h"
#include "PlayerLogin/playerlogin.h"

#include "Game/cmdparse.h"

#include "Game/session.hpp"

//#include "Game/log.hpp"


TCPConnection::TCPConnection(boost::asio::io_service &io)
    :m_socket(io),currentIndex(0)
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
        cout << "发送数据大于1024,长度为:" << size << "Flag:" << t_packHead.Flag << ",Len" << t_packHead.Len << endl;
        return 0;
    }
//    STR_PackHead t_packHead;
//    memcpy(&t_packHead, buff, sizeof(STR_PackHead));
//    cout << "发送数据长度为:" << size << "Flag:" << t_packHead.Flag << ",Len" << t_packHead.Len << endl;

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
        m_socket.async_read_some(boost::asio::buffer(m_buf + currentIndex,TCP_BUFFER_SIZE - currentIndex),
                                boost::bind(&TCPConnection::CallBack_Read_Some,
                                            shared_from_this(),
                                            boost::asio::placeholders::error(),
                                            boost::asio::placeholders::bytes_transferred()));
}

void TCPConnection::CallBack_Read_Some(const boost::system::error_code &ec,size_t size)
{
    if ( !ec )
     {
//        cout << "size = " << size << endl;
        currentIndex += size;
        hf_uint32 start = 0;
        hf_uint32 end = 0;
        SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();

        STR_PackHead head;
        while(start < currentIndex)
        {
            end += sizeof(STR_PackHead);            
            memcpy(&head, m_buf + start, sizeof(STR_PackHead));

            if(end < currentIndex &&
                    start + sizeof(STR_PackHead) + head.Len <= currentIndex)
            {
                memcpy(m_pack.data, m_buf + start, sizeof(STR_PackHead) + head.Len);
                //未登录角色
               if(JudgePlayerLogin(shared_from_this(), head.Flag))
               {
                   m_pack.roleid = (*smap)[shared_from_this()].m_roleid;
                   Server::GetInstance()->PushPackage(m_pack);
               }
               else
               {
                   CommandParseLogin(shared_from_this(), m_pack.data);
               }

                start += sizeof(STR_PackHead) + head.Len;
                if(start == currentIndex)
                {
                    currentIndex = 0;
                    break;
                }
                end = start;
            }
            else
            {
                cout << "有未处理完的数据" << endl;
                hf_char buff[TCP_BUFFER_SIZE] = { 0 };
                memcpy(buff, m_buf+start,currentIndex - start);
                memcpy(m_buf, buff, TCP_BUFFER_SIZE);
//                memcpy(m_buf,m_buf+start,currentIndex - start);
//                memset(m_buf + currentIndex - start,0,TCP_BUFFER_SIZE + start - currentIndex);
                currentIndex -= start;
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


void TCPConnection::CallBack_Write(const boost::system::error_code &code, size_t transfferd)
{
    m_write_lock.unlock();
    if ( code ||  transfferd == 0 )
      {
        Logger::GetLogger()->Debug("Send Data to Player Error");
        return;
      }
}

//判断玩家是否登录角色
bool TCPConnection::JudgePlayerLogin(/*SessionMgr::SessionPointer smap,*/ TCPConnection::Pointer conn, hf_uint8 flag)
{
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    Session* sess = &(*smap)[conn];
    if(sess == NULL)//未登录用户名，只能登录或注册用户
    {
        if(flag != FLAG_PlayerLoginUserId && flag != FLAG_PlayerRegisterUserId)
            return false;
    }

    //已经登录用户不能登录或注册用户，只能退出用户后再登录或注册用户
    if(flag == FLAG_PlayerLoginUserId || flag == FLAG_PlayerRegisterUserId)
        return false;
    if(sess->m_roleid == 0) //未登录角色，只能登录或注册或删除角色
    {
        if(flag == FLAG_PlayerLoginRole || flag == FLAG_PlayerRegisterRole || flag == FLAG_UserDeleteRole)
            return false;
    }
    return true;
}


