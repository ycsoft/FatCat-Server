#ifndef USERREGISTER_HPP
#define USERREGISTER_HPP

#include <boost/asio.hpp>
#include <iostream>
#include <boost/unordered_map.hpp>

#include "cmdtypes.h"
#include "server.h"
#include "Game/transfer.hpp"

#include "memManage/diskdbmanager.h"
#include "memManage/memdbmanager.h"
#include "Game/session.hpp"
#include "Game/userposition.hpp"
#include "log.hpp"

#include "utils/stringbuilder.hpp"

#define     RESULT_SUCCESS                      1
#define     RESULT_ERROR                        2

#define     RESULT_USER_REPEAT                  3
#define     RESULT_EMAIL_REPEAT                 4

#define    RESULT_PASSWORD_ERROR           3
#define    RESULT_USER_NOTEXIST            4

#define    RESULT_NICK_REPEAT              3

#define    ONLINE  1
#define    OFFLINE 2

using boost::asio::ip::tcp;
using namespace std;


//保存玩家角色退出数据，并发送下线通知给其他玩家
void SavePlayerOfflineData(TCPConnection::Pointer conn)
{
    Server *srv = Server::GetInstance();
    SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();

    //将玩家当前数据写进数据库,(位置，任务进度等)
    STR_PlayerPosition* playerPosition =   &((*smap)[conn].m_position);
    hf_int32 roleid = ((*smap)[conn].m_roleid);
    StringBuilder builder;
    builder << "update t_playerposition set pos_x=" << playerPosition->Pos_x << ",pos_y=" << playerPosition->Pos_y << ",pos_z=" << playerPosition->Pos_z << ",direct=" << playerPosition->Direct <<",mapid=" << playerPosition->MapID << ",actid=" << playerPosition->ActID << " where roleid=" << roleid;

    Logger::GetLogger()->Debug(builder.str());
    if(srv->getDiskDB()->Set(builder.str()) == -1)
    {
        Logger::GetLogger()->Error("更新玩家位置信息失败");
    }

    //更新玩家等级，经验

    STR_PackRoleExperience t_RoleExp;
    memcpy(&t_RoleExp, &(*smap)[conn].m_roleExp, sizeof(STR_PackRoleExperience));
    builder.Clear();
    builder << "update t_playerrolelist set level = " << t_RoleExp.level << ",experience = " << t_RoleExp.currentExp << " where roleid = " << roleid << ";";
    Logger::GetLogger()->Debug(builder.str());
    if(srv->getDiskDB()->Set(builder.str()) == -1)
    {
        Logger::GetLogger()->Error("更新玩家等级，经验信息失败");
    }


    umap_taskProcess umap_playerAcceptTask = (*smap)[conn].m_playerAcceptTask;

    builder.Clear();
    builder << "delete from t_taskprocess where roleid = " << roleid << ";";

     Logger::GetLogger()->Debug(builder.str());
    if(srv->getDiskDB()->Set(builder.str()) == -1)
    {
        Logger::GetLogger()->Error("清除玩家任务信息失败");
    }


    builder.Clear();
    builder << "insert into t_taskprocess values(";

    hf_uint32 count = umap_playerAcceptTask->size();
    if(count == 0)
    {
        return;
    }
    hf_uint32 i = 0;
    for(_umap_taskProcess::iterator it = umap_playerAcceptTask->begin(); it != umap_playerAcceptTask->end(); it++)
    {
        builder << roleid << "," << it->second.TaskID << "," << it->second.AimID << "," << it->second.AimCount << "," << it->second.AimAmount << "," << it->second.ExeModeID << ")";
        if(count == (i+1))
        {
            builder << ";";
        }
        else
        {
            builder << ",(";
            i++;
        }
    }

     Logger::GetLogger()->Debug(builder.str());
    if(srv->getDiskDB()->Set(builder.str()) == -1)
    {
        Logger::GetLogger()->Error("更新玩家任务信息失败");
    }


    Logger::GetLogger()->Debug("发送下线通知给可视范围内的玩家");

    //将下线消息通知给其他可是范围内的玩家和队员工会等
    for(_umap_roleSock::iterator it = (*smap)[conn].m_viewRole->begin(); it != (*smap)[conn].m_viewRole->end(); /*it++*/)
    {
        STR_PackHead t_packHead;
        t_packHead.Flag = FLAG_ViewRoleOffline;
        t_packHead.Len = sizeof(roleid);

        it->second->Write_all(&t_packHead, sizeof(STR_PackHead));
        it->second->Write_all(&roleid, sizeof(roleid));

         Logger::GetLogger()->Debug("删除玩家可视范围的自己");
         cout << roleid << endl;
        (*smap)[it->second].m_viewRole->erase(roleid);
         Logger::GetLogger()->Debug("删除自己可视范围内的玩家");
         cout << (*smap)[it->second].m_roleid << endl;
         _umap_roleSock::iterator iter = it;
         it++;
        (*smap)[conn].m_viewRole->erase((*smap)[iter->second].m_roleid);
         Logger::GetLogger()->Debug("删除OK");
    }

    Logger::GetLogger()->Debug("退出成功");

    //删除m_nickSock,m_nameSock
    SessionMgr::umap_nickSock nameSock = SessionMgr::Instance()->GetNameSock();
    SessionMgr::umap_nickSock nickSock = SessionMgr::Instance()->GetNickSock();

    SessionMgr::_umap_nickSock::iterator t_nameSock = nameSock->find(&(*smap)[conn].m_usrid[0]);
    if(t_nameSock != nameSock->end())
    {
        nameSock->erase(t_nameSock);
    }
    cout << (*smap)[conn].m_nick[0] << endl;
    SessionMgr::_umap_nickSock::iterator t_nickSock = nickSock->find(&(*smap)[conn].m_nick[0]);
    if(t_nickSock != nickSock->end())
    {
        nickSock->erase(t_nickSock);
    }


//    nameSock->erase(t_nameSock);
//    nickSock->erase(t_nickSock);

    //测试给所有玩家发送
//    STR_PackHead t_packHead;
//    t_packHead.Len = sizeof(roleid);
//    t_packHead.Flag = FLAG_ViewRoleOffline;
//    for(SessionMgr::SessionMap::iterator it = smap->begin(); it != smap->end(); it++)
//    {
//        if(it->first == sk)
//        {
//            continue;
//        }
//        Transfer::write_n(it->first, (hf_char*)&t_packHead, sizeof(STR_PackHead));
//        Transfer::write_n(it->first, (hf_char*)&roleid, t_packHead.Len);
//    }
}

