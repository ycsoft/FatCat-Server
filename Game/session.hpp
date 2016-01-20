#ifndef SESSION
#define SESSION

#include <boost/unordered_map.hpp>
#include <boost/noncopyable.hpp>
#include <boost/array.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>

#include <cstdio>

#include "postgresqlstruct.h"
#include "./../NetWork/tcpconnection.h"
//#include "log.h"
#include "cmdtypes.h"

using boost::asio::ip::tcp;

class Interchange
{
public:
    Interchange():isInchange(false),isLocked(false),isChangeChecked(false){}
    bool isInchange;                       //是否在交易
    bool isLocked;                           //锁定状态  如果锁定则锁定方不能再选择物品
    bool isChangeChecked;             //是否已经确认交易
    TCPConnection::Pointer partnerConn;       //交易的对方连接
    int roleId;

    std::vector<STR_Goods> changes;    //缓存交易过程中要交易物品
    vector<STR_EquipmentAttr> vecEqui; //缓存交易过程中交易的装备的信息
    interchangeMoney money;   //缓存交易过程中要交易的金钱

    bool inChange()
    {
        return isInchange;
    }

    void goInChange()
    {
        isInchange = true;
    }

    void goOutChange()
    {
        isInchange = false;
    }

    void lock()                                             //锁定 不再改变交易物品
    {
        isLocked = true;
    }

    void unlock()                                       //解除锁定，可以改变交易物品（一方解除锁定，则另一方也变为解除锁定状态）
    {
        isLocked = false;
        isChangeChecked = false;
    }

    void checkChange()                             //确认交易  如果双方都确认交易 则交易成功
    {
        isChangeChecked = true;
    }

    void clear()                                           //恢复到原来未交易状态
    {
        isInchange = false;
        isLocked =false;
        isChangeChecked = false;
        partnerConn.reset();
        changes.clear();
        money.MoneyCount = 0;

    }
};

class Session
{
public:
    Session()
        :m_playerAcceptTask(new _umap_taskProcess)
        ,m_friendList(new _umap_friendList)
        ,m_viewRole(new _umap_roleSock)
        ,m_viewMonster(new _umap_playerViewMonster)
        ,m_skillTime(new _umap_skillTime)
        ,m_playerGoods(new _umap_roleGoods)
        ,m_playerEqu(new _umap_roleEqu)
//        ,m_playerEquAttr(new _umap_roleEquAttr)
        ,m_playerMoney(new _umap_roleMoney)
        ,m_completeTask(new _umap_completeTask)
        ,m_lootGoods(new _umap_lootGoods)
        ,m_lootPosition(new _umap_lootPosition)
        ,m_interchage(new Interchange)
        ,m_taskGoods( new _umap_taskGoods)
    {
        m_publicCoolTime = 0;
        memset(m_usrid, 0, sizeof(m_usrid));
        memset(m_goodsPosition, 0, BAGCAPACITY);
        m_skillUseTime = 0;
        m_roleid = 0;
    }

    ~Session()
    {

    }

    Session(const Session& sess)
    {
        printf("nihao\n\n");

        memcpy(m_usrid, sess.m_usrid, sizeof(m_usrid));
        m_roleid = sess.m_roleid;
        m_publicCoolTime = sess.m_publicCoolTime;
        m_skillUseTime = sess.m_skillUseTime;
        memcpy(&m_roleInfo, &sess.m_roleInfo, sizeof(STR_RoleInfo));
//        m_roleInfo = sess.m_roleInfo;
        memcpy(&m_roleExp, &sess.m_roleExp, sizeof(STR_PackRoleExperience));
//        m_roleExp = sess.m_roleExp;
        memcpy(&m_RoleBaseInfo, &sess.m_RoleBaseInfo, sizeof(STR_RoleBasicInfo));
//        m_RoleBaseInfo = sess.m_RoleBaseInfo;
        memcpy(&m_BodyEqu, &sess.m_BodyEqu, sizeof(STR_BodyEquipment));
//        m_BodyEqu = sess.m_BodyEqu;
        m_playerAcceptTask = sess.m_playerAcceptTask;
        m_friendList = sess.m_friendList;
        m_viewRole = sess.m_viewRole;
        m_viewMonster = sess.m_viewMonster;
        m_skillTime = sess.m_skillTime;
        m_playerGoods = sess.m_playerGoods;
        m_playerEqu = sess.m_playerEqu;
        m_playerMoney = sess.m_playerMoney;
        m_completeTask = sess.m_completeTask;
        memcpy(&m_position, &sess.m_position, sizeof(STR_PackPlayerPosition));
//        m_position = sess.m_position;
        m_lootGoods = sess.m_lootGoods;
        m_lootPosition = sess.m_lootPosition;
        m_interchage = sess.m_interchage;
        memcpy(m_goodsPosition, sess.m_goodsPosition, BAGCAPACITY);
        memcpy(&m_StartPos, &sess.m_StartPos, sizeof(STR_PlayerStartPos));
//        m_StartPos = sess.m_StartPos;
        m_taskGoods = sess.m_taskGoods;
        m_commonAttackTime = sess.m_commonAttackTime;
        m_tradeStatus = sess.m_tradeStatus;
    }

