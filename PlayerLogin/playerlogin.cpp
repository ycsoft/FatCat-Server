#include "Game/userposition.hpp"
#include "OperationGoods/operationgoods.h"
#include "playerlogin.h"
#include "server.h"


#define     RESULT_SUCCESS                 1
#define     RESULT_ERROR                   2

#define     RESULT_USER_REPEAT             2     //用户名已被注册
#define     RESULT_EMAIL_REPEAT            3     //邮箱已被注册

#define    RESULT_PASSWORD_ERROR           2     //密码不正确
#define    RESULT_USER_NOTEXIST            3     //用户名不存在

#define    RESULT_NICK_REPEAT              3     //昵称已被注册

#define    ONLINE  1   //在线
#define    OFFLINE 2   //不在线


PlayerLogin::PlayerLogin()
{

}

PlayerLogin::~PlayerLogin()
{

}

//保存玩家角色退出数据，并发送下线通知给其他玩家
void PlayerLogin::SavePlayerOfflineData(TCPConnection::Pointer conn)
{
    Server *srv = Server::GetInstance();
    SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();
    hf_int32 roleid = ((*smap)[conn].m_roleid);
    if(roleid < 100000000) //没登录角色
    {
        return;
    }
    //将玩家当前数据写进数据库,(位置，任务进度等)
    STR_PackPlayerPosition* playerPosition =   &((*smap)[conn].m_position);
    StringBuilder sbd;
    sbd << "update t_playerposition set pos_x=" << playerPosition->Pos_x << ",pos_y=" << playerPosition->Pos_y << ",pos_z=" << playerPosition->Pos_z << ",direct=" << playerPosition->Direct <<",mapid=" << playerPosition->MapID << ",actid=" << playerPosition->ActID << " where roleid=" << roleid;

    Logger::GetLogger()->Debug(sbd.str());
    if(srv->getDiskDB()->Set(sbd.str()) == -1)
    {
        Logger::GetLogger()->Error("更新玩家位置信息失败");
    }

//    UpdateLevel Playerlevel(roleid, (*smap)[conn].m_roleExp.Level);
//    PlayerLogin::UpdatePlayerLevel(&Playerlevel); //更新玩家等级
//    UpdateExp PlayerExp(roleid, (*smap)[conn].m_roleExp.CurrentExp);
//    PlayerLogin::UpdatePlayerExp(&PlayerExp);  //更新玩家经验
//    SaveRoleTaskProcess(conn);    //保存玩家任务进度
//    SaveRoleBagGoods(conn);       //保存玩家背包里的物品
//    SaveRoleEquAttr(conn);        //保存玩家装备属性
//    SaveRoleMoney(conn);          //保存玩家金币

    FriendOffline(conn);          //发送下线通知给好友
    SaveRoleNotPickGoods(conn);   //保存玩家未捡取的掉落物品
    SendOffLineToViewRole(conn);  //将下线消息通知给可视范围内的玩家
    DeleteNickSock(conn);         //删除m_nickSock

    ///////////////////////////////////////////////////
    SessionMgr::SessionMap::iterator it = smap->find(conn);
    if(it != smap->end())
    {
        if((*smap)[conn].m_interchage->isInchange)         //如果正在交易中
        {
            TCPConnection::Pointer partnerConn = (*smap)[conn].m_interchage->partnerConn;
            SessionMgr::SessionMap::iterator iter = smap->find(partnerConn);
            if(iter != smap->end())
            {
                (*smap)[partnerConn].m_interchage->clear();  //清除交易对方的交易状态
            }
        }
    }
    //////////////////////////////////////////////////

    Logger::GetLogger()->Debug("退出成功");
}

//用户下线删除保存的对应的<nick, sock>
void PlayerLogin::DeleteNickSock(TCPConnection::Pointer conn)
{
    SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();

    SessionMgr::umap_nickSock nickSock = SessionMgr::Instance()->GetNickSock();
    SessionMgr::_umap_nickSock::iterator t_nickSock = nickSock->find(&(*smap)[conn].m_nick[0]);
    if(t_nickSock != nickSock->end())
    {
        nickSock->erase(t_nickSock); //删除m_nickSock
    }
}

void PlayerLogin::DeleteNameSock(TCPConnection::Pointer conn)
{
    SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();

    SessionMgr::umap_nickSock nameSock = SessionMgr::Instance()->GetNameSock();
    SessionMgr::_umap_nickSock::iterator t_nameSock = nameSock->find(&(*smap)[conn].m_usrid[0]);
    if(t_nameSock != nameSock->end())
    {
        nameSock->erase(t_nameSock); //删除m_nameSock
    }
}

void PlayerLogin::RegisterUserID(TCPConnection::Pointer conn, STR_PlayerRegisterUserId *reg)
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
        Logger::GetLogger()->Debug("User ID repeat");
        t_packResult.result =  RESULT_USER_REPEAT;
    }
    else
    {
        sbd.Clear();
        (sbd<<"insert into t_playerLogin values('").Add(reg->userName)<<"','";
        sbd.Add( reg->password)<<"','";
        sbd.Add(reg->Email)<<"');";

        Logger::GetLogger()->Debug(sbd.str());

        hf_int32 t_row = srv->getDiskDB()->Set(sbd.str());
        if(t_row == -1) //邮箱已注册
        {
            t_packResult.result = RESULT_EMAIL_REPEAT;
            Logger::GetLogger()->Debug("User email repeat");
        }
        else// 注册成功
        {
            t_packResult.result = RESULT_SUCCESS;
            Logger::GetLogger()->Debug("Register ID Success");
        }
    }

    conn->Write_all(&t_packResult, sizeof(t_packResult));
    srv->free(reg);
}


