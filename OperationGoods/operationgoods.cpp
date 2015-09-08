#include "operationgoods.h"
#include "memManage/diskdbmanager.h"
#include "GameTask/gametask.h"
#include "utils/stringbuilder.hpp"
#include "Game/session.hpp"
#include "PlayerLogin/playerlogin.h"
#include "GameAttack/gameattack.h"
#include "OperationPostgres/operationpostgres.h"
#include "server.h"



hf_uint32 OperationGoods::m_equipmentID = EquipMentID;

#define  Buy_NotEnoughMoney    1           //金钱不够
#define  Buy_BagFull           2           //背包满了

#define   PICK_SUCCESS         1           //捡取成功
#define   PICK_BAGFULL         2           //背包满了,未捡取物品
#define   PICKPART_BAGFULL     3           //背包满了 ，捡取部分物品
#define   PICK_GOODNOTEXIST    4           //捡取的物品不存在

OperationGoods::OperationGoods()
    :m_goodsPrice(new _umap_goodsPrice)
    ,m_equAttr(new umap_equAttr)
{

}

OperationGoods::~OperationGoods()
{
    if(m_equAttr)
    {
        delete m_equAttr;
        m_equAttr = NULL;
    }
}

//得到装备编号
hf_uint32  OperationGoods::GetEquipmentID()
{
    return ++m_equipmentID;
}

//使用位置
void OperationGoods::UsePos(TCPConnection::Pointer conn, hf_uint16 pos)
{
    SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();
    (*smap)[conn].m_goodsPosition[pos] = POS_NONEMPTY;
}

//释放位置
void OperationGoods::ReleasePos(TCPConnection::Pointer conn, hf_uint16 pos)
{
    SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();
    (*smap)[conn].m_goodsPosition[pos] = POS_EMPTY;
}

//得到空位置总数
hf_uint8 OperationGoods::GetEmptyPosCount(TCPConnection::Pointer conn)
{
    SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();
    hf_uint16 emptyPos = 0;
    for(hf_int32 j = 1; j <= BAGCAPACITY; j++)   //找空位置
    {
        if((*smap)[conn].m_goodsPosition[j] == POS_EMPTY)
        {
            emptyPos++;
        }
    }
    return emptyPos;
}

//查找空位置
hf_uint8 OperationGoods::GetEmptyPos(TCPConnection::Pointer conn)
{
    SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();
    for(hf_int16 j = 1; j <= BAGCAPACITY;)   //找空位置
    {
        if((*smap)[conn].m_goodsPosition[j] == POS_EMPTY)
        {
            (*smap)[conn].m_goodsPosition[j] = POS_NONEMPTY;
            return j;
        }
        j++;
        if(j == BAGCAPACITY)
        {
            return 0;
        }
    }
}


//判断能否放下
hf_uint8 OperationGoods::UseEmptyPos(TCPConnection::Pointer conn, hf_uint8 count)
{
    SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();
    for(hf_int32 j = 1; j <= BAGCAPACITY; j++)   //找空位置
    {
        if((*smap)[conn].m_goodsPosition[j] == POS_EMPTY)
        {
            --count;
        }
        if(count == 0)
            return count;
    }
    return count;
}