    void SendSkillEffectToViewRole(STR_PackSkillAimEffect* effect)
    {
        for(_umap_roleSock::iterator it = m_viewRole->begin(); it != m_viewRole->end(); it++)
        {
            it->second->Write_all(effect, sizeof(STR_PackSkillAimEffect));
        }
    }

    void SendHPToViewRole(STR_RoleAttribute* attr)
    {
        for(_umap_roleSock::iterator it = m_viewRole->begin(); it != m_viewRole->end(); it++)
        {
            it->second->Write_all(attr, sizeof(STR_RoleAttribute));
        }
    }

    void SendPositionToViewRole(STR_PackPlayerPosition* pos)
    {
        STR_PackOtherPlayerPosition OtherPos(m_roleid, pos);
        for(_umap_roleSock::iterator it = m_viewRole->begin(); it != m_viewRole->end(); it++)
        {
            it->second->Write_all(&OtherPos, sizeof(STR_PackOtherPlayerPosition));
        }
    }

    void ViewRoleAdd(hf_uint32 roleid, TCPConnection::Pointer conn)
    {
        (*m_viewRole)[roleid] = conn;
    }

    void ViewRoleDelete(hf_uint32 roleid)
    {
        m_viewRole->erase(roleid);
    }

//    typedef boost::array<char,40>    Buff;
//    Buff                    m_usrid;
    hf_char                 m_usrid[40];         //用户名
    hf_uint32               m_roleid;            //角色ID
    hf_double               m_publicCoolTime;    //公共冷却时间
    hf_double               m_skillUseTime;      //再次使用技能的时间
    STR_RoleInfo            m_roleInfo;          //角色攻击属性
    STR_PackRoleExperience  m_roleExp;           //角色经验
    STR_RoleBasicInfo       m_RoleBaseInfo;      //角色基本信息
    STR_BodyEquipment       m_BodyEqu;           //角色所穿戴装备
    umap_taskProcess        m_playerAcceptTask;  //玩家已接取的任务
    umap_friendList         m_friendList;        //好友列表
    umap_roleSock           m_viewRole;          //可视范围内的玩家
    umap_playerViewMonster  m_viewMonster;       //可视范围内的怪物
    umap_skillTime          m_skillTime;         //保存玩家所有技能的再次使用时间,此处和消耗品共用一个结构，也保存了消耗品再次使用时间
    umap_roleGoods          m_playerGoods;       //玩家背包其他物品
    umap_roleEqu            m_playerEqu;         //玩家背包装备
//    umap_roleEquAttr        m_playerEquAttr;     //玩家背包装备属性
    umap_roleMoney          m_playerMoney;       //玩家金币
    umap_completeTask       m_completeTask;      //玩家已经完成的任务
    STR_PackPlayerPosition  m_position;          //位置
    umap_lootGoods          m_lootGoods;         //掉落物品
    umap_lootPosition       m_lootPosition;      //掉落物品位置，时间
    boost::shared_ptr<Interchange> m_interchage;
    hf_char                 m_goodsPosition[BAGCAPACITY];   //保存玩家物品位置
    STR_PlayerStartPos      m_StartPos;          //保存玩家刷新数据起始点
    umap_taskGoods          m_taskGoods;         //保存玩家任务物品
    hf_double               m_commonAttackTime;  //下一次普通攻击的时间
    hf_uint8                m_tradeStatus;       //玩家交易状态，0表示未进入交易状态，1表示进入交易过程，未锁定，2表示已锁定，未交易确认，3表示交易确认

};



