#ifndef POSTGRESQLSTRUCT_H
#define POSTGRESQLSTRUCT_H

#include "hf_types.h"

using namespace hf_types;
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

//2.任务概况数据
typedef struct _STR_PackTaskProfile
{
    hf_int32 TaskID;             //唯一标识
    hf_char  TaskName[32];       //任务名称
    hf_int32 StartNPCID;         //任务发布NPCID
    hf_int32 FinishNPCID;        //任务交接NPCID
    hf_int32 TaskPropsID;        //任务道具ID
    hf_int32 Time;               //时间，秒为单位
    hf_uint8 AcceptModeID;       //任务接受方式
    hf_uint8 FinishModeID;       //任务完成方式
}STR_PackTaskProfile;

//3.任务对话数据
typedef struct _STR_PackTaskDlg
{
    hf_int32 TaskID;
    hf_char  StartDialogue[256];  //任务开始对话
    hf_char  FinishDialogue[256]; //任务结束对话
}STR_PackTaskDlg;

//4.任务描述数据
typedef struct _STR_TaskDescription
{
    hf_int32 TaskID;
    hf_char  Description[256];   //任务描述
}STR_TaskDescription;

//5.任务目标数据
typedef struct _STR_PackTaskAim
{
    hf_int32 TaskID;
    hf_int32 AimID;              //任务目标ID
    hf_int32 Amount;             //任务数量ID
    hf_uint8 ExeModeID;          //执行方式ID
}STR_PackTaskAim;

//6.任务奖励数据
typedef struct _STR_PackTaskReward
{
    hf_int32 TaskID;
    hf_int32 Experience;         //奖励经验值
    hf_int32 Money;              //奖励金钱
    hf_int32 SkillID;            //奖励技能ID
    hf_int32 TitleID;            //奖励称号ID
    hf_uint8 Attribute;          //奖励属性点
}STR_PackTaskReward;

//7.物品奖励数据
typedef struct _STR_PackGoodsReward
{
    hf_int32  TaskID;
    hf_int32  GoodsID;           //奖励物品ID
    hf_uint16 Count;             //奖励物品数量
}STR_PackGoodsReward;

//8.玩家请求接受任务数据
typedef struct _STR_PackUserAskTask
{
    hf_int32 RoleID;             //玩家角色ID
    hf_int32 TaskID;             //任务ID
}STR_PackUserAskTask;

//9.玩家接受任务结果数据
/*result
 * 0 成功
 * 1 失败，未完成前置任务
 * 2 失败，未接取条件任务
 * 3 失败，未持有任务物品
 * 4 失败，为获得任务条件称号
 * 5 失败，未完成条件副本
 * 6 失败，性别不符
 * 7 失败，等级不足
 * 8 失败，职业不符
 */
typedef struct _STR_PackAskResult
{
    hf_int32 RoleID;
    hf_int32 TaskID;
    hf_uint8 Result;             //根据Result值判断成功或者失败原因
}STR_PackAskResult;

//10.玩家角色任务进度数据
typedef struct _STR_PackTaskProcess
{
    hf_int32  RoleID;
    hf_int32  TaskID;
    hf_int32  AimID;             //任务目标
    hf_uint16 AimCount;          //已完成任务目标数量
    hf_uint16 AimAmount;         //任务目标总数
    hf_uint8  ExeModeID;         //执行方式
}STR_PackTaskProcess;

//11 玩家请求完成任务数据
typedef struct _STR_PackFinishTask
{
    hf_int32 RoleID;
    hf_int32 TaskID;
    hf_int32 SelectGoodsID;      // 可能出现奖励物品多选一，玩家选择需要的物品发送给服务器
}STR_PackFinishTask;

//放弃任务 2015.05.06
typedef struct _STR_PackQuitTask
{
    hf_int32 RoleID;
    hf_int32 TaskID;
}STR_PackQuitTask;

//12.玩家任务结果数据
/*result
 * 0 成功
 * 1 失败，请选择奖励
 * 2 失败，奖励物品不符合
 */
typedef struct _STR_PackUserTaskResult
{
    hf_int32 RoleID;
    hf_int32 TaskID;
    hf_uint8 Result;             //根据Result值判断成功或者失败原因
}STR_PackUserTaskResult;

//13，怪物信息数据
typedef struct _STR_PackMonsterInfo
{
    hf_int32  MonsterID;          //怪物的唯一标识
    hf_char   MonsterName[32];    //怪物类型，用于记录怪物名称以及需要使用的怪物模型
    hf_int32  Current_x;          //当前坐标中x轴分量
    hf_int32  Current_y;          //当前坐标中y轴分量
    hf_int32  Current_z;          //当前坐标中z轴分量
    hf_int32  Target_x;
    hf_int32  Target_y;
    hf_int32  Target_z;
    hf_int32  MoveRate;           //移动速率按百分比计算为某一个固定值的倍数，例如300%，这里只记录300
    hf_int32  HP;                 //怪物当前剩余血量，在一开始，HP = MaxHP
    hf_int32  MaxHP;              //最大血量
    hf_uint8  MonsterLevel;       //怪物等级  2015.05.06
    hf_uint16 Direct;             //怪物朝向角度0...359
    hf_uint8  ActID;              //怪物当前动作的索引
    hf_uint8  Flag;               //是否显示，是否死亡后立即从怪物列表清除等等附加信息
}STR_PackMonsterInfo;

