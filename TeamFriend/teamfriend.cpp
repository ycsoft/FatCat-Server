#include "teamfriend.h"
#include "utils/stringbuilder.hpp"
#include "memManage/diskdbmanager.h"
#include "server.h"
#include "Game/session.hpp"


#include "boost/lockfree/queue.hpp"

TeamFriend::TeamFriend()
{

}

TeamFriend::~TeamFriend()
{

}


void TeamFriend::addFriend(TCPConnection::Pointer conn, STR_PackAddFriend* addFriend)
{
    Server* srv = Server::GetInstance();

    STR_PackAddFriend t_addFriend;
    SessionMgr::SessionMap* smap = SessionMgr::Instance()->GetSession().get();
    t_addFriend.RoleID = (*smap)[conn].m_roleid;
    memcpy(t_addFriend.Nick, ((*smap)[conn].m_RoleBaseInfo.Nick), 32);

    if(addFriend->RoleID >= 100000000) //按roleID添加好友（优先）
    {
        umap_roleSock roleSock = SessionMgr::Instance()->GetRoleSock();
        _umap_roleSock::iterator it = roleSock->find(addFriend->RoleID);
        if(it != roleSock->end()) //要添加的好友在线
        {
            it->second->Write_all(&t_addFriend, sizeof(STR_PackAddFriend));
        }
        if(it == roleSock->end()) //玩家不在线
        {
            StringBuilder sbd;
            sbd << "insert into t_addFriend values(" << t_addFriend.RoleID << ",'" << t_addFriend.Nick << "'," << addFriend->RoleID << ");";
            Logger::GetLogger()->Debug(sbd.str());
            if(srv->getDiskDB()->Set(sbd.str()) == -1)
            {
                Logger::GetLogger()->Error("insert add role friend error");
            }
        }
    }
    else  //按昵称添加好友
    {
         SessionMgr::umap_nickSock nickSock = SessionMgr::Instance()->GetNickSock();
         SessionMgr::_umap_nickSock::iterator it = nickSock->find(addFriend->Nick);
         if(it != nickSock->end()) //在线
         {
             it->second->Write_all(&t_addFriend, sizeof(STR_PackAddFriend));
         }
         else //添加的好友不在线
         {
             hf_char nickbuff[40] = { 0 };
             memcpy(nickbuff, addFriend->Nick, sizeof(addFriend->Nick));
             //在数据库中查找昵称是否存在
             StringBuilder sbd;
             sbd << "select roleid from t_playerrolelist where nick = '" << nickbuff << "';";
             Logger::GetLogger()->Debug(sbd.str());
             hf_uint32 addroleid = 0;
             hf_int32 t_row = srv->getDiskDB()->GetNickRoleid(&addroleid, sbd.str());
             if(t_row == 1) //昵称存在，保存为离线请求
             {
                 hf_uint32 Requestroleid = (*smap)[conn].m_roleid;
                 hf_char   RequestNick[40] = { 0 };
                 memcpy(RequestNick, (*smap)[conn].m_RoleBaseInfo.Nick, 32);
                 sbd.Clear();
                 sbd << "insert into t_addFriend values(" << Requestroleid << ",'" << RequestNick << "'," << addroleid << ");";
                 Logger::GetLogger()->Debug(sbd.str());
                 t_row = srv->getDiskDB()->Set(sbd.str());
                 if( t_row == -1)
                 {
                     Logger::GetLogger()->Error("insert add friend error");
                 }
             }
             else if(t_row == 0) //昵称不存在
             {
                 STR_PackAddFriendReturn Raddfriend;

                 memcpy(Raddfriend.Nick, addFriend->Nick, 32);
                 Raddfriend.value = 3;
                 conn->Write_all(&Raddfriend, sizeof(STR_PackAddFriendReturn));
             }
         }
    }
//    srv->free(addFriend);
}

void TeamFriend::deleteFriend(TCPConnection::Pointer conn, hf_uint32  roleid)
{
    SessionMgr::SessionMap* smap = SessionMgr::Instance()->GetSession().get();
    umap_friendList friendList = (*smap)[conn].m_friendList;

    umap_roleSock roleSock = SessionMgr::Instance()->GetRoleSock();

   _umap_friendList::iterator it = friendList->find(roleid);
    if(it == friendList->end()) //要删除的好友存在
    {
        return;
    }
    _umap_roleSock::iterator iter = roleSock->find(roleid);
    if(iter != roleSock->end()) //要删除的好友在线,从其好友列表里删除
    {
        umap_friendList delete_friendList = (*smap)[iter->second].m_friendList;
        _umap_friendList::iterator delete_it = delete_friendList->find((*smap)[conn].m_roleid);
        if(delete_it != delete_friendList->end())
        {
            delete_friendList->erase(delete_it); //从好友列表中删除自己
        }

        STR_PackDeleteFriend t_delFriend;
        t_delFriend.RoleID = (*smap)[conn].m_roleid;
        iter->second->Write_all(&t_delFriend, sizeof(STR_PackDeleteFriend));//给好友发送删除包
    }
    //从数据库删除
    StringBuilder sbd;
    sbd << "delete from t_friendlist where roleid = " << roleid << " and friendroleid = " << (*smap)[conn].m_roleid << ";";
    Logger::GetLogger()->Debug(sbd.str());
    Server *srv = Server::GetInstance();
    hf_int32 t_row = srv->getDiskDB()->Set(sbd.str());
    if(t_row == 1)
    {
        Logger::GetLogger()->Debug("delete success");
    }

    //从好友列表里删除
    friendList->erase(it);
    //从数据库中删除
    sbd.Clear();
    sbd << "delete from t_friendlist where roleid = " << (*smap)[conn].m_roleid << " and friendroleid = " << roleid << ";";
    Logger::GetLogger()->Debug(sbd.str());
    t_row = srv->getDiskDB()->Set(sbd.str());
    if(t_row == 1)
    {
        Logger::GetLogger()->Debug("delete  friend success");
    }
}

