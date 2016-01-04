#include "./../Game/userposition.hpp"
#include "./../OperationGoods/operationgoods.h"
#include "./../server.h"

#include "playerlogin.h"

PlayerLogin::PlayerLogin()
{
    m_common = new STR_RoleJobAttribute[CommonGradeCount + 1];
    m_sales = new STR_RoleJobAttribute[CommonGradeCount + 1];
    m_technology = new STR_RoleJobAttribute[CommonGradeCount + 1];
    m_administration = new STR_RoleJobAttribute[CommonGradeCount + 1];
}

PlayerLogin::~PlayerLogin()
{
    delete m_common;
    delete m_sales;
    delete m_technology;
    delete m_administration;
}

void PlayerLogin::ReturnRoleListSaveData(TCPConnection::Pointer conn)
{
    conn->LockLoginStatus();
    conn->ChangePlayerLoginStatus(PlayerNotLoginUser);
    conn->UnlockLoginStatus();
    Server *srv = Server::GetInstance();
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    hf_int32 roleid = ((*smap)[conn].m_roleid);

    //将玩家当前数据写进数据库,(位置，任务进度等)
    STR_PackPlayerPosition* playerPosition =   &((*smap)[conn].m_position);
    StringBuilder sbd;
    sbd << "update t_playerposition set pos_x=" << playerPosition->Pos_x << ",pos_y=" << playerPosition->Pos_y << ",pos_z=" << playerPosition->Pos_z << ",direct=" << playerPosition->Direct <<",mapid=" << playerPosition->MapID << ",actid=" << playerPosition->ActID << " where roleid=" << roleid << ";";

    Logger::GetLogger()->Debug(sbd.str());
    if(srv->getDiskDB()->Set(sbd.str()) == -1)
    {
        Logger::GetLogger()->Error("update player position error");
    }

    SaveRoleInfo(conn);           //更新角色属性
    DeleteFromMonsterView(conn);  //从怪物可视范围内删除该玩家
    SaveRoleEquDurability(conn);  //将玩家装备当前耐久度更新到数据库
    FriendOffline(conn);          //发送下线通知给好友
    SaveRoleNotPickGoods(conn);   //保存玩家未捡取的掉落物品
    SendOffLineToViewRole(conn);  //将下线消息通知给可视范围内的玩家
    SessionMgr::Instance()->NickSockErase((*smap)[conn].m_RoleBaseInfo.Nick);
    SessionMgr::Instance()->RoleSockErase(roleid);
}

//保存玩家角色退出数据，并发送下线通知给其他玩家
void PlayerLogin::SavePlayerOfflineData(TCPConnection::Pointer conn)
{
    Server *srv = Server::GetInstance();
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    conn->LockLoginStatus();
    hf_uint8 t_loginState = conn->GetPlayerLoginStatus();
    if(t_loginState == PlayerNotLoginUser)
    {
        Logger::GetLogger()->Debug("not login user exit");
        return;
    }
    else if(t_loginState == PlayerLoginUser)
    {
        SessionMgr::Instance()->NameSockErase(&(*smap)[conn].m_usrid[0]);
        SessionMgr::Instance()->SessionsErase(conn);
        Logger::GetLogger()->Debug("not login role exit");
        return;
    }
    conn->ChangePlayerLoginStatus(PlayerNotLoginUser);
    conn->UnlockLoginStatus();
    hf_int32 roleid = ((*smap)[conn].m_roleid);

    //将玩家当前数据写进数据库,(位置，任务进度等)
    STR_PackPlayerPosition* playerPosition =   &((*smap)[conn].m_position);
    StringBuilder sbd;
    sbd << "update t_playerposition set pos_x=" << playerPosition->Pos_x << ",pos_y=" << playerPosition->Pos_y << ",pos_z=" << playerPosition->Pos_z << ",direct=" << playerPosition->Direct <<",mapid=" << playerPosition->MapID << ",actid=" << playerPosition->ActID << " where roleid=" << roleid << ";";

    Logger::GetLogger()->Debug(sbd.str());
    if(srv->getDiskDB()->Set(sbd.str()) == -1)
    {
        Logger::GetLogger()->Error("update player position error");
    }


    SaveRoleInfo(conn);           //更新角色属性
    DeleteFromMonsterView(conn);  //从怪物可视范围内删除该玩家
    SaveRoleEquDurability(conn);  //将玩家装备当前耐久度更新到数据库
    FriendOffline(conn);          //发送下线通知给好友
    SaveRoleNotPickGoods(conn);   //保存玩家未捡取的掉落物品
    SendOffLineToViewRole(conn);  //将下线消息通知给可视范围内的玩家

    SessionMgr::Instance()->NameSockErase(&(*smap)[conn].m_usrid[0]);
    SessionMgr::Instance()->NickSockErase((*smap)[conn].m_RoleBaseInfo.Nick);
    SessionMgr::Instance()->RoleSockErase(roleid);
    SessionMgr::Instance()->SessionsErase(conn);


    ///////////////////////////////////////////////////
//    SessionMgr::SessionMap::iterator it = smap->find(conn);
//    if(it != smap->end())
//    {
//        if((*smap)[conn].m_interchage->isInchange)         //如果正在交易中
//        {
//            TCPConnection::Pointer partnerConn = (*smap)[conn].m_interchage->partnerConn;
//            SessionMgr::SessionMap::iterator iter = smap->find(partnerConn);
//            if(iter != smap->end())
//            {
//                (*smap)[partnerConn].m_interchage->clear();  //清除交易对方的交易状态
//            }
//        }
//    }
    //////////////////////////////////////////////////

    Logger::GetLogger()->Debug("player exit success");
}

