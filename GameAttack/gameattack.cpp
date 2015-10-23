#include "OperationPostgres/operationpostgres.h"
#include "GameTask/gametask.h"
#include "memManage/diskdbmanager.h"
#include "Game/getdefinevalue.h"
#include "GameTask/gametask.h"
#include "Monster/monster.h"
#include "Game/session.hpp"
#include "gameattack.h"
#include "server.h"

GameAttack::GameAttack()
    :m_attackRole(new _umap_roleAttackAim)
    ,m_attackMonster(new _umap_roleAttackAim)    
    ,m_attackPoint(new _umap_roleAttackPoint)
    ,m_skillInfo(new umap_skillInfo)
{

}

GameAttack::~GameAttack()
{
    delete m_skillInfo;
}

//攻击点
void GameAttack::AttackPoint(TCPConnection::Pointer conn, STR_PackUserAttackPoint* t_attack)
{
//    Server* srv = Server::GetInstance();


//    srv->free(t_attack);
}

//攻击目标
void GameAttack::AttackAim(TCPConnection::Pointer conn, STR_PackUserAttackAim* t_attack)
{
//    Server* srv = Server::GetInstance();
    if(t_attack->SkillID >= 5001)  //技能攻击
    {
        umap_skillInfo::iterator it = m_skillInfo->find(t_attack->SkillID);
        if(it == m_skillInfo->end())
        {
            Logger::GetLogger()->Debug("没有这个技能");
//            srv->free(t_attack);
            return;
        }
        hf_double timep = GameAttack::GetCurrentTime();

        STR_PackSkillResult t_skillResult;
        t_skillResult.SkillID = t_attack->SkillID;
        if(SkillCoolTime(conn, timep, t_attack->SkillID) == 0)
        {
           t_skillResult.result = SKILL_NOTCOOL;
           conn->Write_all(&t_skillResult, sizeof(STR_PackSkillResult));
//           srv->free(t_attack);
           return;
        }
        if(it->second.SkillRangeID == 2)  //自己为圆心
        {
            AimItselfCircle(conn, &it->second, timep);
        }
        else if(it->second.SkillRangeID == 1) //目标
        {
            if(t_attack->AimID > 100000000)
            {

            }
            else if(30000000 <= t_attack->AimID && t_attack->AimID < 40000000)
            {
                AimMonster(conn,&it->second, timep, t_attack->AimID);
            }
        }
        else if(it->second.SkillRangeID == 3) //目标为圆心
        {
            if(t_attack->AimID > 100000000)
            {

            }
            else if(30000000 <= t_attack->AimID && t_attack->AimID < 40000000)
            {
                AimMonsterCircle(conn,&it->second, timep,t_attack->AimID);

            }
        }
    }
    else //普通攻击
    {
        if(t_attack->AimID > 100000000) //攻击玩家
        {
            CommonAttackRole(conn, t_attack);
        }
        else if(30000000 <= t_attack->AimID && t_attack->AimID < 40000000)
        {
            CommonAttackMonster(conn, t_attack);
        }
    }

//    srv->free(t_attack);
}

void GameAttack::CommonAttackRole(TCPConnection::Pointer conn, STR_PackUserAttackAim* t_attack)
{

}