//注册用户名及密码
void RegisterUserId(TCPConnection::Pointer conn, STR_PackPlayerRegisterUserId *reg)
{
    Logger::GetLogger()->Debug("Client Register UserID");

    Server *srv = Server::GetInstance();
    STR_PackResult t_packResult;
    StringBuilder       sbd;

    t_packResult.Flag = FLAG_PlayerRegisterUserId;


    (sbd<< "select * from t_playerLogin where username = '").Add(reg->userName)<<"'";
    Logger::GetLogger()->Debug(sbd.str());
    hf_int32 t_row = srv->getDiskDB()->GetSqlResult(sbd.str());

    if(t_row > 0)  //用户名已注册
    {
         t_packResult.result =  RESULT_USER_REPEAT;
    }
    else
    {
        sbd.Clear();
        (sbd<< "select * from t_playerLogin where email = '").Add(reg->Email)<<"'";

        t_row = srv->getDiskDB()->GetSqlResult(sbd.str());

        if(t_row > 0)//邮箱已注册
        {
             t_packResult.result = RESULT_EMAIL_REPEAT;
             Logger::GetLogger()->Debug("User ID repeat");
        }
        else
        {
            sbd.Clear();
            (sbd<<"insert into t_playerLogin values('").Add(reg->userName)<<"','";
            sbd.Add( reg->password)<<"','";
            sbd.Add(reg->Email)<<"');";

            Logger::GetLogger()->Debug(sbd.str());

            hf_int32 t_row = srv->getDiskDB()->Set(sbd.str());

            if(t_row == -1) //插入数据库失败
            {
                t_packResult.result = RESULT_ERROR;
                Logger::GetLogger()->Debug("Insert into DB Failed");
            }
            else// 注册成功
            {
                t_packResult.result = RESULT_SUCCESS;
                Logger::GetLogger()->Debug("Register ID Success");
            }
        }
    }

    conn->Write_all(&t_packResult, sizeof(t_packResult));
    // save session
    SessionMgr::Instance()->SaveSession( conn,reg->userName);
    srv->free(reg);
}

