/*
 * hf_types.h
 *
 *  Created on: 2015年4月26日
 *      Author: yang
 */

#ifndef HF_TYPES_H_
#define HF_TYPES_H_


#define  CHUNK_SIZE            1024        //内存池块大小
#define  CHUNK_COUNT           4096        //内存池块数
//#define     SRV_PORT_DEFAULT      7000         //监听端口
#define  CORE_NUM              4           //CPU核心数
#define  PI                    3.1415926

#define  View                  1           //能看到，用来表示玩家之间和玩家与怪物之间
#define  NotView               2           //看不到
#define  EquipMentID           20000000    //装备编号
#define  Money_1               101         //1号金钱

#define  BAGCAPACITY           65          //背包容量
#define  GOODSMAXCOUNT         99          //每个格子物品最大数量

#define  PlayerView            100         //玩家可视范围
#define  PlayerAttackView      80          //玩家攻击范围
#define  MonsterView           40          //怪物可视范围

#define  MonsterMoveDistance   5           //怪物移动一次的距离
#define  PlayerMoveDistance    5           //玩家移动一次的距离  单位分米

#define  RefreshDistance       20          //玩家刷新数据的距离

#define  MonsterDeathTime      60          //怪物死亡时间
//位置的状态，0 空闲，1使用，2锁定
#define  POS_EMPTY             0           //位置为空
#define  POS_NONEMPTY          1           //位置非空
#define  POS_LOCKED            2           //位置锁定

#define  PostUpdate            1           //更新
#define  PostInsert            2           //插入
#define  PostDelete            3           //删除

//背包物品来源
#define  Source_Bag            0            //来自背包
#define  Source_Trade          1            //来自交易
#define  Source_Buy            2            //来自购买
#define  Source_Pick           3            //来自捡的
#define  Source_Task           4            //来自任务

#define  DefaultGoods          1            //默认奖励
#define  ChooseGoods           2            //选择奖励

#define  EquTypeMinValue       20000        //装备类型最小值
#define  EquTypeMaxValue       29999        //装备类型最大值


#define  MonsterAlive           1            //怪物复活
#define  MonsterDie             2            //怪物死亡

#define  ActiveMonster          1            //主动怪物
#define  UnactiveMonster        2            //被动怪物

#define  Buy_NotEnoughMoney     1           //金钱不够
#define  Buy_BagFull            2           //背包满了

#define  PICK_SUCCESS           1           //捡取成功
#define  PICK_BAGFULL           2           //背包满了,未捡取物品
#define  PICKPART_BAGFULL       3           //背包满了 ，捡取部分物品
#define  PICK_GOODNOTEXIST      4           //捡取的物品不存在

#define  PUBLIC_COOLTIME        1           //公共冷却时间
#define  GOODS_CONTINUETIME     180         //掉落物品持续时间

#define  RESULT_SUCCESS         1     //成功
#define  RESULT_PRE_TASK        2     //未接取条件任务
#define  RESULT_CONDITION_TASK  3     //未接取条件任务
#define  RESULT_TASK_GOODS      4     //未持有任务物品
#define  RESULT_CONDITION_TITLE 5     //为获得任务条件称号
#define  RESULT_CONDITION_COPY  6     //未完成条件副本
#define  RESULT_SEX             7     //性别不符
#define  RESULT_LEVEL           8     //等级不足
#define  RESULT_PROFESSION      9     //职业不符

#define  NOT_ATTACKVIEW         1      //不在攻击范围
#define  OPPOSITE_DIRECT        2      //方向相反
#define  HIT                    3      //命中
#define  NOT_HIT                4      //未命中
#define  RESIST                 5      //抵挡
#define  NOT_RESIST             6      //未抵挡
#define  CRIT                   7      //暴击
#define  NOT_CRIT               8      //未暴击

#define  SKILL_SUCCESS          3      //技能使用成功
#define  SKILL_ERROR            4      //技能使用错误
#define  SKILL_NOTCOOL          5      //技能没冷却
#define  SKILL_NOTEXIST         6      //技能不存在

#define     RESULT_SUCCESS                 1
#define     RESULT_ERROR                   2

#define     RESULT_USER_REPEAT             2     //用户名已被注册
#define     RESULT_EMAIL_REPEAT            3     //邮箱已被注册

#define    RESULT_PASSWORD_ERROR           2     //密码不正确
#define    RESULT_USER_NOTEXIST            3     //用户名不存在

#define    RESULT_NICK_REPEAT              3     //昵称已被注册

#define    ONLINE  1   //在线
#define    OFFLINE 2   //不在线

namespace hf_types{

typedef unsigned int            hf_uint32;
typedef unsigned short          hf_uint16;
typedef int						hf_int32;
typedef short					hf_int16;
typedef unsigned char           hf_byte;
typedef hf_byte                 hf_uint8;
typedef char                    hf_char;
typedef float                   hf_float;
typedef double                  hf_double;

/**
 *
 * 内存块管理链表
 */
typedef struct _mem_list{
    hf_uint32 index;    		//内存的索引
    struct _mem_list *pre;		//上一个块
	struct _mem_list *next;		//下一个块
}hf_memNode;


typedef struct _unitBlock{
    _unitBlock *next;
    _unitBlock *pre;
    hf_uint32 index;
}UnitBLock;

//memoery block
typedef struct _block{
    hf_int32        m_freecount;
    hf_int32        m_usedcount;
    UnitBLock      *used_head;
    UnitBLock      *used_tail;
    UnitBLock      *free_head;
    UnitBLock      *free_tail;
}Block,*Block_ptr;

typedef void*  (*TaskFun)(void*);

///
/// \brief The Client struct
/// 客户端数据结构
///
//struct Client
//{
//    int         sock;
//    void       *pack;
//    struct   event   *read_ev;
//    struct   event   *write_ev;
//    Mutex     mtx;
//    int         ReadN(int fd,char*buf,int sz)
//    {
//        int res;
//        mtx.Lock();
//        res = NetTask::RecvN(fd,buf,sz);
//        mtx.UnLock();
//        return res;
//    }
//    int WriteN( int fd,char *buf,int sz)
//    {
//        int res;
//        mtx.Lock();
//        res = NetTask::SendN(fd,buf,sz);
//        mtx.UnLock();
//        return res;
//    }
//};


};
#endif /* HF_TYPES_H_ */