//普通攻击怪物
void GameAttack::CommonAttackMonster(TCPConnection::Pointer conn, STR_PackUserAttackAim* t_attack)
{
//    Server *srv = Server::GetInstance();
    umap_monsterAttackInfo* t_monsterAttack = Server::GetInstance()->GetMonster()->GetMonsterAttack();
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();

    STR_PackDamageData t_damageData;
    umap_playerViewMonster t_viewMonster = (*smap)[conn].m_viewMonster;
    _umap_playerViewMonster::iterator it = t_viewMonster->find(t_attack->AimID);
    if(it == t_viewMonster->end())
    {
        return;
    }

    t_damageData.AimID = t_attack->AimID;
    t_damageData.AttackID = (*smap)[conn].m_roleid;
    t_damageData.TypeID = t_attack->SkillID;


    umap_monsterInfo u_monsterInfo = Server::GetInstance()->GetMonster()->GetMonsterBasic();
    STR_MonsterInfo* t_monsterBasicInfo = &(*u_monsterInfo)[it->first];
    STR_PackPlayerPosition* t_AttacketPosition = &((*smap)[conn].m_position);

    hf_float dx = t_monsterBasicInfo->monster.Current_x - t_AttacketPosition->Pos_x;
    hf_float dy = t_monsterBasicInfo->monster.Current_y - t_AttacketPosition->Pos_y;
    hf_float dz = t_monsterBasicInfo->monster.Current_z - t_AttacketPosition->Pos_z;
    //判断是否在攻击范围内
    if(dx*dx + dy*dy + dz*dz > PlayerAttackView*PlayerAttackView)
    {
        t_damageData.Flag = NOT_ATTACKVIEW;  //不在攻击范围
        conn->Write_all(&t_damageData, sizeof(STR_PackDamageData));
        return;
    }


//    //判断方向是否可攻击
    if((dx*cos((t_AttacketPosition->Direct)*PI/180) + dz*sin((t_AttacketPosition->Direct)*PI/180))/sqrt(dx*dx + dz*dz) < 0)
    {
        t_damageData.Flag = OPPOSITE_DIRECT;
        conn->Write_all(&t_damageData, sizeof(STR_PackDamageData));
        return;
    }

    STR_RoleInfo* t_AttacketInfo = &((*smap)[conn].m_roleInfo);
    hf_float t_probHit = t_AttacketInfo->Hit_Rate*1;

    if(t_probHit*100 < rand()%100) //未命中
    {
        t_damageData.Flag = NOT_HIT;
        conn->Write_all(&t_damageData, sizeof(STR_PackDamageData));
        return;
    }

    //查找怪物攻击属性
    STR_MonsterAttackInfo* t_monsterAttackInfo = &(*t_monsterAttack)[t_monsterBasicInfo->monster.MonsterTypeID];
    hf_uint8 t_level = t_monsterAttackInfo->Level;
    if(t_attack->SkillID == PhyAttackSkillID) //物理攻击
    {
        t_damageData.Damage = t_AttacketInfo->PhysicalAttack* GetDamage_reduction(t_level)/t_monsterAttackInfo->PhysicalDefense;
    }
    else if(t_attack->SkillID == MagicAttackSkillID)//魔法攻击
    {
       t_damageData.Damage = t_AttacketInfo->MagicAttack* GetDamage_reduction(t_level)/t_monsterAttackInfo->MagicDefense;
    }

    if(t_AttacketInfo->Crit_Rate*100 >= rand()%100)//暴击
    {
        t_damageData.Flag = CRIT;
        t_damageData.Damage *= 1.5;
    }
    else //未暴击
    {
        t_damageData.Flag = NOT_CRIT;
    }
    DamageDealWith(conn, &t_damageData, t_monsterBasicInfo);
}

//计算伤害
hf_uint32 GameAttack::CalDamage(STR_PackSkillInfo* skillInfo, STR_RoleInfo* roleInfo, STR_MonsterAttackInfo* monster, hf_uint8* type)
{
    if(skillInfo->PhysicalHurt > 0 && skillInfo->MagicHurt > 0)
    {
        *type = PhyMagAttackSkillID;
        return skillInfo->PhysicalHurt + skillInfo->PhyPlus*roleInfo->PhysicalAttack - monster->PhysicalDefense + skillInfo->MagicHurt + skillInfo->MagPlus*roleInfo->MagicAttack - monster->MagicDefense;
    }
    else if(skillInfo->PhysicalHurt > 0 && skillInfo->MagicHurt == 0)
    {
        *type = PhyAttackSkillID;
        return skillInfo->PhysicalHurt + skillInfo->PhyPlus*roleInfo->PhysicalAttack - monster->PhysicalDefense;
    }
    else if(skillInfo->PhysicalHurt == 0 && skillInfo->MagicHurt > 0)
    {
        *type = MagicAttackSkillID;
        return skillInfo->MagicHurt + skillInfo->MagPlus*roleInfo->MagicAttack - monster->MagicDefense;
    }
}

void GameAttack::RoleViewDeleteMonster(hf_uint32 monsterID)
{
    SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();
    _umap_viewRole* t_viewRole = &(*(Server::GetInstance()->GetMonster()->GetMonsterViewRole()))[monsterID];  //得到能看到这个怪物的玩家
    umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
    for(_umap_viewRole::iterator it = t_viewRole->begin(); it != t_viewRole->end(); it++)
    {
        _umap_roleSock::iterator role_it = t_roleSock->find(it->first);
        if(role_it != t_roleSock->end())
        {
            umap_playerViewMonster   t_viewMonster = (*smap)[role_it->second].m_viewMonster;
            _umap_playerViewMonster::iterator iter = t_viewMonster->find(monsterID);
            if(iter == t_viewMonster->end())
            {
                Logger::GetLogger()->Error("怪物看到玩家，玩家没有看到怪");
                continue;
            }
            t_viewMonster->erase(iter);
        }
    }
}

