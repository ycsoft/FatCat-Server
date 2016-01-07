#ifndef CMDPARSELOGIN_HPP
#define CMDPARSELOGIN_HPP

cmd
PlayerLogin* t_playerLogin = srv->GetPlayerLogin();


switch( flag )
{
//注册玩家帐号
case FLAG_PlayerRegisterUserId:
{
    STR_PlayerRegisterUserId* reg = (STR_PlayerRegisterUserId*)srv->malloc();
    memcpy(reg, buf+sizeof(STR_PackHead), len);
    srv->RunTask(boost::bind(&PlayerLogin::RegisterUserID, t_playerLogin, conn, reg));
    break;
}
    //注册玩家角色
case FLAG_PlayerRegisterRole:
{
    STR_PlayerRegisterRole* reg =(STR_PlayerRegisterRole*)srv->malloc();
    memcpy(reg, buf+sizeof(STR_PackHead), len);
    srv->RunTask(boost::bind(&PlayerLogin::RegisterRole, t_playerLogin, conn, reg));
    break;
}
    //删除角色
case FLAG_UserDeleteRole:
{
    STR_PlayerRole* reg = (STR_PlayerRole*)(buf + sizeof(STR_PackHead));
    srv->RunTask(boost::bind(&PlayerLogin::DeleteRole, t_playerLogin, conn, reg->Role));
    break;
}
    //登陆帐号
case FLAG_PlayerLoginUserId:
{
    STR_PlayerLoginUserId* reg = (STR_PlayerLoginUserId*)srv->malloc();
    memcpy(reg, buf+sizeof(STR_PackHead), len);
    srv->RunTask(boost::bind(&PlayerLogin::LoginUserId, t_playerLogin, conn, reg));
    break;
}
    //登陆角色
case FLAG_PlayerLoginRole:
{
    STR_PlayerRole* reg = (STR_PlayerRole*)(buf + sizeof(STR_PackHead));
    srv->RunTask(boost::bind(&PlayerLogin::LoginRole, t_playerLogin, conn, reg->Role));
    break;
}

#endif // CMDPARSELOGIN_HPP