//捡物品
void OperationGoods::PickUpGoods(TCPConnection::Pointer conn, hf_uint16 len, STR_PickGoods* PickGoods)
{
    STR_PickGoods* pickGoods = PickGoods;
    Server* srv = Server::GetInstance();
    SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();
    if((*(*smap)[conn].m_interchage).isInchange == true) //处于交易状态
    {
        Server::GetInstance()->free(pickGoods);
        return;
    }   
    umap_lootGoods lootGoods = (*smap)[conn].m_lootGoods;
    _umap_lootGoods::iterator loot_it = lootGoods->find(pickGoods->GoodsFlag);
    if(loot_it == lootGoods->end())  //掉落者不存在
    {
        Server::GetInstance()->free(pickGoods);
        return;
    }
    STR_PackHead t_packHead;
    hf_uint32 count = len/sizeof(STR_PickGoods);

    STR_PickGoodsResult t_pickResult;
    memset(&t_pickResult, 0 , sizeof(STR_PickGoodsResult));
    t_pickResult.GoodsFlag = pickGoods->GoodsFlag;

    STR_Goods t_goods;
    t_goods.Source = Source_Pick;
    umap_roleGoods playerBagGoods = (*smap)[conn].m_playerGoods;
    umap_roleEqu playerEqu = (*smap)[conn].m_playerEqu;
    umap_roleMoney playerMoney = (*smap)[conn].m_playerMoney;

    hf_uint32 roleid = (*smap)[conn].m_roleid;
    hf_char* pickResultBuff = (hf_char*)srv->malloc();   //捡取结果
    hf_char* moneyBuff = (hf_char*)srv->malloc();        //捡取金钱
    hf_char* newGoodsBuff = (hf_char*)srv->malloc();     //捡取的新物品
    hf_char* equAttrBuff = (hf_char*)srv->malloc();      //新捡的装备属性
    vector<STR_LootGoods>::iterator vec;

    hf_uint8 num = 0;
    hf_uint8 equCount = 0;
    OperationPostgres* t_post = Server::GetInstance()->GetOperationPostgres();
    for(hf_uint32 i = 0; i < count; i++)  //捡取多种物品
    {
        for(vec = loot_it->second.begin(); vec != loot_it->second.end(); vec++)
        {
            if(pickGoods->LootGoodsID == vec->LootGoodsID)
            {
                break;
            }
        }
        if(vec == loot_it->second.end())   //要捡的物品不存在
        {
            t_pickResult.Count = 0;
            t_pickResult.Result = PICK_GOODNOTEXIST;
            memcpy(pickResultBuff+ sizeof(STR_PackHead) + i*sizeof(STR_PickGoodsResult), &t_pickResult, sizeof(STR_PickGoodsResult));
            continue;
        }
        t_pickResult.LootGoodsID = pickGoods->LootGoodsID;

        if(pickGoods->LootGoodsID == Money_1)  //掉落的是金钱
        {
            STR_PlayerMoney* money = &(*playerMoney)[Money_1];
            money->Count += vec->Count;
            memcpy(moneyBuff + sizeof(STR_PackHead), money, sizeof(STR_PlayerMoney));
            t_post->PushUpdateMoney(roleid, money); //将金钱变动插入到list

            t_packHead.Len = sizeof(STR_PlayerMoney);
            t_packHead.Flag = FLAG_PlayerMoney;
            memcpy(moneyBuff, &t_packHead, sizeof(STR_PackHead));
            conn->Write_all(moneyBuff, sizeof(STR_PackHead) + t_packHead.Len);

            t_pickResult.Count = vec->Count;
            t_pickResult.Result = PICK_SUCCESS;
            memcpy(pickResultBuff+ sizeof(STR_PackHead) + i*sizeof(STR_PickGoodsResult), &t_pickResult, sizeof(STR_PickGoodsResult));

        }
        else if(EquTypeMinValue <= pickGoods->LootGoodsID && pickGoods->LootGoodsID <= EquTypeMaxValue)  //直接生成装备ID存起来
        {
            hf_uint8 empty_pos = OperationGoods::GetEmptyPos(conn);
            if(empty_pos == 0) //没有空位置
            {
                t_pickResult.Count = 0;
                t_pickResult.Result = PICK_BAGFULL;
                memcpy(pickResultBuff+ sizeof(STR_PackHead) + i*sizeof(STR_PickGoodsResult), &t_pickResult, sizeof(STR_PickGoodsResult));
                continue;
            }

            STR_PlayerEqu t_equ;
            t_equ.goods.GoodsID = OperationGoods::GetEquipmentID();
            t_equ.goods.TypeID = pickGoods->LootGoodsID;
            t_equ.goods.Position = empty_pos;
            t_equ.goods.Count = 1;
            t_equ.goods.Source = Source_Pick;
            SetEquAttr(&t_equ.equAttr, pickGoods->LootGoodsID);   //给新捡装备属性附初值
            t_equ.equAttr.EquID = t_equ.goods.GoodsID;

            t_pickResult.Count = 1;
            t_pickResult.Result = PICK_SUCCESS;

           (*playerEqu)[t_equ.goods.GoodsID] = t_equ;

            loot_it->second.erase(vec);
             memcpy(newGoodsBuff + sizeof(STR_PackHead) + num*sizeof(STR_Goods), &t_equ.goods, sizeof(STR_Goods));
              t_post->PushUpdateGoods(roleid, &t_equ.goods, PostInsert); //将新买的物品添加到list

             memcpy(equAttrBuff + sizeof(STR_PackHead) + equCount*sizeof(STR_Equipment), &t_equ.equAttr, sizeof(STR_Equipment));
             t_post->PushUpdateEquAttr(roleid, &t_equ.equAttr, PostInsert); //将新买的物品添加到list
             memcpy(pickResultBuff+ sizeof(STR_PackHead) + i*sizeof(STR_PickGoodsResult), &t_pickResult, sizeof(STR_PickGoodsResult));
             num++;
             equCount++;
             srv->GetGameTask()->UpdateCollectGoodsTaskProcess(conn, pickGoods->LootGoodsID, t_equ.goods.Count);
        }
        else   //普通物品
        {
            t_goods.GoodsID = pickGoods->LootGoodsID;
            t_goods.TypeID = pickGoods->LootGoodsID;

            _umap_roleGoods::iterator it = playerBagGoods->find(pickGoods->LootGoodsID);
            if(it != playerBagGoods->end()) //物品已有位置
            {
                for(vector<STR_Goods>::iterator iter = (it->second).begin(); iter != (it->second).end(); iter++)
                {
                    if(iter->Count == GOODSMAXCOUNT) //位置已存满，跳过
                    {
                        continue;
                    }
                    else if(iter->Count + vec->Count <= GOODSMAXCOUNT) //这个位置可以存放下
                    {
                        t_goods.Count = vec->Count;
                        t_goods.Position = iter->Position;

                        t_pickResult.Count = vec->Count;
                        t_pickResult.Result = PICK_SUCCESS;
                        iter->Count += t_goods.Count;

                        loot_it->second.erase(vec);

                         memcpy(newGoodsBuff + sizeof(STR_PackHead) + num*sizeof(STR_Goods), &t_goods, sizeof(STR_Goods));
                         t_post->PushUpdateGoods(roleid, &t_goods, PostInsert); //将新买的物品添加到list
                         memcpy(pickResultBuff+ sizeof(STR_PackHead) + i*sizeof(STR_PickGoodsResult), &t_pickResult, sizeof(STR_PickGoodsResult));
                         num++;
                         srv->GetGameTask()->UpdateCollectGoodsTaskProcess(conn, pickGoods->LootGoodsID, t_goods.Count);
                        break;
                    }
                    else //这个位置存放不下，存满当前位置，继续查找新位置
                    {
                        t_goods.Count = GOODSMAXCOUNT - iter->Count;
                        t_goods.Position = iter->Position;
                        t_pickResult.Count += t_goods.Count;
                        t_pickResult.Result = PICK_SUCCESS;

                        iter->Count = GOODSMAXCOUNT;
                        vec->Count -= t_goods.Count;

                        memcpy(newGoodsBuff + sizeof(STR_PackHead) + num*sizeof(STR_Goods), &t_goods, sizeof(STR_Goods));
                        t_post->PushUpdateGoods(roleid, &t_goods, PostUpdate); //将新买的物品添加到list
                        num++;
                        srv->GetGameTask()->UpdateCollectGoodsTaskProcess(conn, pickGoods->LootGoodsID, t_goods.Count);
                    }
                }
                if(vec->Count != 0) //开辟新位置存放剩余的物品
                {
                    hf_uint8 empty_pos = OperationGoods::GetEmptyPos(conn);
                    if(empty_pos == 0) //没有空位置
                    {
                        t_pickResult.Result = PICKPART_BAGFULL;
                        memcpy(pickResultBuff+ sizeof(STR_PackHead) + i*sizeof(STR_PickGoodsResult), &t_pickResult, sizeof(STR_PickGoodsResult));
                        break;
                    }
                    t_goods.Count = vec->Count;
                    loot_it->second.erase(vec);
                    t_goods.Position = empty_pos;
                    t_pickResult.Count += t_goods.Count;
                    t_pickResult.Result = PICK_SUCCESS;

                   (*playerBagGoods)[t_goods.GoodsID].push_back(t_goods);

                     memcpy(newGoodsBuff + sizeof(STR_PackHead) + num*sizeof(STR_Goods), &t_goods, sizeof(STR_Goods));
                     t_post->PushUpdateGoods(roleid, &t_goods, PostInsert); //将新买的物品添加到list
                     num++;
                     //查找此任务是否为任务进度里收集物品，如果是，更新任务进度
                     srv->GetGameTask()->UpdateCollectGoodsTaskProcess(conn, pickGoods->LootGoodsID, t_goods.Count);
                }
                memcpy(pickResultBuff+ sizeof(STR_PackHead) + i*sizeof(STR_PickGoodsResult), &t_pickResult, sizeof(STR_PickGoodsResult));
            }
            else  //物品没有位置
            {
                hf_uint8 empty_pos = OperationGoods::GetEmptyPos(conn);
                if(empty_pos == 0) //没有空位置
                {
                    t_pickResult.Count = 0;
                    t_pickResult.Result = PICK_BAGFULL;
                    memcpy(pickResultBuff+ sizeof(STR_PackHead) + i*sizeof(STR_PickGoodsResult), &t_pickResult, sizeof(STR_PickGoodsResult));
                    continue;
                }

                t_goods.Count = vec->Count;
                t_goods.Position = empty_pos;
                t_pickResult.Count += t_goods.Count;
                t_pickResult.Result = PICK_SUCCESS;

                loot_it->second.erase(vec);

                vector<STR_Goods> t_vec;
                t_vec.push_back(t_goods);
               (*playerBagGoods)[t_goods.GoodsID] = t_vec;

                 memcpy(newGoodsBuff + sizeof(STR_PackHead) + num*sizeof(STR_Goods), &t_goods, sizeof(STR_Goods));
                 t_post->PushUpdateGoods(roleid, &t_goods, PostInsert); //将新买的物品添加到list

                 memcpy(pickResultBuff+ sizeof(STR_PackHead) + i*sizeof(STR_PickGoodsResult), &t_pickResult, sizeof(STR_PickGoodsResult));
                 num++;
                 srv->GetGameTask()->UpdateCollectGoodsTaskProcess(conn, pickGoods->LootGoodsID, t_goods.Count);
            }
        }

        if(loot_it->second.empty())
        {
            lootGoods->erase(loot_it);
        }
        pickGoods++;
    }
    t_packHead.Flag = FLAG_PickGoodsResult;
    t_packHead.Len = sizeof(STR_PickGoodsResult)*count;
    memcpy(pickResultBuff, &t_packHead, sizeof(STR_PackHead));
    conn->Write_all(pickResultBuff, t_packHead.Len + sizeof(STR_PackHead));


    //发送玩家新拥有物品数据
    if(num != 0)
    {
        t_packHead.Flag = FLAG_BagGoods;
        t_packHead.Len = sizeof(STR_Goods)*num;
        memcpy(newGoodsBuff, &t_packHead, sizeof(STR_PackHead));
        conn->Write_all(newGoodsBuff, t_packHead.Len + sizeof(STR_PackHead));
    }

    //发送装备属性
    if(equCount != 0)
    {
        t_packHead.Flag = FLAG_EquGoodsAttr;
        t_packHead.Len = sizeof(STR_Equipment)*equCount;
        memcpy(equAttrBuff, &t_packHead, sizeof(STR_PackHead));
        conn->Write_all(equAttrBuff, t_packHead.Len + sizeof(STR_PackHead));
    }
    srv->free(pickResultBuff);
    srv->free(newGoodsBuff);
    srv->free(equAttrBuff);
    srv->free(PickGoods);
}