//用户下线删除保存的对应的<nick, sock>
//void PlayerLogin::DeleteNickSock(TCPConnection::Pointer conn)
//{
//    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();

//    SessionMgr::umap_nickSock nickSock = SessionMgr::Instance()->GetNickSock();
//    SessionMgr::_umap_nickSock::iterator t_nickSock = nickSock->find((*smap)[conn].m_RoleBaseInfo.Nick);
//    if(t_nickSock != nickSock->end())
//    {
//        nickSock->erase(t_nickSock); //删除m_nickSock
//    }
//}

//void PlayerLogin::DeleteNameSock(TCPConnection::Pointer conn)
//{
//    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();

//    SessionMgr::umap_nickSock nameSock = SessionMgr::Instance()->GetNameSock();
//    SessionMgr::_umap_nickSock::iterator t_nameSock = nameSock->find(&(*smap)[conn].m_usrid[0]);
//    if(t_nameSock != nameSock->end())
//    {
//        nameSock->erase(t_nameSock); //删除m_nameSock
//    }
//}

// void PlayerLogin::DeleteRoleSock(hf_uint32 roleid)
// {
//     umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
//     _umap_roleSock::iterator role_it = t_roleSock->find(roleid);
//     if(role_it != t_roleSock->end())
//     {
//         t_roleSock->erase(roleid); //删除m_roleSock
//     }
// }

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
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    hf_char *pname =  & ( (*smap)[conn].m_usrid[0]);
    sbd<< "insert into t_PlayerRoleList(username,nick,sex,figure,figurecolor,face,eye,hair,haircolor,modeid,skirtid) values('" <<pname<<"','"<< nickbuff << "'," << reg->Sex<<","<<reg->Figure<<","<<reg->FigureColor<<","<< reg->Face << "," << reg->Eye << "," << reg->Hair <<"," << reg->HairColor << "," << reg->ModeID << "," << reg->SkirtID <<");";

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
        Logger::GetLogger()->Debug("role register success");

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
            Logger::GetLogger()->Error("insert player init pos error");
        }

        //写入玩家初始属性
        sbd.Clear();
        sbd << "insert into t_roleinformation(roleid) values(" << t_roleInfo.RoleInfo.RoleID << ");";
        Logger::GetLogger()->Debug(sbd.str());
        t_row = srv->getDiskDB()->Set(sbd.str());
        if(t_row != 1)
        {
            Logger::GetLogger()->Error("insert player init attr error");
        }

        //写入初始金钱
        sbd.Clear();
        sbd << "insert into t_playermoney values(" << t_roleInfo.RoleInfo.RoleID << ",1000," << Money_1 << ");";
        Logger::GetLogger()->Debug(sbd.str());
        t_row = srv->getDiskDB()->Set(sbd.str());
        if(t_row != 1)
        {
            Logger::GetLogger()->Error("insert player init money error");
        }

        //写入玩家初始装备
        sbd.Clear();
        sbd << "insert into t_playerbodyequipment values(" << t_roleInfo.RoleInfo.RoleID << ");";
        Logger::GetLogger()->Debug(sbd.str());
        t_row = srv->getDiskDB()->Set(sbd.str());
        if(t_row != 1)
        {
            Logger::GetLogger()->Error("insert player init bodyequipment");
        }

    }
    srv->free(reg);
}