//14.怪物属性数据包
typedef struct _STR_PackMonsterAttrbt
{
    hf_int32 MonsterID;         //怪物ID；
    hf_int32 HP;                //当前血量
}STR_PackMonsterAttrbt;

//15.怪物位置数据包
typedef struct _STR_PackMonsterPosition
{
    hf_int32  MonsterID;         //怪物ID，在客户端中需要转换为字符串，表示唯一标识
    hf_int32  Pos_x;             //当前坐标中x轴分量
    hf_int32  Pos_y;             //当前坐标中y轴分量
    hf_int32  Pos_z;             //当前坐标中z轴分量
    hf_uint16 Direct;            //怪物朝向角度0...359
    hf_uint8  ActID;             //怪物当前动作的索引
}STR_PackMonsterPosition;

//16.玩家攻击信息数据包
typedef struct _STR_PackUserAttack
{
    hf_int32  RoleID;
    hf_int32  MonsterID;         //攻击怪物ID
    hf_int32  SkillID;           //使用的技能ID
    hf_uint16 Direct;            //玩家朝向0...359
}STR_PackUserAttack;

//17.掉落物品数据包
typedef struct _STR_PackLootGoods
{
    hf_int32  LootGoodsID;
    hf_int32  pos_x;
    hf_int32  pos_y;
    hf_int32  pos_z;
    hf_int16  GoodsFlag;         //用于区分掉落的物品，例如同一地点有多个相同物品
    hf_uint16 Count;
}STR_PackLootGoods;

//18.玩家拾取数据包
typedef struct _STR_PackUserPick
{
    hf_int32 RoleID;
    hf_int32 LootGoodsID;
    hf_uint16 GoodsFlag;
}STR_PackUserPick;

//19.玩家获得物品数据包
typedef struct _STR_PackUserGainGoods
{
    hf_int32  RoleID;
    hf_int32  LootGoodsID;
    hf_uint16 Count;
}STR_PackUserGainGoods;

//玩家注册数据包
typedef struct _STR_PackPlayerRegister
{
    hf_char userName[32];      //用户名
    hf_char password[32];      //密码
    hf_uint16 Role;            //角色
    hf_uint16 Sex;             //性别 0标识男，1标识女
    hf_char Email[32];         //邮箱
}STR_PackPlayerRegister;

//玩家登录数据包
typedef struct _STR_PackPlayerLogin
{
    hf_char userName[32];
    hf_char password[32];
    hf_int16 Role;
}STR_PackPlayerLogin;

typedef struct _STR_PlayerPosition
{
    hf_char username[32];    //用户名
    hf_float Pos_x;          //x坐标
    hf_float Pos_y;
    hf_float Pos_z;
}STR_PlayerPostion;

//表结构体
//1.任务要求
typedef struct _T_TaskPremise
{
    hf_int32  TaskID;               //任务ID
    hf_int32  PreTaskID;            //前置任务ID，只有完成了某一项任务之后才可以接取当前任务
    hf_int32  CrmtTaskID;           //持有任务，只有角色接取了某一项任务之后才可以接取当前任务
    hf_int32  GoodsID;              //物品ID，玩家持有某项物品才可以接取当前任务
    hf_uint16 TitleID;              //称号ID，玩家得到某一称号后才可以接取
    hf_uint16 DungeonID;            //副本ID，玩家爱通过某一副本试炼之后才可以接取
    hf_uint8  GenderID;             //性别ID，一般为婚姻任务要求
    hf_uint8  Level;                //等级，玩家达到固定等级才能接取任务
    hf_uint8  JobID;                //职业ID，特定职业的任务
}T_TaskPremise;

//2.任务概述
typedef struct _T_TaskProfile
{
    hf_int32 TaskID;                 //唯一标识
    hf_char  TaskName[32];           //任务名称
    hf_int32 StartNPCID;             //任务发布NPCID
    hf_int32 FInishNPCID;            //任务交接NPCID
    hf_int32 TaskPropsID;            //任务道具ID
    hf_int32 Time;                   //时间，单位秒
    hf_uint8 AcceptModeID;           //任务接受方式ID
    hf_uint8 FinishModeId;           //任务完成方式ID
}T_TaskProfile;

//3.任务对话
typedef struct _T_TaskDialogue
{
    hf_int32 TaskID;
    hf_char  StartDialogue[256];     //任务开始对话
    hf_char  FinishDialogue[256];    //任务完成对话
}T_TaskDialogue;

//4.任务描述
typedef struct _T_TaskDescription
{
    hf_int32 TaskID;
    hf_char  Description[256];       //任务描述
}T_TaskDescription;

