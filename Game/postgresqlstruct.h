#ifndef POSTGRESQLSTRUCT_H
#define POSTGRESQLSTRUCT_H

#include <string.h>
#include <vector>
//#include <boost/atomic.hpp>

#include "./../hf_types.h"
#include "packheadflag.h"

using namespace std;
using namespace hf_types;

//#pragma pack(1)

//数据包
//1.数据头
typedef struct _STR_PackHead
{
    hf_uint16 Len;               // 数据包内容的长度，不包含数据包头长度。
    hf_uint8  H1;
    hf_uint8  H2;
    hf_uint8  H3;
    hf_uint8  H4;
    hf_uint16 Flag;              //标识位，用于记录数据包内容的类型，用以解析数据。
}STR_PackHead;

//任务概述
typedef struct _STR_TaskProfile
{
    hf_uint32 TaskID;                 //唯一标识
    hf_char   TaskName[32];           //任务名称
    hf_uint32 StartNPCID;             //任务发布NPCID
    hf_uint32 FinishNPCID;            //任务交接NPCID
    hf_uint8  AcceptModeID;           //任务接受方式ID
    hf_uint8  FinishModeID;           //任务完成方式ID
    hf_uint8  Status;                 //1未接取，2已接取
}STR_TaskProfile;

//任务对话数据
typedef struct _STR_TaskDlg
{
    hf_uint32  TaskID;
    hf_uint16  StartLen;            //开始对话长度

    hf_uint16  FinishLen;           //结束对话长度
    hf_char    StartDialogue[512];  //任务开始对话

    hf_char    FinishDialogue[512]; //任务结束对话
}STR_TaskDlg;

//任务执行对话
typedef struct _STR_TaskExeDlg
{
    hf_uint32  TaskID;
    hf_uint32  AimID;               //目标ID
    hf_uint16  ExeLen;              //执行对话长度
    hf_char    ExeDialogue[512];    //任务执行对话
}STR_TaskExeDlg;

//4.任务描述数据
typedef struct _STR_PackTaskDescription
{
    STR_PackHead        head;
    hf_uint32 TaskID;
    hf_uint32 Time;              //时间 单位秒
    hf_uint32 TaskPropsID;       //任务道具
    hf_char   Description[512];  //任务描述

    _STR_PackTaskDescription()
    {
        bzero(&head,sizeof(_STR_PackTaskDescription));
        head.Flag = FLAG_TaskDescription;
        head.Len = sizeof(_STR_PackTaskDescription)-sizeof(STR_PackHead);
    }
}STR_PackTaskDescription;


//任务目标数据
typedef struct _STR_TaskAim
{
    hf_uint32 TaskID;
    hf_uint32 AimID;              //任务目标ID
    hf_uint32 Amount;             //任务数量ID
    hf_uint8  ExeModeID;          //执行方式ID
}STR_TaskAim;


//6.任务奖励数据
typedef struct _STR_TaskReward
{
    hf_uint32 TaskID;
    hf_uint32 Experience;            //奖励经验值
    hf_uint32 Money;                 //奖励金钱
    hf_uint32 SkillID;               //奖励技能ID
    hf_uint32 TitleID;               //奖励称号ID
    hf_uint8  Attribute;             //奖励属性点
}STR_TaskReward;

//7.物品奖励数据
typedef struct _STR_GoodsReward
{
    hf_uint32  GoodsID;           //奖励物品ID
    hf_uint16  Count;             //奖励物品数量
    hf_uint8   Type;              //奖励类型，1为固定奖励，2为可选奖励
}STR_GoodsReward;

//8.玩家请求接受任务数据
typedef struct _STR_PackUserAskTask
{
    STR_PackHead        head;
    hf_uint32           TaskID;             //任务ID

    _STR_PackUserAskTask()
    {
        bzero(&head,sizeof(_STR_PackUserAskTask));
        head.Flag = FLAG_UserAskTask;
        head.Len = sizeof(_STR_PackUserAskTask)-sizeof(STR_PackHead);
    }

}STR_PackUserAskTask;

//9.玩家接受任务结果数据
/*result
 * 1 成功
 * 2 失败，未完成前置任务
 * 3 失败，未接取条件任务
 * 4 失败，未持有任务物品
 * 5 失败，为获得任务条件称号
 * 6 失败，未完成条件副本
 * 7 失败，性别不符
 * 8 失败，等级不足
 * 9 失败，职业不符
 */
typedef struct _STR_PackAskResult
{
    STR_PackHead        head;
    hf_uint32           TaskID;
    hf_uint8            Result;    //根据Result值判断成功或者失败原因

    _STR_PackAskResult()
    {
        bzero(&head,sizeof(_STR_PackAskResult));
        head.Flag = FLAG_AskResult;
        head.Len = sizeof(_STR_PackAskResult)-sizeof(STR_PackHead);
    }
}STR_PackAskResult;


//任务执行对话
//typedef struct _STR_PackTaskExeDialogue
//{
//    STR_PackHead        head;
//    _STR_PackTaskExeDialogue()
//    {
//        bzero(&head, sizeof(_STR_PackTaskExeDialogue));
//        head.Flag = 0;
//        head.Len = sizeof(_STR_PackTaskExeDialogue) - sizeof(STR_PackHead);
//    }

//    hf_uint32 TaskID;
//    hf_uint32 AimID;
//    hf_uint32 ExeModeID;
//    hf_char   TaskExeDialogue[512];
//}STR_PackTaskExeDialogue;

//11 玩家请求完成任务数据
typedef struct _STR_FinishTask
{
    hf_uint32           TaskID;
    hf_uint32           SelectGoodsID;  // 可能出现奖励物品多选一，玩家选择需要的物品发送给服务器
}STR_FinishTask;

typedef struct _STR_PackFinishTaskResult
{
    _STR_PackFinishTaskResult()
    {
        bzero(&head, sizeof(_STR_PackFinishTaskResult));
        head.Flag = FLAG_UserTaskResult;
        head.Len = sizeof(_STR_PackFinishTaskResult) - sizeof(STR_PackHead);
    }

    STR_PackHead head;
    hf_uint32 TaskID;
    hf_uint8  Result;   //结果 1 成功,0 失败
}STR_PackFinishTaskResult;

//放弃任务 2015.05.06
typedef struct _STR_PackQuitTask
{
    STR_PackHead        head;
    hf_uint32 TaskID;
    _STR_PackQuitTask()
    {
        bzero(&head,sizeof(_STR_PackQuitTask));
        head.Flag = /*htons*/(FLAG_QuitTask);
        head.Len = /*htons*/(sizeof(_STR_PackQuitTask)-sizeof(STR_PackHead));
    }
}STR_PackQuitTask;


//13，怪物信息数据

typedef struct _STR_MonsterBasicInfo
{
    hf_uint32  MonsterID;          //怪物的唯一标识
    hf_uint32  MonsterTypeID;      //怪物类型ID
    hf_char    MonsterName[32];    //怪物类型，用于记录怪物名称以及需要使用的怪物模型
    hf_uint32  MapID;              //地图ID  2015.06.11
    hf_float   Current_x;          //当前坐标中x轴分量
    hf_float   Current_y;          //当前坐标中y轴分量
    hf_float   Current_z;          //当前坐标中z轴分量
    hf_float   Target_x;           //目标坐标中x轴分量
    hf_float   Target_y;           //目标坐标中y轴分量
    hf_float   Target_z;           //目标坐标中z轴分量
    hf_uint32  MoveRate;           //移动速率按百分比计算为某一个固定值的倍数，例如300%，这里只记录300
    hf_uint32  HP;                 //怪物当前剩余血量，在一开始，HP = MaxHP
    hf_uint32  MaxHP;              //最大血量
    hf_float   Direct;             //怪物朝向角度0...359
    hf_uint8   Level;              //怪物等级  2015.05.06
    hf_uint8   RankID;             //类别ID    2015.05.20
    hf_uint8   ActID;              //怪物当前动作的索引
    hf_uint8   Flag;               //1主动攻击怪 2被动攻击怪
}STR_MonsterBasicInfo;

typedef struct _STR_PackMonsterBasicInfo
{
    _STR_PackMonsterBasicInfo(STR_MonsterBasicInfo* _monster)
    {
        bzero(&head, sizeof(_STR_PackMonsterBasicInfo));
        head.Flag = FLAG_MonsterCome;
        head.Len = sizeof(_STR_PackMonsterBasicInfo) - sizeof(STR_PackHead);
        memcpy(&monster, _monster, sizeof(STR_MonsterBasicInfo));
    }
    STR_PackHead head;
    STR_MonsterBasicInfo monster;
}STR_PackMonsterBasicInfo;