//查询物品价格
void OperationGoods::QueryGoodsPrice()
{
    StringBuilder       sbd;
    sbd << "select goodsid,buyprice,sellprice from t_equipmentprice;";
    DiskDBManager *db = Server::GetInstance()->getDiskDB();
    hf_int32 count = db->GetGoodsPrice(m_goodsPrice,sbd.str());
    if ( count < 0 )
    {
        Logger::GetLogger()->Error("Query equipment price error");
        return;
    }

    sbd.Clear();
    sbd << "select goodsid,buyprice,sellprice from t_consumableprice;";
    count = db->GetGoodsPrice(m_goodsPrice, sbd.str());
    if ( count < 0 )
    {
        Logger::GetLogger()->Error("Query consumable price error");
        return;
    }

    sbd.Clear();
    sbd << "select goodsid,buyprice,sellprice from t_materialprice;";
    count = db->GetGoodsPrice(m_goodsPrice, sbd.str());
    if ( count < 0 )
    {
        Logger::GetLogger()->Error("Query materia price error");
        return;
    }
}

//查询装备属性
void OperationGoods::QueryEquAttr()
{
    DiskDBManager *db = Server::GetInstance()->getDiskDB();
    StringBuilder       sbd;
    sbd << "select * from t_equipmentAttr;";    
    hf_int32 count = db->GetEquAttr(m_equAttr, sbd.str());
    if ( count < 0 )
    {
        Logger::GetLogger()->Error("Query equipment price error");
        return;
    }
}

