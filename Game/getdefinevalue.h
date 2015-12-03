#ifndef GETDEFINEVALUE_HPP
#define GETDEFINEVALUE_HPP
#include "Game/levelexperience.h"
#include "hf_types.h"

using namespace hf_types;
//得到伤害减免系数
hf_uint32 GetDamage_reduction(hf_uint8 level);

//得到怪物奖励经验
hf_uint32 GetRewardExperience(hf_uint8 level);

//得到怪物奖励金钱
hf_uint32 GetRewardMoney(hf_uint8 level);

//得到升级经验
hf_uint32 GetUpgradeExprience(hf_uint8 level);

#endif // GETDEFINEVALUE_HPP