typedef struct _STR_Position
{
    hf_float             Come_x;
    hf_float             Come_y;
    hf_float             Come_z;
}STR_Position;

//怪物移动位置
typedef struct _STR_MonsterMovePos
{
    hf_uint32  MonsterID;
    hf_float   Pos_x;
    hf_float   Pos_y;
    hf_float   Pos_z;
    hf_float   Target_x;
    hf_float   Target_y;
    hf_float   Target_z;
    hf_uint32  MapID;
    hf_uint16  Derect;
    hf_uint8   ActID;
}STR_MonsterMovePos;

//怪物攻击属性信息
typedef struct _STR_MonsterAttackInfo
{
    hf_uint32 MonsterID;        //怪物ID
    hf_uint32 PhysicalAttack;   //物理攻击
    hf_uint32 MagicAttack;      //魔法攻击
    hf_uint32 PhysicalDefense;  //物理防御
    hf_uint32 MagicDefense;     //魔法防御
    hf_float  Crit_Rate;        //暴击率
    hf_float  Dodge_Rate;       //闪避率
    hf_float  Hit_Rate;         //命中率
//    hf_uint8  Level;            //等级
}STR_MonsterAttackInfo;

//14.怪物属性数据包 受攻击后怪物属性，暂时只有HP变化
typedef struct _STR_PackMonsterAttrbt
{
    STR_PackHead        head;
    hf_uint32 MonsterID;         //怪物ID；
    hf_uint32 HP;                //当前血量

    _STR_PackMonsterAttrbt()
    {
        bzero(&head,sizeof(_STR_PackMonsterAttrbt));
        head.Flag = /*htons*/(FLAG_MonsterAttribute);
        head.Len = /*htons*/(sizeof(_STR_PackMonsterAttrbt)-sizeof(STR_PackHead));
    }
}STR_PackMonsterAttrbt;

//玩家受攻击(攻击)后属性，暂时变化的只有HP
typedef struct _STR_RoleAttribute
{
    STR_PackHead        head;
    hf_uint32 RoleID;
    hf_uint32 HP;
    hf_uint32 Magic;
    _STR_RoleAttribute(hf_uint32 _RoleID, hf_uint32 _HP, hf_uint32 _Magic = 0)
    {
        bzero(&head,sizeof(_STR_RoleAttribute));
        head.Flag = /*htons*/(FLAG_RoleAttribute);
        head.Len = /*htons*/(sizeof(_STR_RoleAttribute)-sizeof(STR_PackHead));
        RoleID = _RoleID;
        HP = _HP;
        Magic = _Magic;
    }
}STR_RoleAttribute;

//15.怪物位置数据包
typedef struct _STR_PackMonsterPosition
{
    STR_PackHead        head;
    hf_uint32  MonsterID;         //怪物ID，在客户端中需要转换为字符串，表示唯一标识
    hf_float   Pos_x;             //当前坐标中x轴分量
    hf_float   Pos_y;             //当前坐标中y轴分量
    hf_float   Pos_z;             //当前坐标中z轴分量
    hf_float   Direct;            //怪物朝向角度0...359
    hf_uint8   ActID;             //怪物当前动作的索引

    _STR_PackMonsterPosition()
    {
        bzero(&head,sizeof(_STR_PackMonsterPosition));
        head.Flag = /*htons*/(FLAG_MonsterPosition);
        head.Len = /*htons*/(sizeof(_STR_PackMonsterPosition)-sizeof(STR_PackHead));
    }

}STR_PackMonsterPosition;

//16.玩家攻击信息数据包
typedef struct _STR_PackUserAttackAim
{
    STR_PackHead        head;
    hf_uint32  AimID;             //目标ID
    hf_uint32  SkillID;           //使用的技能ID
    hf_float   Direct;            //玩家朝向0...359

    _STR_PackUserAttackAim()
    {
        bzero(&head,sizeof(_STR_PackUserAttackAim));
        head.Flag = /*htons*/(FLAG_UserAttackAim);
        head.Len = /*htons*/(sizeof(_STR_PackUserAttackAim)-sizeof(STR_PackHead));
    }
}STR_PackUserAttackAim;

//角色技能攻击地图点
typedef struct _STR_PackUserAttackPoint
{
    STR_PackHead        head;
    hf_float   Pos_x;
    hf_float   Pos_y;
    hf_float   Pos_z;
    hf_uint32  MapID;
    hf_uint32  SkillID;           //使用的技能ID
    hf_float   Direct;
    _STR_PackUserAttackPoint()
    {
        bzero(&head,sizeof(_STR_PackUserAttackPoint));
        head.Flag = FLAG_UserAttackPoint;
        head.Len = sizeof(_STR_PackUserAttackPoint)-sizeof(STR_PackHead);
    }
}STR_PackUserAttackPoint;


//17.掉落物品位置
typedef struct _STR_LootGoodsPos
{
    hf_float  Pos_x;
    hf_float  Pos_y;
    hf_float  Pos_z;
    hf_uint32 MapID;
    hf_uint32 GoodsFlag;           //物品掉落物品标记,一般为怪物ID或任务ID
}STR_LootGoodsPos;

//物品掉落时间，位置
typedef struct _LootPositionTime
{
    hf_uint32 timep;                 //掉落时间
    hf_uint32 continueTime;          //持续时间
    STR_LootGoodsPos goodsPos;   //物品位置
}LootPositionTime;


//掉落物品
typedef struct _STR_LootGoods
{
    hf_uint32 LootGoodsID;      //掉落的物品ID
    hf_uint16 Count;            //掉落数量
}STR_LootGoods;

//超时物品
typedef struct _LootGoodsOverTime
{
    _LootGoodsOverTime()
    {
        head.Len = sizeof(loot);
        head.Flag = FLAG_LootGoodsOverTime;
    }
    STR_PackHead head;
    hf_uint32 loot;
}LootGoodsOverTime;


//玩家物品变动数据包,分割移动物品}
typedef struct _STR_MoveBagGoods
{
    hf_uint32 CurrentGoodsID;    //当前位置物品ID
    hf_uint32 TargetGoodsID;     //目标位置物品ID
    hf_uint16 Count;             //当前位置移动数量
    hf_uint16 CurrentPos;        //当前位置
    hf_uint16 TargetPos;         //目标位置
}STR_MoveBagGoods;

//玩家丢弃背包物品数据包
typedef struct _STR_RemoveBagGoods
{
    hf_uint32 GoodsID;   //物品ID
    hf_uint8  Position;       //物品在背包里的位置
}STR_RemoveBagGoods;

//从商店购买东西
typedef struct _STR_BuyGoods
{
    hf_uint32 GoodsID;   //物品ID
    hf_uint16 Count;     //数量
}STR_BuyGoods;

//将东西卖给商店
typedef struct _STR_SellGoods
{
    hf_uint32 GoodsID;
    hf_uint8  Position;
}STR_SellGoods;

//物品价格
typedef struct _STR_GoodsPrice
{
    hf_uint32 GoodsID;    //物品ID
    hf_uint32 BuyPrice;   //购买价格
    hf_uint32 SellPrice;  //出售价格
}STR_GoodsPrice;

//18.玩家拾取数据包
typedef struct _STR_PickGoods
{
    hf_uint32 LootGoodsID; //物品ID
    hf_uint32 GoodsFlag;   //掉落者 怪物ID
}STR_PickGoods;

//19.玩家捡取物品结果数据包
typedef struct _STR_PickGoodsResult
{
    hf_uint32  LootGoodsID;
    hf_uint32  GoodsFlag;
    hf_uint16  Count;
    hf_uint8   Result;
}STR_PickGoodsResult;

typedef struct _STR_PackPickGoodsResult
{
    _STR_PackPickGoodsResult()
    {
        bzero(&head,sizeof(_STR_PackPickGoodsResult));
        head.Len = sizeof(_STR_PackPickGoodsResult) - sizeof(STR_PackHead);
        head.Flag = FLAG_PickGoodsResult;
    }
    _STR_PackPickGoodsResult(hf_uint32 _LootGoodsID, hf_uint32 _GoodsFlag, hf_uint16 _Count, hf_uint8 _Result)
    {
        bzero(&head,sizeof(_STR_PackPickGoodsResult));
        head.Len = sizeof(_STR_PackPickGoodsResult) - sizeof(STR_PackHead);
        head.Flag = FLAG_PickGoodsResult;
        LootGoodsID = _LootGoodsID;
        GoodsFlag  = _GoodsFlag;
        Count = _Count;
        Result = _Result;
    }

    STR_PackHead head;
    hf_uint32  LootGoodsID;
    hf_uint32  GoodsFlag;
    hf_uint16  Count;
    hf_uint8   Result;
}STR_PackPickGoodsResult;