//丢弃物品
void OperationGoods::RemoveBagGoods(TCPConnection::Pointer conn, STR_RemoveBagGoods* removeGoods)
{
    SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();

    if(removeGoods->GoodsID >= EquipMentID)
    {
        _umap_roleEqu::iterator equ_it = (*smap)[conn].m_playerEqu->find(removeGoods->GoodsID);
        if(equ_it == (*smap)[conn].m_playerEqu->end())
        {
            Server::GetInstance()->free(removeGoods);
            return;
        }
        if((*smap)[conn].m_goodsPosition[removeGoods->Position] == POS_LOCKED)
        {
            Server::GetInstance()->free(removeGoods);
            return;
        }

        OperationGoods::ReleasePos(conn, removeGoods->Position);
        equ_it->second.goods.Count = 0;
        STR_PackGoods t_goods(&equ_it->second.goods);
        conn->Write_all(&t_goods, sizeof(STR_PackGoods));

        Server::GetInstance()->GetOperationPostgres()->PushUpdateGoods((*smap)[conn].m_roleid, &(equ_it->second.goods), PostDelete);
        Server::GetInstance()->GetOperationPostgres()->PushUpdateEquAttr((*smap)[conn].m_roleid, &(equ_it->second.equAttr), PostDelete);

        (*smap)[conn].m_playerEqu->erase(equ_it);
        Server::GetInstance()->free(removeGoods);
        return;
    }

    umap_roleGoods  playerBagGoods = (*smap)[conn].m_playerGoods;
    _umap_roleGoods::iterator it = playerBagGoods->find(removeGoods->GoodsID);
    if(it == playerBagGoods->end())
    {
        Server::GetInstance()->free(removeGoods);
        return;
    }
    for(vector<STR_Goods>::iterator iter = it->second.begin(); iter != it->second.end(); iter++)
    {
        if(removeGoods->Position == iter->Position)  //找到要删除的位置
        {
            if((*smap)[conn].m_goodsPosition[removeGoods->Position] == POS_LOCKED)
            {
                Server::GetInstance()->free(removeGoods);
                return;
            }
            OperationGoods::ReleasePos(conn, removeGoods->Position);

            iter->Count = 0;
            STR_PackGoods t_goods(&(*iter));
            conn->Write_all(&t_goods, sizeof(STR_PackGoods));

            Server::GetInstance()->GetOperationPostgres()->PushUpdateGoods((*smap)[conn].m_roleid, &(*iter), PostDelete);

            it->second.erase(iter);
            Server::GetInstance()->free(removeGoods);
        }
    }
}

//移动或分割物品
void OperationGoods::MoveBagGoods(TCPConnection::Pointer conn, STR_MoveBagGoods* moveGoods)
{
    if(moveGoods->CurrentPos == moveGoods->TargetPos)
    {
        Server::GetInstance()->free(moveGoods);
        return;
    }
    SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();
    if((*smap)[conn].m_goodsPosition[moveGoods->CurrentPos] == POS_LOCKED || (*smap)[conn].m_goodsPosition[moveGoods->TargetPos] == POS_LOCKED)  //当前位置或者目标位置商品处于锁定状态，不可以移动
    {
        Server::GetInstance()->free(moveGoods);
        return;
    }

    if(moveGoods->CurrentGoodsID != moveGoods->TargetGoodsID)
    {
        ExchangeBagGoods(conn, moveGoods);
        Server::GetInstance()->free(moveGoods);
        return;
    }

    umap_roleGoods  playerBagGoods = (*smap)[conn].m_playerGoods;
    vector<STR_Goods>::iterator curPos;
    _umap_roleGoods::iterator cur_goodsID = playerBagGoods->find(moveGoods->CurrentGoodsID);
    if(cur_goodsID == playerBagGoods->end())
    {
        Server::GetInstance()->free(moveGoods);
        return;
    }

    for(curPos = cur_goodsID->second.begin(); curPos != cur_goodsID->second.end();)
    {
        if(moveGoods->CurrentPos == curPos->Position )
        {
            if(moveGoods->Count > curPos->Count) //移动数量大于当前数量返回
            {
                Server::GetInstance()->free(moveGoods);
                return;
            }
            break;
        }
        curPos++;
        if(curPos == cur_goodsID->second.end())
        {
            Server::GetInstance()->free(moveGoods);
            return;
        }
    }

    vector<STR_Goods>::iterator tarPos;
    for(tarPos = cur_goodsID->second.begin(); tarPos != cur_goodsID->second.end();)
    {
        if(tarPos->Position == moveGoods->TargetPos) break;
        tarPos++;
        if(tarPos == cur_goodsID->second.end()) //没找到目标位置
        {
            Server::GetInstance()->free(moveGoods);
            return;
        }
    }

    hf_char* buff = (hf_char*)Server::GetInstance()->malloc();


    if(curPos->Count == GOODSMAXCOUNT || tarPos->Count == GOODSMAXCOUNT) //有一个位置已经放满
    {
        hf_uint16 pos = curPos->Position;
        curPos->Position = tarPos->Position;
        tarPos->Position = pos;
        memcpy(buff + sizeof(STR_PackHead), &(*curPos), sizeof(STR_Goods));
        memcpy(buff + sizeof(STR_PackHead) + sizeof(STR_Goods), &(*tarPos), sizeof(STR_Goods));
        Server::GetInstance()->GetOperationPostgres()->PushUpdateGoods((*smap)[conn].m_roleid, &(*curPos), PostUpdate);
        Server::GetInstance()->GetOperationPostgres()->PushUpdateGoods((*smap)[conn].m_roleid, &(*tarPos), PostUpdate);
    }
    else
    {
        if(moveGoods->Count == curPos->Count)//全部移动
        {
            if(tarPos->Count + moveGoods->Count > GOODSMAXCOUNT)
            {   //目标位置放不下,将目标位置放满
                curPos->Count = curPos->Count - (GOODSMAXCOUNT - tarPos->Count);
                tarPos->Count = GOODSMAXCOUNT;
                memcpy(buff + sizeof(STR_PackHead), &(*curPos), sizeof(STR_Goods));
                Server::GetInstance()->GetOperationPostgres()->PushUpdateGoods((*smap)[conn].m_roleid, &(*curPos), PostUpdate);
            }
            else
            {
                curPos->Count = 0;
                memcpy(buff + sizeof(STR_PackHead), &(*curPos), sizeof(STR_Goods));
                tarPos->Count += moveGoods->Count;  //将当前位置的物品添加到目标位置
                Server::GetInstance()->GetOperationPostgres()->PushUpdateGoods((*smap)[conn].m_roleid, &(*curPos), PostDelete);

                cur_goodsID->second.erase(curPos);
                OperationGoods::ReleasePos(conn, moveGoods->CurrentPos);
            }
            memcpy(buff + sizeof(STR_PackHead) + sizeof(STR_Goods), &(*tarPos), sizeof(STR_Goods));
            Server::GetInstance()->GetOperationPostgres()->PushUpdateGoods((*smap)[conn].m_roleid, &(*tarPos), PostUpdate);
        }
        else //移动部分
        {
            if(tarPos->Count + moveGoods->Count > GOODSMAXCOUNT)
            { //目标位置放不下，将目标位置放满
                curPos->Count = curPos->Count - (GOODSMAXCOUNT - tarPos->Count);
                tarPos->Count = GOODSMAXCOUNT;
            }
            else
            {
                curPos->Count -= moveGoods->Count;
                tarPos->Count += moveGoods->Count;
            }
            memcpy(buff + sizeof(STR_PackHead), &(*curPos), sizeof(STR_Goods));
            memcpy(buff + sizeof(STR_PackHead) + sizeof(STR_Goods), &(*tarPos), sizeof(STR_Goods));
            Server::GetInstance()->GetOperationPostgres()->PushUpdateGoods((*smap)[conn].m_roleid, &(*curPos), PostUpdate);
            Server::GetInstance()->GetOperationPostgres()->PushUpdateGoods((*smap)[conn].m_roleid, &(*tarPos), PostUpdate);
        }
    }
    STR_PackHead t_packHead;
    t_packHead.Len = sizeof(STR_Goods) * 2;
    t_packHead.Flag = FLAG_BagGoods;
    memcpy(buff, &t_packHead, sizeof(STR_PackHead));
    conn->Write_all(buff, sizeof(STR_PackHead) + t_packHead.Len);
    Server::GetInstance()->free(buff);
    Server::GetInstance()->free(moveGoods);
    return;
}