//注册角色，该函数必须在用户名已注册的情况下调用
void RegisterRole(TCPConnection::Pointer conn, STR_PackPlayerRegisterRole *reg)
{
    Server *srv = Server::GetInstance();
    STR_PackResult t_packResult;
    StringBuilder       builder;

    t_packResult.Flag = FLAG_PlayerRegisterRole;

    hf_char nickbuff[33] = { 0 };
    memcpy(nickbuff, reg->Nick, sizeof(reg->Nick));
    SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();
    char *pname =  & ( (*smap)[conn].m_usrid[0]);

    builder << "select * from t_playerrolelist where nick ='" << nickbuff <<"';";

    Logger::GetLogger()->Debug(builder.str());
    hf_int32 t_row = srv->getDiskDB()->GetSqlResult(builder.str());
    if(t_row == -1)
    {
        t_packResult.result = RESULT_ERROR;
        conn->Write_all(&t_packResult, sizeof(t_packResult));
    }
    else if(t_row == 1)  //昵称已注册
    {
        t_packResult.result = RESULT_NICK_REPEAT;
        conn->Write_all(&t_packResult, sizeof(t_packResult));
    }
    else
    {
        builder.Clear();
        builder<< "insert into t_PlayerRoleList(username,nick,profession,sex,figure,figurecolor,face,eye,hair,haircolor) values('" <<pname<<"','"<< nickbuff << "'," << reg->Profession << "," <<reg->Sex<<","<<reg->Figure<<","<<reg->FigureColor<<","<< reg->Face << "," << reg->Eye << "," << reg->Hair <<"," << reg->HairColor <<");";

        Logger::GetLogger()->Debug(builder.str());
        t_row = srv->getDiskDB()->Set(builder.str());

        if(t_row == -1) //插入数据库失败
        {
            t_packResult.result = RESULT_ERROR;
            conn->Write_all(&t_packResult, sizeof(t_packResult));
        }
        else
        {
            t_packResult.result = RESULT_SUCCESS;
            conn->Write_all(&t_packResult, sizeof(t_packResult));

            builder.Clear();
            builder << "select nick,roleid,profession,level,sex,figure,figurecolor,face,eye,hair,haircolor from t_playerrolelist where nick ='" << nickbuff <<"';";

            Logger::GetLogger()->Debug(builder.str());

            STR_PackRoleBasicInfo t_roleInfo;
            memset(&t_roleInfo, 0, sizeof(STR_PackRoleBasicInfo));
            hf_int32 t_row = srv->getDiskDB()->GetPlayerRegisterRoleInfo(&t_roleInfo, builder.str());

            if(t_row > 0)
            {
                STR_PackHead t_packHead;
                t_packHead.Flag = FLAG_PlayerRoleList;
                t_packHead.Len = sizeof(STR_PackRoleBasicInfo);
                //发送新注册的角色信息包
                conn->Write_all(&t_packHead, sizeof(STR_PackHead));
                conn->Write_all(&t_roleInfo, sizeof(STR_PackRoleBasicInfo));
            }

            //写入玩家初始位置
            builder.Clear();
            builder << "insert into t_playerposition(roleid) values(" << t_roleInfo.RoleID << ");";
            Logger::GetLogger()->Debug(builder.str());
            t_row = srv->getDiskDB()->Set(builder.str());
            if(t_row == -1)
            {
                Logger::GetLogger()->Error("写入玩家初始位置失败");
            }

            //写入玩家初始属性
            builder.Clear();
            builder << "insert into t_roleinfo(roleid) values(" << t_roleInfo.RoleID << ");";
            Logger::GetLogger()->Debug(builder.str());
            t_row = srv->getDiskDB()->Set(builder.str());
            if(t_row == -1)
            {
                Logger::GetLogger()->Error("写入玩家初始属性失败");
            }
        }
    }
    srv->free(reg);

}