//装备属性 <表结构>
typedef struct _STR_EquipmentAttr
{
    hf_uint32 EquID;
    hf_uint32 TypeID;                   //类型ID
    hf_float  Crit_Rate;                //暴击率
    hf_float  Dodge_Rate;               //闪避率
    hf_float  Hit_Rate;                 //命中率
    hf_float  Resist_Rate;              //抵挡率
    hf_float  Caster_Speed;             //施法速度
    hf_float  Move_Speed;               //移动速度
    hf_float  Hurt_Speed;               //攻击速度
    hf_float  RecoveryLife_Percentage;  //每秒恢复生命值百分比
    hf_uint32 RecoveryLife_value;       //每秒恢复生命值
    hf_float  RecoveryMagic_Percentage; //每秒恢复魔法值百分比
    hf_uint32 RecoveryMagic_value;      //每秒恢复魔法值
    hf_float  MagicHurt_Reduction;      //法术伤害减免
    hf_float  PhysicalHurt_Reduction;   //物理伤害减免

    hf_float  CritHurt;                 //暴击伤害          变
    hf_float  CritHurt_Reduction;       //暴击伤害减免      变
    hf_float  Magic_Pass;               //法透
    hf_float  Physical_Pass;            //物透
    hf_uint32 SuitSkillID;              //套装技能ID

    hf_uint16 HP;                       //附加血量          变
    hf_uint16 Magic;                    //附加魔法值        变
    hf_uint16 PhysicalDefense;          //物理防御          变
    hf_uint16 MagicDefense;             //魔法防御          变
    hf_uint16 PhysicalAttack;           //物理攻击          变
    hf_uint16 MagicAttack;              //魔法攻击          变

    hf_uint16 Rigorous;                 //严谨    有可能变
    hf_uint16 Will;                     //机变    有可能变
    hf_uint16 Wise;                     //睿智    有可能变
    hf_uint16 Mentality;                //心态    有可能变
    hf_uint16 Physical_fitness;         //体能    有可能变

    hf_uint8  JobID;                    //职业ID
    hf_uint8  BodyPos;                  //装备身体部位
    hf_uint8  Grade;                    //装备品级
    hf_uint8  Level;                    //装备等级
    hf_uint8  StrengthenLevel;          //强化等级         变
    hf_uint8  MaxDurability;            //耐久度上限
    hf_uint8  Durability;               //当前耐久度       变
}STR_EquipmentAttr;

typedef struct _STR_PackEquipmentAttr
{
    _STR_PackEquipmentAttr()
    {
        bzero(&head,sizeof(_STR_PackEquipmentAttr));
        head.Len = sizeof(_STR_PackEquipmentAttr) - sizeof(STR_PackHead);
        head.Flag = FLAG_EquGoodsAttr;
    }
    _STR_PackEquipmentAttr(STR_EquipmentAttr* _equAttr)
    {
        bzero(&head,sizeof(_STR_PackEquipmentAttr));
        head.Len = sizeof(_STR_PackEquipmentAttr) - sizeof(STR_PackHead);
        head.Flag = FLAG_EquGoodsAttr;
        memcpy(&equAttr, _equAttr, sizeof(STR_EquipmentAttr));
    }
    STR_PackHead head;
    STR_EquipmentAttr equAttr;
}STR_PackEquipmentAttr;

//生成时
typedef struct _STR_PackCreateEqu
{
    _STR_PackCreateEqu()
    {
        bzero(&head,sizeof(_STR_PackCreateEqu));
        head.Len = sizeof(_STR_PackCreateEqu) - sizeof(STR_PackHead);
        head.Flag = FLAG_CreateEqu;
    }
    STR_PackHead head;
    hf_uint32 EquID;
    hf_uint32 TypeID;                   //类型ID
    hf_uint16 Rigorous;                 //严谨
    hf_uint16 Will;                     //机变
    hf_uint16 Wise;                     //睿智
    hf_uint16 Mentality;                //心态
    hf_uint16 Physical_fitness;         //体能
}TR_PackCreateEqu;


//强化时
typedef struct _STR_PackStrengthenEqu
{
    _STR_PackStrengthenEqu()
    {
        bzero(&head,sizeof(_STR_PackStrengthenEqu));
        head.Len = sizeof(_STR_PackStrengthenEqu) - sizeof(STR_PackHead);
        head.Flag = FLAG_StrengthenEqu;
    }
    STR_PackHead head;
    hf_uint32 EquID;
    hf_uint32 TypeID;                   //类型ID
    hf_uint16 HP;                       //附加血量
    hf_uint16 Magic;                    //附加魔法值
    hf_uint16 PhysicalDefense;          //物理防御
    hf_uint16 MagicDefense;             //魔法防御
    hf_uint16 PhysicalAttack;           //物理攻击
    hf_uint16 MagicAttack;              //魔法攻击
    hf_uint8  StrengthenLevel;          //强化等级
}STR_PackStrengthenEqu;

//使用过程中变化
typedef struct _STR_PackUseEqu
{
    _STR_PackUseEqu()
    {
        bzero(&head,sizeof(_STR_PackUseEqu));
        head.Len = sizeof(_STR_PackUseEqu) - sizeof(STR_PackHead);
        head.Flag = FLAG_UseEqu;
    }
    STR_PackHead head;
    hf_uint32 EquID;
    hf_uint32 TypeID;                   //类型ID
    hf_uint8  Durability;               //耐久度
}STR_PackUseEqu;

//玩家普通物品数据包
typedef struct _STR_Goods
{
    hf_uint32 GoodsID;   //物品ID
    hf_uint32 TypeID;    //类型ID
    hf_uint16 Count;     //数量
    hf_uint8  Position;  //位置
    hf_uint8  Source;    //来源  0 固有物品，1 来自交易，2 买的物品，3 捡的物品
}STR_Goods;

typedef struct _STR_PackGoods
{
    _STR_PackGoods(STR_Goods* _goods)
    {
        head.Flag = FLAG_BagGoods;
        head.Len = sizeof(_STR_PackGoods) - sizeof(STR_PackHead);
        memcpy(&goods, _goods, sizeof(STR_Goods));
    }

    STR_PackHead head;
    STR_Goods    goods;
}STR_PackGoods;

//玩家装备属性数据包
//typedef struct _STR_Equipment
//{
//    hf_uint32 EquID;              //物品ID
//    hf_uint32 TypeID;             //类型ID
//    hf_uint32 PhysicalAttack;     //物理攻击
//    hf_uint32 PhysicalDefense;    //物理防御
//    hf_uint32 MagicAttack;        //魔法攻击
//    hf_uint32 MagicDefense;       //魔法防御
//    hf_uint32 AddHp;              //附加血量
//    hf_uint32 AddMagic;           //附加魔法值
//    hf_uint8  Durability;         //耐久度
//}STR_Equipment;

//玩家装备信息
typedef struct _STR_PlayerEqu
{
    _STR_PlayerEqu()
    {
        memset(&goods, 0, sizeof(_STR_PlayerEqu));
    }
    STR_Goods goods;    //当Position为0时，表示穿在身上，不可操作
    STR_EquipmentAttr equAttr;
}STR_PlayerEqu;

//玩家金币
typedef struct _STR_PlayerMoney
{
    hf_uint32 Count;    //数量
    hf_uint8  TypeID;   //类型
}STR_PlayerMoney;

typedef struct _STR_PackPlayerMoney
{
    _STR_PackPlayerMoney(STR_PlayerMoney* _money)
    {
        head.Flag = FLAG_PlayerMoney;
        head.Len = sizeof(_STR_PackPlayerMoney) - sizeof(STR_PackHead);
        memcpy(&money, _money, sizeof(STR_PlayerMoney));
    }

    STR_PackHead head;
    STR_PlayerMoney money;
}STR_PackPlayerMoney;