//注册角色，该函数必须在用户名已注册的情况下调用
void PlayerLogin::RegisterRole(TCPConnection::Pointer conn, STR_PlayerRegisterRole *reg)
{
    Server *srv = Server::GetInstance();
    STR_PackResult t_packResult;
    StringBuilder       sbd;

    t_packResult.Flag = FLAG_PlayerRegisterRole;

    hf_char nickbuff[33] = { 0 };
    memcpy(nickbuff, reg->Nick, sizeof(reg->Nick));
    SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();
    hf_char *pname =  & ( (*smap)[conn].m_usrid[0]);
    sbd<< "insert into t_PlayerRoleList(username,nick,profession,sex,figure,figurecolor,face,eye,hair,haircolor,modeid,skirtid) values('" <<pname<<"','"<< nickbuff << "'," << reg->Profession << "," <<reg->Sex<<","<<reg->Figure<<","<<reg->FigureColor<<","<< reg->Face << "," << reg->Eye << "," << reg->Hair <<"," << reg->HairColor << "," << reg->ModeID << "," << reg->SkirtID <<");";

    Logger::GetLogger()->Debug(sbd.str());
    hf_int32 t_row = srv->getDiskDB()->Set(sbd.str());

    if(t_row == -1) //昵称已注册
    {
        t_packResult.result = RESULT_NICK_REPEAT;
        conn->Write_all(&t_packResult, sizeof(t_packResult));
    }
    else
    {
        t_packResult.result = RESULT_SUCCESS;
        conn->Write_all(&t_packResult, sizeof(t_packResult));
        Logger::GetLogger()->Debug("角色注册成功");

        sbd.Clear();
        sbd << "select nick,roleid,profession,level,sex,figure,figurecolor,face,eye,hair,haircolor,modeid,skirtid from t_playerrolelist where nick ='" << nickbuff <<"';";

        Logger::GetLogger()->Debug(sbd.str());

        STR_PackRoleBasicInfo t_roleInfo;

        hf_int32 t_row = srv->getDiskDB()->GetPlayerRegisterRoleInfo(&t_roleInfo.RoleInfo, sbd.str());
        if(t_row > 0)
        {
            //发送新注册的角色信息包
            conn->Write_all(&t_roleInfo, sizeof(STR_PackRoleBasicInfo));
        }

        //写入玩家初始位置
        sbd.Clear();
        sbd << "insert into t_playerposition(roleid) values(" << t_roleInfo.RoleInfo.RoleID << ");";
        Logger::GetLogger()->Debug(sbd.str());
        t_row = srv->getDiskDB()->Set(sbd.str());
        if(t_row != 1)
        {
            Logger::GetLogger()->Error("写入玩家初始位置失败");
        }

        //写入玩家初始属性
        sbd.Clear();
        sbd << "insert into t_roleinfo(roleid) values(" << t_roleInfo.RoleInfo.RoleID << ");";
        Logger::GetLogger()->Debug(sbd.str());
        t_row = srv->getDiskDB()->Set(sbd.str());
        if(t_row != 1)
        {
            Logger::GetLogger()->Error("写入玩家初始属性失败");
        }

        //写入初始金钱
        sbd.Clear();
        sbd << "insert into t_playermoney values(" << t_roleInfo.RoleInfo.RoleID << ",0," << Money_1 << ");";
        Logger::GetLogger()->Debug(sbd.str());
        t_row = srv->getDiskDB()->Set(sbd.str());
        if(t_row != 1)
        {
            Logger::GetLogger()->Error("写入玩家初始金钱失败");
        }
    }
    srv->free(reg);
}

//删除角色 此函数只能在登录用户，未登录角色的前提下调用
void PlayerLogin::DeleteRole(TCPConnection::Pointer conn, hf_uint32 roleid)
{
    SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();
    StringBuilder       sbd;
    hf_char* NameBuff = &(*smap)[conn].m_usrid[0];
    time_t timep;
    time(&timep);
    //删除该用户的该角色
    sbd << "update t_playerrolelist set ifdelete = 1,deletetime = " << (hf_uint32)timep << "where roleid = " << roleid << " and username = '" << NameBuff << "';";   //不删除，只是将标志置为1表示删除，更新改变标志时的时间，已方便请求找回和过固定时间后删除该用户的所有信息
//    sbd << "delete from t_playerrolelist where roleid = " << roleid << " and username = '" << NameBuff << "';";
    Logger::GetLogger()->Debug(sbd.str());
    hf_int32 value = Server::GetInstance()->getDiskDB()->Set(sbd.str());
    if(value == 1)
    {
        Logger::GetLogger()->Debug("delete roleid success");
        //删除该角色相关的所有信息

    }
    else
    {
        Logger::GetLogger()->Debug("delete roleid fail");
    }
}

//登陆帐号,并保存用户会话
void PlayerLogin::LoginUserId(TCPConnection::Pointer conn, STR_PlayerLoginUserId *reg)
{
    Server *srv = Server::GetInstance();
    StringBuilder       sbd;

    STR_PackResult t_PackResult;
    t_PackResult.Flag = FLAG_PlayerLoginUserId;

    hf_char namebuff[33] = { 0 };
    memcpy(namebuff, reg->userName, sizeof(reg->userName));

    sbd<<"select * from t_playerLogin where username = '"<<namebuff  << "';";
    Logger::GetLogger()->Debug("Login User ID");
    Logger::GetLogger()->Debug(sbd.str());

    STR_PlayerLoginUserId user;

    //查询该用户的相关信息
    hf_int32 t_row = srv->getDiskDB()->GetPlayerUserId(&user, sbd.str());
    if(t_row == 1)
    {
        if(strncmp(user.password, reg->password, 32) == 0) //用户名密码正确
        {
            SessionMgr::umap_nickSock nameSock = SessionMgr::Instance()->GetNameSock();
            SessionMgr::_umap_nickSock::iterator it = nameSock->find(reg->userName);
            if(it != nameSock->end()) //如果用户在线，保存该用户的相关信息，不断开连接
            {
                STR_PackHead t_packHead;
                t_packHead.Flag = FlAG_UserRepeatLogin;
                t_packHead.Len = 0;
                it->second->Write_all(&t_packHead, sizeof(STR_PackHead));

                SavePlayerOfflineData(it->second);
                SessionMgr::Instance()->RemoveSession(it->second);
            }

            t_PackResult.result = RESULT_SUCCESS;
            conn->Write_all(&t_PackResult, sizeof(STR_PackResult));

            SessionMgr::Instance()->SaveSession(conn, reg->userName);
            (*nameSock)[reg->userName] = conn;
            //发送角色列表
            SendRoleList(conn, reg->userName);
        }
        else //密码不正确
        {
            t_PackResult.result = RESULT_PASSWORD_ERROR;
            conn->Write_all(&t_PackResult, sizeof(STR_PackResult));
        }
    }
    else
    {       
        //用户名不存在 t_row = 0
        t_PackResult.result = RESULT_USER_NOTEXIST;
        conn->Write_all(&t_PackResult, sizeof(STR_PackResult));
    }
    srv->free(reg);
}