//登陆帐号,并保存用户会话
void LoginUserId(TCPConnection::Pointer conn, STR_PackPlayerLoginUserId *reg)
{
    Server *srv = Server::GetInstance();
    ResRoleList t_Rolelist;
    STR_PackResult t_PackResult;
    t_PackResult.Flag = FLAG_PlayerLoginUserId;

    StringBuilder       sbd;

    hf_char namebuff[33] = { 0 };
    memcpy(namebuff, reg->userName, sizeof(reg->userName));

    sbd<<"select * from t_playerLogin where username = '"<<namebuff  << "';";
    Logger::GetLogger()->Debug("Login User ID");
    Logger::GetLogger()->Debug(sbd.str());

    STR_PackPlayerLoginUserId user;

    //查询该用户的相关信息
    hf_int32 t_row = srv->getDiskDB()->GetPlayerUserId(&user, sbd.str());
    if(t_row == 1)
    {
        if(strncmp(user.password, reg->password, 32) == 0) //用户名密码正确
        {
            SessionMgr::umap_nickSock nameSock = SessionMgr::Instance()->GetNameSock();
            SessionMgr::_umap_nickSock::iterator it = nameSock->find(reg->userName);
            if(it != nameSock->end()) //如果用户在线，保存该用户的相关信息，断开连接
            {
                STR_PackHead t_packHead;
                hf_uint8 value = 1;
                t_packHead.Flag = FlAG_UserRepeatLogin;
                t_packHead.Len = sizeof(value);
                it->second->Write_all(&t_packHead, sizeof(STR_PackHead));
                it->second->Write_all(&value, t_packHead.Len);
                SavePlayerOfflineData(it->second);
                SessionMgr::Instance()->RemoveSession(it->second);
            }

            t_PackResult.result = RESULT_SUCCESS;
            conn->Write_all(&t_PackResult, sizeof(STR_PackResult));

            SessionMgr::Instance()->SaveSession(conn, reg->userName);

            (*nameSock)[reg->userName] = conn;

            sbd.Clear();
            sbd<<"select nick,roleid,profession,level,sex,figure,figurecolor,face,eye,hair,haircolor from t_playerrolelist where username='"<<reg->userName<<"'";

            Logger::GetLogger()->Debug(sbd.str());
            //查询角色列表
            hf_int32 t_row = srv->getDiskDB()->GetPlayerRoleList(&t_Rolelist,sbd.str());
            if(t_row <= 0) //角色查询失败或没注册角色
            {
                return;
            }
            else
            {
                STR_PackHead t_packHead;
                t_packHead.Flag = FLAG_PlayerRoleList;
                hf_int32 len = sizeof(STR_PackRoleBasicInfo)*t_Rolelist.m_Role.size();
                t_packHead.Len = len;

                hf_char buff[1024] = { 0 };

                int roleCount = t_Rolelist.m_Role.size() ;

                //暂时发送最多20个角色列表
                roleCount = ( roleCount > 20 ?  20 : roleCount );
                for(size_t i = 0; i < roleCount; i++)
                {
                    memcpy(buff + i*sizeof(STR_PackRoleBasicInfo),&(t_Rolelist.m_Role[i]),sizeof(STR_PackRoleBasicInfo));
                    cout << t_Rolelist.m_Role[i].Nick << endl;
                }
                Logger::GetLogger()->Debug("Login ID Success");

                conn->Write_all(&t_packHead, sizeof(STR_PackHead));
                conn->Write_all(buff, len);
            }
        }
        else //密码不正确
        {
            t_PackResult.result = RESULT_PASSWORD_ERROR;
            conn->Write_all(&t_PackResult, sizeof(STR_PackResult));
        }
    }
    else
    {
        if(t_row == -1)//查询失败
        {
            t_PackResult.result = RESULT_ERROR;
        }
        else//用户名不存在 t_row = 0
        {
            t_PackResult.result = RESULT_USER_NOTEXIST;
        }
        conn->Write_all(&t_PackResult, sizeof(STR_PackResult));
    }
    srv->free(reg);
}