//玩家位置
typedef struct _STR_PackPlayerPosition
{
    STR_PackHead   head;
    hf_float       Pos_x;      //x坐标
    hf_float       Pos_y;
    hf_float       Pos_z;
    hf_uint32      MapID;      //地图ID
    hf_float       Direct;     //方向
    hf_uint8       ActID;      //动作索引
    _STR_PackPlayerPosition()
    {
        bzero(&head,sizeof(_STR_PackPlayerPosition));
        head.Flag = FLAG_PlayerPosition;
        head.Len = sizeof(_STR_PackPlayerPosition)-sizeof(STR_PackHead);
    }
}STR_PackPlayerPosition;

//其他玩家位置
typedef struct _STR_OtherPlayerPosition
{
//    hf_uint32     Roleid;     //角色名
    hf_float      Pos_x;      //x坐标
    hf_float      Pos_y;
    hf_float      Pos_z;
    hf_uint32     MapID;      //地图ID
    hf_float      Direct;     //方向
    hf_uint8      ActID;      //动作索引
}STR_OtherPlayerPosition;

//其他玩家位置信息
typedef struct _STR_PackOtherPlayerPosition
{
    _STR_PackOtherPlayerPosition(hf_uint32 _roleid, STR_PackPlayerPosition* pos)
    {
        bzero(&head,sizeof(_STR_PackOtherPlayerPosition));
        head.Flag = FLAG_OtherPlayerPosition;
        head.Len = sizeof(_STR_PackOtherPlayerPosition)-sizeof(STR_PackHead);
        roleid = _roleid;
        OtherPos.Pos_x = pos->Pos_x;
        OtherPos.Pos_y = pos->Pos_y;
        OtherPos.Pos_z = pos->Pos_z;
        OtherPos.Direct = pos->Direct;
        OtherPos.ActID = pos->ActID;
    }
    STR_PackHead  head;
    hf_uint32     roleid;
    STR_OtherPlayerPosition OtherPos;
}STR_PackOtherPlayerPosition;


typedef struct _STR_PackOtherPlayerDirect
{
    _STR_PackOtherPlayerDirect(hf_uint32 _roleid, hf_float _direct)
    {
        bzero(&head,sizeof(_STR_PackOtherPlayerDirect));
        head.Flag = FLAG_PlayerDirect;
        head.Len = sizeof(_STR_PackOtherPlayerDirect)-sizeof(STR_PackHead);
        roleid = _roleid;
        direct = _direct;
    }
    STR_PackHead head;
    hf_uint32 roleid;
    hf_float  direct;
}STR_PackOtherPlayerDirect;

typedef struct _STR_PackOtherPlayerAction
{
    _STR_PackOtherPlayerAction(hf_uint32 _roleid, hf_uint8 _action)
    {
        bzero(&head,sizeof(_STR_PackOtherPlayerAction));
        head.Flag = FLAG_PlayerAction;
        head.Len = sizeof(_STR_PackOtherPlayerAction)-sizeof(STR_PackHead);
        roleid = _roleid;
        action = _action;
    }
    STR_PackHead head;
    hf_uint32 roleid;
    hf_uint8  action;
}STR_PackOtherPlayerAction;

typedef struct _STR_PackMonsterDirect
{
    _STR_PackMonsterDirect(hf_uint32 _monsterID, hf_float _direct)
    {
        bzero(&head,sizeof(_STR_PackMonsterDirect));
        head.Flag = FLAG_MonsterDirect;
        head.Len = sizeof(_STR_PackMonsterDirect)-sizeof(STR_PackHead);
        monsterID = _monsterID;
        direct = _direct;
    }
    STR_PackHead head;
    hf_uint32 monsterID;
    hf_float  direct;
}STR_PackMonsterDirect;

typedef struct _STR_PackMonsterAction
{
    _STR_PackMonsterAction(hf_uint32 _monsterID, hf_uint8 _action)
    {
        bzero(&head,sizeof(_STR_PackMonsterAction));
        head.Flag = FLAG_MonsterAction;
        head.Len = sizeof(_STR_PackMonsterAction)-sizeof(STR_PackHead);
        monsterID = _monsterID;
        action = _action;
    }
    STR_PackHead head;
    hf_uint32 monsterID;
    hf_uint8  action;
}STR_PackMonsterAction;


typedef struct _STR_PosDis
{
    _STR_PosDis(hf_float _dis, hf_float _dx, hf_float _dz)
        :dis(_dis), dx(_dx), dz(_dz)
    {

    }
    _STR_PosDis()
    {

    }

    hf_float dis;
    hf_float dx;
    hf_float dz;
    hf_float Current_x;
    hf_float Current_z;
}STR_PosDis;

//玩家刷新数据起始点
typedef struct _STR_PlayerStartPos
{
    hf_float Pos_x;
    hf_float Pos_y;
    hf_float Pos_z;
}STR_PlayerStartPos;

//玩家移动包
typedef struct _STR_PlayerMove
{
    hf_float     Direct; //玩家方向
    hf_uint8     Opt;    //移动方向
    hf_uint8     ActID;  //玩家动作
}STR_PlayerMove;

typedef struct _STR_PackPlayerOffline
{
    STR_PackHead  head;
    hf_uint8      type; // 1表示玩家断开连接，2表示玩家不断开连接，玩家返回角色列表，服务器需要重新发送玩家角色列表
    _STR_PackPlayerOffline()
    {
        bzero(&head,sizeof(_STR_PackPlayerOffline));
        head.Flag = FLAG_PlayerOffline;
        head.Len = sizeof(type);
    }
}STR_PackPlayerOffline;


//返回结果
typedef struct _STR_PackResult
{
    STR_PackHead        head;
    hf_uint16           Flag;
    hf_uint16           result;
    _STR_PackResult()
    {
        bzero(&head,sizeof(_STR_PackResult));
        head.Flag = FLAG_Result;
        head.Len = sizeof(_STR_PackResult)-sizeof(STR_PackHead);
    }

}STR_PackResult;

typedef struct _STR_PackOtherResult
{
    STR_PackHead        head;
    hf_uint16           Flag;
    hf_uint16           result;
    _STR_PackOtherResult()
    {
        bzero(&head,sizeof(_STR_PackOtherResult));
        head.Flag = FLAG_OtherResult;
        head.Len = sizeof(_STR_PackOtherResult)-sizeof(STR_PackHead);
    } 
}STR_PackOtherResult;

//角色基本信息
typedef struct _STR_RoleBasicInfo
{
    _STR_RoleBasicInfo()
    {
        memset(Nick, 0, 32);
    }

    hf_char   Nick[32];
    hf_uint32 RoleID;
    hf_uint16 ModeID;        //模型ID
    hf_uint16 SkirtID;       //裙子ID
    hf_uint8  Profession;    //职业编号
    hf_uint8  Level;         //等级
    hf_uint8  Sex;           //性别
    hf_uint8  Figure;        //体型
    hf_uint8  FigureColor;   //肤色
    hf_uint8  Face;          //脸型
    hf_uint8  Eye;           //眼睛
    hf_uint8  Hair;          //发型
    hf_uint8  HairColor;     //发色
}STR_RoleBasicInfo;

typedef struct _STR_BodyEquipment
{
    hf_uint32  roleid;
    //受到攻击时，头，上身，下身，鞋子，腰带的耐久度减小0.1，发出攻击时武器耐久度减小0.1
    hf_uint32  Head;          //头部
    hf_uint32  HeadType;
    hf_uint32  UpperBody;     //上身
    hf_uint32  UpperBodyType;
    hf_uint32  Pants;         //下身
    hf_uint32  PantsType;
    hf_uint32  Shoes;         //鞋子
    hf_int32   ShoesType;
    hf_uint32  Belt;          //腰带
    hf_uint32  BeltType;
    hf_uint32  Neaklace;      //项链
    hf_uint32  NeaklaceType;
    hf_uint32  Bracelet;      //手镯
    hf_uint32  BraceletType;
    hf_uint32  LeftRing;      //左手戒指
    hf_uint32  LeftRingType;
    hf_uint32  RightRing;     //右手戒指
    hf_uint32  RightRingType;
    hf_uint32  Phone;         //手机
    hf_uint32  PhoneType;
    hf_uint32  Weapon;        //武器
    hf_uint32  WeaponType;
}STR_BodyEquipment;

typedef struct _STR_PackBodyEquipment
{
    _STR_PackBodyEquipment(STR_BodyEquipment* _bodyEqu)
        {
            bzero(&head, sizeof(_STR_PackBodyEquipment));
            head.Flag = FLAG_PlayerBodyEqu;
            head.Len = sizeof(_STR_PackBodyEquipment) - sizeof(STR_PackHead);
            memcpy(&bodyEqu, _bodyEqu, sizeof(STR_BodyEquipment));
        }
        STR_PackHead head;
        STR_BodyEquipment bodyEqu;
}STR_PackBodyEquipment;