//角色登陆 该函数只能在用户登录的条件下使用
void PlayerLogin::LoginRole(TCPConnection::Pointer conn, hf_uint32 roleid)
{
    Server *srv = Server::GetInstance();
    STR_PackResult t_packResult;
    t_packResult.Flag = FLAG_PlayerLoginRole;

    SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();

    hf_char *pname =  & ( (*smap)[conn].m_usrid[0]);
    StringBuilder sbd;
    sbd << "select * from t_playerrolelist where username='"
         <<pname<<"' and roleid=" << roleid << " and ifdelete = 0;";
    Logger::GetLogger()->Debug(sbd.str());

    hf_int32 t_row = srv->getDiskDB()->GetSqlResult(sbd.str());
    if(t_row <= 0) //登录失败
    {
        t_packResult.result = RESULT_ERROR;
        conn->Write_all(&t_packResult, sizeof(STR_PackResult));
    }
    else   //角色查询成功
    {
        t_packResult.result = RESULT_SUCCESS;
        conn->Write_all(&t_packResult, sizeof(STR_PackResult));
        Logger::GetLogger()->Debug("Login Role Success");

        SessionMgr::Instance()->SaveSession(conn, roleid);


        //查询角色昵称，后面添加好友用
        //查询角色经验，发送给玩家
        //查询角色基本信息
        sbd.Clear();
        sbd << "select * from t_playerrolelist where roleid = " << roleid << ";";
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
        STR_PackRoleInfo t_roleInfo;
        sbd.Clear();
        sbd << "select * from t_roleInfo where roleid = " << roleid << ";";
        Logger::GetLogger()->Debug(sbd.str());
        t_row = srv->getDiskDB()->GetRoleInfo(&t_roleInfo.RoleInfo, sbd.str());
        if(t_row == 1)
        {
            conn->Write_all(&t_roleInfo, sizeof(STR_PackRoleInfo));
            SessionMgr::Instance()->SaveSession(conn, &t_roleInfo.RoleInfo);
        }

        Logger::GetLogger()->Debug("Send Position data.....");
        UserPosition::Position_push(conn, roleid);

        SendFriendList(conn, roleid);       //发送好友列表
        SendRoleMoney(conn, roleid);        //发送玩家金币
        SendRoleGoods(conn, roleid);        //发送玩家背包物品
        SendRoleEquAttr(conn, roleid);      //发送玩家装备属性
        SendRoleNotPickGoods(conn, roleid); //发送玩家未捡取的物品

        Server* srv = Server::GetInstance();
        srv->GetGameAttack()->SendPlayerSkill(conn);       //玩家可使用的技能
        srv->GetMonster()->PushViewMonsters(conn);         //玩家可视范围内的怪物
        srv->GetPlayerLogin()->SendViewRole(conn);         //玩家可视范围内的玩家
        srv->GetGameTask()->SendPlayerTaskProcess(conn);   //玩家任务进度
        srv->GetGameTask()->SendPlayerViewTask(conn);      //玩家可接任务
        srv->GetTeamFriend()->SendAskAddFriend(conn);      //离线的添加好友请求
    }
}

//用户下线，发送下线通知给其他玩家，保存改用户的相关信息，断开sock连结
void PlayerLogin::PlayerOffline(TCPConnection::Pointer conn, STR_PackPlayerOffline *reg)
{
    Server* srv = Server::GetInstance();
    SessionMgr::SessionPointer smap = SessionMgr::Instance()->GetSession();
    SavePlayerOfflineData(conn);
    //将下线消息通知给其他可是范围内的玩家和队员工会等

    if(reg->type == 1)  //断开链接
    {
        DeleteNameSock(conn);
        SessionMgr::Instance()->RemoveSession(conn);
    }
    else if(reg->type  == 2) //返回角色列表
    {
        Session s;
        (*smap)[conn].m_usrid.swap(s.m_usrid);

        (*smap)[conn] = s;
        hf_char* userID = (hf_char*)&(*smap)[conn].m_usrid;
        SendRoleList(conn, userID);
    }
    srv->free(reg);
}


//发送角色列表
void PlayerLogin::SendRoleList(TCPConnection::Pointer conn, hf_char* userID)
{
    Server *srv = Server::GetInstance();
    StringBuilder sbd;

    hf_char namebuff[36] = { 0 };
    memcpy(namebuff, userID, 32);
    sbd<<"select nick,roleid,profession,level,sex,figure,figurecolor,face,eye,hair,haircolor,modeid,skirtid from t_playerrolelist where username='"<<namebuff<<"' and ifdelete = 0;";
    Logger::GetLogger()->Debug(sbd.str());

    //查询角色列表
    ResRoleList t_Rolelist;
    hf_int32 t_row = srv->getDiskDB()->GetPlayerRoleList(&t_Rolelist, sbd.str());
    if(t_row > 0)
    {
        hf_char* buff = (hf_char*)srv->malloc();
        hf_int32 roleCount = t_Rolelist.m_Role.size() ;
        hf_uint32 j = 0;
        STR_PackHead t_packHead;
        t_packHead.Flag = FLAG_PlayerRoleList;
        for(hf_int32 i = 0; i < roleCount; i++)
        {
            memcpy(buff + sizeof(STR_PackHead) +  i*sizeof(STR_RoleBasicInfo),&(t_Rolelist.m_Role[i]),sizeof(STR_RoleBasicInfo));
            j++;
            if(j == (CHUNK_SIZE - sizeof(STR_PackHead))/sizeof(STR_RoleBasicInfo))
            {
                t_packHead.Len = sizeof(STR_RoleBasicInfo) * j;
                memcpy(buff, &t_packHead, sizeof(STR_PackHead));
                conn->Write_all(buff, sizeof(STR_PackHead) + t_packHead.Len);
                memset(buff, 0, CHUNK_SIZE);
                j = 0;
            }
        }
        if(j != (CHUNK_SIZE - sizeof(STR_PackHead))/sizeof(STR_RoleBasicInfo))
        {
            t_packHead.Len = sizeof(STR_RoleBasicInfo) * j;
            memcpy(buff, &t_packHead, sizeof(STR_PackHead));
            conn->Write_all(buff, sizeof(STR_PackHead) + t_packHead.Len);
        }
        srv->free(buff);
    }
}


//发送好友列表
void PlayerLogin::SendFriendList(TCPConnection::Pointer conn, hf_uint32 RoleID)
{
    StringBuilder sbd;
    Server* srv = Server::GetInstance();
    SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();
    umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
    sbd << "select friendroleid,nick from t_friendlist,t_playerrolelist where t_friendlist.roleid = " << RoleID << " and t_friendlist.roleid = t_playerrolelist.roleid and ifdelete = 0;";
    Logger::GetLogger()->Debug(sbd.str());
    umap_friendList t_friendList = ((*smap)[conn]).m_friendList;
    hf_uint32 t_row = srv->getDiskDB()->GetFriendList(t_friendList, sbd.str());
    if(t_row > 0) //有好友列表
    {
        hf_char* buff = (hf_char*)srv->malloc();
        hf_int32 i = 0;
        STR_PackFriendOnLine t_friendOnLine;
        t_friendOnLine.Role = RoleID;
        for(_umap_friendList::iterator it = t_friendList->begin(); it != t_friendList->end(); it++)
        {
            _umap_roleSock::iterator iter = t_roleSock->find(it->first); //查询好友是否在线
            if(iter != t_roleSock->end())
            {
                it->second.Status = ONLINE;
                //给其他在线的好友发送该玩家上线通知
                iter->second->Write_all(&t_friendOnLine, sizeof(STR_PackFriendOnLine));
            }
            memcpy(buff + sizeof(STR_PackHead) + i*sizeof(STR_FriendInfo), &(it->second), sizeof(STR_FriendInfo));
            i++;
        }
        STR_PackHead t_packHead;
        t_packHead.Flag = FLAG_FriendList;
        t_packHead.Len = i * sizeof(STR_FriendInfo);
        memcpy(buff, &t_packHead, sizeof(STR_PackHead));
        conn->Write_all(buff, t_packHead.Len + sizeof(STR_PackHead));
        srv->free(buff);
    }
}

