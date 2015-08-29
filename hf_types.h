/*
 * hf_types.h
 *
 *  Created on: 2015年4月26日
 *      Author: yang
 */

#ifndef HF_TYPES_H_
#define HF_TYPES_H_


#define     CHUNK_SIZE            1024           //内存池块大小
#define     CHUNK_COUNT           4096           //内存池块数
//#define     SRV_PORT_DEFAULT      7000            //监听端口
#define     CORE_NUM              4              //CPU核心数
#define     PI                    3.1415926
#define     PlayerView            1000
#define     PlayerAttackView      1000
#define     View                  1              //能看到，用来表示玩家之间和玩家与怪物之间
#define     NotView               2              //看不到
#define     EquipMentID           20000000       //装备编号
#define     Money_1               101            //1号金钱

#define     PostUpdate                 1     //更新
#define     PostInsert                 2     //插入
#define     PostDelete                 3     //删除
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