//穿装备
typedef struct _STR_WearEqu
{
    hf_uint32 equid;
    hf_uint8  pos;    //当为戒指时1表示左手，2表示右手 ，其它装备都为0
}STR_WearEqu;

typedef struct _STR_PackRoleBasicInfo
{
    _STR_PackRoleBasicInfo()
    {
        bzero(&head, sizeof(_STR_PackRoleBasicInfo));
        head.Flag = FLAG_PlayerRoleList;
        head.Len = sizeof(_STR_PackRoleBasicInfo) - sizeof(STR_PackHead);
    }
    STR_PackHead head;
    STR_RoleBasicInfo RoleInfo;
}STR_PackRoleBasicInfo;

//玩家注册数据包
typedef struct _STR_PlayerRegisterUserId
{
    hf_char userName[32];          //用户名
    hf_char password[32];          //密码
    hf_char Email[32];             //邮箱
}STR_PlayerRegisterUserId;

//角色职业属性
typedef struct _STR_RoleJobAttribute
{
    hf_uint32 MaxHP;
    hf_uint32 MaxMagic;
    hf_uint32 PhysicalDefense;
    hf_uint32 MagicDefense;
    hf_uint32 PhysicalAttack;
    hf_uint32 MagicAttack;
    hf_uint16 Rigorous;
    hf_uint16 Will;
    hf_uint16 Wise;
    hf_uint16 Mentality;
    hf_uint16 Physical_fitness;
}STR_RoleJobAttribute;

typedef struct _STR_PlayerRegisterRole
{
    hf_char   Nick[32];
    hf_uint16 ModeID;        //模型ID
    hf_uint16 SkirtID;       //裙子ID
    hf_uint8  Profession;    //职业编号
    hf_uint8  Sex;           //性别
    hf_uint8  Figure;        //体型
    hf_uint8  FigureColor;   //肤色
    hf_uint8  Face;          //脸型
    hf_uint8  Eye;           //眼睛
    hf_uint8  Hair;          //发型
    hf_uint8  HairColor;     //发色
}STR_PlayerRegisterRole;




//角色编号
typedef struct _STR_PlayerRole
{
    hf_uint32 Role;  //角色编号
}STR_PlayerRole;

typedef struct _STR_OtherPlayerInfo
{
    hf_uint32 MaxHP;                 //最大生命值
    hf_uint32 HP;                    //当前生命值
    hf_uint32 MaxMagic;              //最大法力值
    hf_uint32 Magic;                 //当前法力值
}STR_OtherPlayerInfo;

typedef struct _STR_PackRoleCome
{
    _STR_PackRoleCome()
    {
        bzero(&head, sizeof(_STR_PackRoleCome));
        head.Flag = FLAG_ViewRoleCome;
        head.Len = sizeof(_STR_PackRoleCome) - sizeof(STR_PackHead);
    }
    STR_PackHead head;
    STR_RoleBasicInfo roleBasicInfo;
    STR_OtherPlayerPosition  otherRolePos;
    STR_OtherPlayerInfo      otherPlayerInfo;
}STR_PackRoleCome;


//玩家离开可视范围
typedef struct _STR_PackRoleLeave
{
    _STR_PackRoleLeave()
    {
        bzero(&head, sizeof(_STR_PackRoleLeave));
        head.Flag = FLAG_ViewRoleLeave;
        head.Len = sizeof(_STR_PackRoleLeave) - sizeof(STR_PackHead);
    }
    STR_PackHead head;
    hf_uint32 Role;  //角色编号
}STR_PackRoleLeave;

typedef struct _STR_PackFriendOnLine
{
    _STR_PackFriendOnLine()
    {
        bzero(&head, sizeof(_STR_PackFriendOnLine));
        head.Flag = FLAG_FriendOnline;
        head.Len = sizeof(_STR_PackFriendOnLine) - sizeof(STR_PackHead);
    }
    STR_PackHead head;
    hf_uint32 Role;
}STR_PackFriendOnLine;

//玩家登录数据包
typedef struct _STR_PlayerLoginUserId
{
    hf_char userName[32];
    hf_char password[32];
}STR_PlayerLoginUserId;

//玩家复活  test
typedef struct _STR_PlayerRelive
{
    hf_uint16  mode;
}STR_PlayerRelive;

//表结构体
//任务要求
typedef struct _STR_TaskPremise
{
    hf_uint32  TaskID;               //任务ID
    hf_uint32  PreTaskID;            //前置任务ID，只有完成了某一项任务之后才可以接取当前任务
    hf_uint32  CrmtTaskID;           //持有任务，只有角色接取了某一项任务之后才可以接取当前任务
    hf_uint32  GoodsID;              //物品ID，玩家持有某项物品才可以接取当前任务
    hf_uint16  TitleID;              //称号ID，玩家得到某一称号后才可以接取
    hf_uint16  DungeonID;            //副本ID，玩家爱通过某一副本试炼之后才可以接取
    hf_uint8   GenderID;             //性别ID，一般为婚姻任务要求
    hf_uint8   Level;                //等级，玩家达到固定等级才能接取任务
    hf_uint8   JobID;                //职业ID，特定职业的任务
}STR_TaskPremise;


//任务接受方式
/*
1	触发NPC
2	NPC对话
3	使用物品
4	特定地点触发
5	特定地点使用物品
6	邮件触发
7	系统自动接取
*/
typedef struct _STR_TaskAcceptMode
{
    hf_uint8 ModeID;
    hf_char  Mode[16];
}STR_TaskAcceptMode;

//任务执行方式
/*
1	打怪
2	对话
3	收集物品
4	使用物品
5	护送
6	升级
7	选择
8	地点触发对话
*/
typedef struct _STR_TaskExecuteMode
{
    hf_uint8 ModeID;
    hf_char  Mode[16];
}STR_TaskExecuteMode;

//任务交接方式
/*
1	触发NPC
2	NPC对话
3	使用物品
4	特定地点触发
5	特定地点使用物品
6	自动完成
*/
typedef struct _STR_TaskFinishMode
{
    hf_uint8 ModeID;
    hf_char  Mode[16];
}STR_TaskFinishMode;

//怪物
//怪物类型属性
typedef struct _STR_MonsterType
{
    hf_uint32 MonsterTypeID;   //怪物类型ID
    hf_char   MonsterName[32]; //怪物名称
    hf_uint32 HP;              //生命值
    hf_uint32 PhysicalAttack;  //物理攻击
    hf_uint32 MagicAttack;     //魔法攻击
    hf_uint32 PhysicalDefense; //物理防御
    hf_uint32 MagicDefense;    //魔法防御
    hf_uint32 Attackrate;      //攻击速度
    hf_uint32 MoveRate;        //移动速度
    hf_float  Crit_Rate;       //暴击率
    hf_float  Dodge_Rate;      //闪避率
    hf_float  Hit_Rate;        //命中率
    hf_uint8  RankID;          //怪物类别ID
    hf_uint8  Level;           //怪物等级
    hf_uint8  AttackTypeID;    //攻击类型ID
//    hf_uint8  AttackRange;     //攻击距离
}STR_MonsterType;

//怪物类别
typedef struct _STR_MonsterRank
{
    hf_uint8 MonsterRankID;
    hf_char  MonsterRank[16];
}STR_MonsterRank;

//怪物攻击类型
typedef struct _STR_MonsterAttackType
{
    hf_uint8 AttackTypeID;
    hf_char  AttackType[16];
}STR_MonsterAttackType;

//怪物刷新位置
typedef struct _STR_MonsterSpawns
{
    hf_uint32  MonsterTypeID;  //怪物类型ID
    hf_uint32  SpawnsPosID;    //刷新标识索引
    hf_float   Pos_x;          //坐标（x分量）
    hf_float   Pos_y;          //坐标（y分量）
    hf_float   Pos_z;          //坐标（z分量）
    hf_float   Boundary;       //范围
    hf_uint32  MapID;          //地图ID 2015.06.11
    hf_uint8   Amount;         //数量
}STR_MonsterSpawns;

//怪物掉落物品
typedef struct _STR_MonsterLoot
{
    hf_uint32  MonsterTypeID;  //怪物类型ID
    hf_uint32  PreCondition;   //掉落物品前提条件
    hf_uint32  LootGoodsID;    //掉落物品的ID
    hf_float   LootProbability;
    hf_uint8   Count;    //数量
}STR_MonsterLoot;