//发送角色金币
void PlayerLogin::SendRoleMoney(TCPConnection::Pointer conn, hf_uint32 RoleID)
{
    StringBuilder sbd;
    STR_PackHead t_packHead;
    Server* srv = Server::GetInstance();
    SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();
    sbd << "select count,typeid from t_playermoney where roleid = " << RoleID << ";";
    Logger::GetLogger()->Debug(sbd.str());
    umap_roleMoney  t_playerMoney = (*smap)[conn].m_playerMoney;
    hf_uint32 t_row = srv->getDiskDB()->GetPlayerMoney(t_playerMoney, sbd.str());
    if(t_row > 0)
    {
        hf_int32 count = t_playerMoney->size();
        hf_char* buff = (hf_char*)srv->malloc();

        memset(&t_packHead, 0, sizeof(STR_PackHead));
        t_packHead.Flag = FLAG_PlayerMoney;
        t_packHead.Len = count * sizeof(STR_PlayerMoney);
        memcpy(buff, &t_packHead, sizeof(STR_PackHead));
        hf_int32 i = 0;
        for(_umap_roleMoney::iterator it = t_playerMoney->begin(); it != t_playerMoney->end(); it++)
        {
            memcpy(buff + sizeof(STR_PackHead) + i*sizeof(STR_PlayerMoney), &(it->second), sizeof(STR_PlayerMoney));
            i++;
        }
        conn->Write_all(buff, t_packHead.Len + sizeof(STR_PackHead));
        srv->free(buff);
    }
}


//发送角色背包里的物品
void PlayerLogin::SendRoleGoods(TCPConnection::Pointer conn, hf_uint32 RoleID)
{
    StringBuilder sbd;
    STR_PackHead t_packHead;
    Server* srv = Server::GetInstance();
    SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();
    sbd << "select goodsid,typeid,count,position from t_playergoods where roleid = " << RoleID << ";";
    Logger::GetLogger()->Debug(sbd.str());

    umap_roleGoods  playerGoods = (*smap)[conn].m_playerGoods;
    umap_roleEqu    playerEqu = (*smap)[conn].m_playerEqu;
    hf_uint32 t_row = srv->getDiskDB()->GetPlayerGoods(playerGoods, playerEqu, sbd.str());
    if(t_row > 0)
    {
        hf_char* buff = (hf_char*)srv->malloc();

        hf_int32 i = 0;
        for(_umap_roleEqu::iterator it = playerEqu->begin(); it != playerEqu->end(); it++)
        {
            memcpy(buff + sizeof(STR_PackHead) + i*sizeof(STR_Goods), &(it->second), sizeof(STR_Goods));
            OperationGoods::UsePos(conn, it->second.goods.Position);
            i++;
        }
        for(_umap_roleGoods::iterator it = playerGoods->begin(); it != playerGoods->end(); it++)
        {
            for(vector<STR_Goods>::iterator iter = it->second.begin(); iter != it->second.end(); iter++)
            {
                memcpy(buff + sizeof(STR_PackHead) + i*sizeof(STR_Goods), &(*iter), sizeof(STR_Goods));
                 OperationGoods::UsePos(conn, iter->Position);
                i++;
            }
        }
        t_packHead.Flag = FLAG_BagGoods;
        t_packHead.Len = i * sizeof(STR_Goods);
        memcpy(buff, &t_packHead, sizeof(STR_PackHead));
        conn->Write_all(buff, t_packHead.Len + sizeof(STR_PackHead));
        srv->free(buff);
    }
}


//发送角色背包里装备的属性
void PlayerLogin::SendRoleEquAttr(TCPConnection::Pointer conn, hf_uint32 RoleID)
{      
    SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();
    umap_roleEqu  playerEqu = (*smap)[conn].m_playerEqu;
    if(playerEqu->size() == 0)
    {
        return;
    }

    StringBuilder sbd;
    sbd << "select equid,typeid,physicalattack,physicaldefense,magicattack,magicdefense,addhp,addmagic,durability from t_playerequ where roleid = " << RoleID << ";";
    Logger::GetLogger()->Debug(sbd.str());

    STR_PackHead t_packHead;
    hf_uint32 t_row = Server::GetInstance()->getDiskDB()->GetPlayerEqu(playerEqu, sbd.str());
    if(t_row > 0)
    {
        hf_char* buff = (hf_char*)Server::GetInstance()->malloc();
        memset(&t_packHead, 0, sizeof(STR_PackHead));
        t_packHead.Flag = FLAG_EquGoodsAttr;
        t_packHead.Len = playerEqu->size() * sizeof(STR_Equipment);
        memcpy(buff, &t_packHead, sizeof(STR_PackHead));
        hf_int32 i = 0;
        for(_umap_roleEqu::iterator it = playerEqu->begin(); it != playerEqu->end(); it++)
        {
            memcpy(buff + sizeof(STR_PackHead) + i*sizeof(STR_Equipment), &(it->second.equAttr), sizeof(STR_Equipment));
            i++;
        }
        conn->Write_all(buff, t_packHead.Len + sizeof(STR_PackHead));
        Server::GetInstance()->free(buff);
    }
}