//技能处理函数
void GameAttack::DamageDealWith(TCPConnection::Pointer conn, STR_PackDamageData* damage, STR_MonsterInfo* monster)
{
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    //发送产生的伤害
    cout << "wait skill" << damage->Damage << endl;
    conn->Write_all(damage, sizeof(STR_PackDamageData));

    STR_PackMonsterAttrbt t_monsterBt;
    t_monsterBt.MonsterID = monster->monster.MonsterID;
    hf_uint32 roleid = (*smap)[conn].m_roleid;
    t_monsterBt.HP = monster->ReduceHp(roleid, damage->Damage);

    umap_monsterViewRole  monsterViewRole = Server::GetInstance()->GetMonster()->GetMonsterViewRole();


    hf_uint32* t_hatredValue = &((*monsterViewRole)[t_monsterBt.MonsterID])[roleid];

    *t_hatredValue += damage->Damage;

    cout << "攻击前：" << monster->hatredRoleid << endl;
    if(monster->hatredRoleid != roleid)
    {
        if(monster->hatredRoleid != 0)
        {
            if(*t_hatredValue > ((*monsterViewRole)[t_monsterBt.MonsterID])[monster->hatredRoleid])
            {
                monster->ChangeHatredRoleid(roleid);
            }
        }
        else
        {
            monster->ChangeHatredRoleid(roleid);
        }
    }

    cout << "攻击后:" << monster->hatredRoleid << endl;

    cout << "hatredRoleid" << monster->hatredRoleid << ",hatred:" << ((*monsterViewRole)[t_monsterBt.MonsterID])[monster->hatredRoleid] << endl;
    //发送怪物当前血量给可视范围内的玩家
    Server::GetInstance()->GetMonster()->SendMonsterHPToViewRole(&t_monsterBt);
    if(t_monsterBt.HP == 0)
    {
        //怪物死亡，清除该怪物的仇恨值，仇恨对象
        monster->hatredRoleid = 0;

        //怪物死亡，发送奖励经验，玩家经验，查找掉落物品
        MonsterDeath(conn, monster);
        //从玩家可视范围内的怪物列表中删除该怪物
        RoleViewDeleteMonster(t_monsterBt.MonsterID);
        //删除该怪物可视范围内的玩家

        (*monsterViewRole)[t_monsterBt.MonsterID].clear();
//        monsterViewRole->erase(t_monsterBt.MonsterID);
    }
}


//怪物死亡处理函数
void GameAttack::MonsterDeath(TCPConnection::Pointer conn, STR_MonsterInfo* monster)
{
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    Server* srv = Server::GetInstance();
    //查找此任务是否为任务进度里要打的怪，如果是，更新任务进度
    srv->GetGameTask()->UpdateAttackMonsterTaskProcess(conn, monster->monster.MonsterTypeID);

    //前15级怪物死亡不掉经验
     if(monster->monster.Level >= 15)
     {
        STR_PackRewardExperience t_RewardExp;
        t_RewardExp.ID = monster->monster.MonsterID;
        t_RewardExp.Experience = GetRewardExperience(monster->monster.Level);
        conn->Write_all(&t_RewardExp, sizeof(STR_PackRewardExperience));

        STR_PackRoleExperience* t_RoleExp = &(*smap)[conn].m_roleExp;

        //玩家升级
        if(t_RoleExp->CurrentExp + t_RewardExp.Experience >= t_RoleExp->UpgradeExp)
        {
            t_RoleExp->Level += 1;
            Server::GetInstance()->GetOperationPostgres()->PushUpdateLevel((*smap)[conn].m_roleid, t_RoleExp->Level);
            srv->GetGameTask()->UpdateAttackUpgradeTaskProcess(conn, t_RoleExp->Level);
            t_RoleExp->UpgradeExp = GetUpgradeExprience(t_RoleExp->Level);
            t_RoleExp->CurrentExp = t_RoleExp->CurrentExp + t_RewardExp.Experience - t_RoleExp->UpgradeExp;
        }
        else
        {
            t_RoleExp->CurrentExp = t_RoleExp->CurrentExp + t_RewardExp.Experience;
            memcpy(&(*smap)[conn].m_roleExp, &t_RoleExp, sizeof(STR_PackRoleExperience));
        }
        Server::GetInstance()->GetOperationPostgres()->PushUpdateExp((*smap)[conn].m_roleid, t_RoleExp->UpgradeExp);
        conn->Write_all(t_RoleExp, sizeof(STR_PackRoleExperience));
      }

    umap_monsterLoot* t_monsterLoot = Server::GetInstance()->GetMonster()->GetMonsterLoot();
    umap_monsterLoot::iterator iter = t_monsterLoot->find(monster->monster.MonsterTypeID);
    if(iter != t_monsterLoot->end())
    {
        STR_LootGoods t_lootGoods;
        hf_char* buff = (hf_char*)srv->malloc();
        hf_int32 i = 0;
        umap_lootGoods lootGoods = (*smap)[conn].m_lootGoods;

        t_lootGoods.Count = GetRewardMoney(monster->monster.Level);;
        //暂时只取奖励的金钱，后面会加一些计算公式，计算后奖励的金钱可能为0

        if(t_lootGoods.Count > 0)
        {
            t_lootGoods.LootGoodsID = Money_1;
            vector<STR_LootGoods> t_vec;
            t_vec.push_back(t_lootGoods);
            (*lootGoods)[monster->monster.MonsterID] = t_vec;
            memcpy(buff + sizeof(STR_PackHead) + sizeof(STR_LootGoodsPos) + i* sizeof(STR_LootGoods), &t_lootGoods, sizeof(STR_LootGoods));
            i++;
        }

        //可能掉落多个物品，分别判断
        for(vector<STR_MonsterLoot>::iterator vec = iter->second.begin(); vec != iter->second.end(); vec++)
        {
            //掉落条件判断

            //掉落可能性判断
            if(vec->LootProbability*100 >= rand()%100)
            {
                t_lootGoods.LootGoodsID = vec->LootGoodsID;
                t_lootGoods.Count = vec->Count;
                memcpy(buff + sizeof(STR_PackHead) + sizeof(STR_LootGoodsPos) + i* sizeof(STR_LootGoods), &t_lootGoods, sizeof(STR_LootGoods));
                i++;

                _umap_lootGoods::iterator it = lootGoods->find(monster->monster.MonsterID); //保存掉落物品
                if(it != lootGoods->end())
                {
                    it->second.push_back(t_lootGoods);
                }
                else
                {
                    vector<STR_LootGoods> t_vec;
                    t_vec.push_back(t_lootGoods);
                    (*lootGoods)[monster->monster.MonsterID] = t_vec;
                }
            }
        }                

        STR_PackHead t_packHead;
        t_packHead.Flag = FLAG_LootGoods;
        t_packHead.Len = sizeof(STR_LootGoodsPos) + i*sizeof(STR_LootGoods);

        STR_LootGoodsPos t_PacklootGoods;
        t_PacklootGoods.Pos_x = monster->monster.Current_x;
        t_PacklootGoods.Pos_y = monster->monster.Current_y;
        t_PacklootGoods.Pos_z = monster->monster.Current_z;
        t_PacklootGoods.MapID = monster->monster.MapID;
        t_PacklootGoods.GoodsFlag = monster->monster.MonsterID * 10;

        LootPositionTime t_lootPositionTime;
        time_t timep;
        time(&timep);
        t_lootPositionTime.timep = (hf_uint32)timep;
        t_lootPositionTime.continueTime = GOODS_CONTINUETIME;

        memcpy(&t_lootPositionTime.goodsPos, &t_PacklootGoods, sizeof(STR_LootGoodsPos));
        (*((*smap)[conn].m_lootPosition))[t_PacklootGoods.GoodsFlag] = t_lootPositionTime; //保存掉落物品时间位置
        memcpy(buff, &t_packHead, sizeof(STR_PackHead));
        memcpy(buff + sizeof(STR_PackHead), &t_PacklootGoods, sizeof(STR_LootGoodsPos));
        conn->Write_all(buff, t_packHead.Len + sizeof(STR_PackHead));
        srv->free(buff);
    }
}