//怪物技能
typedef struct _STR_MonsterSkill
{
    hf_uint32 MonsterID;      //怪物ID
    hf_uint32 SkillID;        //技能ID
}STR_MonsterSkill;

//请求任务数据
typedef struct _AskTaskData
{
    hf_uint32 TaskID;
    hf_uint16 Flag;
}AskTaskData;

typedef struct _STR_AskTaskExeDlg
{
    hf_uint32 TaskID;
    hf_uint32 AimID;
}STR_AskTaskExeDlg;

typedef struct _RoleNick
{
    _RoleNick()
    {
        memset(nick, 0, 32);
    }
    hf_char nick[32];
}RoleNick;

typedef struct _STR_TaskProcess
{
    hf_uint32  TaskID;
    hf_uint32  AimID;             //任务目标
    hf_uint16  FinishCount;       //已完成任务目标数量
    hf_uint16  AimAmount;         //任务目标总数
    hf_uint8   ExeModeID;         //执行方式
}STR_TaskProcess;

typedef struct _STR_PackTaskProcess
{
    _STR_PackTaskProcess()
    {
        bzero(&head, sizeof(_STR_PackTaskProcess));
        head.Flag = FLAG_TaskProcess;
        head.Len = sizeof(_STR_PackTaskProcess) - sizeof(STR_PackHead);
    }
    _STR_PackTaskProcess(STR_TaskProcess* _process)
    {
        bzero(&head, sizeof(_STR_PackTaskProcess));
        head.Flag = FLAG_TaskProcess;
        head.Len = sizeof(_STR_PackTaskProcess) - sizeof(STR_PackHead);
        memcpy(&process, _process, sizeof(STR_TaskProcess));
    }

    STR_PackHead head;
    STR_TaskProcess process;
}STR_PackTaskProcess;

//typedef struct _STR_RoleInfo
//{

//}STR_RoleInfo;

//角色信息
typedef struct _STR_RoleInfo
{
    _STR_RoleInfo()
    {

    }
    hf_uint32 MaxHP;                 //最大生命值   
    hf_uint32 HP;                    //当前生命值
    hf_uint32 MaxMagic;              //最大法力值
    hf_uint32 Magic;                 //当前法力值
    hf_uint32 PhysicalDefense;       //物理防御值
    hf_uint32 MagicDefense;          //法力防御值
    hf_uint32 PhysicalAttack;        //物理攻击力
    hf_uint32 MagicAttack;           //法术攻击力

    //下面这9个字段的值确定固定不变，跟职业和等级无关
    hf_float  Crit_Rate;             //暴击率 0.02
    hf_float  Dodge_Rate;            //闪避率 0.05
    hf_float  Hit_Rate;              //命中率 0.80
    hf_float  Resist_Rate;           //抵挡率 0.05
    hf_float  Caster_Speed;          //施法速度 1.00
    hf_float  Move_Speed;            //移动速度 1.00  每秒移动的距离
    hf_float  Hurt_Speed;            //攻击速度 1.00  每秒攻击的次数
    hf_uint16 Small_Universe;        //当前小宇宙 在某个等级之后才会有值，假定60级之后会变为100，60级之前都为0
    hf_uint16 maxSmall_Universe;     //最大小宇宙 100

    //下面这10个字段的值从装备获得
    hf_float  RecoveryLife_Percentage;  //每秒恢复生命值百分比
    hf_uint32 RecoveryLife_value;       //每秒恢复生命值
    hf_float  RecoveryMagic_Percentage; //每秒恢复魔法值百分比
    hf_uint32 RecoveryMagic_value;      //每秒恢复魔法值
    hf_float  MagicHurt_Reduction;      //法术伤害减免
    hf_float  PhysicalHurt_Reduction;   //物理伤害减免
    hf_float  CritHurt;                 //暴击伤害
    hf_float  CritHurt_Reduction;       //暴击伤害减免

    //从其他系统来
    hf_float  Magic_Pass;               //法透
    hf_float  Physical_Pass;            //物透

    hf_uint16 Rigorous;                 //严谨
    hf_uint16 Will;                     //机变
    hf_uint16 Wise;                     //睿智
    hf_uint16 Mentality;                //心态
    hf_uint16 Physical_fitness;         //体能
}STR_RoleInfo;

typedef struct _STR_PackRoleInfo
{
    _STR_PackRoleInfo(STR_RoleInfo* _RoleInfo)
    {
        bzero(&head, sizeof(_STR_PackRoleInfo));
        head.Flag = FLAG_RoreInfo;
        head.Len = sizeof(_STR_PackRoleInfo) - sizeof(STR_PackHead);
        memcpy(&RoleInfo, _RoleInfo, sizeof(STR_RoleInfo));
    }

    _STR_PackRoleInfo()
    {
        bzero(&head, sizeof(_STR_PackRoleInfo));
        head.Flag = FLAG_RoreInfo;
        head.Len = sizeof(_STR_PackRoleInfo) - sizeof(STR_PackHead);
    }
    STR_PackHead head;
    STR_RoleInfo RoleInfo;
}STR_PackRoleInfo;


//攻击产生的伤害
typedef struct _STR_PackDamageData
{
    _STR_PackDamageData(hf_uint32 _AimID, hf_uint32 _AttackID, hf_uint32 _Damage, hf_uint8 _TypeID, hf_uint8 _Flag)
    {
        bzero(&head,sizeof(_STR_PackDamageData));
        head.Flag = FLAG_DamageData;
        head.Len = sizeof(_STR_PackDamageData)-sizeof(STR_PackHead);

        AimID = _AimID;
        AttackID = _AttackID;
        Damage = _Damage;
        TypeID = _TypeID;
        Flag = _Flag;
    }

    _STR_PackDamageData()
    {
        bzero(&head,sizeof(_STR_PackDamageData));
        head.Flag = FLAG_DamageData;
        head.Len = sizeof(_STR_PackDamageData)-sizeof(STR_PackHead);
    }

    STR_PackHead head;
    hf_uint32    AimID;       //目标ID
    hf_uint32    AttackID;    //攻击者ID
    hf_uint32    Damage;      //伤害
    hf_uint8     TypeID;      //伤害类型
    hf_uint8     Flag;        //附加标记 暴击，闪避等
} STR_PackDamageData;

//玩家经验
typedef struct _STR_PackRoleExperience
{
    STR_PackHead head;
    hf_uint32 CurrentExp;
    hf_uint32 UpgradeExp;
    hf_uint8  Level;
    _STR_PackRoleExperience()
    {
        bzero(&head,sizeof(_STR_PackRoleExperience));
        head.Flag = FLAG_Experence;
        head.Len = sizeof(_STR_PackRoleExperience)-sizeof(STR_PackHead);
    }
}STR_PackRoleExperience;

//奖励经验
typedef struct _STR_PackRewardExperience
{
    STR_PackHead head;
    hf_uint32  ID;         //可能为怪物ID，也可能为任务ID
    hf_uint32  Experience;
    _STR_PackRewardExperience()
    {
        bzero(&head,sizeof(_STR_PackRewardExperience));
        head.Flag = FLAG_RewardExperience;
        head.Len = sizeof(_STR_PackRewardExperience)-sizeof(STR_PackHead);
    }
}STR_PackRewardExperience;

//好友上线通知包
typedef struct _STR_PackFriendLoginInfo
{
    STR_PackHead head;
    hf_uint32  RoleID;
    _STR_PackFriendLoginInfo()
    {
        bzero(&head,sizeof(_STR_PackFriendLoginInfo));
        head.Flag = FLAG_FriendOnline;
        head.Len = sizeof(_STR_PackFriendLoginInfo)-sizeof(STR_PackHead);
    }
}STR_PackFriendLoginInfo;

//好友列表
typedef struct _STR_FriendInfo
{
    hf_uint32  RoleID;         //角色ID
    hf_char    Nick[32];       //昵称
    hf_uint8   Status;         //状态  是否在线 在线1 不在线2
}STR_FriendInfo;

//增加好友
typedef struct _STR_AddFriend
{
    hf_uint32 RoleID;
    hf_char   Nick[32];
}STR_AddFriend;

typedef struct _STR_PackAddFriend
{
    _STR_PackAddFriend()
    {
        bzero(&head, sizeof(_STR_PackAddFriend));
        head.Flag = FLAG_AddFriend;
        head.Len = sizeof(_STR_PackAddFriend) - sizeof(STR_PackHead);
    }

    STR_PackHead head;
    hf_uint32 RoleID;
    hf_char   Nick[32];
}STR_PackAddFriend;