//查询玩家未捡取的物品
void PlayerLogin::SendRoleNotPickGoods(TCPConnection::Pointer conn, hf_uint32 RoleID)
{
    StringBuilder sbd;
    STR_PackHead t_packHead;
    t_packHead.Flag = FLAG_LootGoods;
    Server* srv = Server::GetInstance();
    SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();
    sbd << "select continuetime,lootid,pos_x,pos_y,pos_z,mapid from t_notpickgoodspos where roleid = " << RoleID << ";";
    Logger::GetLogger()->Debug(sbd.str());

    umap_lootGoods lootGoods = (*smap)[conn].m_lootGoods;
    umap_lootPosition lootPosition = (*smap)[conn].m_lootPosition;
    hf_int32 t_row = srv->getDiskDB()->GetNotPickGoodsPosition(lootPosition, sbd.str());
    if(t_row <= 0)
    {
        return;
    }

    sbd.Clear();
    sbd << "select lootid,goodsid,count from t_notpickgoods where roleid = " << RoleID << ";";
    Logger::GetLogger()->Debug(sbd.str());
    t_row = srv->getDiskDB()->GetNotPickGoods(lootGoods, sbd.str());
    if(t_row <= 0)
    {
        return;
    }

    hf_char* buff = (hf_char*)srv->malloc();
    for(_umap_lootPosition::iterator it = lootPosition->begin(); it != lootPosition->end(); it++)
    {
        memcpy(buff + sizeof(STR_PackHead), &(it->second.goodsPos), sizeof(STR_LootGoodsPos));
        _umap_lootGoods::iterator iter = lootGoods->find(it->first);
        if(iter == lootGoods->end())
        {
            return;
        }
        hf_uint32 i = 0;
        for(vector<STR_LootGoods>::iterator vec = iter->second.begin(); vec != iter->second.end(); vec++)
        {
            memcpy(buff + sizeof(STR_PackHead) + sizeof(STR_LootGoodsPos) + i*sizeof(STR_LootGoods), &(*vec), sizeof(STR_LootGoods));
            i++;
        }
        t_packHead.Len = sizeof(STR_LootGoodsPos) + sizeof(STR_LootGoods)*i;
        memcpy(buff, &t_packHead, sizeof(STR_PackHead));
        conn->Write_all(buff, t_packHead.Len + sizeof(STR_PackHead));
    }
    srv->free(buff);
}

//将玩家背包里的物品写进数据库
void PlayerLogin::SaveRoleBagGoods(TCPConnection::Pointer conn)
{
    SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();
    umap_roleGoods t_roleGoods = (*smap)[conn].m_playerGoods;
    hf_uint32 roleid = (*smap)[conn].m_roleid;

    StringBuilder sbd;
    sbd << "delete from t_playergoods where roleid = " << roleid << ";";
     Logger::GetLogger()->Debug(sbd.str());
    if(Server::GetInstance()->getDiskDB()->Set(sbd.str()) == -1)
    {
        Logger::GetLogger()->Error("清除玩家背包物品信息失败");
    }

    sbd.Clear();
    sbd << "insert into t_playergoods values(";
    hf_uint16 count = t_roleGoods->size();
    if(count == 0)
    {
        return;
    }
    hf_uint16 i = 0;
    for(_umap_roleGoods::iterator it = t_roleGoods->begin(); it != t_roleGoods->end(); it++)
    {
        hf_uint32 vec_count = it->second.size();
        hf_uint32 j = 0;
        for(vector<STR_Goods>::iterator iter = it->second.begin(); iter != it->second.end(); iter++)
        {
            sbd << roleid << "," << (*iter).GoodsID << "," << (*iter).TypeID << "," << (*iter).Count << "," << (*iter).Position << ")";
            if(vec_count != j+1)
            {
                sbd << ",(";
                j++;
            }
        }
        if(count == i+1)
        {
            sbd << ";";
        }
        else
        {
            sbd << ",(";
            i++;
        }
    }
    Logger::GetLogger()->Debug(sbd.str());
    if(Server::GetInstance()->getDiskDB()->Set(sbd.str()) == -1)
    {
        Logger::GetLogger()->Error("玩家背包物品写入数据库失败");
    }
}

//将玩家装备属性写进数据库
void PlayerLogin::SaveRoleEquAttr(TCPConnection::Pointer conn)
{
    SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();
    umap_roleEqu roleEqu = (*smap)[conn].m_playerEqu;
    StringBuilder sbd;
    hf_uint32 roleid = (*smap)[conn].m_roleid;

    sbd << "delete from t_playerequ where roleid = " << roleid << ";";

     Logger::GetLogger()->Debug(sbd.str());
    if(Server::GetInstance()->getDiskDB()->Set(sbd.str()) == -1)
    {
        Logger::GetLogger()->Error("清除玩家装备属性信息失败");
    }

    sbd.Clear();
    sbd << "insert into t_playerequ values(";
    hf_uint32 count = roleEqu->size();
    if(count == 0)
    {
        return;
    }
    hf_uint32 i = 0;
    for(_umap_roleEqu::iterator it = roleEqu->begin(); it != roleEqu->end(); it++)
    {
        sbd << roleid << "," << it->second.equAttr.EquID << "," << it->second.equAttr.TypeID << "," << it->second.equAttr.PhysicalAttack << "," << it->second.equAttr.PhysicalDefense << "," << it->second.equAttr.MagicAttack << "," << it->second.equAttr.MagicDefense << "," << it->second.equAttr.AddHp << "," << it->second.equAttr.AddMagic << "," << it->second.equAttr.Durability << ")";
        if(count == i+1)
        {
            sbd << ";";
        }
        else
        {
            sbd << ",(";
            i++;
        }
    }
    Logger::GetLogger()->Debug(sbd.str());
    if(Server::GetInstance()->getDiskDB()->Set(sbd.str()) == -1)
    {
        Logger::GetLogger()->Error("玩家装备属性写入数据库失败");
    }
}

//将玩家金钱写进数据库
void PlayerLogin::SaveRoleMoney(TCPConnection::Pointer conn)
{
    SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();
    umap_roleMoney t_roleMoney = (*smap)[conn].m_playerMoney;
    hf_uint32 roleid = (*smap)[conn].m_roleid;

    StringBuilder sbd;
    sbd << "delete from t_playermoney where roleid = " << roleid << ";";

     Logger::GetLogger()->Debug(sbd.str());
    if(Server::GetInstance()->getDiskDB()->Set(sbd.str()) == -1)
    {
        Logger::GetLogger()->Error("清除玩家金钱信息失败");
    }

    sbd.Clear();
    sbd << "insert into t_playermoney values(";
    hf_uint32 count = t_roleMoney->size();
    if(count == 0)
    {
        return;
    }
    hf_uint32 i = 0;
    for(_umap_roleMoney::iterator it = t_roleMoney->begin(); it != t_roleMoney->end(); it++)
    {
        sbd << roleid << "," << it->second.Count << "," << it->second.TypeID << ")";
        if(count == i+1)
        {
            sbd << ";";
        }
        else
        {
            sbd << ",(";
            i++;
        }
    }
    Logger::GetLogger()->Debug(sbd.str());
    if(Server::GetInstance()->getDiskDB()->Set(sbd.str()) == -1)
    {
        Logger::GetLogger()->Error("玩家金币写入数据库失败");
    }
}