//角色技能伤害
void GameAttack::RoleSkillAttack()
{
    SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();
    umap_roleSock t_roleSock = SessionMgr::Instance()->GetRoleSock();
    umap_monsterAttackInfo* t_monsterAttack = Server::GetInstance()->GetMonster()->GetMonsterAttack();
    umap_monsterInfo u_monsterInfo = Server::GetInstance()->GetMonster()->GetMonsterBasic();
    STR_PackDamageData t_damageData;

    //遍历m_attackMonster,m_attackRole,m_attackPoint 根据时间判断，计算伤害发送给玩家
    while(1)
    {
        hf_double timep = GameAttack::GetCurrentTime();
        for(_umap_roleAttackAim::iterator it = m_attackMonster->begin(); it != m_attackMonster->end();)
        {
            if(timep < (it->second).HurtTime) //时间没到
            {
                it++;
                continue;
            }

            _umap_roleSock::iterator iter = t_roleSock->find(it->first);
            if(iter == t_roleSock->end())  //攻击的玩家不在线
            {
                _umap_roleAttackAim::iterator aim = it;
                it++;
                m_attackMonster->erase(aim);
                continue;
            }

            STR_PackSkillAimEffect t_skillEffect(it->second.AimID,it->second.SkillID,it->first);
            iter->second->Write_all(&t_skillEffect, sizeof(STR_PackSkillAimEffect));  //发送施法效果

            umap_playerViewMonster t_viewMonster = (*smap)[iter->second].m_viewMonster;
            STR_PackPlayerPosition* t_pos = &(*smap)[iter->second].m_position;
            STR_PackSkillInfo* t_skillInfo = &((*m_skillInfo)[it->second.SkillID]);
            STR_RoleInfo* t_roleInfo = &(*smap)[iter->second].m_roleInfo;
            if(t_skillInfo->SkillRangeID == 1)  //目标
            {
                _umap_playerViewMonster::iterator monster = t_viewMonster->find(it->second.AimID);
                if(monster == t_viewMonster->end())
                {
                    continue;
                }

                STR_MonsterInfo* t_monsterInfo = &(*u_monsterInfo)[it->second.AimID];

                 STR_MonsterAttackInfo* t_monsterAttackInfo = &(*t_monsterAttack)[it->second.AimID];
                 hf_float dx = t_monsterInfo->monster.Current_x - t_pos->Pos_x;
                 hf_float dy = t_monsterInfo->monster.Current_y - t_pos->Pos_y;
                 hf_float dz = t_monsterInfo->monster.Current_z - t_pos->Pos_z;
                 if( (dx*dx + dy*dy + dz*dz) >= t_skillInfo->NearlyDistance * t_skillInfo->NearlyDistance &&(dx*dx + dy*dy + dz*dz) <= t_skillInfo->FarDistance * t_skillInfo->FarDistance)
                 {
                     t_damageData.Damage = CalDamage(t_skillInfo, t_roleInfo, t_monsterAttackInfo, &t_damageData.TypeID);
                     cout << "wait Skill" << t_damageData.Damage << endl;
                     DamageDealWith(iter->second, &t_damageData, t_monsterInfo); //发送计算伤害
                 }
            }
            else if(t_skillInfo->SkillRangeID == 2) //自己为圆心
            {
                for(_umap_playerViewMonster::iterator monster = t_viewMonster->begin(); monster != t_viewMonster->end(); monster++)
                {

                     STR_MonsterAttackInfo* t_monsterAttackInfo = &(*t_monsterAttack)[monster->first];
                     STR_MonsterInfo* t_monsterInfo = &(*u_monsterInfo)[monster->first];
                     hf_float dx = t_monsterInfo->monster.Current_x - t_pos->Pos_x;
                     hf_float dy = t_monsterInfo->monster.Current_y - t_pos->Pos_y;
                     hf_float dz = t_monsterInfo->monster.Current_z - t_pos->Pos_z;
                     if( (dx*dx + dy*dy + dz*dz) >= t_skillInfo->NearlyDistance * t_skillInfo->NearlyDistance &&(dx*dx + dy*dy + dz*dz) <= t_skillInfo->FarDistance * t_skillInfo->FarDistance)
                     {
                         t_damageData.Damage = CalDamage(t_skillInfo, t_roleInfo, t_monsterAttackInfo, &t_damageData.TypeID);                         
                         DamageDealWith(iter->second, &t_damageData, t_monsterInfo); //发送计算伤害
                     }
                }
            }
            else if( t_skillInfo->SkillRangeID == 3) //目标为圆心
            {
                _umap_playerViewMonster::iterator monster = t_viewMonster->find(it->second.AimID);
                if(monster == t_viewMonster->end())
                {
                    continue;
                }
                STR_MonsterInfo* t_monsterInfo = &(*u_monsterInfo)[monster->first];
                hf_float dx = t_monsterInfo->monster.Current_x - t_pos->Pos_x;
                hf_float dy = t_monsterInfo->monster.Current_y - t_pos->Pos_y;
                hf_float dz = t_monsterInfo->monster.Current_z - t_pos->Pos_z;
                if( (dx*dx + dy*dy + dz*dz) < t_skillInfo->NearlyDistance * t_skillInfo->NearlyDistance || (dx*dx + dy*dy + dz*dz) > t_skillInfo->FarDistance * t_skillInfo->FarDistance)
                {
                    continue;
                }
                for(_umap_playerViewMonster::iterator monster = t_viewMonster->begin(); monster != t_viewMonster->end(); monster++)
                {
                    if(monster->first == t_monsterInfo->monster.MonsterID)
                    {
                        continue;
                    }
                    STR_MonsterAttackInfo* t_monsterAttackInfo = &(*t_monsterAttack)[monster->first];
                    STR_MonsterInfo* t_monster = &(*u_monsterInfo)[monster->first];

                    hf_float dx = t_monsterInfo->monster.Current_x - t_monster->monster.Current_x;
                    hf_float dy = t_monsterInfo->monster.Current_y - t_monster->monster.Current_x;
                    hf_float dz = t_monsterInfo->monster.Current_z - t_monster->monster.Current_x;
                    if( (dx*dx + dy*dy + dz*dz) >= t_skillInfo->NearlyDistance * t_skillInfo->NearlyDistance &&(dx*dx + dy*dy + dz*dz) <= t_skillInfo->FarDistance * t_skillInfo->FarDistance)
                    {
                      t_damageData.Damage = CalDamage(t_skillInfo, t_roleInfo, t_monsterAttackInfo, &t_damageData.TypeID);
                      DamageDealWith(iter->second, &t_damageData, t_monster); //发送计算伤害
                    }
                }
            }
            _umap_roleAttackAim::iterator aim = it;
            it++;
            m_attackMonster->erase(aim);
        }
//        for(_umap_roleAttackAim::iterator it = m_attackRole->begin(); it != m_attackRole->end(); it++)
//        {  //攻击角色

//        }

//        for(_umap_roleAttackPoint::iterator it = m_attackPoint->begin(); it != m_attackPoint->end(); it++)
//        { //攻击点

//        }
    }
}