//接收到客户端添加好友的回复
void TeamFriend::ReciveAddFriend(TCPConnection::Pointer conn, STR_PackAddFriendReturn* addFriend)
{
    Server* srv = Server::GetInstance();
    SessionMgr::SessionMap* smap = SessionMgr::Instance()->GetSession().get();
    umap_roleSock roleSock = SessionMgr::Instance()->GetRoleSock();
    if(addFriend->value == 1) //同意添加
    {
        StringBuilder sbd;
        //将要添加的好友写入到该好友列表中
        sbd << "insert into t_friendlist values(" << addFriend->RoleID << "," << (*smap)[conn].m_roleid << ");";
        Logger::GetLogger()->Debug(sbd.str());
        hf_int32 t_row = srv->getDiskDB()->Set(sbd.str());
        if(t_row == -1)
        {
            Logger::GetLogger()->Error("insert add friendlist error");
        }

        //将该好友添加到要添加的好友列表中
        sbd.Clear();
        sbd << "insert into t_friendlist values(" << (*smap)[conn].m_roleid << "," << addFriend->RoleID<< ");";
        Logger::GetLogger()->Debug(sbd.str());
        t_row = srv->getDiskDB()->Set(sbd.str());
        if(t_row == -1)
        {
            Logger::GetLogger()->Error("insert add friendlist error");
        }

        _umap_roleSock::iterator it = roleSock->find(addFriend->RoleID);
        if(it != roleSock->end()) //在线
        {
            //更新在线好友列表
            STR_FriendInfo t_friendInfo;
            t_friendInfo.RoleID = addFriend->RoleID;
            memcpy(t_friendInfo.Nick, addFriend->Nick, 32);
            t_friendInfo.Status = 1;

            umap_friendList friendList = (*smap)[conn].m_friendList;
            (*friendList)[addFriend->RoleID] = t_friendInfo;

            //更新要添加的好友的在线好友列表
            t_friendInfo.RoleID = (*smap)[conn].m_roleid;
            memcpy(t_friendInfo.Nick, (*smap)[conn].m_RoleBaseInfo.Nick, 32);

            friendList = (*smap)[it->second].m_friendList;
            (*friendList)[(*smap)[conn].m_roleid] = t_friendInfo;


            //发送添加好友返回数据包
            addFriend->RoleID = (*smap)[conn].m_roleid;
            memcpy(addFriend->Nick, (*smap)[conn].m_RoleBaseInfo.Nick, 32);
            it->second->Write_all(addFriend, sizeof(STR_PackAddFriend));
        }
    }
    else //不同意
    {
        _umap_roleSock::iterator it = roleSock->find(addFriend->RoleID);
        if(it != roleSock->end()) //在线
        {
            addFriend->RoleID = (*smap)[conn].m_roleid;
            memcpy(addFriend->Nick, (*smap)[conn].m_RoleBaseInfo.Nick, 32);
            it->second->Write_all(addFriend, sizeof(STR_PackAddFriendReturn));
        }
    }
//    srv->free(addFriend);
}

//玩家上线，发送离线的添加好友请求
 void TeamFriend::SendAskAddFriend(TCPConnection::Pointer conn)
 {
     SessionMgr::SessionMap* smap = SessionMgr::Instance()->GetSession().get();
     hf_uint32 roleid = (*smap)[conn].m_roleid;
     StringBuilder sbd;
     sbd << "select requestroleid,requestnick from t_addFriend where addroleid = " << roleid << ";";
     Logger::GetLogger()->Debug(sbd.str());

     Server* srv = Server::GetInstance();
     vector<STR_AddFriend> addFriend;
     hf_int32 t_row = srv->getDiskDB()->GetAskAddFriend(addFriend, sbd.str());
     if(t_row > 0)
     {
         hf_char* buff = (hf_char*)srv->malloc();
         STR_PackHead t_packHead;
         t_packHead.Flag = FLAG_AddFriend;
         t_packHead.Len = t_row*sizeof(STR_AddFriend);
         cout << "离线好友请求:" << t_row << endl;
         hf_int32 i = 0;
         for(vector<STR_AddFriend>::iterator it = addFriend.begin(); it != addFriend.end(); it++)
         {
             memcpy(buff + sizeof(STR_PackHead) + i*sizeof(STR_AddFriend),&(*it), sizeof(STR_AddFriend));
             i++;
         }
         memcpy(buff, &t_packHead, sizeof(STR_PackHead));
         conn->Write_all(buff,t_packHead.Len + sizeof(STR_PackHead));

         srv->free(buff);

         //删除保存的离线添加请求
         sbd.Clear();
         sbd << "delete from t_addFriend where addroleid = " << roleid << ";";
         Logger::GetLogger()->Debug(sbd.str());
         t_row = srv->getDiskDB()->Set(sbd.str());
         if(t_row == 1)
         {
             Logger::GetLogger()->Debug("delete asd addfriend success");
         }
     }
 }
