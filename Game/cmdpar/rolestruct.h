#ifndef ROLESTRUCT_H
#define ROLESTRUCT_H

#include <boost/atomic.hpp>
#include "./../hf_types.h"

using namespace hf_types;


typedef struct _STR_Roel
{
    boost::atomic_uint32_t hp;
    boost::atomic_uint32_t maxHp;
}STR_Roel;


//typedef struct _STR_RoleInfo
//{
//    _STR_RoleInfo& operator=(_STR_RoleInfo& role)
//    {
//        hf_uint32 hp = role.HP;
//        HP = hp;
//    }

//    _STR_RoleInfo(_STR_RoleInfo& role)
//    {
//        hf_uint32 hp = role.HP;
//        HP = hp;
//    }

//    hf_uint32 MaxHP;                 //最大生命值
//    boost::atomic_uint32_t HP;                    //当前生命值
//    hf_uint32 MaxMagic;              //最大法力值
//    hf_uint32 Magic;                 //当前法力值
//    hf_uint32 PhysicalDefense;       //物理防御值
//    hf_uint32 MagicDefense;          //法力防御值
//    hf_uint32 PhysicalAttack;        //物理攻击力
//    hf_uint32 MagicAttack;           //法术攻击力

//    //下面这9个字段的值确定固定不变，跟职业和等级无关
//    hf_float  Crit_Rate;             //暴击率 0.02
//    hf_float  Dodge_Rate;            //闪避率 0.05
//    hf_float  Hit_Rate;              //命中率 0.80
//    hf_float  Resist_Rate;           //抵挡率 0.05
//    hf_float  Caster_Speed;          //施法速度 1.00
//    hf_float  Move_Speed;            //移动速度 1.00  每秒移动的距离
//    hf_float  Hurt_Speed;            //攻击速度 1.00  每秒攻击的次数
//    hf_uint16 Small_Universe;        //当前小宇宙 在某个等级之后才会有值，假定60级之后会变为100，60级之前都为0
//    hf_uint16 maxSmall_Universe;     //最大小宇宙 100

//    //下面这10个字段的值从装备获得
//    hf_float  RecoveryLife_Percentage;  //每秒恢复生命值百分比
//    hf_uint32 RecoveryLife_value;       //每秒恢复生命值
//    hf_float  RecoveryMagic_Percentage; //每秒恢复魔法值百分比
//    hf_uint32 RecoveryMagic_value;      //每秒恢复魔法值
//    hf_float  MagicHurt_Reduction;      //法术伤害减免
//    hf_float  PhysicalHurt_Reduction;   //物理伤害减免
//    hf_float  CritHurt;                 //暴击伤害
//    hf_float  CritHurt_Reduction;       //暴击伤害减免

//    //从其他系统来
//    hf_float  Magic_Pass;               //法透
//    hf_float  Physical_Pass;            //物透

//    hf_uint16 Rigorous;                 //严谨
//    hf_uint16 Will;                     //机变
//    hf_uint16 Wise;                     //睿智
//    hf_uint16 Mentality;                //心态
//    hf_uint16 Physical_fitness;         //体能
//}STR_RoleInfo;

#endif // ROLESTRUCT_H