//查询所有技能信息
void GameAttack::QuerySkillInfo()
{
    DiskDBManager *db = Server::GetInstance()->getDiskDB();

    if ( db->GetSkillInfo(m_skillInfo) < 0 )
    {
        Logger::GetLogger()->Error("Query TaskDialogue error");
        return;
    }
}

//发送玩家可以使用的技能
void GameAttack::SendPlayerSkill(TCPConnection::Pointer conn)
{
    Server* srv = Server::GetInstance();
    hf_char* buff = (hf_char*)srv->malloc();

    STR_PlayerSkill t_skill;
    STR_PackHead t_packHead;
    hf_uint32 i = 0;
    for(umap_skillInfo::iterator it = m_skillInfo->begin(); it != m_skillInfo->end(); it++)
    {
        t_skill.SkillID = it->second.SkillID;
        t_skill.CoolTime = it->second.CoolTime;
        t_skill.CastingTime = it->second.CastingTime;
        t_skill.LeadTime = it->second.LeadTime;
        memcpy(buff + sizeof(STR_PackHead) + sizeof(STR_PlayerSkill)*i, &t_skill, sizeof(STR_PlayerSkill));
        i++;
    }
    if(i != 0)
    {
        t_packHead.Len = sizeof(STR_PlayerSkill) * i;
        t_packHead.Flag = FLAG_CanUsedSkill;
        memcpy(buff, &t_packHead, sizeof(STR_PackHead));
        conn->Write_all(buff, t_packHead.Len + sizeof(STR_PackHead));
    }
    srv->free(buff);
}

