#ifndef DISKDBMANAGER_H
#define DISKDBMANAGER_H


#include <boost/container/vector.hpp>

#include "idbmanager.h"
#include "Game/postgresqlstruct.h"

/**
 * @brief The DiskDbManager class
 * 操作postgresql结构体，提供连接，断开，执行sql操作的函数
 */
class DiskDBManager : public IDBManager
{
public:
    DiskDBManager();
    ~DiskDBManager();
    bool Connect(Configuration con);
    bool Disconnect();
    /**
     * @brief Set 改变数据库内容
     * @param str sql语句
     * @return Result结构体和影响的行数
     */
    hf_int32 Set(const hf_char* str,...);
    /**
     * @brief Get 从数据库中提取数据
     * @param str  sql语句
     * @return  Result结构体和返回的内容
     */
    void* Get(const char* str);
    /**
     * @brief GetLoginInfo
     * @param str
     * @return
     */
    hf_int32 GetPlayerUserId(STR_PlayerLoginUserId* user, const hf_char* str);


    /**
     * @brief GetPlayerRoleList  得到玩家角色列表
     * @param str
     * @return
     */
    hf_int32 GetPlayerRoleList(ResRoleList* Role, const hf_char* str);
    /**
       * @brief GetMonster  从数据库中查取怪物
       * @return
     */

    /**
     * @brief GetPlayerInitPos
     * @param pos : 玩家位置数据包
     * @param sql ： 查询玩家位置的SQL
     * @return  ： 查询到的记录条数，返回-1表示查询失败
     */
    hf_int32 GetPlayerInitPos( STR_PackPlayerPosition *pos, const hf_char *sql);

    /**
     * @brief GetMonsterSpawns  怪物刷新位置
     * @param vec
     * @return
     */
    hf_int32 GetMonsterSpawns(umap_monsterSpawns* monsterSpawns);

     /**
      * @brief GetMonsterType   从数据库中查询怪物的类型属性
      * @return
      */
     hf_int32 GetMonsterType(umap_monsterType* monsterType);

     /**
      * @brief GetTaskProfile  得到任务概述
      * @return
      */
     hf_int32 GetTaskProfile(umap_taskProfile TaskProfile);



     /**
      * @brief GetTaskDialogue  得到任务对话
      * @param TaskDialogue
      * @return
      */
     hf_int32 GetTaskDialogue(umap_dialogue* TaskDialogue);

     /**
      * @brief GetTaskDescription得到任务描述
      * @param TaskDesc
      * @return
      */
     hf_int32 GetTaskDescription(umap_taskDescription* TaskDesc);

     /**
      * @brief GetTaskAim            得到任务目标
      * @param TaskAim
      * @return
      */
     hf_int32 GetTaskAim(umap_taskAim* TaskAim);

     /**
      * @brief GetTaskReward          得到任务奖励
      * @param TaskReward
      * @return
      */
     hf_int32 GetTaskReward(umap_taskReward* TaskReward);

     /**
      * @brief GetGoodsReward        得到物品奖励
      * @param GoodsReward
      * @return
      */
     hf_int32 GetGoodsReward(umap_goodsReward* GoodsReward);


     hf_int32 GetSqlResult(const hf_char* str);

     hf_int32 GetPlayerRegisterRoleInfo(STR_RoleBasicInfo* Role, const hf_char* str);

     //查询任务条件
     hf_int32 GetTaskPremise(umap_taskPremise* t_TaskPremise);

     //查询任务进度
     hf_int32 GetPlayerTaskProcess(umap_taskProcess t_TaskProcess, const hf_char* str);


     //查询角色信息
     hf_int32 GetRoleInfo(STR_RoleInfo* roleinfo, const hf_char* str);

     //查询玩家接取任务条件,查询角色经验
     hf_int32 GetRoleExperience(RoleNick* nick, STR_PackRoleExperience* RoleExp, STR_RoleBasicInfo* RoleBaseInfo, const hf_char* str);

     //查询好友列表
     hf_int32 GetFriendList(umap_friendList t_friendList, const hf_char* str);

     //查询某个昵称的roleid
     hf_int32 GetNickRoleid(hf_uint32* roleid, const hf_char* str);

     //查询添加好友请求
     hf_int32 GetAskAddFriend(vector<STR_AddFriend>& addFriend, const hf_char* str);

     //查询NPC信息
     hf_int32 GetNPCInfo(umap_npcInfo* npcInfo);

     //查询怪物掉落物品
     hf_int32 GetMonsterLoot(umap_monsterLoot* monsterLoot);

     //查询技能信息
     hf_int32 GetSkillInfo(umap_skillInfo* skillInfo);

     //查询玩家金币
     hf_int32 GetPlayerMoney(umap_roleMoney playerMoney, const hf_char* str);

     //查询玩家物品
     hf_int32 GetPlayerGoods(umap_roleGoods playerGoods, umap_roleEqu playerEqu, const hf_char* str);

     //查询玩家装备属性
     hf_int32 GetPlayerEqu(umap_roleEqu playerEqu, const hf_char* str);

     //查询玩家未捡取的物品位置
     hf_int32 GetNotPickGoodsPosition(umap_lootPosition lootPosition, const hf_char* str);

     //查询玩家未捡取的物品
     hf_int32 GetNotPickGoods(umap_lootGoods lootGoods, const hf_char* str);

     //查询物品价格
     hf_int32 GetGoodsPrice(umap_goodsPrice goodsPrice, const hf_char* str);
     //查询装备属性
     hf_uint32 GetEquAttr(umap_equAttr* equAttr, const hf_char* str);

     //查询数据库中装备现在的最大值
     hf_uint32 GetEquIDMaxValue();
private:

    PGconn *m_PGconn;

    bool        IsConnected();
};

#endif // DISKDBMANAGER_H