//删除好友
typedef struct _STR_DeleteFriend
{
    hf_uint32 RoleID;
}STR_DeleteFriend;

typedef struct _STR_PackDeleteFriend
{
    _STR_PackDeleteFriend()
    {
        bzero(&head, sizeof(_STR_PackDeleteFriend));
        head.Flag = FLAG_DeleteFriend;
        head.Len = sizeof(_STR_PackDeleteFriend) - sizeof(STR_PackHead);
    }

    STR_PackHead head;
    hf_uint32 RoleID;
}STR_PackDeleteFriend;

//添加好友返回
typedef struct _STR_PackAddFriendReturn
{
    _STR_PackAddFriendReturn()
    {
        bzero(&head, sizeof(_STR_PackAddFriendReturn));
        head.Flag = FLAG_AddFriendReturn;
        head.Len = sizeof(_STR_PackAddFriendReturn) - sizeof(STR_PackHead);
    }

    STR_PackHead head;
    hf_uint32 RoleID;
    hf_char   Nick[32];
    hf_uint8  value;
}STR_PackAddFriendReturn;


typedef struct _NPCInfo
{
    hf_uint32 NpcID;
    hf_uint32 Mapid;
    hf_float  Pos_x;
    hf_float  Pos_y;
    hf_float  Pos_z;
}NPCInfo;


//计算玩家选取地图上一个点攻击产生的伤害
typedef struct _RoleAttackPoint
{
    hf_uint32  HurtTime;
    hf_float   Pos_x;
    hf_float   Pos_y;
    hf_float   Pos_z;
    hf_uint32  SkillID;
}RoleAttackPoint;

//计算玩家使用技能攻击目标产生的伤害
typedef struct _RoleAttackAim
{
    hf_double  HurtTime;          //技能生效时间
    hf_uint32  AimID;             //目标ID
    hf_uint32  SkillID;
}RoleAttackAim;

typedef struct _STR_PackSkillInfo
{
    hf_uint32  SkillID;        //技能ID
    hf_uint32  UseGoodsID;     //消耗物品ID
    hf_uint32  UseMagic;       //消耗魔法值
    hf_float   CoolTime;       //冷却时间 冷却时间>施法时间+引导时间
    hf_float   CastingTime;    //施法时间
    hf_float   LeadTime;       //引导时间
    hf_uint32  PhysicalHurt;   //物理伤害
    hf_float   PhyPlus;        //物理伤害加成
    hf_uint32  MagicHurt;      //魔法伤害
    hf_float   MagPlus;        //魔法伤害加成
    hf_uint16  UseGoodsCount;  //消耗物品数量
    hf_uint16  FarDistance;    //技能范围具体参数2
    hf_uint8   NearlyDistance; //技能范围具体参数1
    hf_uint8   TriggerID;      //触发类型ID
    hf_uint8   SkillRangeID;   //技能范围类型ID
    hf_uint8   UseAnger;       //消耗怒气值
    hf_uint8   CasterNumber;   //施法次数
}STR_PackSkillInfo;

typedef struct _STR_PlayerSkill
{
    hf_uint32 SkillID;
    hf_uint32 CoolTime;          //冷却时间
    hf_uint32 CastingTime;       //施法时间
    hf_uint32 LeadTime;          //引导时间
}STR_PlayerSkill;

//技能释放结果
typedef struct _STR_PackSkillResult
{
    STR_PackHead head;
    hf_uint32 SkillID;
    hf_uint8  result;
    _STR_PackSkillResult()
    {
        bzero(&head,sizeof(_STR_PackSkillResult));
        head.Flag =  FLAG_SKILLRESULT;
        head.Len = sizeof(_STR_PackSkillResult)-sizeof(STR_PackHead);
    }
}STR_PackSkillResult;

//施法效果
typedef struct _STR_PackSkillAimEffect
{
    _STR_PackSkillAimEffect(hf_uint32 aim = 0,hf_uint32 skill = 0,hf_uint32 role = 0)
        :AimID(aim),SkillID(skill),RoleID(role)
    {
        bzero(&head,sizeof(STR_PackHead));
        head.Flag =  FLAG_SkillAimResult;
        head.Len = sizeof(_STR_PackSkillAimEffect)-sizeof(STR_PackHead);
    }
    STR_PackHead head;
    hf_uint32    AimID;     //施法目标
    hf_uint32    SkillID;   //技能ID
    hf_uint32    RoleID;    //施法者
}STR_PackSkillAimEffect;

//施法位置效果
typedef struct _STR_PackSkillPosEffect
{
    _STR_PackSkillPosEffect()
    {
        bzero(&head,sizeof(STR_PackHead));
        head.Flag =  FLAG_SkillPosResult;
        head.Len = sizeof(_STR_PackSkillPosEffect)-sizeof(STR_PackHead);
    }
    STR_PackHead head;
    hf_float     Pos_x;
    hf_float     Pos_y;
    hf_float     Pos_z;
    hf_uint32    MapID;
    hf_uint32    SkillID;   //技能ID
    hf_uint32    RoleID;    //施法者
}STR_PackSkillPosEffect;


typedef struct _STR_PackRecvChat
{
    _STR_PackRecvChat()
    {
        bzero(&head,sizeof(STR_PackHead));
        head.Flag =  FLAG_Chat;
        head.Len = sizeof(_STR_PackRecvChat)-sizeof(STR_PackHead);
    }
    STR_PackHead head;
    hf_char  chatMessage[256];
}STR_PackRecvChat;

typedef struct _STR_PackSendChat
{
    _STR_PackSendChat(hf_char* _chatMessage, char* _userName)
    {
        bzero(&head,sizeof(STR_PackHead));
        head.Flag =  FLAG_Chat;
        head.Len = sizeof(_STR_PackSendChat)-sizeof(STR_PackHead);
        memcpy(userName, _userName, 32);
        memcpy(&chatMessage, _chatMessage, 256);
    }
    STR_PackHead head;
    hf_char  userName[32];
    hf_char  chatMessage[256];
}STR_PackSendChat;

//下面一些结构体为实时更新用户数据而定义
///////////////////////////////////////////////////////////////////////////
typedef struct _UpdateMoney             //更新金钱
{
    _UpdateMoney(hf_uint32 roleid, STR_PlayerMoney* money)
        :RoleID(roleid)
    {
        memcpy(&Money, money, sizeof(STR_PlayerMoney));
    }
    _UpdateMoney()
    {

    }

    hf_uint32 RoleID;
    STR_PlayerMoney Money;
}UpdateMoney;

typedef struct _UpdateLevel             //更新等级
{
    _UpdateLevel(hf_uint32 roleid, hf_uint8 level)
        :RoleID(roleid),Level(level)
    {
    }
    _UpdateLevel()
    {

    }
    hf_uint32 RoleID;
    hf_uint8  Level;
}UpdateLevel;

typedef struct _UpdateExp              //更新经验
{
    _UpdateExp(hf_uint32 roleid, hf_uint32 exp)
        :RoleID(roleid),Exp(exp)
    {
    }
    _UpdateExp()
    {
    }
    hf_uint32 RoleID;
    hf_uint32 Exp;
}UpdateExp;

typedef struct _UpdateGoods            //更新背包某位置的物品
{
    _UpdateGoods(hf_uint32 roleid, STR_Goods* goods, hf_uint8 operate)
        :RoleID(roleid), Operate(operate)
    {
        memcpy(&Goods, goods, sizeof(STR_Goods));
    }
    _UpdateGoods()
    {
    }
    hf_uint32 RoleID;
    STR_Goods Goods;
    hf_uint8  Operate;
}UpdateGoods;



typedef struct _UpdateEquAttr        //更新某装备的属性
{
    _UpdateEquAttr(hf_uint32 roleid, STR_EquipmentAttr* equ, hf_uint8 operate)
        :RoleID(roleid),Operate(operate)
    {
        memcpy(&EquAttr, equ, sizeof(STR_EquipmentAttr));
    }
    _UpdateEquAttr()
    {
    }
    hf_uint32 RoleID;
    STR_EquipmentAttr EquAttr;
    hf_uint8  Operate;
}UpdateEquAttr;