//角色登陆
void LoginRole(TCPConnection::Pointer conn, STR_PackPlayerRole *reg)
{
    Server *srv = Server::GetInstance();
    STR_PackResult t_packResult;
    t_packResult.Flag = FLAG_PlayerLoginRole;

    SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();

    hf_char *pname =  & ( (*smap)[conn].m_usrid[0]);
    StringBuilder sbd;
    sbd << "select * from t_playerrolelist where username='"
         <<pname<<"' and roleid="
         <<(hf_int32)(reg->Role)<<";";

    Logger::GetLogger()->Debug(sbd.str());

    hf_int32 t_row = srv->getDiskDB()->GetSqlResult(sbd.str());

    if(t_row <= 0) //角色查询失败
    {
        t_packResult.result = RESULT_ERROR;
        conn->Write_all(&t_packResult, sizeof(STR_PackResult));
    }
    else   //角色查询成功
    {
        t_packResult.result = RESULT_SUCCESS;
        conn->Write_all(&t_packResult, sizeof(STR_PackResult));
        Logger::GetLogger()->Debug("Login Role Success");

        SessionMgr::Instance()->SaveSession(conn, reg->Role);


        //查询角色昵称，后面添加好友用
        //查询角色经验，发送给玩家
        //查询角色基本信息
        sbd.Clear();
//        sbd << "select nick,level,experience from t_playerrolelist where roleid = " << reg->Role << ";";
        sbd << "select * from t_playerrolelist where roleid = " << reg->Role << ";";
        SessionMgr::umap_nickSock t_nickSock = SessionMgr::Instance()->GetNickSock();

        RoleNick t_nick;
        t_row = srv->getDiskDB()->GetRoleExperience(&t_nick, &(*smap)[conn].m_roleExp, &(*smap)[conn].m_RoleBaseInfo, sbd.str());
        if(t_row == 1)
        {
            (*t_nickSock)[t_nick.nick] = conn;
            memcpy(&(*smap)[conn].m_nick[0], t_nick.nick, sizeof(t_nick.nick));

            conn->Write_all(&(*smap)[conn].m_roleExp, sizeof(STR_PackRoleExperience));
        }

        //查询角色属性，发送给玩家
        STR_RoleInfo t_roleInfo;
        STR_PackHead t_packHead;
        sbd.Clear();
        sbd << "select * from t_roleInfo where roleid = " << reg->Role << ";";
        Logger::GetLogger()->Debug(sbd.str());
        t_row = srv->getDiskDB()->GetRoleInfo(&t_roleInfo, sbd.str());
        if(t_row == 1)
        {
            t_packHead.Flag = FLAG_RoreInfo;
            t_packHead.Len = sizeof(STR_RoleInfo);
            conn->Write_all(&t_packHead, sizeof(STR_PackHead));
            conn->Write_all(&t_roleInfo, t_packHead.Len);
            SessionMgr::Instance()->SaveSession(conn, &t_roleInfo);
        }

        //发送好友列表
        SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();
        umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
        sbd.Clear();
        sbd << "select friendroleid,nick from t_friendlist,t_playerrolelist where t_friendlist.roleid = " << reg->Role << " and t_friendlist.roleid = t_playerrolelist.roleid;";

        umap_friendList t_friendList = ((*smap)[conn]).m_friendList;
        t_row = srv->getDiskDB()->GetFriendList(t_friendList, sbd.str());


        hf_int32 count = t_friendList->size();
        hf_char* buff = new char[sizeof(STR_FriendInfo)*count + sizeof(STR_PackHead)];

        memset(&t_packHead, 0, sizeof(STR_PackHead));
        t_packHead.Flag = FLAG_FriendList;
        t_packHead.Len = count * sizeof(STR_FriendInfo);
        memcpy(buff, &t_packHead, sizeof(STR_PackHead));

        if(t_row == 0) //好友列表为空
        {

        }
        else if(t_row > 0) //有好友列表
        {
            hf_int32 i = 0;
            STR_PackHead packHead;
            packHead.Len = sizeof(STR_PackPlayerRole);
            packHead.Flag = FLAG_FriendOnline;

            for(_umap_friendList::iterator it = t_friendList->begin(); it != t_friendList->end(); it++)
            {
                //查询好友是否在线
                _umap_roleSock::iterator iter = t_roleSock->find(it->first);
                if(iter != t_roleSock->end())
                {
                    it->second.Status = ONLINE;
                    //给其他在线的好友发送该玩家上线通知
                    iter->second->Write_all(&packHead, sizeof(STR_PackHead));
                    iter->second->Write_all(&reg->Role,packHead.Len);
                }
                memcpy(buff + sizeof(STR_PackHead) + i*sizeof(STR_FriendInfo), &(it->second), sizeof(STR_FriendInfo));
            }
            conn->Write_all(buff, t_packHead.Len + sizeof(STR_PackHead));
            delete[] buff;
        }


        Logger::GetLogger()->Debug("Send Position data.....");
        UserPosition::Position_push(conn, reg->Role);
    }
    srv->free(reg);
}