//将玩家任务进度写进数据库
void PlayerLogin::SaveRoleTaskProcess(TCPConnection::Pointer conn)
{
    SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();
    umap_taskProcess playerAcceptTask = (*smap)[conn].m_playerAcceptTask;
    hf_uint32 roleid = (*smap)[conn].m_roleid;

    StringBuilder sbd;
    sbd << "delete from t_playertaskprocess where roleid = " << roleid << ";";

     Logger::GetLogger()->Debug(sbd.str());
    if(Server::GetInstance()->getDiskDB()->Set(sbd.str()) == -1)
    {
        Logger::GetLogger()->Error("清除玩家任务进度信息失败");
    }

    sbd.Clear();
    sbd << "insert into t_playertaskprocess values(";
    hf_uint32 count = playerAcceptTask->size();
    if(count == 0)
    {
        return;
    }
    hf_uint32 i = 0;
    for(_umap_taskProcess::iterator it = playerAcceptTask->begin(); it != playerAcceptTask->end(); it++)
    {
        for(vector<STR_TaskProcess>::iterator iter = it->second.begin(); iter != it->second.end(); iter++)
        {
            sbd << roleid << "," << iter->TaskID << "," << iter->AimID << "," << iter->FinishCount << "," << iter->AimAmount << "," << iter->ExeModeID << ")";
            if(iter != it->second.end())
            {
                sbd << ",(";
            }
        }
        if(count == (i+1))
        {
            sbd << ";";
        }
        else
        {
            sbd << ",(";
            i++;
        }
    }
    Logger::GetLogger()->Debug(sbd.str());
    if(Server::GetInstance()->getDiskDB()->Set(sbd.str()) == -1)
    {
        Logger::GetLogger()->Error("更新玩家任务进度信息失败");
    }
}


//将离开可视范围的消息发送给可视范围内的玩家
void PlayerLogin::SendOffLineToViewRole(TCPConnection::Pointer conn)
{
    SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();
    hf_uint32 roleid = (*smap)[conn].m_roleid;

    STR_PackRoleLeave RoleLeave;
    RoleLeave.Role = roleid;

    for(_umap_roleSock::iterator it = (*smap)[conn].m_viewRole->begin(); it != (*smap)[conn].m_viewRole->end();)
    {
        it->second->Write_all(&RoleLeave, sizeof(STR_PackRoleLeave));
        (*smap)[it->second].m_viewRole->erase(roleid);
         _umap_roleSock::iterator iter = it;
         it++;
        (*smap)[conn].m_viewRole->erase((*smap)[iter->second].m_roleid);
         Logger::GetLogger()->Debug("删除OK");
    }
}



//保存玩家未捡取的物品
void PlayerLogin::SaveRoleNotPickGoods(TCPConnection::Pointer conn)
{
    time_t timep;
    time(&timep);
    SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();
    umap_lootPosition      t_lootPosition = (*smap)[conn].m_lootPosition;
    hf_uint32 roleid = (*smap)[conn].m_roleid;

    StringBuilder sbd;
    sbd << "delete from t_notpickgoods where roleid = " << roleid << ";";
     Logger::GetLogger()->Debug(sbd.str());
    if(Server::GetInstance()->getDiskDB()->Set(sbd.str()) == -1)
    {
        Logger::GetLogger()->Error("清除玩家未捡取物品信息失败");
        return;
    }

    sbd.Clear();
    sbd << "insert into t_notpickgoods values(";
    umap_lootGoods  t_lootGoods = (*smap)[conn].m_lootGoods;
    hf_uint16 i = 0;
    hf_uint16 count = t_lootGoods->size();
    if(count == 0)
    {
        return;
    }
    for(_umap_lootGoods::iterator it = t_lootGoods->begin(); it != t_lootGoods->end(); it++)
    {
        hf_uint16 vec_count = it->second.size();
        hf_uint16 j = 0;
        for(vector<STR_LootGoods>::iterator iter = it->second.begin(); iter != it->second.end(); iter++)
        {
            sbd << roleid << "," << it->first << "," << iter->LootGoodsID << "," << iter->Count <<  ")";
            if(vec_count != j+1)
            {
                sbd << ",(";
                j++;
            }
        }
        if(count == i+1)
        {
            sbd << ";";
        }
        else
        {
            sbd << ",(";
            i++;
        }
    }
    Logger::GetLogger()->Debug(sbd.str());
    if(Server::GetInstance()->getDiskDB()->Set(sbd.str()) == -1)
    {
        Logger::GetLogger()->Error("更新玩家未捡取物品信息失败");
    }


    sbd.Clear();
    sbd << "delete from t_notpickgoodspos where roleid = " << roleid << ";";
    Logger::GetLogger()->Debug(sbd.str());
   if(Server::GetInstance()->getDiskDB()->Set(sbd.str()) == -1)
   {
       Logger::GetLogger()->Error("清除玩家未捡取物品位置信息失败");
   }
    count = t_lootPosition->size();
    if(count == 0)
    {
        return;
    }
    i = 0;
    sbd.Clear();
    sbd << "insert into t_notpickgoodspos values(";
    for(_umap_lootPosition::iterator it = t_lootPosition->begin(); it != t_lootPosition->end(); it++)
    {
        sbd << roleid << "," << it->second.timep + it->second.continueTime - (hf_uint32)timep << "," << it->second.goodsPos.GoodsFlag << "," << it->second.goodsPos.Pos_x << "," << it->second.goodsPos.Pos_y << "," << it->second.goodsPos.Pos_z << "," << it->second.goodsPos.MapID << ")";
        if(count == (i+1))
        {
            sbd << ";";
        }
        else
        {
            sbd << ",(";
            i++;
        }
    }
    Logger::GetLogger()->Debug(sbd.str());
    if(Server::GetInstance()->getDiskDB()->Set(sbd.str()) == -1)
    {
        Logger::GetLogger()->Error("更新玩家未捡取物品位置信息失败");
    }    
}

//更新玩家金钱
void PlayerLogin::UpdatePlayerMoney(UpdateMoney* upMoney)
{
    StringBuilder sbd;
    sbd << "update t_playermoney set typeid = " << upMoney->Money.TypeID << ",count = " << upMoney->Money.Count << " where roleid = " << upMoney->RoleID << ";";
     Logger::GetLogger()->Debug(sbd.str());
    if(Server::GetInstance()->getDiskDB()->Set(sbd.str()) == -1)
    {
        Logger::GetLogger()->Error("更新玩家金钱信息失败");
    }
}

//更新玩家等级
void PlayerLogin::UpdatePlayerLevel(UpdateLevel* upLevel)
{
    StringBuilder sbd;
    sbd << "update t_playerrolelist set level = " << upLevel->Level << " where roleid = " << upLevel->RoleID << ";";
     Logger::GetLogger()->Debug(sbd.str());
    if(Server::GetInstance()->getDiskDB()->Set(sbd.str()) == -1)
    {
        Logger::GetLogger()->Error("更新玩家等级信息失败");
    }
}