//判断技能是否过了冷却时间
hf_uint8 GameAttack::SkillCoolTime(TCPConnection::Pointer conn, hf_double timep, hf_uint32 skillID)
{
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    hf_double t_skillUseTime = (*smap)[conn].m_skillUseTime;
    umap_skillTime t_skillTime = (*smap)[conn].m_skillTime;

    _umap_skillTime::iterator iter = t_skillTime->find(skillID);
    if(iter != t_skillTime->end()) //这个技能使用过
    {
        if(t_skillUseTime > timep || (*smap)[conn].m_publicCoolTime > timep || iter->second > timep)
            return 0;
    }
    else
    {       
        if(t_skillUseTime > timep || (*smap)[conn].m_publicCoolTime > timep)//没过技能冷却时间或没过公共冷却时间
            return 0;
    }
    return 1;
}

//以自己为圆心
void GameAttack::AimItselfCircle(TCPConnection::Pointer conn, STR_PackSkillInfo* skillInfo, hf_double timep)
{
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    Server* srv = Server::GetInstance();
//    umap_monsterBasicInfo t_viewMonster = (*smap)[conn].m_viewMonster;
//    //取攻击者角色的属性信息
//    STR_RoleInfo* t_AttacketInfo = &((*smap)[conn].m_roleInfo);
//    umap_monsterAttackInfo* t_monsterAttack = srv->GetMonster()->GetMonsterAttack();

//    STR_PackPlayerPosition* t_pos = &(*smap)[conn].m_position;
//    STR_PackSkillResult t_skillResult;
//    STR_PackDamageData t_damageData;

//    (*smap)[conn].m_publicCoolTime = timep + PUBLIC_COOLTIME; //保存玩家使用技能的公共冷却时间
//    (*smap)[conn].m_skillUseTime = timep + skillInfo->CoolTime + skillInfo->CastingTime;  //保存技能的冷却时间
//    t_skillResult.result = SKILL_SUCCESS;
//    conn->Write_all(&t_skillResult, sizeof(STR_PackSkillResult));

//    if(skillInfo->CastingTime > 0)   //延时类技能
//    {
//        RoleAttackAim t_attackAim;
//        t_attackAim.AimID = 0;
//        t_attackAim.HurtTime = timep + skillInfo->CastingTime;
//        t_attackAim.SkillID = skillInfo->SkillID;

//        (*m_attackMonster)[(*smap)[conn].m_roleid] = t_attackAim;
//        (*smap)[conn].m_skillUseTime = t_attackAim.HurtTime + skillInfo->CoolTime;
//        return;
//    }
//    STR_PackSkillAimEffect t_skillEffect(t_damageData.AimID,skillInfo->SkillID,t_damageData.AttackID);
//    conn->Write_all(&t_skillEffect, sizeof(STR_PackSkillAimEffect));  //发送施法效果

//    for(_umap_monsterBasicInfo::iterator it = t_viewMonster->begin(); it != t_viewMonster->end(); it++)
//    {
//         STR_MonsterAttackInfo* t_monsterAttackInfo = &(*t_monsterAttack)[it->first];

//         hf_float dx = it->second.Current_x - t_pos->Pos_x;
//         hf_float dy = it->second.Current_y - t_pos->Pos_y;
//         hf_float dz = it->second.Current_z - t_pos->Pos_z;

//         if( (dx*dx + dy*dy + dz*dz) < skillInfo->NearlyDistance * skillInfo->NearlyDistance ||
//                 (dx*dx + dy*dy + dz*dz) > skillInfo->FarDistance * skillInfo->FarDistance) //不在攻击范围
//         {
//             t_skillResult.result = NOT_ATTACKVIEW;
//             conn->Write_all(&t_skillResult, sizeof(STR_PackSkillResult));
//             return;
//         }
//        if(skillInfo->CastingTime == 0)//无延时类技能
//        {
//            t_damageData.Damage = CalDamage(skillInfo, t_AttacketInfo, t_monsterAttackInfo, &t_damageData.TypeID);
//            DamageDealWith(conn, &t_damageData, it->first); //发送计算伤害
//        }
//    }
}