typedef struct _UpdateTask         //更新任务进度
{
    _UpdateTask(hf_uint32 roleid, STR_TaskProcess* task, hf_uint8 operate)
        :RoleID(roleid),Operate(operate)
    {
        memcpy(&TaskProcess, task, sizeof(STR_TaskProcess));
    }
    _UpdateTask()
    {
    }
    hf_uint32     RoleID;
    STR_TaskProcess TaskProcess;
    hf_uint8  Operate;
}UpdateTask;


typedef struct _UpdateCompleteTask
{
    _UpdateCompleteTask(hf_uint32 _RoleID, hf_uint32 _TaskID)
        :RoleID(_RoleID),TaskID(_TaskID)
    {

    }
    _UpdateCompleteTask()
    {

    }

    hf_uint32 RoleID;
    hf_uint32 TaskID;
}UpdateCompleteTask;


typedef struct _STR_Consumable
{
    hf_uint32  GoodsID;          //物品ID
    hf_uint32  HP;               //血量瞬间恢复值
    hf_uint32  Magic;            //魔法值瞬间恢复值
    hf_uint16  ColdTime;         //冷却时间
    hf_uint16  StackNumber;      //堆叠数
    hf_uint16  PersecondHP;      //每秒恢复血量值
    hf_uint16  PersecondMagic;   //每秒恢复魔法值
    hf_uint8   UserLevel;        //等级限制
    hf_uint8   ContinueTime;     //持续时间
    hf_uint8   Type;             //恢复类别
}STR_Consumable;

typedef struct _STR_UseBagGoods
{
    hf_uint32 goodsid;   //物品ID
    hf_uint8  pos;       //在背包的位置
}STR_UseBagGoods;


typedef struct _STR_RecoveryHP
{
    _STR_RecoveryHP(hf_double _Timep, hf_uint16 _HP, hf_uint8 _Count)
        :Timep(_Timep),HP(_HP),Count(_Count)
    {

    }
    _STR_RecoveryHP()
    {

    }

    hf_double Timep;    //下次恢复时间
    hf_uint16 HP;       //恢复血量值
    hf_uint8  Count;    //剩余恢复时间（次数） 1秒恢复1次
}STR_RecoveryHP;

typedef struct _STR_RecoveryMagic
{
    _STR_RecoveryMagic(hf_double _Timep, hf_uint16 _Magic, hf_uint8 _Count)
        :Timep(_Timep),Magic(_Magic),Count(_Count)
    {

    }
    _STR_RecoveryMagic()
    {

    }

    hf_double Timep;    //下次恢复时间
    hf_uint16 Magic;    //恢复魔法值
    hf_uint8  Count;    //剩余恢复时间（次数） 1秒恢复1次
}STR_RecoveryMagic;


typedef struct _STR_RecoveryHPMagic
{
    _STR_RecoveryHPMagic(hf_double _Timep, hf_uint16 _HP, hf_uint16 _Magic, hf_uint8 _Count)
        :Timep(_Timep),HP(_HP),Magic(_Magic),Count(_Count)
    {

    }
    _STR_RecoveryHPMagic()
    {

    }

    hf_double Timep;    //下次恢复时间
    hf_uint16 HP;       //恢复血量值
    hf_uint16 Magic;    //恢复魔法值
    hf_uint8  Count;    //剩余恢复时间（次数） 1秒恢复1次
}STR_RecoveryHPMagic;





////////////////////////////////////////////////////////////////////////////////


//NPC动作索引
enum NPCActionIndex
{
    idle = 1,                //空闲
    walk,                    //行走
    hurt,                    //被攻击
    attack,                  //发出攻击
    death                    //死亡
};


//任务接受方式
enum taskAcceptMode
{
    trigger_NPC_ACC = 1,	//触发NPC
    dialogue_NPC_ACC,    	//NPC对话
    use_items_ACC,      	//使用物品
    location_trigger_ACC,	//特定地点触发
    location_use_items_ACC,	//特定地点使用物品
    email_trigger_ACC,   	//邮件触发
    auto_ACC     	        //系统自动接取
};

//任务交接方式
enum taskFinishMode
{
    trigger_NPC_FIN = 1,	//触发NPC
    dialogue_NPC_FIN,	    //NPC对话
    use_items_FIN,	        //使用物品
    location_trigger_FIN,	//特定地点触发
    location_use_items_FIN,	//特定地点使用物品
    auto_FIN	            //自动完成
};

//任务执行方式
enum TaskExeMode
{
    EXE_attack_monster = 1,     //打怪
    EXE_dialogue,               //对话
    EXE_collect_goods,          //收集物品
    EXE_use_goods,              //使用物品
    EXE_escort,                 //护送
    EXE_upgrade,                //升级
    EXE_choice,                 //选择
    EXE_email_dialogue          //地点触发对话
};


//请求
typedef struct _STR_PackRequestOper
{
    _STR_PackRequestOper()
    {
        bzero(&head,sizeof(_STR_PackRequestOper));
        head.Flag =  FLAG_OperRequest;
        head.Len = sizeof(_STR_PackRequestOper)-sizeof(STR_PackHead);
    }

    _STR_PackRequestOper(hf_uint32 _RoleID, hf_uint8 _OperType)
    {
        bzero(&head,sizeof(_STR_PackRequestOper));
        head.Flag =  FLAG_OperRequest;
        head.Len = sizeof(_STR_PackRequestOper)-sizeof(STR_PackHead);
        RoleID = _RoleID;
        OperType = _OperType;
    }

    STR_PackHead head;
    hf_uint32    RoleID;
    hf_uint8     OperType;
}STR_PackRequestOper;

//请求返回结果
typedef struct _STR_PackRequestReply
{    
    _STR_PackRequestReply()
    {
        bzero(&head,sizeof(_STR_PackRequestReply));
        head.Flag =  FLAG_OperResult;
        head.Len = sizeof(_STR_PackRequestReply)-sizeof(STR_PackHead);
    }
    _STR_PackRequestReply(hf_uint32 _RoleID, hf_uint8 _OperType, hf_uint8 _OperResult)
    {
        bzero(&head,sizeof(_STR_PackRequestReply));
        head.Flag =  FLAG_OperResult;
        head.Len = sizeof(_STR_PackRequestReply)-sizeof(STR_PackHead);

        RoleID = _RoleID;
        OperType = _OperType;
        OperResult = _OperResult;
    }

    STR_PackHead head;
    hf_uint32    RoleID;
    hf_uint8     OperType;
    hf_uint8     OperResult;
}STR_PackRequestReply;



typedef struct _operationRequest
{
    STR_PackHead head;
    hf_uint32    RoleID;
    hf_char      operType;
    _operationRequest()
    {
        bzero(&head,sizeof(_operationRequest));
        head.Flag =  FLAG_OperRequest;
        head.Len = sizeof(_operationRequest)-sizeof(STR_PackHead);
    }
}operationRequest;

typedef struct _operationRequestResult
{
    STR_PackHead head;
    hf_uint32    RoleID;
    hf_char      operType;
    hf_char      operResult;
    _operationRequestResult()
    {
        bzero(&head,sizeof(_operationRequestResult));
        head.Flag =  FLAG_OperResult;
        head.Len = sizeof(_operationRequestResult)-sizeof(STR_PackHead);
    }
}operationRequestResult;


typedef struct _interchangeOperGoods
{
    STR_PackHead head;
    hf_uint32    RoleID;
    hf_uint32    goodsId;
    hf_uint32    goodsType;
    hf_uint16    goodsCount;
    hf_uint8    position;
    hf_char      operType;
    _interchangeOperGoods()
    {
        bzero(&head,sizeof(_interchangeOperGoods));
        head.Flag =  FLAG_InterchangeGoods;
        head.Len = sizeof(_interchangeOperGoods)-sizeof(STR_PackHead);
    }
}interchangeOperGoods;

typedef struct _interchangeOperPro
{
    STR_PackHead head;
    hf_char      operType;
    _interchangeOperPro()
    {
        bzero(&head,sizeof(_interchangeOperPro));
        head.Flag =  FLAG_InterchangeOperPro;
        head.Len = sizeof(_interchangeOperPro)-sizeof(STR_PackHead);
    }
}interchangeOperPro;

typedef struct _interchangeMoney
{
    STR_PackHead head;
    hf_uint32    roleId;
    hf_uint32    MoneyCount;
    hf_char      MoneyType;
    _interchangeMoney()
    {
        bzero(&head,sizeof(_interchangeMoney));
        head.Flag =  FLAG_InterchangeMoneyCount;
        head.Len = sizeof(_interchangeMoney)-sizeof(STR_PackHead);
    }
}interchangeMoney;

#endif // POSTGRESQLSTRUCT_H

