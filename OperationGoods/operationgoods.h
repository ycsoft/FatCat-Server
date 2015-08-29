#ifndef OPERATIONGOODS_H
#define OPERATIONGOODS_H

#include "Game/postgresqlstruct.h"
#include "NetWork/tcpconnection.h"
#include "Game/cmdtypes.h"


class OperationGoods
{
public:
    OperationGoods();
    ~OperationGoods();


    //捡物品
    void PickUpGoods(TCPConnection::Pointer conn, hf_uint16 len, STR_PickGoods* t_pickGoods);

    //查询物品价格
    void QueryGoodsPrice();
    //查询装备属性
    void QueryEquAttr();
    //丢弃物品
    void RemoveBagGoods(TCPConnection::Pointer conn, STR_RemoveBagGoods* removeGoods);
    //移动或分割物品
    void MoveBagGoods(TCPConnection::Pointer conn, STR_MoveBagGoods* moveGoods);
    //交换物品
    void ExchangeBagGoods(TCPConnection::Pointer conn, STR_MoveBagGoods* moveGoods);
    //购买物品
    void BuyGoods(TCPConnection::Pointer conn, STR_BuyGoods* buyGoods);
    //购买装备
    void BuyEquipment(TCPConnection::Pointer conn, STR_BuyGoods* buyGoods, STR_PlayerMoney* money, hf_uint32 price);
    //购买其他物品
    void BuyOtherGoods(TCPConnection::Pointer conn, STR_BuyGoods* buyGoods, STR_PlayerMoney* money, hf_uint32 price);
    //出售物品
    void SellGoods(TCPConnection::Pointer conn, STR_PackSellGoods* moveGoods);
    //得到新的装备编号
    static hf_uint32  GetEquipmentID();

    void UsePos(TCPConnection::Pointer conn, hf_uint16 pos);      //使用位置
    void ReleasePos(TCPConnection::Pointer conn, hf_uint16 pos);  //释放位置
    hf_uint16 GetEmptyPosCount(TCPConnection::Pointer conn);      //得到空位置总数
    hf_uint16 GetEmptyPos(TCPConnection::Pointer conn);           //查找空位置

    //给新捡的装备属性 附初值
    void SetEquAttr(STR_Equipment* equAttr, hf_uint32 typeID);
private:
    umap_goodsPrice    m_goodsPrice;   //物品价格
    umap_equAttr*      m_equAttr;      //装备属性
    static hf_uint32   m_equipmentID;  //装备ID
};

#endif // OPERATIONGOODS_H
