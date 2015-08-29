#ifndef FLAGMACROS_H
#define FLAGMACROS_H


//登录分为登录用户帐号和角色
#define FLAG_PlayerLoginUserId       100     //玩家登录用户名Flag
#define FLAG_PlayerLoginRole         101     //玩家登录角色Flag

#define FLAG_PlayerRegisterUserId    102     //玩家注册用户名Flag
#define FLAG_PlayerRegisterRole      103     //玩家注册角色Flag

//用户操作结果
#define FLAG_Result                  105

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
#endif // FLAGMACROS_H

