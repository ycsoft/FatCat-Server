#include "gamechat.h"
#include "./../Game/session.hpp"
#include "./../server.h"

GameChat::GameChat()
{

}

GameChat::~GameChat()
{

}

void GameChat::Chat(TCPConnection::Pointer conn, STR_PackRecvChat* chat)
{
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    hf_char* userName = (*smap)[conn].m_RoleBaseInfo.Nick;
    STR_PackSendChat t_chat(chat->chatMessage, userName);
    for(SessionMgr::SessionMap::iterator it = smap->begin(); it != smap->end(); it++)
    {
        it->first->Write_all(&t_chat, sizeof(STR_PackSendChat));
    }
    Server::GetInstance()->free(chat);
}