//更新玩家经验
void PlayerLogin::UpdatePlayerExp(UpdateExp* upExp)
{
    StringBuilder sbd;
    sbd << "update t_playerrolelist set experience = " << upExp->Exp << " where roleid = " << upExp->RoleID << ";";
     Logger::GetLogger()->Debug(sbd.str());
    if(Server::GetInstance()->getDiskDB()->Set(sbd.str()) == -1)
    {
        Logger::GetLogger()->Error("更新玩家经验信息失败");
    }
}

//更新玩家物品
void PlayerLogin::UpdatePlayerGoods(hf_uint32 roleid, STR_Goods* upGoods)
{
    StringBuilder sbd;
    sbd << "update t_playergoods set count = " << upGoods->Count << " where roleid = " << roleid << " and position = " << upGoods->Position << ";";
     Logger::GetLogger()->Debug(sbd.str());
     hf_int32 t_value = Server::GetInstance()->getDiskDB()->Set(sbd.str());
    if(t_value == -1)
    {
        Logger::GetLogger()->Error("更新玩家背包物品信息失败");
    }
//    else if(t_value == 0)
//    {
//        PlayerLogin::InsertPlayerGoods(roleid, upGoods);
//    }
}

//新物品更新背包
void PlayerLogin::InsertPlayerGoods(hf_uint32 roleid, STR_Goods* insGoods)
{
    StringBuilder sbd;
    sbd << "insert into t_playergoods values(" << roleid << "," << insGoods->GoodsID << "," << insGoods->TypeID << "," << insGoods->Count << "," << insGoods->Position << ");";
    Logger::GetLogger()->Debug(sbd.str());
    if(Server::GetInstance()->getDiskDB()->Set(sbd.str()) == -1)
    {
        Logger::GetLogger()->Error("更新玩家背包物品信息失败");
    }
}

//玩家背包删除物品
void PlayerLogin::DeletePlayerGoods(hf_uint32 roleid, hf_uint16 pos)
{
    StringBuilder sbd;
    sbd << "delete from t_playergoods where roleid = " << roleid << " and position = " << pos << ";";
     Logger::GetLogger()->Debug(sbd.str());
    if(Server::GetInstance()->getDiskDB()->Set(sbd.str()) == -1)
    {
        Logger::GetLogger()->Error("玩家背包删除物品失败");
    }
}

//更新玩家装备属性
void PlayerLogin::UpdatePlayerEquAttr(hf_uint32 roleid, STR_Equipment* upEquAttr)
{
    StringBuilder sbd;
    sbd << "update t_playerequ set roleid = " << roleid << ",typeid = " << upEquAttr->TypeID << ",physicalattack = " << upEquAttr->PhysicalAttack << ",physicaldefense = " << upEquAttr->PhysicalDefense << ",magicattack = " << upEquAttr->MagicAttack << ",magicdefense = " << upEquAttr->MagicDefense << ",addhp = " << upEquAttr->AddHp << ",addmagic = " << upEquAttr->AddMagic << ",durability = " << upEquAttr->Durability << " where equid = " << upEquAttr->EquID << ";";
     Logger::GetLogger()->Debug(sbd.str());
     hf_int32 t_value = Server::GetInstance()->getDiskDB()->Set(sbd.str());
    if(t_value == -1)
    {
        Logger::GetLogger()->Error("更新玩家装备属性失败");
    }
//    else if(t_value == 0)
//    {
//       PlayerLogin::InsertPlayerEquAttr(roleid, upEquAttr);
//    }
}

//新装备更新属性
void PlayerLogin::InsertPlayerEquAttr(hf_uint32 roleid, STR_Equipment* insEquAttr)
{
    StringBuilder sbd;
    sbd << "insert into t_playerequ values(" << roleid << "," << insEquAttr->EquID << "," << insEquAttr->TypeID << "," << insEquAttr->PhysicalAttack << "," << insEquAttr->PhysicalDefense << "," << insEquAttr->MagicAttack << "," << insEquAttr->MagicDefense << "," << insEquAttr->AddHp << "," << insEquAttr->AddMagic << "," << insEquAttr->Durability << ");";
    Logger::GetLogger()->Debug(sbd.str());
    if(Server::GetInstance()->getDiskDB()->Set(sbd.str()) == -1)
    {
        Logger::GetLogger()->Error("写入玩家装备属性失败");
    }
}

//删除玩家背包装备属性
void PlayerLogin::DeletePlayerEquAttr(hf_uint32 roleid, hf_uint32 equid)
{
    StringBuilder sbd;
    sbd << "delete from t_playerequ where roleid = " << roleid << " and equid = " << equid << ";";
     Logger::GetLogger()->Debug(sbd.str());
    if(Server::GetInstance()->getDiskDB()->Set(sbd.str()) == -1)
    {
        Logger::GetLogger()->Error("删除玩家背包装备属性失败");
    }
}

//更新玩家任务进度
void PlayerLogin::UpdatePlayerTask(hf_uint32 roleid, STR_TaskProcess* upTask)
{
    StringBuilder sbd;
    sbd << "update t_playertaskprocess set taskid = " << upTask->TaskID << ",aimid = " << upTask->AimID << ",aimcount = " << upTask->FinishCount << ",aimamount = " << upTask->AimAmount << ",exemodeid = " << upTask->ExeModeID << " where roleid = " << roleid << ";";
     Logger::GetLogger()->Debug(sbd.str());
     hf_int32 t_value = Server::GetInstance()->getDiskDB()->Set(sbd.str());
    if(t_value == -1)
    {
        Logger::GetLogger()->Error("更新玩家任务进度失败");
    }
//    else if(t_value == 0)
//    {
//        InsertPlayerTask(roleid, upTask);
//    }
}

//插入新任务
void PlayerLogin::InsertPlayerTask(hf_uint32 roleid, STR_TaskProcess* insTask)
{
    StringBuilder sbd;
    sbd << "insert into t_playertaskprocess values(" << roleid << "," << insTask->TaskID << "," << insTask->AimID << "," << insTask->FinishCount << "," << insTask->AimAmount << "," << insTask->ExeModeID << ");";
    Logger::GetLogger()->Debug(sbd.str());
    if(Server::GetInstance()->getDiskDB()->Set(sbd.str()) == -1)
    {
        Logger::GetLogger()->Error("插入新任务失败");
    }
}

