#ifndef STRUCT_H
#define STRUCT_H

#include <stdint.h>
#include <string.h>

#include "hf_types.h"
#include "postgresqlstruct.h"

using namespace hf_types;


//登录分为登录用户帐号和角色
#define FLAG_PlayerLoginUserId       100     //玩家登录用户名Flag
#define FLAG_PlayerLoginRole         101     //玩家登录角色Flag

#define FLAG_PlayerRegisterUserId    102     //玩家注册用户名Flag
#define FLAG_PlayerRegisterRole      103     //玩家注册角色Flag

//用户操作结果
#define FLAG_Result                  104

#define FLAG_LootGoods        201   //掉落物品数据包Flag
#define FLAG_UserPick         202   //玩家拾取数据包Flag
#define FLAG_UserGainGoods    203   //玩家获得物品数据包Flag

#define FLAG_MonsterInfo      301   //怪物信息数据包Flag
#define FLAG_MonsterAttrbt    302   //怪物属性数据包Flag
#define FLAG_MonsterPosition  303   //怪物位置数据包Flag

#define FLAG_TaskProfile      401   //任务概况数据包Flag
#define FLAG_TaskDlg          402   //任务对话数据包Flag
#define FLAG_TaskDescription  403   //任务描述数据包Flag
#define FLAG_TaskAim          404   //任务目标数据包Flag
#define FLAG_TaskReward       405   //任务奖励数据包Flag
#define FLAG_GoodsReward      406   //物品奖励数据包Flag
#define FLAG_UserAskTask      407   //玩玩家请求接受任务数据包Flag
#define FLAG_AskResult        408   //玩家接受任务结果数据包Flag
#define FLAG_TaskProcess      409   //玩家角色任务进度数据包Flag
#define FLAG_FinishTask       410   //玩家请求完成任务数据包Flag
#define FLAG_UserTaskResult   411   //玩家任务结果数据包Flag
#define FLAG_QuitTask         412   //放弃任务包Flag

#define FLAG_UserAttack       501   //玩家攻击信息数据包Flag


struct Configuration
{
    const char* ip;
    const char* port;
    const char* dbName;     //数据库名
    const char* user;       //用户名
    const char* password;   //用户密码
};

//怪物条目
typedef struct _ResMonsterSpawns
{
    _ResMonsterSpawns()
    {
        m_row = -1;
        m_MonsterSpawns = NULL;
    }
    ~_ResMonsterSpawns()
    {
        if(m_MonsterSpawns)
        {
            delete[] m_MonsterSpawns;
            m_MonsterSpawns = NULL;
        }
    }
    int m_row;
    T_MonsterSpawns* m_MonsterSpawns;
}ResMonsterSpawns;

//怪物属性返回
typedef struct _ResMonsterType
{
    _ResMonsterType()
    {
        m_row = -1;
        m_MonsterType = NULL;
    }
    ~_ResMonsterType()
    {
        if(m_MonsterType)
        {
            delete[] m_MonsterType;
            m_MonsterType = NULL;
        }
    }
    int m_row;
    T_MonsterType* m_MonsterType;
}ResMonsterType;

//怪物信息
typedef struct _PackMonsterInfo
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
    hf_byte  MonsterName[32]; //怪物名称
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
}PackMonsterInfo;

//怪物信息返回
typedef struct _ResPackMonsterInfo
{
    _ResPackMonsterInfo()
    {
        m_row = -1;
        m_PackMonsterInfo = NULL;
    }
    ~_ResPackMonsterInfo()
    {
        if(m_PackMonsterInfo)
        {
            delete[] m_PackMonsterInfo;
            m_PackMonsterInfo = NULL;
        }
    }
    int m_row;
    PackMonsterInfo* m_PackMonsterInfo;
}ResPackMonsterInfo;


struct ThreadParameter
{
    ThreadParameter()
    {
        sock = 0;
        data = new hf_char[512];
        memset(data, 0, 512);
    }
    ~ThreadParameter()
    {
        if(data)
        {
            delete[] data;
            data = NULL;
        }
    }
    hf_int32 sock;
    hf_char* data;
};


struct Result
{
    bool result;
};

typedef struct _UserInfo
{
    uint64_t id;
    char name[32];
    char family[32];
    uint8_t age;
}UserInfo;

typedef struct _ResUserInfo
{
    _ResUserInfo()
    {
        row  = -1;
        m_UserInto = NULL;
    }
    ~_ResUserInfo()
    {
        if(m_UserInto)
        {
            delete m_UserInto;
            m_UserInto = NULL;
        }
    }
    int row;
    UserInfo* m_UserInto;
}ResUserInfo;


struct ResLogin
{
    ResLogin()
    {
        row = -1;
        m_Login = NULL;
    }
    ~ResLogin()
    {
        if(m_Login)
        {
            delete m_Login;
            m_Login = NULL;
        }
    }

    hf_int32 row;
    STR_PackPlayerLogin* m_Login;
};


typedef struct _EffectRow
{
    int row;
}EffectRow;


#endif // STRUCT_H

