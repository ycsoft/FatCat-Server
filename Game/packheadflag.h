#ifndef PACKHEADFLAG_H
#define PACKHEADFLAG_H

#define PhyAttackSkillID    1
#define MagicAttackSkillID  2
#define PhyMagAttackSkillID 3

//登录分为登录用户帐号和角色
#define FLAG_PlayerLoginUserId       100     //玩家登录用户名Flag
#define FLAG_PlayerLoginRole         101     //玩家登录角色Flag
#define FLAG_PlayerRegisterUserId    102     //玩家注册用户名Flag
#define FLAG_PlayerRegisterRole      103     //玩家注册角色Flag
#define FLAG_PlayerRoleList          104     //玩家角色列表
#define FLAG_Result                  105     //用户操作结果
#define FLAG_Experence               106     //玩家经验
#define FLAG_RoreInfo                107     //玩家属性
#define FLAG_AddFriend               108     //添加好友
#define FLAG_DeleteFriend            109     //删除好友
#define FLAG_PlayerOffline           110     //玩家下线请求
#define FLAG_ViewRoleLeave           111     //玩家离开可视范围
#define FLAG_FriendList              112     //好友列表信息
#define FLAG_OtherPlayerRoleInfo     113     //其他玩家角色信息
#define FLAG_FriendOnline            114     //好友上线通知
#define FLAG_AddFriendReturn         115     //添加好友返回Flag
#define FLAG_RewardExperience        116     //奖励经验
#define FLAG_ViewRoleCome            117     //玩家进入可视范围
#define FlAG_UserRepeatLogin         118     //用户重复登录
#define FLAG_UserDeleteRole          119     //用户删除角色
#define FLAG_FriendOffLine           120     //好友下线

#define FLAG_BagGoods                130    //玩家新捡的物品
#define FLAG_PlayerMoney             131    //玩家金币
#define FLAG_EquGoodsAttr            132    //装备属性
#define FLAG_RemoveGoods             133    //玩家失去物品
#define FLAG_MoveGoods               134    //玩家背包物品移动
#define FLAG_OtherResult             135    //操作结果

#define FLAG_BuyGoods                154    //从商店购买物品
#define FLAG_SellGoods               155    //出售物品到商店

#define FLAG_LootGoods               201    //掉落物品数据包Flag
#define FLAG                         202    //
#define FLAG_PlayerMove              203    //玩家移动(发送方向)
#define FLAG_OtherPlayerPosition     204    //其他玩家位置
#define FLAG_PlayerPosition          205    //玩家位置移动（发送坐标test）
#define FLAG_PickGoods               206    //玩家捡取物品
#define FLAG_PickGoodsResult         207    //玩家捡取物品结果
#define FLAG_LootGoodsOverTime       208    //掉落物品延时


#define FLAG_MonsterCome      301   //怪物进入可视范围
#define FLAG_MonsterAttribute 302   //怪物受攻击(攻击)属性数据包Flag
#define FLAG_MonsterPosition  303   //怪物位置数据包Flag
#define FLAG_RoleAttribute    304   //玩家受攻击(攻击)后属性
#define FLAG_MonsterLeave     305   //怪物离开可视范围



#define FLAG_TaskProfile      401   //任务概况数据包Flag
#define FLAG_StartTaskDlg     402   //任务对话数据包Flag
#define FLAG_TaskDescription  403   //任务描述数据包Flag
#define FLAG_TaskAim          404   //任务目标数据包Flag
#define FLAG_TaskReward       405   //任务奖励数据包Flag
#define FLAG_GoodsReward      406   //物品奖励数据包Flag
#define FLAG_UserAskTask      407   //玩家请求接受任务数据包Flag
#define FLAG_AskResult        408   //玩家接受任务结果数据包Flag
#define FLAG_TaskProcess      409   //玩家角色任务进度数据包Flag
#define FLAG_AskFinishTask    410   //玩家请求完成任务数据包Flag
#define FLAG_UserTaskResult   411   //玩家任务结果数据包Flag
#define FLAG_QuitTask         412   //放弃任务包Flag
#define FLAG_FinishTaskDlg    413   //任务结束对话Flag
#define FLAG_TaskExeDlg       414   //任务执行对话数据包Flag
#define FLAG_AskTaskData      415   //请求任务数据Flag


#define FLAG_UserAttackAim    501   //玩家攻击信息数据包Flag
#define FLAG_DamageData       502   //攻击返回数据包
#define FLAG_UserAttackPoint  503   //玩家使用技能攻击范围

#define FLAG_SKILLRESULT      505   //技能使用结果

#define FLAG_CanUsedSkill     506   //玩家可以使用的技能

#define FLAG_SkillAimResult   507   //施法目标结果
#define FLAG_SkillPosResult   508   //施法位置结果

#define CanSeeEachOther( x1,y1,z1,x2,y2,z2)               (x1 - x2 < PlayerView)

#define FLAG_OperRequest 140 //玩家操作请求
#define FLAG_OperResult  141  //玩家交易请求结果

#define FLAG_InterchangeSucReport 153   //玩家交易成功报告
#define FLAG_InterchangeMoneyCount 151  //玩家交易金币数量
#define FLAG_InterchangeGoods 152       //玩家交易变化
#define FLAG_InterchangeOperPro   150   //玩家交易过程中操作

#define FLAG_WantToChange 1

#endif // PACKHEADFLAG_H

