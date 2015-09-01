#include "getdefinevalue.h"

hf_int32 GetDamage_reduction(hf_uint8 level)
{
    switch(level)
    {
    case 1:
        return DEFENSE_1;
    case 2:
        return DEFENSE_2;
    case 3:
        return DEFENSE_3;
    case 4:
        return DEFENSE_4;
    case 5:
        return DEFENSE_5;
    case 6:
        return DEFENSE_6;
    case 7:
        return DEFENSE_7;
    case 8:
        return DEFENSE_8;
    case 9:
        return DEFENSE_9;
    case 10:
        return DEFENSE_10;
    case 11:
        return DEFENSE_11;
    case 12:
        return DEFENSE_12;
    case 13:
        return DEFENSE_13;
    case 14:
        return DEFENSE_14;
    case 15:
        return DEFENSE_15;

    default:
        return 0;
    }
}

hf_uint32 GetRewardExperience(hf_uint8 level)
{
    switch(level)
    {
    case 1:
        return 100;
    case 2:
        return 200;
    case 3:
        return 300;
    case 16:
        return REWARD_EXP16;
    case 17:
        return REWARD_EXP17;

    default:
        return 0;
    }
}


hf_uint32 GetUpgradeExprience(hf_uint8 level)
{
    switch(level)
    {
    case 1:
        return GRADE_1;
    case 2:
        return GRADE_2;
    case 3:
        return GRADE_3;
    case 4:
        return GRADE_4;
    case 5:
        return GRADE_5;
    case 6:
        return GRADE_6;
    case 7:
        return GRADE_7;
    case 8:
        return GRADE_8;
    case 9:
        return GRADE_9;
    case 10:
        return GRADE_10;
    case 11:
        return GRADE_11;
    case 12:
        return GRADE_12;
    case 13:
        return GRADE_13;
    case 14:
        return GRADE_14;
    case 15:
        return GRADE_15;
    case 16:
        return GRADE_16;
    case 17:
        return GRADE_17;
    case 18:
        return GRADE_18;
    case 19 :
        return GRADE_19;
    case 20:
        return GRADE_20;
    case 21:
        return GRADE_21;
    case 22:
        return GRADE_22;
    case 23:
        return GRADE_23;
    case 24:
        return GRADE_24;
    case 25:
        return GRADE_25;
    case 26:
        return GRADE_26;
    case 27:
        return GRADE_27;
    case 28:
        return GRADE_28;
    case 29:
        return GRADE_29;
    case 30:
        return GRADE_30;

    default:
        return 0;

    }
}
