/*
 * hf_types.h
 *
 *  Created on: 2015年4月26日
 *      Author: yang
 */

#ifndef HF_TYPES_H_
#define HF_TYPES_H_


#define     BLOCK_SIZE_SMALL        1024
#define     BLOCK_SIZE_MID          2048
#define     BLOCK_SIZE_LARGE        4096

#define     BLOCK_COUNT             10240

#define     SRV_PORT_DEFAULT        4567

namespace hf_types{

typedef unsigned int            hf_uint32;
typedef unsigned short          hf_uint16;
typedef int						hf_int32;
typedef short					hf_int16;
typedef unsigned char           hf_byte;
typedef char                    hf_char;
typedef hf_byte                 hf_uint8;
typedef float                   hf_float;
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



};
#endif /* HF_TYPES_H_ */