class SessionMgr:private boost::noncopyable,public boost::enable_shared_from_this<SessionMgr>
{
public:

    typedef boost::unordered_map<TCPConnection::Pointer,Session>    SessionMap;
    typedef boost::shared_ptr<SessionMap>           SessionPointer;
    typedef boost::shared_ptr<SessionMgr>            Pointer;

    typedef boost::unordered_map<string,TCPConnection::Pointer> _umap_nickSock;
    typedef boost::shared_ptr<_umap_nickSock> umap_nickSock;




//     void    SaveSession(TCPConnection::Pointer sock,char *name)
//     {
//        m_sessionsMtx.lock();
//        memcpy((*m_sessions)[sock].m_usrid, name, 32);
//        (*m_sessions)[sock].m_roleid = 0;
//        m_sessionsMtx.unlock();
////        SessionsAdd(sock, s);
////         (*ssmap)[sock] = s;
//     }

     void SaveSession(TCPConnection::Pointer sock, STR_PackPlayerPosition* playerPosition)
     {
         memcpy(&(*m_sessions)[sock].m_position, playerPosition, sizeof(STR_PackPlayerPosition));
//          SessionMgr::SessionMap* ssmap = m_sessions.get();
//          memcpy(&(*ssmap)[sock].m_position, playerPosition, sizeof(STR_PackPlayerPosition));
     }

     void SaveSession(TCPConnection::Pointer conn, hf_int32 roleid)
     {
         (*m_sessions)[conn].m_roleid = roleid;
         (*m_roleSock)[roleid] = conn;
//        SessionMgr::SessionMap *ssmap = m_sessions.get();
//         (*ssmap)[conn].m_roleid = roleid;

//        (*m_roleSock)[roleid] = conn;
     }

     void SaveSession(TCPConnection::Pointer sock, STR_RoleInfo* roleInfo)
     {
         memcpy(&(*m_sessions)[sock].m_roleInfo, roleInfo, sizeof(STR_RoleInfo));
//         SessionMgr::SessionMap *ssmap = m_sessions.get();
//         memcpy(&(*ssmap)[sock].m_roleInfo, roleInfo, sizeof(STR_RoleInfo));
     }


     void   RemoveSession( TCPConnection::Pointer conn)
     {
        Logger::GetLogger()->Info("Remove Session");
        m_sessions->erase(conn);
     }

    ~SessionMgr()               {}

    static    Pointer             Instance()
    {
         static Pointer           m_inst = Pointer(new SessionMgr());
//        if ( m_inst.get() == NULL )
//            m_inst = Pointer(new SessionMgr());
        return m_inst;
    }


    SessionPointer             GetSession()
    {
        return m_sessions;
    }

    umap_roleSock             GetRoleSock()
    {
        return m_roleSock;
    }

    umap_nickSock GetNickSock()
    {
        return m_nickSock;
    }

    umap_nickSock GetNameSock()
    {
        return m_nameSock;
    }

    umap_recoveryHP GetRecoveryHP()
    {
        return m_recoveryHP;
    }

    umap_recoveryMagic GetRecoveryMagic()
    {
        return m_recoveryMagic;
    }

    umap_recoveryHPMagic GetRecoveryHPMagic()
    {
        return m_recoveryHPMagic;
    }

    void SessionsNameAdd(TCPConnection::Pointer conn, hf_char* name)
    {
        m_sessNameMtx.lock();
        memcpy((*m_sessions)[conn].m_usrid, name, 32);
        (*m_sessions)[conn].m_roleid = 0;
        Logger::GetLogger()->Debug("m_sessions add end:%d",m_sessions->size());
        (*m_nameSock)[name] = conn;
        Logger::GetLogger()->Debug("m_nameSock add end:%d", m_nameSock->size());
        m_sessNameMtx.unlock();
    }