//删除角色 此函数只能在登录用户，未登录角色的前提下调用
void PlayerLogin::DeleteRole(TCPConnection::Pointer conn, hf_uint32 roleid)
{
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
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
//                SessionMgr::Instance()->NameSockErase(reg->userName);
            }

            t_PackResult.result = RESULT_SUCCESS;
            conn->Write_all(&t_PackResult, sizeof(STR_PackResult));
            conn->ChangePlayerLoginStatus(PlayerLoginUser);

            Logger::GetLogger()->Debug("%s:%s user login success:%s",typeid(this).name(), __FUNCTION__,reg->userName);
            SessionMgr::Instance()->SessionsAdd(conn, reg->userName);
            SessionMgr::Instance()->NameSockAdd(reg->userName, conn);
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

    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();

    hf_char *pname =  & ( (*smap)[conn].m_usrid[0]);
    StringBuilder sbd;
    sbd << "select * from t_playerrolelist where username='"
         <<pname<<"' and roleid=" << roleid << " and ifdelete = 0;";
    Logger::GetLogger()->Debug(sbd.str());

    STR_RoleBasicInfo* t_roleBaseInfo = &(*smap)[conn].m_RoleBaseInfo;
    hf_int32 t_row = srv->getDiskDB()->GetRoleBasicInfo(t_roleBaseInfo, sbd.str());
    if(t_row <= 0) //登录失败
    {
        t_packResult.result = RESULT_ERROR;
        conn->Write_all(&t_packResult, sizeof(STR_PackResult));
    }
    else   //角色查询成功
    {
        t_packResult.result = RESULT_SUCCESS;
        conn->Write_all(&t_packResult, sizeof(STR_PackResult));
        conn->ChangePlayerLoginStatus(PlayerLoginRole);
        Logger::GetLogger()->Debug("Login Role Success");

        (*smap)[conn].m_roleid = roleid;

        SessionMgr::Instance()->RoleSockAdd(roleid, conn);
        SessionMgr::Instance()->NickSockAdd(t_roleBaseInfo->Nick, conn);


        sbd.Clear();
        sbd << "select * from t_playerbodyequipment where roleid = " << roleid << ";";
        Logger::GetLogger()->Debug(sbd.str());
        t_row = srv->getDiskDB()->GetRoleBodyEqu(&(*smap)[conn].m_BodyEqu, sbd.str());
        if(t_row == -1)
        {
            Logger::GetLogger()->Error("queue role bodyEqu error");
        }

        //查询角色属性，发送给玩家
        STR_PackRoleInfo t_roleInfo;
        sbd.Clear();
        sbd << "select * from t_roleInformation where roleid = " << roleid << ";";
        Logger::GetLogger()->Debug(sbd.str());
        t_row = srv->getDiskDB()->GetRoleInfo(&t_roleInfo.RoleInfo, sbd.str());
        if(t_row == 1)
        {
            conn->Write_all(&t_roleInfo, sizeof(STR_PackRoleInfo));
            SessionMgr::Instance()->SaveSession(conn, &t_roleInfo.RoleInfo);
            sbd.Clear();
            sbd << "delete from t_roleinformation where roleid = " << roleid << ";";
            Logger::GetLogger()->Debug(sbd.str());
            t_row = srv->getDiskDB()->Set(sbd.str());
            if(t_row != 1)
            {
                Logger::GetLogger()->Error("delete jole attribute error");
            }
        }
        else if(t_row == 0) //服务器挂了，没有将玩家的属性信息写入数据库，需要重新计算角色属性值
        {
            CalculationRoleAttribute(&t_roleInfo.RoleInfo, &(*smap)[conn].m_BodyEqu, t_roleBaseInfo->Profession, t_roleBaseInfo->Level);

            conn->Write_all(&t_roleInfo, sizeof(STR_PackRoleInfo));
            SessionMgr::Instance()->SaveSession(conn, &t_roleInfo.RoleInfo);
        }


        Logger::GetLogger()->Debug("Send Position data.....");
        UserPosition::Position_push(conn, roleid);

        SendRoleExperence(conn);            //发送角色经验
        SendFriendList(conn, roleid);       //发送好友列表
        SendRoleMoney(conn, roleid);        //发送玩家金币
        SendRoleGoods(conn, roleid);        //发送玩家背包物品
        SendRoleEquAttr(conn, roleid);      //发送玩家装备属性
        SendRoleNotPickGoods(conn, roleid); //发送玩家未捡取的物品
        GetPlayerCompleteTask(conn);        //查询玩家已经完成的任务

        Server* srv = Server::GetInstance();
        srv->GetGameAttack()->SendPlayerSkill(conn);       //玩家可使用的技能
        srv->GetMonster()->PushViewMonsters(conn);         //玩家可视范围内的怪物
        srv->GetPlayerLogin()->SendViewRole(conn);         //玩家可视范围内的玩家
        srv->GetGameTask()->SendPlayerTaskProcess(conn);   //玩家任务进度
        srv->GetGameTask()->SendPlayerViewTask(conn);      //玩家可接任务        
        srv->GetTeamFriend()->SendAskAddFriend(conn);      //离线的添加好友请求
        Logger::GetLogger()->Debug("%u login data send success", roleid);


        //test
//        hf_double* timep = &(*(*smap)[conn].m_skillTime)[100];
//        printf("%lf\n", *timep);
//        *timep = 1000.56;
//        timep = &(*(*smap)[conn].m_skillTime)[100];
//        printf("%lf\n", *timep);
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
        SessionMgr::Instance()->NameSockErase(&(*smap)[conn].m_usrid[0]);
        SessionMgr::Instance()->SessionsErase(conn);
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

        sbd.Clear();
        sbd << "select t_playerbodyequipment.roleid,head,headtype,upperbody,upperbodytype,pants,pantstype,shoes,shoestype,belt,belttype,neaklace,neaklacetype,bracelet,bracelettype,leftring,leftringtype,rightring,rightringtype,phone,phonetype,weapon,weapontype from t_playerbodyequipment,t_playerrolelist where t_playerbodyequipment.roleid = t_playerrolelist.roleid and username = '" <<   userID << "' and ifdelete = 0;";
        Logger::GetLogger()->Debug(sbd.str());
        t_row = srv->getDiskDB()->GetUserBodyEqu(buff, sbd.str());
        if(t_row > 0)
        {
            t_packHead.Flag = FLAG_PlayerBodyEqu;
            t_packHead.Len = sizeof(STR_BodyEquipment)*t_row;
            memcpy(buff, &t_packHead, sizeof(STR_PackHead));
            conn->Write_all(buff, sizeof(STR_PackHead) + t_packHead.Len);
        }
        Logger::GetLogger()->Debug("role list send success:name%s",namebuff);
        srv->free(buff);
    }   
}