//5.任务目标
typedef struct _T_TaskAim
{
    hf_int32 TaskID;
    hf_int32 AimId;                  //任务目标ID
    hf_int32 Amount;                 //任务数量
    hf_uint8 ExeModeID;              //执行方式ID
}T_TaskAim;

//6.任务奖励
typedef struct _T_TaskReward
{
    hf_int32 TaskID;
    hf_int32 Experience;             //经验值
    hf_int32 Money;                  //金钱
    hf_int32 SkillID;                //技能ID
    hf_int32 TitleID;                //奖励称号
    hf_uint8 Attribute;              //奖励属性点
}T_TaskReward;

//7.物品奖励
typedef struct _T_GoodsReward
{
    hf_int32  TaskID;
    hf_int32  GoodsID;              //奖励物品ID
    hf_uint16 Count;                //奖励物品数量
}T_GoodsReward;

//8.职业类型
typedef struct _T_JobType
{
    hf_uint8 JobID;                //玩家角色ID
    hf_char  JobName[16];          //玩家名
}T_JobType;

//9.性别
typedef struct _T_Gender
{
    hf_uint8 GenderID;
    hf_char  Gender[4];
}T_Gender;

//10.任务接受方式
/*
1	触发NPC
2	NPC对话
3	使用物品
4	特定地点触发
5	特定地点使用物品
6	邮件触发
7	系统自动接取
*/
typedef struct _T_TaskAcceptMode
{
    hf_uint8 ModeID;
    hf_char  Mode[16];
}T_TaskAcceptMode;

//11.任务执行方式
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
typedef struct _T_TaskExecuteMode
{
    hf_uint8 ModeID;
    hf_char  Mode[16];
}T_TaskExecuteMode;

//12.任务交接方式
/*
1	触发NPC
2	NPC对话
3	使用物品
4	特定地点触发
5	特定地点使用物品
6	自动完成
*/
typedef struct _T_TaskFinishMode
{
    hf_uint8 ModeID;
    hf_char  Mode[16];
}T_TaskFinishMode;

//怪物
//13.怪物类型属性
typedef struct _T_MonsterType
{
    hf_int32 MonsterTypeID;   //怪物类型ID
    hf_char  MonsterName[32]; //怪物名称
    hf_uint8 MonsterRankID;   //怪物类别ID
    hf_uint8 MonsterLevel;    //怪物等级
    hf_uint8 AttackTypeID;    //攻击类型ID
    hf_int32 HP;              //生命值
    hf_int32 Attack;          //物理攻击
    hf_int32 MagicAttack;     //魔法攻击
    hf_int32 Defence;         //物理防御
    hf_int32 MagicDefence;    //魔法防御
    hf_int32 Attackrate;      //攻击速度
    hf_int32 MoveRate;        //移动速度
    hf_int32 Experience;      //经验值
}T_MonsterType;

//14.怪物类别
typedef struct _T_MonsterRank
{
    hf_uint8 MonsterRankID;
    hf_char  MonsterRank[16];
}T_MonsterRank;

//15.怪物攻击类型
typedef struct _T_MonsterAttackType
{
    hf_uint8 AttackTypeID;
    hf_char  AttackType;
}T_MonsterAttackType;

//16.怪物刷新位置
typedef struct _T_MonsterSpawns
{
    hf_int32  MonsterTypeID;  //怪物类型ID
    hf_int32  SpawnsPosID;    //刷新标识索引
    hf_float  Pos_x;          //坐标（x分量）
    hf_float  Pos_y;          //坐标（y分量）
    hf_float  Pos_z;          //坐标（z分量）
    hf_float  Boundary_x;     //x坐标取值范围
    hf_float  Boundary_y;     //y坐标取值范围
    hf_float  Boundary_z;     //z坐标取值范围
    hf_uint8  Amount;         //数量
}T_MonsterSpawns;

//17.怪物掉落物品
typedef struct _T_MonsterLoot
{
    hf_int32  MonsterTypeID;  //怪物类型ID
    hf_int32  LootGoodsID;    //掉落物品的ID
    hf_float  LootProbability;
}T_MonsterLoot;

//18.怪物技能
typedef struct _T_MonsterSkill
{
    hf_int32 MonsterID;      //怪物ID
    hf_int32 SkillID;        //技能ID
}T_MonsterSkill;

//玩家登录表
typedef struct _T_PlayerLogin
{
    hf_char userName[32];
    hf_char password[32];
    hf_int16 Role;
}T_PlayerLogin;

//玩家注册表
typedef struct _T_PlayerRegister
{
    hf_char userName[32];      //用户名
    hf_char password[32];      //密码
    hf_uint16 Role;            //角色
    hf_uint16 Sex;             //性别 0标识男，1标识女
    hf_char Email[32];         //邮箱
}T_PlayerRegister;

//NPC动作索引
enum NPCActionIndex
{
    idle = 1,                //空闲
    walk,                    //行走
    hurt,                    //被攻击
    attack,                  //发出攻击
    death                    //死亡
};

#endif // POSTGRESQLSTRUCT_H