//删除任务
void PlayerLogin::DeletePlayerTask(hf_uint32 roleid, hf_uint32 taskid)
{
    StringBuilder sbd;
    sbd << "delete from t_playertaskprocess where roleid = " << roleid << " and taskid = " << taskid << ";";
     Logger::GetLogger()->Debug(sbd.str());
    if(Server::GetInstance()->getDiskDB()->Set(sbd.str()) == -1)
    {
        Logger::GetLogger()->Error("删除玩家任务失败");
    }
}

//刷新可视范围内的玩家
void PlayerLogin::SendViewRole(TCPConnection::Pointer conn)
{
    SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();
    umap_roleSock viewRole = (*smap)[conn].m_viewRole;
    STR_PackPlayerPosition* pos = &(*smap)[conn].m_position;
    hf_uint32 roleid = (*smap)[conn].m_roleid;
    Server* srv = Server::GetInstance();
    hf_char* comebuff = (hf_char*)srv->malloc();
    hf_char* leavebuff = (hf_char*)srv->malloc();
    hf_uint16 pushCount = 0;
    hf_uint16 popCount = 0;
    STR_PackHead t_packHead;

    hf_uint16 len = sizeof(STR_RoleBasicInfo) + sizeof(STR_OtherPlayerPosition);

    STR_PackRoleCome t_roleCome;
    memcpy(&t_roleCome.roleBasicInfo, &((*smap)[conn].m_RoleBaseInfo), sizeof(STR_RoleBasicInfo));
    memcpy(&t_roleCome.otherRolePos, (hf_char*)pos + sizeof(STR_PackHead), sizeof(STR_OtherPlayerPosition));

    STR_PackRoleLeave t_leaveHead;
    t_leaveHead.Role = roleid;
    for(SessionMgr::SessionMap::iterator it = smap->begin(); it != smap->end(); it++)
    {
        if ( it->first == conn)
            continue;
        if(caculateDistanceWithRole(pos, &(it->second.m_position)) == View)
        {
            _umap_roleSock::iterator iter = viewRole->find(it->second.m_roleid);
            if(iter != viewRole->end()) //原先就在范围内
            {
                continue;
            }
            memcpy(comebuff + sizeof(STR_PackHead) + pushCount*len, &(it->second.m_RoleBaseInfo), sizeof(STR_RoleBasicInfo));
            memcpy(comebuff + sizeof(STR_PackHead) + pushCount*len + sizeof(STR_RoleBasicInfo), ((hf_char*)&(it->second.m_position)) + sizeof(STR_PackHead), sizeof(STR_OtherPlayerPosition));
            pushCount++;


            (*viewRole)[it->second.m_roleid] = it->first;
            (*(it->second.m_viewRole))[roleid] = conn;
            it->first->Write_all(&t_roleCome, sizeof(STR_PackRoleCome));

            if(pushCount == (CHUNK_SIZE - sizeof(STR_PackHead))/(sizeof(STR_RoleBasicInfo) + sizeof(STR_OtherPlayerPosition)))
            {
                t_packHead.Flag = FLAG_ViewRoleCome;
                t_packHead.Len = (sizeof(STR_RoleBasicInfo) + sizeof(STR_OtherPlayerPosition)) * pushCount;
                memcpy(comebuff, &t_packHead, sizeof(STR_PackHead));

                //发送新看到的玩家
                conn->Write_all(comebuff, sizeof(STR_PackHead) + t_packHead.Len);
                pushCount = 0;
            }
        }
        else
        {
            _umap_roleSock::iterator iter = viewRole->find(it->second.m_roleid);
            if(iter != viewRole->end())  //离开范围
            {
                cout << it->second.m_roleid << "likaishiyefanwei" << endl;
                memcpy(leavebuff + sizeof(STR_PackHead) + popCount*4, &(it->second.m_roleid), 4);
                popCount++;

                it->first->Write_all(&t_leaveHead, sizeof(STR_PackRoleLeave));

                (*smap)[iter->second].m_viewRole->erase(roleid);
                (*smap)[conn].m_viewRole->erase((*smap)[iter->second].m_roleid);
            }
        }
    }
    if(pushCount != (CHUNK_SIZE - sizeof(STR_PackHead))/(sizeof(STR_RoleBasicInfo) + sizeof(STR_OtherPlayerPosition)) && pushCount != 0)
    {
        t_packHead.Flag = FLAG_ViewRoleCome;
        t_packHead.Len = (sizeof(STR_RoleBasicInfo) + sizeof(STR_OtherPlayerPosition)) * pushCount;
        memcpy(comebuff, &t_packHead, sizeof(STR_PackHead));

        //发送新看到的玩家
        conn->Write_all(comebuff, sizeof(STR_PackHead) + t_packHead.Len);
    }
    if(popCount != 0)
    {
        t_packHead.Flag = FLAG_ViewRoleLeave;
        t_packHead.Len = 4 * popCount;
        memcpy(leavebuff, &t_packHead, sizeof(STR_PackHead));
        //发送离开可视范围内的玩家
        conn->Write_all(leavebuff, sizeof(STR_PackHead) + t_packHead.Len);
    }

    srv->free(comebuff);
    srv->free(leavebuff);
}

//判断两个玩家能否看到
hf_uint8 PlayerLogin::caculateDistanceWithRole(STR_PackPlayerPosition* pos1, STR_PackPlayerPosition* pos2)
{
    hf_float dx = pos1->Pos_x - pos2->Pos_x,
             dy = pos1->Pos_y - pos2->Pos_y,
             dz = pos1->Pos_z - pos2->Pos_z;
    if(dx*dx + dy*dy + dz*dz <= PlayerView*PlayerView)
        return View;
    else
        return NotView;
}

//好友下线
void PlayerLogin::FriendOffline(TCPConnection::Pointer conn)
{
    SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();
    umap_roleSock roleSock = SessionMgr::Instance()->GetRoleSock();

    umap_friendList friendList = ((*smap)[conn]).m_friendList;
    hf_uint32 roleid = (*smap)[conn].m_roleid;

    STR_PackHead t_packHead;
    t_packHead.Flag = FLAG_FriendOffLine;
    t_packHead.Len = sizeof(roleid);
    hf_char* buff = (hf_char*)Server::GetInstance()->malloc();
    memcpy(buff, &t_packHead, sizeof(STR_PackHead));
    memcpy(buff + sizeof(STR_PackHead), &roleid, sizeof(roleid));
    for(_umap_friendList::iterator it = friendList->begin(); it != friendList->end(); it++)
    {
        _umap_roleSock::iterator t_sock = roleSock->find(it->first);
        if(t_sock != roleSock->end())
        {
            t_sock->second->Write_all(buff, sizeof(STR_PackHead) + t_packHead.Len);
            (*(*smap)[t_sock->second].m_friendList)[roleid].Status = OFFLINE;
        }
    }
    Server::GetInstance()->free(buff);
}