//怪物为目标
void GameAttack::AimMonster(TCPConnection::Pointer conn, STR_PackSkillInfo* skillInfo, double timep, hf_uint32 AimID)
{
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    STR_PackPlayerPosition* t_pos = &(*smap)[conn].m_position;
    STR_PackSkillResult t_skillResult;
    STR_PackDamageData t_damageData;

    _umap_playerViewMonster::iterator it = (*smap)[conn].m_viewMonster->find(AimID); //查找可范围内是否有这个怪物
    if(it != (*smap)[conn].m_viewMonster->end())
    {
        t_damageData.AimID = AimID;
        t_damageData.AttackID = (*smap)[conn].m_roleid;

        umap_monsterInfo u_monsterInfo = Server::GetInstance()->GetMonster()->GetMonsterBasic();
        STR_MonsterInfo* t_monsterInfo = &(*u_monsterInfo)[AimID];

        hf_float dx = t_monsterInfo->monster.Current_x - t_pos->Pos_x;
        hf_float dy = t_monsterInfo->monster.Current_y - t_pos->Pos_y;
        hf_float dz = t_monsterInfo->monster.Current_z - t_pos->Pos_z;

        if( (dx*dx + dy*dy + dz*dz) < skillInfo->NearlyDistance * skillInfo->NearlyDistance ||
                (dx*dx + dy*dy + dz*dz) > skillInfo->FarDistance * skillInfo->FarDistance)    //不在攻击范围
        {
            t_skillResult.result = NOT_ATTACKVIEW;
            conn->Write_all(&t_skillResult, sizeof(STR_PackSkillResult));
            return;
        }
        if((dx*cos((t_pos->Direct)*PI/180) + dz*sin((t_pos->Direct)*PI/180))/sqrt(dx*dx + dz*dz) < 0)  //不在攻击角度
        {
            t_skillResult.result = OPPOSITE_DIRECT;
            conn->Write_all(&t_skillResult, sizeof(STR_PackSkillResult));
            return;
        }

        (*smap)[conn].m_publicCoolTime = timep + PUBLIC_COOLTIME; //保存玩家使用技能的公共冷却时间
        (*smap)[conn].m_skillUseTime = timep + skillInfo->CoolTime + skillInfo->CastingTime;  //保存技能的冷却时间

        //取攻击者角色的属性信息
        STR_RoleInfo* t_AttacketInfo = &((*smap)[conn].m_roleInfo);
        umap_monsterAttackInfo* t_monsterAttack = Server::GetInstance()->GetMonster()->GetMonsterAttack();
        STR_MonsterAttackInfo* t_monsterAttackInfo = &(*t_monsterAttack)[AimID];

        if(skillInfo->CastingTime == 0)//无延时类技能
        {
            t_skillResult.result = SKILL_SUCCESS;
            conn->Write_all(&t_skillResult, sizeof(STR_PackSkillResult));
            t_damageData.Damage = CalDamage(skillInfo, t_AttacketInfo, t_monsterAttackInfo, &t_damageData.TypeID);

            cout << "damage:" << t_damageData.Damage << endl;
            STR_PackSkillAimEffect t_skillEffect(t_damageData.AimID,skillInfo->SkillID,t_damageData.AttackID);
            conn->Write_all(&t_skillEffect, sizeof(STR_PackSkillAimEffect));  //发送施法效果
            DamageDealWith(conn, &t_damageData, t_monsterInfo); //发送计算伤害
        }
        else   //延时类技能
        {
            RoleAttackAim t_attackAim;
            t_attackAim.AimID = AimID;
            t_attackAim.HurtTime = (hf_uint32)timep + skillInfo->CastingTime;
            t_attackAim.SkillID = skillInfo->SkillID;

            (*m_attackMonster)[(*smap)[conn].m_roleid] = t_attackAim;
            (*smap)[conn].m_skillUseTime = t_attackAim.HurtTime + skillInfo->CoolTime;
        }
    }
}