//交换物品
void OperationGoods::ExchangeBagGoods(TCPConnection::Pointer conn, STR_MoveBagGoods* moveGoods)
{
    SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();
    umap_roleGoods  playerBagGoods = (*smap)[conn].m_playerGoods;

    vector<STR_Goods>::iterator curPos;
    vector<STR_Goods>::iterator tarPos;
    _umap_roleGoods::iterator cur_goodsID = playerBagGoods->find(moveGoods->CurrentGoodsID);
    if(cur_goodsID == playerBagGoods->end())//没有找到要移动的商品
    {
        return;
    }
    for(curPos = cur_goodsID->second.begin(); curPos != cur_goodsID->second.end();)
    { //查找当前位置
        if(moveGoods->CurrentPos == curPos->Position)
        {
            if(moveGoods->Count > curPos->Count) //移动数量大于实际数量
            {
                return;
            }
            break;
        }
        curPos++;
        if(curPos == cur_goodsID->second.end()) return;
    }

    if((*smap)[conn].m_goodsPosition[moveGoods->TargetPos] == POS_EMPTY) //如果目标位置为空
    {
        hf_char* buff = (hf_char*)Server::GetInstance()->malloc();
        STR_Goods t_goods;
        memset(&t_goods, 0, sizeof(STR_Goods));

        if(curPos->Count - moveGoods->Count != 0)
        {
            t_goods.GoodsID = curPos->GoodsID;
            t_goods.TypeID = curPos->TypeID;
            t_goods.Count = moveGoods->Count;
            t_goods.Position = moveGoods->TargetPos;
            curPos->Count -= moveGoods->Count;
            cur_goodsID->second.push_back(t_goods);
            Server::GetInstance()->GetOperationPostgres()->PushUpdateGoods((*smap)[conn].m_roleid, &t_goods, PostUpdate);
            Server::GetInstance()->GetOperationPostgres()->PushUpdateGoods((*smap)[conn].m_roleid, &(*curPos), PostUpdate);
        }
        else
        {
            t_goods.Position = moveGoods->CurrentPos;
            curPos->Position = moveGoods->TargetPos;
            Server::GetInstance()->GetOperationPostgres()->PushUpdateGoods((*smap)[conn].m_roleid, &t_goods, PostDelete);
            Server::GetInstance()->GetOperationPostgres()->PushUpdateGoods((*smap)[conn].m_roleid, &(*curPos), PostInsert);
        }
         memcpy(buff + sizeof(STR_PackHead), &t_goods, sizeof(STR_Goods));
         memcpy(buff + sizeof(STR_PackHead) + sizeof(STR_Goods), &(*curPos), sizeof(STR_Goods));

        STR_PackHead t_packHead;
        t_packHead.Len = sizeof(STR_Goods) * 2;
        t_packHead.Flag = FLAG_BagGoods;
        memcpy(buff, &t_packHead, sizeof(STR_PackHead));
        conn->Write_all(buff, sizeof(STR_PackHead) + t_packHead.Len);
        Server::GetInstance()->free(buff);
        return;
    }

    _umap_roleGoods::iterator tar_goodsID = playerBagGoods->find(moveGoods->TargetGoodsID);
    if(tar_goodsID == playerBagGoods->end())
    {
        return;
    }

    for(tarPos = tar_goodsID->second.begin(); tarPos != tar_goodsID->second.end();)
    { //查找目标位置
        if(moveGoods->TargetPos == tarPos->Position) break;
        tarPos++;
        if(tarPos == tar_goodsID->second.end()) return;
    }

    hf_char* buff = (hf_char*)Server::GetInstance()->malloc();
    hf_uint16 t_pos = curPos->Position;
    curPos->Position = tarPos->Position;
    tarPos->Position = t_pos;

    memcpy(buff + sizeof(STR_PackHead), &(*curPos), sizeof(STR_Goods));
    memcpy(buff + sizeof(STR_PackHead) + sizeof(STR_Goods), &(*tarPos), sizeof(STR_Goods));
    Server::GetInstance()->GetOperationPostgres()->PushUpdateGoods((*smap)[conn].m_roleid, &(*curPos), PostUpdate);
    Server::GetInstance()->GetOperationPostgres()->PushUpdateGoods((*smap)[conn].m_roleid, &(*tarPos), PostUpdate);

    STR_PackHead t_packHead;
    t_packHead.Len = sizeof(STR_Goods) * 2;
    t_packHead.Flag = FLAG_BagGoods;
    memcpy(buff, &t_packHead, sizeof(STR_PackHead));
    conn->Write_all(buff, sizeof(STR_PackHead) + t_packHead.Len);
    Server::GetInstance()->free(buff);
    return;
}