//用户下线，发送下线通知给其他玩家，保存改用户的相关信息，断开sock连结
void PlayerOffline(TCPConnection::Pointer conn, STR_PackPlayerOffline *reg)
{
    Server *srv = Server::GetInstance();
    SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();

    StringBuilder builder;

    SavePlayerOfflineData(conn);
    //将下线消息通知给其他可是范围内的玩家和队员工会等


    if(reg->type == 1)  //断开链接
    {
        SessionMgr::Instance()->RemoveSession(conn);
    }
    else if(reg->type  == 2) //返回角色列表
    {
        hf_char* userName =   (hf_char*)&((*smap)[conn].m_usrid);

        builder.Clear();
        builder<<"select nick,roleid,profession,level,sex,figure,figurecolor,face,eye,hair,haircolor from t_playerrolelist where username='"<<userName<<"';";

        Logger::GetLogger()->Debug(builder.str());
        //查询角色列表
        ResRoleList t_Rolelist;
        hf_int32 t_row = srv->getDiskDB()->GetPlayerRoleList(&t_Rolelist, builder.str());
        if(t_row > 0)
        {
            STR_PackHead t_packHead;
            t_packHead.Flag = FLAG_PlayerRoleList;
            hf_int32 len = sizeof(STR_PackRoleBasicInfo)*t_Rolelist.m_Role.size();
            t_packHead.Len = len;

            hf_char buff[1024] = { 0 };

            hf_int32 roleCount = t_Rolelist.m_Role.size() ;

            //暂时发送最多20个角色列表
            roleCount = ( roleCount > 20 ?  20 : roleCount );
            for(size_t i = 0; i < roleCount; i++)
            {
                memcpy(buff + i*sizeof(STR_PackRoleBasicInfo),&(t_Rolelist.m_Role[i]),sizeof(STR_PackRoleBasicInfo));
                cout << t_Rolelist.m_Role[i].Nick << endl;
            }
            Logger::GetLogger()->Debug("Login ID Success");

            conn->Write_all(&t_packHead, sizeof(STR_PackHead));
            conn->Write_all(buff, len);
        }
    }
}





#endif // USERREGISTER_HPP