//怪物为圆心
void GameAttack::AimMonsterCircle(TCPConnection::Pointer conn, STR_PackSkillInfo* skillInfo, double timep, hf_uint32 AimID)
{
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    umap_playerViewMonster t_viewMonster = (*smap)[conn].m_viewMonster;
    STR_PackPlayerPosition* t_pos = &(*smap)[conn].m_position;
    STR_PackSkillResult t_skillResult;
    STR_PackDamageData t_damageData;

    _umap_playerViewMonster::iterator it = t_viewMonster->find(AimID); //查找可范围内是否有这个怪物
    if(it != (*smap)[conn].m_viewMonster->end())
    {
        umap_monsterInfo u_monsterInfo = Server::GetInstance()->GetMonster()->GetMonsterBasic();

        STR_MonsterInfo* t_monsterInfo = &(*u_monsterInfo)[AimID];
        hf_float dx = t_monsterInfo->monster.Current_x - t_pos->Pos_x;
        hf_float dy = t_monsterInfo->monster.Current_y - t_pos->Pos_y;
        hf_float dz = t_monsterInfo->monster.Current_z - t_pos->Pos_z;

        if( (dx*dx + dy*dy + dz*dz) < skillInfo->NearlyDistance * skillInfo->NearlyDistance ||
                (dx*dx + dy*dy + dz*dz) > skillInfo->FarDistance * skillInfo->FarDistance)    //不在攻击范围
        {
            t_skillResult.result = NOT_ATTACKVIEW;
            conn->Write_all(&t_skillResult, sizeof(STR_PackSkillResult));
            return;
        }
        if((dx*cos((t_pos->Direct)*PI/180) + dz*sin((t_pos->Direct)*PI/180))/sqrt(dx*dx + dz*dz) < 0)  //不在攻击角度
        {
            t_skillResult.result = OPPOSITE_DIRECT;
            conn->Write_all(&t_skillResult, sizeof(STR_PackSkillResult));
            return;
        }

        (*smap)[conn].m_publicCoolTime = timep + PUBLIC_COOLTIME; //保存玩家使用技能的公共冷却时间
        (*smap)[conn].m_skillUseTime = timep + skillInfo->CoolTime + skillInfo->CastingTime;  //保存技能的冷却时间
        t_skillResult.result = SKILL_SUCCESS;
        conn->Write_all(&t_skillResult, sizeof(STR_PackSkillResult));  //发送施法结果


//        //取攻击者角色的属性信息
        STR_RoleInfo* t_roleInfo = &((*smap)[conn].m_roleInfo);
        umap_monsterAttackInfo* t_monsterAttack = Server::GetInstance()->GetMonster()->GetMonsterAttack();
        STR_MonsterAttackInfo* t_monsterAttackInfo = &(*t_monsterAttack)[AimID];

        if(skillInfo->CastingTime > 0)
        {
            RoleAttackAim t_attackAim;
            t_attackAim.AimID = AimID;
            t_attackAim.HurtTime = timep + skillInfo->CastingTime;
            t_attackAim.SkillID = skillInfo->SkillID;

            (*m_attackMonster)[(*smap)[conn].m_roleid] = t_attackAim;
            (*smap)[conn].m_skillUseTime = t_attackAim.HurtTime + skillInfo->CoolTime;
            return;
        }
        STR_PackSkillAimEffect t_skillEffect(t_damageData.AimID,skillInfo->SkillID,t_damageData.AttackID);
        conn->Write_all(&t_skillEffect, sizeof(STR_PackSkillAimEffect));  //发送施法效果

        for(_umap_playerViewMonster::iterator iter = t_viewMonster->begin(); iter != t_viewMonster->end(); iter++)
        {

            if(iter->first == t_monsterInfo->monster.MonsterID)
            {
                continue;
            }
            STR_MonsterInfo* t_monster = &(*u_monsterInfo)[iter->first];

            hf_float dx = t_monsterInfo->monster.Current_x - t_monster->monster.Current_x;
            hf_float dy = t_monsterInfo->monster.Current_y - t_monster->monster.Current_x;
            hf_float dz = t_monsterInfo->monster.Current_z - t_monster->monster.Current_x;
            if( (dx*dx + dy*dy + dz*dz) >= skillInfo->NearlyDistance * skillInfo->NearlyDistance &&(dx*dx + dy*dy + dz*dz) <= skillInfo->FarDistance * skillInfo->FarDistance)
            {
              t_damageData.Damage = CalDamage(skillInfo, t_roleInfo, t_monsterAttackInfo, &t_damageData.TypeID);
              DamageDealWith(conn, &t_damageData, t_monster); //发送计算伤害
            }
        }
    }
}


//清除超过时间的掉落物品
void GameAttack::DeleteOverTimeGoods()
{
    SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();
    time_t timep;
    LootGoodsOverTime t_loot;
    while(1)
    {
        time(&timep);
        for(SessionMgr::SessionMap::iterator it = smap->begin(); it != smap->end(); it++)
        {
            umap_lootPosition  t_lootPosition = it->second.m_lootPosition;
            umap_lootGoods     t_lootGoods = it->second.m_lootGoods;
            for(_umap_lootPosition::iterator pos_it = t_lootPosition->begin(); pos_it != t_lootPosition->end();)
            {
                if((hf_uint32)timep >= pos_it->second.timep + pos_it->second.continueTime)
                {
                    t_loot.loot = pos_it->first;
                    it->first->Write_all(&t_loot, sizeof(LootGoodsOverTime));

                    _umap_lootGoods::iterator goods_it = t_lootGoods->find(pos_it->first);
                    if(goods_it != t_lootGoods->end())
                    {
                        t_lootGoods->erase(goods_it);
                    }
                    _umap_lootPosition::iterator _pos_it = pos_it;
                    pos_it++;
                    t_lootPosition->erase(_pos_it);
                }
                else
                {
                    pos_it++;
                    continue;
                }
            }
        }
        sleep(1);
    }
}