//购买物品
void OperationGoods::BuyGoods(TCPConnection::Pointer conn, STR_BuyGoods* buyGoods)
{
    SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();
    if((*(*smap)[conn].m_interchage).isInchange == true)
    {
        Server::GetInstance()->free(buyGoods);
        return;
    }
    umap_roleMoney t_playerMoney = (*smap)[conn].m_playerMoney;
    _umap_goodsPrice::iterator price_it = m_goodsPrice->find(buyGoods->GoodsID);
    if(price_it == m_goodsPrice->end())  //购买的商品不存在
    {
        Server::GetInstance()->free(buyGoods);
        return;
    }
    if(price_it->second.BuyPrice == 0)   //不可买物品
    {
        Logger::GetLogger()->Debug("此物品不可购买");
        Server::GetInstance()->free(buyGoods);
        return;
    }

    if((*t_playerMoney)[Money_1].Count < buyGoods->Count * price_it->second.BuyPrice)  //金钱不够
    {
        STR_PackOtherResult t_result;
        t_result.Flag = FLAG_BuyGoods;
        t_result.result = Buy_NotEnoughMoney;
        conn->Write_all(&t_result, sizeof(STR_PackOtherResult));
        Server::GetInstance()->free(buyGoods);
        return;
    }

    if(EquTypeMinValue <= buyGoods->GoodsID && buyGoods->GoodsID <= EquTypeMaxValue)
    {
        BuyEquipment(conn, buyGoods, &(*t_playerMoney)[Money_1], price_it->second.BuyPrice);
    }
    else
    {
        BuyOtherGoods(conn, buyGoods, &(*t_playerMoney)[Money_1], price_it->second.BuyPrice);
    }
    Server::GetInstance()->free(buyGoods);
}

//出售物品
void OperationGoods::SellGoods(TCPConnection::Pointer conn, STR_PackSellGoods* sellGoods)
{
    SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();
    if((*(*smap)[conn].m_interchage).isInchange == true)
    {
        Server::GetInstance()->free(sellGoods);
        return;
    }

    umap_roleMoney t_playerMoney = (*smap)[conn].m_playerMoney;
    hf_uint32 roleid = (*smap)[conn].m_roleid;    
    OperationPostgres* t_post = Server::GetInstance()->GetOperationPostgres();

    if(sellGoods->GoodsID >= EquipMentID)   //出售装备
    {
        umap_roleEqu playerEqu = (*smap)[conn].m_playerEqu;
        _umap_roleEqu::iterator equ_it = playerEqu->find(sellGoods->GoodsID);
        if(equ_it == playerEqu->end())
        {
            Server::GetInstance()->free(sellGoods);
            return;
        }
        STR_GoodsPrice* equPrice = &(*m_goodsPrice)[equ_it->second.goods.TypeID]; //装备价格
        STR_PackGoods t_goods(&(equ_it->second.goods));
        conn->Write_all(&t_goods, sizeof(STR_PackGoods));
        t_post->PushUpdateGoods(roleid, &(equ_it->second.goods), PostDelete);
        //装备属性
        t_post->PushUpdateEquAttr(roleid, &(equ_it->second.equAttr), PostDelete);

        playerEqu->erase(equ_it);

        (*t_playerMoney)[Money_1].Count += equPrice->SellPrice;

        STR_PackPlayerMoney t_money(&(*t_playerMoney)[Money_1]);
        conn->Write_all(&t_money, sizeof(STR_PackPlayerMoney));
        t_post->PushUpdateMoney(roleid, &(*t_playerMoney)[Money_1]);  //将金钱变动插入到list

        Server::GetInstance()->free(sellGoods);
        return;
    }

    umap_roleGoods t_playerGoods = (*smap)[conn].m_playerGoods;
    _umap_roleGoods::iterator goods_it = t_playerGoods->find(sellGoods->GoodsID);
    if(goods_it == t_playerGoods->end()) //背包没有这种物品
    {
        Server::GetInstance()->free(sellGoods);
        return;
    }
    for(vector<STR_Goods>::iterator pos_it = goods_it->second.begin(); pos_it != goods_it->second.end();)
    {
        STR_GoodsPrice* goodsPrice = &(*m_goodsPrice)[sellGoods->GoodsID];
        if(pos_it->Position == sellGoods->Position)
        {          
            pos_it->Count = 0;
            t_post->PushUpdateGoods(roleid, &(*pos_it), PostDelete);
            STR_PackGoods t_goods(&(*pos_it));
            conn->Write_all(&t_goods, sizeof(STR_PackGoods));

            goods_it->second.erase(pos_it);
            if(goods_it->second.begin() == goods_it->second.end())
            {
                t_playerGoods->erase(goods_it);
            }

            (*t_playerMoney)[Money_1].Count += goodsPrice->SellPrice * pos_it->Count;
            STR_PackPlayerMoney t_money(&(*t_playerMoney)[Money_1]);
            conn->Write_all(&t_money, sizeof(STR_PackPlayerMoney));
            t_post->PushUpdateMoney(roleid, &(*t_playerMoney)[Money_1]);  //将金钱变动插入到list
            Server::GetInstance()->free(sellGoods);
            return;
        }
        pos_it++;
        if(pos_it == goods_it->second.end()) //到结尾了没找到这种物品
        {
            Server::GetInstance()->free(sellGoods);
            return;
        }
    }
}

