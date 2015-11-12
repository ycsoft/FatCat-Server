#ifndef GAMEATTACK_H
#define GAMEATTACK_H

#include "Game/postgresqlstruct.h"
#include "NetWork/tcpconnection.h"
#include "Game/cmdtypes.h"
#include "Monster/monsterstruct.h"
#include "Game/rolestruct.h"

/**
 * @brief The GameAttack class
 * 主要完成玩家与玩家，玩家与怪物之间的攻击。
 */

class GameAttack
{
public:
    GameAttack();
    ~GameAttack();

    //角色延时类技能伤害
    void RoleSkillAttack();
    //删除过了时间的掉落物品
    void DeleteOverTimeGoods();

    //攻击点
    void AttackPoint(TCPConnection::Pointer conn, STR_PackUserAttackPoint* t_attack);
    //攻击目标
    void AttackAim(TCPConnection::Pointer conn, STR_PackUserAttackAim* t_attack);

    //查询所有技能信息
    void QuerySkillInfo();
    //发送玩家可以使用的技能
    void SendPlayerSkill(TCPConnection::Pointer conn);
    //计算技能产生的伤害
    hf_uint32 CalDamage(STR_PackSkillInfo* skillInfo, STR_RoleInfo* roleInfo, STR_MonsterAttackInfo* monster, hf_uint8* type);

    //伤害处理函数
    void DamageDealWith(TCPConnection::Pointer conn, STR_PackDamageData* damage, STR_MonsterInfo* monster);

    //怪物死亡处理函数
    void MonsterDeath(TCPConnection::Pointer conn, STR_MonsterInfo* monster);

    //从角色可视范围中删除该怪物
    void RoleViewDeleteMonster(hf_uint32 monsterID);

    //判断技能是否过了冷却时间
    hf_uint8 SkillCoolTime(TCPConnection::Pointer conn, hf_double timep, hf_uint32 skillID);


    //技能攻击怪物函数
    //自己为圆心
    void AimItselfCircle(TCPConnection::Pointer conn, STR_PackSkillInfo* skillInfo, hf_double timep);
    //怪物为目标
    void AimMonster(TCPConnection::Pointer conn, STR_PackSkillInfo* skillInfo, double timep, hf_uint32 AimID);
    //怪物为圆心
    void AimMonsterCircle(TCPConnection::Pointer conn, STR_PackSkillInfo* skillInfo, double timep, hf_uint32 AimID);

    //普通攻击函数
    void CommonAttackRole(TCPConnection::Pointer conn, STR_PackUserAttackAim* t_attack);   //普通攻击角色
    void CommonAttackMonster(TCPConnection::Pointer conn, STR_PackUserAttackAim* t_attack);//普通攻击怪物


    //发送玩家血量给周围玩家
    void SendRoleHpToViewRole(TCPConnection::Pointer conn, STR_RoleAttribute* roleAttr);
    //返回当前时间
    hf_double GetCurrentTime()
    {
        struct timeval start;
        gettimeofday( &start, NULL );
        return (hf_double)start.tv_sec + (hf_double)start.tv_usec/1000000;
    }


    umap_roleAttackAim GetRoleAttackRole()
    {
        return m_attackRole;
    }

    umap_roleAttackAim GetRoleAttackMonster()
    {
        return m_attackMonster;
    }

    umap_roleAttackPoint GetRoleAttackPoint()
    {
        return m_attackPoint;
    }

    umap_skillInfo* GetSkillInfo()
    {
        return m_skillInfo;
    }

private:
    umap_roleAttackAim       m_attackRole;      //玩家使用技能攻击角色
    umap_roleAttackAim       m_attackMonster;   //玩家使用技能攻击怪物
    umap_roleAttackPoint     m_attackPoint;     //玩家使用技能攻击地图上一个点
    umap_skillInfo*          m_skillInfo;       //技能信息
};

#endif // GAMEATTACK_H
