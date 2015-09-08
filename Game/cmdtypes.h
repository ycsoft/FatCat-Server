#ifndef CMDTYPES_H
#define CMDTYPES_H

#include <stdint.h>
#include <string.h>
#include <iostream>
#include <vector>

#include <boost/unordered_map.hpp>

#include "hf_types.h"
#include "postgresqlstruct.h"

#include "NetWork/tcpconnection.h"
using namespace std;
using namespace hf_types;

typedef boost::unordered_map<hf_uint32,STR_PackTaskDlg>            umap_dialogue;
typedef boost::unordered_map<hf_uint32,STR_PackTaskDescription>    umap_taskDescription;
typedef boost::unordered_map<hf_uint32,vector<STR_TaskAim> >       umap_taskAim;
typedef boost::unordered_map<hf_uint32,STR_PackTaskReward>         umap_taskReward;

//任务奖励
// <任务ID，物品奖励>
typedef boost::unordered_map<hf_uint32, vector<STR_PackGoodsReward> >  umap_goodsReward;

typedef boost::unordered_map<hf_uint32,STR_TaskPremise>              umap_taskPremise;


//<NPCID,NPC信息>
typedef boost::unordered_map<hf_uint32,NPCInfo> umap_npcInfo;

//<怪物类型,掉落物品>
typedef boost::unordered_map<hf_uint32,vector<STR_MonsterLoot> > umap_monsterLoot;


//<怪物ID，怪物基本属性信息>
typedef boost::unordered_map<hf_uint32,STR_MonsterBasicInfo> _umap_monsterBasicInfo ;
typedef boost::shared_ptr<_umap_monsterBasicInfo> umap_monsterBasicInfo;


//怪物攻击信息
typedef boost::unordered_map<hf_uint8,STR_MonsterAttackInfo> umap_monsterAttackInfo;

//玩家延时技能攻击数据，选取地图上的目标
typedef boost::unordered_map<hf_uint32, RoleAttackAim> _umap_roleAttackAim;
typedef boost::shared_ptr<_umap_roleAttackAim> umap_roleAttackAim;

//玩家延时技能攻击数据，选取地图上的点
typedef boost::unordered_map<hf_uint32, RoleAttackPoint> _umap_roleAttackPoint;
typedef boost::shared_ptr<_umap_roleAttackPoint> umap_roleAttackPoint;

//玩家技能信息
typedef boost::unordered_map<hf_uint32, STR_PackSkillInfo> umap_skillInfo;

//玩家可以使用技能的时间 <技能ID，这个时间之后可以使用>
typedef boost::unordered_map<hf_uint32, hf_double> _umap_skillTime;
typedef boost::shared_ptr<_umap_skillTime> umap_skillTime;



//任务编号，任务概述
typedef boost::unordered_map<hf_uint32,STR_TaskProfile> _umap_taskProfile;
//<任务编号，任务进度>
typedef boost::unordered_map<hf_uint32,vector<STR_TaskProcess> > _umap_taskProcess;

typedef boost::shared_ptr<_umap_taskProfile> umap_taskProfile;
typedef boost::shared_ptr<_umap_taskProcess> umap_taskProcess;

//<好友角色编号，好友信息>
typedef boost::unordered_map<hf_uint32,STR_FriendInfo> _umap_friendList;
typedef boost::shared_ptr<_umap_friendList> umap_friendList;

//角色编号，描述符
typedef boost::unordered_map<hf_uint32, TCPConnection::Pointer> _umap_roleSock;
typedef boost::shared_ptr<_umap_roleSock> umap_roleSock;


//<怪物ID,_umap_roleSock>
typedef boost::unordered_map<hf_uint32, _umap_roleSock> _umap_monsterViewRole;
typedef boost::shared_ptr<_umap_monsterViewRole> umap_monsterViewRole;

//装备属性
typedef boost::unordered_map<hf_uint32, STR_EquipmentAttr> umap_equAttr;

//玩家物品包<物品ID，基本信息>
typedef boost::unordered_map<hf_uint32, vector<STR_Goods> >_umap_roleGoods;
typedef boost::shared_ptr<_umap_roleGoods> umap_roleGoods;

//玩家背包装备 <装备ID，基本信息>
typedef boost::unordered_map<hf_uint32, STR_PlayerEqu> _umap_roleEqu;
typedef boost::shared_ptr<_umap_roleEqu> umap_roleEqu;

////玩家装备包 <装备ID，属性>
//typedef boost::unordered_map<hf_uint32, STR_Equipment> _umap_roleEquAttr;
//typedef boost::shared_ptr<_umap_roleEquAttr>umap_roleEquAttr;

//玩家金币 <金币类型ID，金币属性>
typedef boost::unordered_map<hf_uint8, STR_PlayerMoney>_umap_roleMoney;
typedef boost::shared_ptr<_umap_roleMoney> umap_roleMoney;


//掉落物品 <怪物ID/任务ID，掉落物品> 如果是装备，则vector.size() = 1;
typedef boost::unordered_map<hf_uint32, vector<STR_LootGoods> > _umap_lootGoods;
typedef boost::shared_ptr<_umap_lootGoods> umap_lootGoods;

//物品掉落位置
typedef boost::unordered_map<hf_uint32, LootPositionTime> _umap_lootPosition;
typedef boost::shared_ptr<_umap_lootPosition> umap_lootPosition;

//物品价格
typedef boost::unordered_map<hf_uint32, STR_GoodsPrice> _umap_goodsPrice;
typedef boost::shared_ptr<_umap_goodsPrice> umap_goodsPrice;

//怪物刷新点 <怪物ID,刷新点ID>
typedef boost::unordered_map<hf_uint32, hf_uint32> umap_monsterSpawnsPos;
//<刷新点ID,刷新点信息>
typedef boost::unordered_map<hf_uint32, STR_MonsterSpawns> umap_monsterSpawns;
//<怪物类型ID,类型信息>
typedef boost::unordered_map<hf_uint32, STR_MonsterType> umap_monsterType;
//保存怪物死亡信息 <怪物ID,刷新信息>
typedef boost::unordered_map<hf_uint32, MonsterDeath> _umap_monsterDeath;
typedef boost::shared_ptr<_umap_monsterDeath> umap_monsterDeath;

//保存玩家任务物品 <物品ID,vector<任务编号> >  一个物品同时可能为多个任务目标
typedef boost::unordered_map<hf_uint32, vector<hf_uint32> > _umap_taskGoods;
typedef boost::shared_ptr<_umap_taskGoods> umap_taskGoods;

typedef struct _Configuration
{
    const char* ip;
    const char* port;
    const char* dbName;     //数据库名
    const char* user;       //用户名
    const char* password;   //用户密码
}Configuration;


//怪物信息返回
typedef  struct _ResPackMonsterInfo
{
    _ResPackMonsterInfo()
    {
        m_MonsterInfo.resize(10);
        m_MonsterInfo.clear();
    }
    vector<STR_MonsterBasicInfo> m_MonsterInfo;
}ResPackMonsterInfo;


typedef struct _ResRoleList
{
    _ResRoleList()
    {
        m_Role.resize(10);
        m_Role.clear();
    }

    vector<STR_RoleBasicInfo> m_Role;
}ResRoleList;

typedef struct _EffectRow
{
    hf_uint32 row;
}EffectRow;


#endif // CMDTYPES_H