//得到玩家背包中某种/某类物品的数量
hf_uint32  OperationGoods::GetThisGoodsCount(TCPConnection::Pointer conn, hf_uint32 goodsID)
{
    SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();
    umap_roleGoods playerGoods = (*smap)[conn].m_playerGoods;
    if(EquTypeMinValue <= goodsID && EquTypeMaxValue <= goodsID)
    {
        umap_roleEqu playerEqu = (*smap)[conn].m_playerEqu;
        if(playerEqu->size() == 0)
        {
            return 0;
        }
        hf_uint32 count = 0;
        for(_umap_roleEqu::iterator it = playerEqu->begin(); it != playerEqu->end(); it++)
        {
            if(it->second.goods.TypeID == goodsID)
            {
                count++;
            }
        }
        return count;
    }
    _umap_roleGoods::iterator it = playerGoods->find(goodsID);
    if(it == playerGoods->end())
    {
        return 0;
    }
    else
    {
        hf_uint32 count = 0;
        for(vector<STR_Goods>::iterator iter = it->second.begin(); iter != it->second.end(); iter++)
        {
            count += iter->Count;
        }
        return count;
    }
}

//给新捡的装备属性 附初值
void OperationGoods::SetEquAttr(STR_Equipment* equAttr, hf_uint32 typeID)
{
    umap_equAttr::iterator attr_it = m_equAttr->find(typeID);
    if(attr_it != m_equAttr->end())
    {
        equAttr->TypeID = attr_it->second.TypeID;
        equAttr->PhysicalAttack = attr_it->second.PhysicalAttack;
        equAttr->PhysicalDefense = attr_it->second.PhysicalDefense;
        equAttr->MagicAttack = attr_it->second.MagicAttack;
        equAttr->MagicDefense = attr_it->second.MagicDefense;
        equAttr->AddHp = attr_it->second.AddHp;
        equAttr->AddMagic = attr_it->second.AddMagic;
        equAttr->Durability = attr_it->second.Durability;
    }
}

//购买装备
void OperationGoods::BuyEquipment(TCPConnection::Pointer conn, STR_BuyGoods* buyGoods, STR_PlayerMoney* money, hf_uint32 price)
{
    SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();
    STR_PackOtherResult t_result;
    t_result.Flag = FLAG_BuyGoods;

    hf_uint16 emptyPosCount = OperationGoods::GetEmptyPosCount(conn);
    if(buyGoods->Count > emptyPosCount) //背包满了
    {
        t_result.result = Buy_BagFull;
        conn->Write_all(&t_result, sizeof(STR_PackOtherResult));
        return;
    }
    umap_roleEqu playerEqu = (*smap)[conn].m_playerEqu;
    hf_char* buff = (hf_char*)Server::GetInstance()->malloc();
    hf_char* attrbuff = (hf_char*)Server::GetInstance()->malloc();

    STR_PlayerEqu t_equ;
    hf_uint32 roleid = (*smap)[conn].m_roleid;
    OperationPostgres* t_post = Server::GetInstance()->GetOperationPostgres();

    SetEquAttr(&t_equ.equAttr, buyGoods->GoodsID);
    t_equ.goods.TypeID = buyGoods->GoodsID;
    t_equ.goods.Count = 1;
    t_equ.goods.Source = Source_Buy;

    for(hf_uint16 i = 0; i < buyGoods->Count; i++)
    {
        t_equ.goods.Position = OperationGoods::GetEmptyPos(conn);
        t_equ.goods.GoodsID = GetEquipmentID();
        (*playerEqu)[t_equ.goods.GoodsID] = t_equ;

        memcpy(buff + sizeof(STR_PackHead) + i * sizeof(STR_Goods), &t_equ.goods, sizeof(STR_Goods));
        t_post->PushUpdateGoods(roleid, &t_equ.goods, PostInsert); //将新买的物品添加到list

        memcpy(attrbuff + sizeof(STR_PackHead) + i * sizeof(STR_Equipment), &t_equ.equAttr, sizeof(STR_Equipment)); //装备属性
        t_post->PushUpdateEquAttr(roleid, &t_equ.equAttr, PostInsert); //将新买的物品添加到list
    }

    STR_PackHead t_packHead;
    t_packHead.Len = sizeof(STR_Goods)*buyGoods->Count;
    t_packHead.Flag = FLAG_BagGoods;
    memcpy(buff, &t_packHead, sizeof(STR_PackHead));
    conn->Write_all(buff, sizeof(STR_PackHead) + t_packHead.Len);

    t_packHead.Len = sizeof(STR_Equipment)*buyGoods->Count;
    t_packHead.Flag = FLAG_EquGoodsAttr;
    memcpy(attrbuff, &t_packHead, sizeof(STR_PackHead));
    conn->Write_all(attrbuff, sizeof(STR_PackHead) + t_packHead.Len);

    money->Count -= buyGoods->Count * price;
    t_post->PushUpdateMoney(roleid, money);  //将金钱变动插入到list

    STR_PackPlayerMoney t_money(money);
    conn->Write_all(&t_money, sizeof(STR_PackPlayerMoney));
    Server::GetInstance()->free(buff);
    Server::GetInstance()->free(attrbuff);
}

