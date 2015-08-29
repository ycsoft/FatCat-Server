#ifndef TEAMFRIEND_H
#define TEAMFRIEND_H

#include "Game/postgresqlstruct.h"
#include "NetWork/tcpconnection.h"

class TeamFriend
{
public:
    TeamFriend();
    ~TeamFriend();

    //添加好友
    void addFriend(TCPConnection::Pointer conn, STR_PackAddFriend* addFriend);
    //删除好友
    void deleteFriend(TCPConnection::Pointer conn, hf_uint32  roleid);
    //接收到客户端添加好友的回复
    void ReciveAddFriend(TCPConnection::Pointer conn, STR_PackAddFriendReturn* addFriend);
    //发送离线的添加好友请求
    void SendAskAddFriend(TCPConnection::Pointer conn);

};

#endif // TEAMFRIEND_H