    void SessionsNameErase(TCPConnection::Pointer conn)
    {
        m_sessNameMtx.lock();
        m_nameSock->erase((*m_sessions)[conn].m_usrid);
        Logger::GetLogger()->Debug("m_nameSock delete end:%d", m_nameSock->size());

        m_sessions->erase(conn);
        Logger::GetLogger()->Debug("m_sessions delete end:%d",m_sessions->size());
        m_sessNameMtx.unlock();
    }

    void RoleNickAdd(hf_uint32 roleid, hf_char* nick, TCPConnection::Pointer conn)
    {
        m_roleNickMtx.lock();
        (*m_roleSock)[roleid] = conn;
        Logger::GetLogger()->Debug("m_roleSock add end:%d",m_roleSock->size());
        (*m_nickSock)[nick] = conn;
        Logger::GetLogger()->Debug("m_nickSock add end:%d",m_nickSock->size());
        m_roleNickMtx.unlock();
    }

    void RoleNickErase(hf_uint32 roleid, TCPConnection::Pointer conn)
    {
        m_roleNickMtx.lock();
        m_roleSock->erase(roleid);
        Logger::GetLogger()->Debug("m_roleSock delete end:%d",m_roleSock->size());
        m_nickSock->erase((*m_sessions)[conn].m_RoleBaseInfo.Nick);
        Logger::GetLogger()->Debug("m_nickSock delete end:%d",m_nickSock->size());
        m_roleNickMtx.unlock();
    }

    void RecoveryHPAdd(TCPConnection::Pointer conn, STR_RecoveryHP hp)
    {
        m_ReHPMtx.lock();
        (*m_recoveryHP)[conn] = hp;
        m_ReHPMtx.unlock();
    }

    void RecoveryHPDelete(TCPConnection::Pointer conn)
    {
        m_ReHPMtx.lock();
        m_recoveryHP->erase(conn);
        m_ReHPMtx.unlock();
    }
    void RecoveryMagicAdd(TCPConnection::Pointer conn, STR_RecoveryMagic magic)
    {
        m_ReMagicMtx.lock();
        (*m_recoveryMagic)[conn] = magic;
        m_ReMagicMtx.unlock();
    }

    void RecoveryMagicDelete(TCPConnection::Pointer conn)
    {
        m_ReMagicMtx.lock();
        m_recoveryHP->erase(conn);
        m_ReMagicMtx.unlock();
    }

    void RecoveryHPMagicAdd(TCPConnection::Pointer conn, STR_RecoveryHPMagic hpMagic)
    {
        m_ReHPMagicMtx.lock();
        (*m_recoveryHPMagic)[conn] = hpMagic;
        m_ReHPMagicMtx.unlock();
    }

    void RecoveryHPMagicDelete(TCPConnection::Pointer conn)
    {
        m_ReHPMagicMtx.lock();
        m_recoveryHPMagic->erase(conn);
        m_ReHPMagicMtx.unlock();
    }



private:

    SessionMgr():
      m_sessions(new SessionMap()),
      m_nameSock(new _umap_nickSock),
      m_roleSock(new _umap_roleSock),
      m_nickSock(new _umap_nickSock),   
      m_recoveryHP(new _umap_recoveryHP),
      m_recoveryMagic(new _umap_recoveryMagic),
      m_recoveryHPMagic(new _umap_recoveryHPMagic)
    {
        cout<<"\n===================Create SessionMgr================="<<endl;
    }

    SessionPointer              m_sessions;
    umap_nickSock               m_nameSock;

    umap_roleSock               m_roleSock;
    umap_nickSock               m_nickSock;

    umap_recoveryHP             m_recoveryHP;      //使用延时恢复血量的消耗品
    umap_recoveryMagic          m_recoveryMagic;   //使用延时恢复魔法的消耗品
    umap_recoveryHPMagic        m_recoveryHPMagic; //使用延时恢复血量魔法的消耗品

    boost::mutex                m_sessNameMtx;
    boost::mutex                m_roleNickMtx;
//    boost::mutex                m_nickMtx;
//    boost::mutex                m_nameMtx;
    boost::mutex                m_ReHPMtx;
    boost::mutex                m_ReMagicMtx;
    boost::mutex                m_ReHPMagicMtx;
};


#endif // SESSION