//发送好友列表
void PlayerLogin::SendFriendList(TCPConnection::Pointer conn, hf_uint32 RoleID)
{
    StringBuilder sbd;
    Server* srv = Server::GetInstance();
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
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
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
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
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
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
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    umap_roleEqu  playerEqu = (*smap)[conn].m_playerEqu;
    if(playerEqu->size() == 0)
    {
        return;
    }

    StringBuilder sbd;
    sbd << "select * from t_playerequattr where roleid = " << RoleID << ";";
    Logger::GetLogger()->Debug(sbd.str());

    STR_PackHead t_packHead;
    hf_uint32 t_row = Server::GetInstance()->getDiskDB()->GetPlayerEqu(playerEqu, sbd.str());
    if(t_row > 0)
    {
        hf_char* buff = (hf_char*)Server::GetInstance()->malloc();
        memset(&t_packHead, 0, sizeof(STR_PackHead));
        t_packHead.Flag = FLAG_EquGoodsAttr;
        t_packHead.Len = playerEqu->size() * sizeof(STR_EquipmentAttr);
        memcpy(buff, &t_packHead, sizeof(STR_PackHead));
        hf_int32 i = 0;
        for(_umap_roleEqu::iterator it = playerEqu->begin(); it != playerEqu->end(); it++)
        {
            STR_EquipmentAttr* equattr = &it->second.equAttr;
//            printf("equID=%d\n", equattr->EquID);
            memcpy(buff + sizeof(STR_PackHead) + i*sizeof(STR_EquipmentAttr), &(it->second.equAttr), sizeof(STR_EquipmentAttr));
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
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
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

//    sbd << "delete from t_notpickgoodspos where roleid = " << RoleID << ";";
//    Logger::GetLogger()->Debug(sbd.str());
//    srv->getDiskDB()->Set(sbd.str());

//    sbd.Clear();
//    sbd << "delete from t_notpickgoods where roleid = " << RoleID << ";";
//    Logger::GetLogger()->Debug(sbd.str());
//    srv->getDiskDB()->Set(sbd.str());

    srv->free(buff);
}

//查询玩家已经完成的任务
void PlayerLogin::GetPlayerCompleteTask(TCPConnection::Pointer conn)
{
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    umap_completeTask completeTask = (*smap)[conn].m_completeTask;
    hf_uint32 roleid = (*smap)[conn].m_roleid;
    StringBuilder sbd;
    sbd << "select taskid from t_playercompletetask where roleid = " << roleid << ";";
    Logger::GetLogger()->Debug(sbd.str());
    if(Server::GetInstance()->getDiskDB()->GetPlayerCompleteTask(completeTask, sbd.str()) == -1)
    {
        Logger::GetLogger()->Error("queue palyer complete task error");
    }
}

//将玩家背包里的物品写进数据库
void PlayerLogin::SaveRoleBagGoods(TCPConnection::Pointer conn)
{
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    umap_roleGoods t_roleGoods = (*smap)[conn].m_playerGoods;
    hf_uint32 roleid = (*smap)[conn].m_roleid;

    StringBuilder sbd;
    sbd << "delete from t_playergoods where roleid = " << roleid << ";";
     Logger::GetLogger()->Debug(sbd.str());
    if(Server::GetInstance()->getDiskDB()->Set(sbd.str()) == -1)
    {
        Logger::GetLogger()->Error("delete player bag goods error");
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
        Logger::GetLogger()->Error("player bag goods insert error");
    }
}

//将玩家装备当前耐久度更新到数据库
void PlayerLogin::SaveRoleEquDurability(TCPConnection::Pointer conn)
{
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    umap_roleEqu roleEqu = (*smap)[conn].m_playerEqu;    
    StringBuilder sbd;
    for(_umap_roleEqu::iterator it = roleEqu->begin(); it != roleEqu->end(); it++)
    {
        sbd.Clear();
        sbd << "update t_playerequattr set durability=" << it->second.equAttr.Durability << " where equid=" << it->second.equAttr.EquID << ";";
        Logger::GetLogger()->Debug(sbd.str());
        if(Server::GetInstance()->getDiskDB()->Set(sbd.str()) == -1)
        {
            Logger::GetLogger()->Error("player equipment attr insert error");
        }
    }   
}

//将玩家金钱写进数据库
void PlayerLogin::SaveRoleMoney(TCPConnection::Pointer conn)
{
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    umap_roleMoney t_roleMoney = (*smap)[conn].m_playerMoney;
    hf_uint32 roleid = (*smap)[conn].m_roleid;

    StringBuilder sbd;
    sbd << "delete from t_playermoney where roleid = " << roleid << ";";

     Logger::GetLogger()->Debug(sbd.str());
    if(Server::GetInstance()->getDiskDB()->Set(sbd.str()) == -1)
    {
        Logger::GetLogger()->Error("delete player money error");
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
        Logger::GetLogger()->Error("insert player money error");
    }
}


//将玩家任务进度写进数据库
void PlayerLogin::SaveRoleTaskProcess(TCPConnection::Pointer conn)
{
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    umap_taskProcess playerAcceptTask = (*smap)[conn].m_playerAcceptTask;
    hf_uint32 roleid = (*smap)[conn].m_roleid;

    StringBuilder sbd;
    sbd << "delete from t_playertaskprocess where roleid = " << roleid << ";";

     Logger::GetLogger()->Debug(sbd.str());
    if(Server::GetInstance()->getDiskDB()->Set(sbd.str()) == -1)
    {
        Logger::GetLogger()->Error("delete player task process error");
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
        Logger::GetLogger()->Error("insert player task process error");
    }
}


//将离开可视范围的消息发送给可视范围内的玩家
void PlayerLogin::SendOffLineToViewRole(TCPConnection::Pointer conn)
{
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
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
    }
}



//保存玩家未捡取的物品
void PlayerLogin::SaveRoleNotPickGoods(TCPConnection::Pointer conn)
{
    time_t timep;
    time(&timep);
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    umap_lootPosition      t_lootPosition = (*smap)[conn].m_lootPosition;
    hf_uint32 roleid = (*smap)[conn].m_roleid;

    StringBuilder sbd;
    sbd << "delete from t_notpickgoods where roleid = " << roleid << ";";
     Logger::GetLogger()->Debug(sbd.str());
    if(Server::GetInstance()->getDiskDB()->Set(sbd.str()) == -1)
    {
        Logger::GetLogger()->Error("delete player not pickgoods error");
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
        Logger::GetLogger()->Error("insert player not pick goods error");
    }


    sbd.Clear();
    sbd << "delete from t_notpickgoodspos where roleid = " << roleid << ";";
    Logger::GetLogger()->Debug(sbd.str());
   if(Server::GetInstance()->getDiskDB()->Set(sbd.str()) == -1)
   {
       Logger::GetLogger()->Error("delete player not pick goods pos error");
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

//        printf("插入物品位置，goods:物品掉落时间：%u,%u,当点时间：%u\n", it->second.timep,it->second.continueTime,timep);
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
        Logger::GetLogger()->Error("insert player not pick goods pos error");
    }    
}

//玩家角色属性
void PlayerLogin::SaveRoleInfo(TCPConnection::Pointer conn)
{
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    STR_RoleInfo      t_roleinfo = (*smap)[conn].m_roleInfo;
    hf_uint32 roleid = (*smap)[conn].m_roleid;
    StringBuilder sbd;
    sbd << "insert into t_roleinformation values(" << roleid << "," << t_roleinfo.MaxHP << "," << t_roleinfo.HP << "," << t_roleinfo.MaxMagic << "," << t_roleinfo.Magic << "," << t_roleinfo.PhysicalDefense << "," << t_roleinfo.MagicDefense << "," << t_roleinfo.PhysicalAttack << "," << t_roleinfo.MagicAttack << "," << t_roleinfo.Crit_Rate << "," << t_roleinfo.Dodge_Rate << "," << t_roleinfo.Hit_Rate << "," << t_roleinfo.Resist_Rate << "," << t_roleinfo.Caster_Speed << "," << t_roleinfo.Move_Speed << "," << t_roleinfo.Hurt_Speed << "," << t_roleinfo.Small_Universe << "," << t_roleinfo.maxSmall_Universe << "," << t_roleinfo.RecoveryLife_Percentage << "," << t_roleinfo.RecoveryLife_value << "," << t_roleinfo.RecoveryMagic_Percentage << "," << t_roleinfo.RecoveryMagic_value << "," << t_roleinfo.MagicHurt_Reduction << "," << t_roleinfo.PhysicalHurt_Reduction << "," << t_roleinfo.CritHurt << "," << t_roleinfo.CritHurt_Reduction << "," << t_roleinfo.Magic_Pass << "," << t_roleinfo.Physical_Pass << "," << t_roleinfo.Rigorous << "," << t_roleinfo.Will << "," << t_roleinfo.Wise << "," << t_roleinfo.Mentality << "," << t_roleinfo.Physical_fitness << ");";
    Logger::GetLogger()->Debug(sbd.str());
    if(Server::GetInstance()->getDiskDB()->Set(sbd.str()) != 1)
    {
        Logger::GetLogger()->Error("insert player roleinfo error");
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
        Logger::GetLogger()->Error("update player money error");
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
        Logger::GetLogger()->Error("update player level error");
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
        Logger::GetLogger()->Error("update player experience error");
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
        Logger::GetLogger()->Error("update player bag goods error");
    }
}

//新物品更新背包
void PlayerLogin::InsertPlayerGoods(hf_uint32 roleid, STR_Goods* insGoods)
{
    StringBuilder sbd;
    sbd << "insert into t_playergoods values(" << roleid << "," << insGoods->GoodsID << "," << insGoods->TypeID << "," << insGoods->Count << "," << insGoods->Position << ");";
    Logger::GetLogger()->Debug(sbd.str());
    if(Server::GetInstance()->getDiskDB()->Set(sbd.str()) == -1)
    {
        Logger::GetLogger()->Error("player bag insert goods error");
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
        Logger::GetLogger()->Error("player bag delete goods error");
    }
}

//更新玩家装备属性
void PlayerLogin::UpdatePlayerEquAttr(STR_EquipmentAttr* equ)
{
    StringBuilder sbd;
    sbd << "update t_playerequattr set crithurt=" << equ->CritHurt << ",crithurt_reduction=" << equ->CritHurt_Reduction << ",addhp=" << equ->HP << ",addmagic=" << equ->Magic << ",physicaldefense=" << equ->PhysicalDefense << ",magicdefense=" << equ->MagicDefense << ",physicalattack=" << equ->PhysicalAttack << ",magicattack=" << equ->MagicAttack << ",rigorous=" << equ->Rigorous << ",will=" << equ->Will << ",wise=" << equ->Wise << ",mentality=" << equ->Mentality << ",physical_fitness=" << equ->Physical_fitness << "strengthenlevel=" << equ->StrengthenLevel << " where equid = " << equ->EquID << ";";
     Logger::GetLogger()->Debug(sbd.str());
     hf_int32 t_value = Server::GetInstance()->getDiskDB()->Set(sbd.str());
    if(t_value == -1)
    {
        Logger::GetLogger()->Error("update player equattr error");
    }
}

//新装备插入属性
void PlayerLogin::InsertPlayerEquAttr(hf_uint32 roleid, STR_EquipmentAttr* equ)
{
    StringBuilder sbd;
    sbd << "insert into t_playerequattr values(" << roleid << "," << equ->EquID << "," << equ->TypeID << "," << equ->CritHurt << "," << equ->Dodge_Rate << "," << equ->Hit_Rate << "," << equ->Resist_Rate << "," << equ->Caster_Speed << "," << equ->Move_Speed << "," << equ->Hurt_Speed << "," << equ->RecoveryLife_Percentage << "," << equ->RecoveryLife_value << "," << equ->RecoveryMagic_Percentage << "," << equ->RecoveryMagic_value << "," << equ->MagicHurt_Reduction << "," << equ->PhysicalHurt_Reduction << "," << equ->CritHurt << "," << equ->CritHurt_Reduction << "," << equ->Magic_Pass << "," << equ->Physical_Pass << "," << equ->SuitSkillID << "," << equ->HP << "," << equ->Magic << "," << equ->PhysicalDefense << "," << equ->MagicDefense << "," << equ->PhysicalAttack << "," << equ->MagicAttack << "," << equ->Rigorous << "," << equ->Will << "," << equ->Wise << "," << equ->Mentality << "," << equ->Physical_fitness << "," << equ->JobID << "," << equ->BodyPos << "," << equ->Grade << "," << equ->Level << "," << equ->StrengthenLevel << "," << equ->MaxDurability << "," << equ->Durability << ");";

    Logger::GetLogger()->Debug(sbd.str());
    if(Server::GetInstance()->getDiskDB()->Set(sbd.str()) == -1)
    {
        Logger::GetLogger()->Error("insert player equattr error");
    }
}

//删除玩家背包装备属性
void PlayerLogin::DeletePlayerEquAttr(hf_uint32 equid)
{
    StringBuilder sbd;
    sbd << "delete from t_playerequattr where equid = " << equid << ";";
     Logger::GetLogger()->Debug(sbd.str());
    if(Server::GetInstance()->getDiskDB()->Set(sbd.str()) == -1)
    {
        Logger::GetLogger()->Error("delete player equattr error");
    }
}

//更新玩家任务进度
void PlayerLogin::UpdatePlayerTask(hf_uint32 roleid, STR_TaskProcess* upTask)
{
    StringBuilder sbd;
    sbd << "update t_playertaskprocess set aimcount = " << upTask->FinishCount << " where roleid = " << roleid << " and taskid = " << upTask->TaskID << ";";
     Logger::GetLogger()->Debug(sbd.str());
     hf_int32 t_value = Server::GetInstance()->getDiskDB()->Set(sbd.str());
    if(t_value == -1)
    {
        Logger::GetLogger()->Error("update player taskprocess error");
    }
}

//插入新任务
void PlayerLogin::InsertPlayerTask(hf_uint32 roleid, STR_TaskProcess* insTask)
{
    StringBuilder sbd;
    sbd << "insert into t_playertaskprocess values(" << roleid << "," << insTask->TaskID << "," << insTask->AimID << "," << insTask->FinishCount << "," << insTask->AimAmount << "," << insTask->ExeModeID << ");";
    Logger::GetLogger()->Debug(sbd.str());
    if(Server::GetInstance()->getDiskDB()->Set(sbd.str()) == -1)
    {
        Logger::GetLogger()->Error("insert new taskprocess error");
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
        Logger::GetLogger()->Error("delete player task error");
    }
}

void PlayerLogin::InsertPlayerCompleteTask(hf_uint32 roleid, hf_uint32 taskid)
{
    StringBuilder sbd;
    sbd << "insert into t_playercompletetask values(" << roleid << "," << taskid << ");";
     Logger::GetLogger()->Debug(sbd.str());
    if(Server::GetInstance()->getDiskDB()->Set(sbd.str()) == -1)
    {
        Logger::GetLogger()->Error("insert player comeletetask error");
    }
}

//刷新可视范围内的玩家
void PlayerLogin::SendViewRole(TCPConnection::Pointer conn)
{
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    umap_roleSock viewRole = (*smap)[conn].m_viewRole;
    STR_PackPlayerPosition* pos = &(*smap)[conn].m_position;
    hf_uint32 roleid = (*smap)[conn].m_roleid;
    Server* srv = Server::GetInstance();
    hf_char* comebuff = (hf_char*)srv->malloc();
    hf_char* leavebuff = (hf_char*)srv->malloc();
    hf_uint16 pushCount = 0;
    hf_uint16 popCount = 0;
    STR_PackHead t_packHead;

    STR_RoleInfo* t_roleInfo = &(*smap)[conn].m_roleInfo;
    STR_OtherPlayerInfo playerInfo;
    playerInfo.MaxHP = t_roleInfo->MaxHP;
    playerInfo.HP = t_roleInfo->HP;
    playerInfo.MaxMagic = t_roleInfo->MaxMagic;
    playerInfo.Magic = t_roleInfo->Magic;

    hf_uint16 len = sizeof(STR_RoleBasicInfo) + sizeof(STR_OtherPlayerPosition) + sizeof(STR_OtherPlayerInfo);

    STR_PackRoleCome t_roleCome;
    memcpy(&t_roleCome.roleBasicInfo, &((*smap)[conn].m_RoleBaseInfo), sizeof(STR_RoleBasicInfo));
    memcpy(&t_roleCome.otherRolePos, (hf_char*)pos + sizeof(STR_PackHead), sizeof(STR_OtherPlayerPosition));
    memcpy(&t_roleCome.otherPlayerInfo, &playerInfo, sizeof(STR_OtherPlayerInfo));

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

            STR_RoleInfo* t_otherRoleInfo = &(it->second.m_roleInfo);
            STR_OtherPlayerInfo t_otherPlayerInfo;
            t_otherPlayerInfo.MaxHP = t_otherRoleInfo->MaxHP;
            t_otherPlayerInfo.HP = t_otherRoleInfo->HP;
            t_otherPlayerInfo.MaxMagic = t_otherRoleInfo->MaxMagic;
            t_otherPlayerInfo.Magic = t_otherRoleInfo->Magic;
            memcpy(comebuff + sizeof(STR_PackHead) + pushCount*len + sizeof(STR_RoleBasicInfo) + sizeof(STR_OtherPlayerPosition), &t_otherPlayerInfo, sizeof(STR_OtherPlayerInfo));
            pushCount++;


            (*viewRole)[it->second.m_roleid] = it->first;
            (*(it->second.m_viewRole))[roleid] = conn;
            it->first->Write_all(&t_roleCome, sizeof(STR_PackRoleCome));

            if(pushCount == (CHUNK_SIZE - sizeof(STR_PackHead))/len)
            {
                t_packHead.Flag = FLAG_ViewRoleCome;
                t_packHead.Len = len * pushCount;
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
    if(pushCount != (CHUNK_SIZE - sizeof(STR_PackHead))/len && pushCount != 0)
    {
        t_packHead.Flag = FLAG_ViewRoleCome;
        t_packHead.Len = len * pushCount;
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


//发送角色经验，属性
void PlayerLogin::SendRoleExperence(TCPConnection::Pointer conn)
{
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    hf_uint32 roleid = (*smap)[conn].m_roleid;
    STR_PackRoleExperience* exp = &(*smap)[conn].m_roleExp;
    StringBuilder sbd;
    sbd << "select level,experience from t_playerrolelist where roleid = " << roleid << ";";
    Logger::GetLogger()->Debug(sbd.str());
    if(Server::GetInstance()->getDiskDB()->GetRoleExperience(exp, sbd.str()) != 1)
    {
        Logger::GetLogger()->Error("select role experience error");
        return;
    }
    conn->Write_all(exp, sizeof(STR_PackRoleExperience));
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
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
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



//查询角色职业属性
void PlayerLogin::QueryRoleJobAttribute()
{
    StringBuilder sbd;
    sbd << "select * from t_roleattribute where job=" << CommonJob << ";";
    Logger::GetLogger()->Debug(sbd.str());
    if(Server::GetInstance()->getDiskDB()->GetJobAttribute(m_common, sbd.str()) != CommonGradeCount)
    {
        Logger::GetLogger()->Error("queue common roleattribute error");
    }
    sbd.Clear();
    sbd << "select * from t_roleattribute where job=" << SaleJob << ";";
    Logger::GetLogger()->Debug(sbd.str());
    STR_RoleJobAttribute* t_sales = m_sales + ChangeProfessionGrade;
    if(Server::GetInstance()->getDiskDB()->GetJobAttribute(t_sales, sbd.str()) != OtherGradeCount)
    {
        Logger::GetLogger()->Error("queue sale roleattribute error");
    }

    sbd.Clear();
    sbd << "select * from t_roleattribute where job=" << TechnologyJob << ";";
    Logger::GetLogger()->Debug(sbd.str());
    STR_RoleJobAttribute* t_technology = m_technology + ChangeProfessionGrade;
    if(Server::GetInstance()->getDiskDB()->GetJobAttribute(t_technology, sbd.str()) != OtherGradeCount)
    {
        Logger::GetLogger()->Error("queue technology roleattribute error");
    }

    sbd.Clear();
    sbd << "select * from t_roleattribute where job=" << AdministrationJob << ";";
    Logger::GetLogger()->Debug(sbd.str());
    STR_RoleJobAttribute* t_administration = m_administration + ChangeProfessionGrade;
    if(Server::GetInstance()->getDiskDB()->GetJobAttribute(t_administration, sbd.str()) != OtherGradeCount)
    {
        Logger::GetLogger()->Error("queue administ roleattribute error");
    }
}

//计算玩家属性
void PlayerLogin::CalculationRoleAttribute(STR_RoleInfo* roleInfo, STR_BodyEquipment* bodyEqu, hf_uint8 profession, hf_uint8 level)
{
    STR_RoleJobAttribute  t_roleAttr;
    if(profession == CommonJob)
    {
        memcpy(&t_roleAttr, &m_common[level], sizeof(STR_RoleJobAttribute));
    }
    else if(profession == SaleJob)
    {
        memcpy(&t_roleAttr, &m_technology[level], sizeof(STR_RoleJobAttribute));
    }
    else if(profession == TechnologyJob)
    {
        memcpy(&t_roleAttr, &m_technology[level], sizeof(STR_RoleJobAttribute));
    }
    else if(profession == AdministrationJob)
    {
        memcpy(&t_roleAttr, &m_administration[level], sizeof(STR_RoleJobAttribute));
    }

    if(level >= SmallUniverseLevelValue)
    {
        roleInfo->Small_Universe = SmallValue;
    }
    else
    {
        roleInfo->Small_Universe = 0;
    }
    roleInfo->HP = t_roleAttr.MaxHP;
    roleInfo->MaxHP = roleInfo->HP;
    roleInfo->Magic = t_roleAttr.MaxMagic;
    roleInfo->MaxMagic = roleInfo->Magic;
    roleInfo->PhysicalDefense = t_roleAttr.PhysicalDefense;
    roleInfo->MagicDefense = t_roleAttr.MagicDefense;
    roleInfo->PhysicalAttack = t_roleAttr.PhysicalAttack;
    roleInfo->MagicAttack = t_roleAttr.MagicAttack;
    roleInfo->Crit_Rate = RoleCritRate;
    roleInfo->Dodge_Rate = RoleDodgeRate;
    roleInfo->Hit_Rate = RoleHitRate;
    roleInfo->Resist_Rate = RoleResistRate;
    roleInfo->Caster_Speed = CasterSpeed;
    roleInfo->Move_Speed = MoveSpeed;
    roleInfo->Hurt_Speed = HurtSpeed;
    roleInfo->maxSmall_Universe = 100;

    roleInfo->Rigorous = t_roleAttr.Rigorous;
    roleInfo->Will = t_roleAttr.Will;
    roleInfo->Wise = t_roleAttr.Wise;
    roleInfo->Mentality = t_roleAttr.Mentality;
    roleInfo->Physical_fitness = t_roleAttr.Physical_fitness;


    OperationGoods* t_operGoods = Server::GetInstance()->GetOperationGoods();
    if(bodyEqu->Head != 0)
    {
        t_operGoods->AddEquAttrToRole(roleInfo, bodyEqu->HeadType);
    }
    if(bodyEqu->UpperBody != 0)
    {
        t_operGoods->AddEquAttrToRole(roleInfo, bodyEqu->UpperBodyType);
    }
    if(bodyEqu->Pants != 0)
    {
        t_operGoods->AddEquAttrToRole(roleInfo, bodyEqu->PantsType);
    }
    if(bodyEqu->Shoes != 0)
    {
        t_operGoods->AddEquAttrToRole(roleInfo, bodyEqu->ShoesType);
    }
    if(bodyEqu->Belt != 0)
    {
        t_operGoods->AddEquAttrToRole(roleInfo, bodyEqu->BeltType);
    }
    if(bodyEqu->Neaklace != 0)
    {
        t_operGoods->AddEquAttrToRole(roleInfo, bodyEqu->NeaklaceType);
    }
    if(bodyEqu->Bracelet != 0)
    {
        t_operGoods->AddEquAttrToRole(roleInfo, bodyEqu->BraceletType);
    }
    if(bodyEqu->LeftRing != 0)
    {
        t_operGoods->AddEquAttrToRole(roleInfo, bodyEqu->LeftRingType);
    }
    if(bodyEqu->RightRing != 0)
    {
        t_operGoods->AddEquAttrToRole(roleInfo, bodyEqu->RightRingType);
    }
    if(bodyEqu->Phone != 0)
    {
        t_operGoods->AddEquAttrToRole(roleInfo, bodyEqu->PhoneType);
    }
    if(bodyEqu->Weapon != 0)
    {
        t_operGoods->AddEquAttrToRole(roleInfo, bodyEqu->WeaponType);
    }
}


//从怪物可视范围内删除该玩家
void PlayerLogin::DeleteFromMonsterView(TCPConnection::Pointer conn)
{
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    hf_uint32 roleid = (*smap)[conn].m_roleid;
    umap_monsterViewRole  monsterViewRole = Server::GetInstance()->GetMonster()->GetMonsterViewRole();
    umap_playerViewMonster  viewMonster = (*smap)[conn].m_viewMonster;
    for(_umap_playerViewMonster::iterator it = viewMonster->begin(); it != viewMonster->end(); it++)
    {
        (*monsterViewRole)[it->first].erase(roleid);
    }
}

//玩家复活
void PlayerLogin::PlayerRelive(TCPConnection::Pointer conn, hf_uint16 mode)
{
    SessionMgr::SessionPointer smap = SessionMgr::Instance()->GetSession();
    STR_RoleInfo* t_roleInfo = &(*smap)[conn].m_roleInfo;
    mode = 0;
    t_roleInfo->HP = t_roleInfo->MaxHP;
    Logger::GetLogger()->Debug("player:%u spawns %u\n",(*smap)[conn].m_roleid, t_roleInfo->HP);

    STR_RoleAttribute t_roleAttr((*smap)[conn].m_roleid, t_roleInfo->HP);
    conn->Write_all(&t_roleAttr, sizeof(STR_RoleAttribute));

    umap_roleSock t_viewRole = (*smap)[conn].m_viewRole;
    for(_umap_roleSock::iterator view_it = t_viewRole->begin(); view_it != t_viewRole->end(); view_it++)
    {
        view_it->second->Write_all(&t_roleAttr, sizeof(STR_RoleAttribute));
    }
}


//玩家升级，更新属性
void PlayerLogin::UpdateJobAttr(hf_uint8 profession, hf_uint8 level, STR_RoleInfo* roleInfo)
{
    STR_RoleJobAttribute t_jobAttr;

    if(profession == CommonJob && level <= 15)
        memcpy(&t_jobAttr, &m_common[level], sizeof(STR_RoleJobAttribute));
    else if(profession == SaleJob)
        memcpy(&t_jobAttr, &m_sales[level], sizeof(STR_RoleJobAttribute));
    else if(profession == TechnologyJob)
        memcpy(&t_jobAttr, &m_technology[level], sizeof(STR_RoleJobAttribute));
    else if(profession == AdministrationJob)
        memcpy(&t_jobAttr, &m_administration[level], sizeof(STR_RoleJobAttribute));

    roleInfo->MaxHP = t_jobAttr.MaxHP;
    roleInfo->MaxMagic = t_jobAttr.MaxMagic;
    roleInfo->PhysicalDefense = t_jobAttr.PhysicalDefense;
    roleInfo->MagicDefense = t_jobAttr.MagicDefense;
    roleInfo->PhysicalAttack = t_jobAttr.PhysicalAttack;
    roleInfo->MagicAttack = t_jobAttr.MagicAttack;
    roleInfo->Rigorous = t_jobAttr.Rigorous;
    roleInfo->Will = t_jobAttr.Will;
    roleInfo->Wise = t_jobAttr.Wise;
    roleInfo->Mentality = t_jobAttr.Mentality;
    roleInfo->Physical_fitness = t_jobAttr.Physical_fitness;
}