//购买其他物品
void OperationGoods::BuyOtherGoods(TCPConnection::Pointer conn, STR_BuyGoods* buyGoods, STR_PlayerMoney* money, hf_uint32 price)
{
    SessionMgr::SessionMap *smap =  SessionMgr::Instance()->GetSession().get();
    hf_uint16 emptyPosCount = OperationGoods::GetEmptyPosCount(conn);
    hf_uint16 size = buyGoods->Count/GOODSMAXCOUNT + (buyGoods->Count % GOODSMAXCOUNT ? 1 : 0);
    if(size > emptyPosCount)
    {
        STR_PackOtherResult t_result;
        t_result.Flag = FLAG_BuyGoods;
        t_result.result = Buy_BagFull;
        conn->Write_all(&t_result, sizeof(STR_PackOtherResult));
        return;
    }

    STR_Goods t_goods;
    t_goods.Source = Source_Buy;
    t_goods.GoodsID = buyGoods->GoodsID;
    t_goods.TypeID = buyGoods->GoodsID;
    hf_char* buff = (hf_char*)Server::GetInstance()->malloc();

    hf_uint16 buyCount = buyGoods->Count;
    umap_roleGoods t_playerGoods = (*smap)[conn].m_playerGoods;
    _umap_roleGoods::iterator goods_it = t_playerGoods->find(buyGoods->GoodsID);
    hf_uint32 roleid = (*smap)[conn].m_roleid;

    OperationPostgres* t_post = Server::GetInstance()->GetOperationPostgres();
    if(goods_it == t_playerGoods->end())  //背包还没这种物品
    {
        vector<STR_Goods> t_vec;
        for(hf_uint16 i = 0; i < size; i++)
        {
            t_goods.Position = OperationGoods::GetEmptyPos(conn);
            if(buyGoods->Count < GOODSMAXCOUNT)
            {
                t_goods.Count = buyGoods->Count;
            }
            else
            {
                t_goods.Count = GOODSMAXCOUNT;
                buyGoods->Count -= GOODSMAXCOUNT;
            }
            t_vec.push_back(t_goods);
            memcpy(buff + sizeof(STR_PackHead) + i * sizeof(STR_Goods), &t_goods, sizeof(STR_Goods));

            t_post->PushUpdateGoods(roleid, &t_goods, PostInsert);  //将新买的物品添加到list
        }

        (*t_playerGoods)[t_goods.GoodsID] = t_vec;
        STR_PackHead t_packHead;
        t_packHead.Len = sizeof(STR_Goods)*size;
        t_packHead.Flag = FLAG_BagGoods;
        memcpy(buff, &t_packHead, sizeof(STR_PackHead));
        conn->Write_all(buff, sizeof(STR_PackHead) + t_packHead.Len);

        money->Count -= buyCount * price;
        t_post->PushUpdateMoney(roleid, money);

        memset(buff, 0, CHUNK_SIZE);
        t_packHead.Len = sizeof(STR_PlayerMoney);
        t_packHead.Flag = FLAG_PlayerMoney;

        memcpy(buff, &t_packHead, sizeof(STR_PackHead));
        memcpy(buff + sizeof(STR_PackHead), money, sizeof(STR_Goods));
        conn->Write_all(buff, sizeof(STR_PackHead) + t_packHead.Len);
        Server::GetInstance()->free(buff);
        return;
    }

    for(hf_uint16 i = 0; i < size; i++)
    {
        if(buyGoods->Count >= GOODSMAXCOUNT)
        {
            t_goods.Position = OperationGoods::GetEmptyPos(conn);
            t_goods.Count = GOODSMAXCOUNT;
            buyGoods->Count -= GOODSMAXCOUNT;
            goods_it->second.push_back(t_goods);
            memcpy(buff + sizeof(STR_PackHead) + i * sizeof(STR_Goods), &t_goods, sizeof(STR_Goods));

            t_post->PushUpdateGoods(roleid, &t_goods, PostInsert); //将新买的物品添加到list
            continue;
        }
        for(vector<STR_Goods>::iterator vec = goods_it->second.begin(); vec != goods_it->second.end();)
        {
            if(vec->Count + buyGoods->Count <= GOODSMAXCOUNT) //当前位置能放下剩余的物品
            {
                t_goods.Position = vec->Position;
                t_goods.Count = vec->Count + buyGoods->Count;
                vec->Count = t_goods.Count;
                memcpy(buff + sizeof(STR_PackHead) + i * sizeof(STR_Goods), &t_goods, sizeof(STR_Goods));

                t_post->PushUpdateGoods(roleid, &t_goods, PostUpdate); //将新买的物品添加到list
                break;
            }
            vec++;
            if(vec == goods_it->second.end()) //开辟新位置放下剩余的物品
            {
                t_goods.Position = OperationGoods::GetEmptyPos(conn);
                t_goods.Count = buyGoods->Count;
                goods_it->second.push_back(t_goods);
                memcpy(buff + sizeof(STR_PackHead) + i * sizeof(STR_Goods), &t_goods, sizeof(STR_Goods));

                t_post->PushUpdateGoods(roleid, &t_goods, PostInsert); //将新买的物品添加到list
                break;
            }
        }
    }
    //给客户端更新背包物品变化和金钱
    STR_PackHead t_packHead;
    t_packHead.Len = sizeof(STR_Goods)*size;
    t_packHead.Flag = FLAG_BagGoods;
    memcpy(buff, &t_packHead, sizeof(STR_PackHead));
    conn->Write_all(buff, sizeof(STR_PackHead) + t_packHead.Len);

    money->Count -= buyCount * price;
    t_post->PushUpdateMoney(roleid, money);  //将金钱变动插入到list

    memset(buff, 0, CHUNK_SIZE);
    t_packHead.Len = sizeof(STR_PlayerMoney);
    t_packHead.Flag = FLAG_PlayerMoney;

    memcpy(buff, &t_packHead, sizeof(STR_PackHead));
    memcpy(buff + sizeof(STR_PackHead), money, sizeof(STR_Goods));
    conn->Write_all(buff, sizeof(STR_PackHead) + t_packHead.Len);
    Server::GetInstance()->free(buff);
}


//test 查询数据库中装备编号的最大值
void OperationGoods::SetEquIDInitialValue()
{
    m_equipmentID = Server::GetInstance()->getDiskDB()->GetEquIDMaxValue();
}
