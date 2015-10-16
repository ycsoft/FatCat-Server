#ifndef GAMECHAT_H
#define GAMECHAT_H
#include "Game/postgresqlstruct.h"
#include "NetWork/tcpconnection.h"

class GameChat
{
public:
    GameChat();
    ~GameChat();
    void Chat(TCPConnection::Pointer conn, STR_PackRecvChat* chat);
};

#endif // GAMECHAT_H
