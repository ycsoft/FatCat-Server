#include "gameinterchange.h"
#include "memManage/diskdbmanager.h"
#include "utils/stringbuilder.hpp"
#include "Game/log.hpp"
#include "server.h"
#include "OperationPostgres/operationpostgres.h"
#include "GameTask/gametask.h"

#define Result_Accept 1        //接受交易
#define Result_Reject 2         //拒绝交易
#define Result_RejectTimeOut 3     //超时拒绝交易

#define Goods_Add 1             //向交易框中添加物品
#define Goods_Remove 2      //从交易框中删除物品

#define Operation_CheckChange 1        //确认交易
#define Operation_Lock     2                   //锁定交易
#define Operation_UnLock 3                  //解除锁定
#define Operation_CancelChange 4       //取消交易
#define Operation_BothUnLock 5          //对方确认交易情况下，取消锁定，则双方都取消锁定
#define Operation_Done 6              //交易完成
#define Operation_NotLockedCheckChange 7 //对方没有锁定情况下，确认交易请求无效
#define Operation_MoneyNotEnough 8       //钱不够
#define Operation_PosNotEnough 9

#define max_EquipMentId 30000000
#define min_EquipMentId  20000000

GameInterchange::GameInterchange()
{

}

GameInterchange::~GameInterchange()
{

}

void GameInterchange::operRequest(TCPConnection::Pointer conn, operationRequest*  operReq)
{
    Server* srv = Server::GetInstance();
    SessionMgr::SessionPointer sp = SessionMgr::Instance()->GetSession();

    if((*sp)[conn].m_interchage->isInchange)         //已经在交易状态，不能再发请求交易的数据包
    {
        srv->free(operReq);
        return;
    }

    umap_roleSock rsock =  SessionMgr::Instance()->GetRoleSock();
    auto iter = rsock->find(operReq->RoleID);
    if(iter == rsock->end())           //对方不在线 由服务器发拒绝的包返回
    {
        operationRequestResult resp;
        resp.operType = operReq->operType;
        resp.operResult = Result_Reject;
        conn->Write_all((char*)&resp, sizeof(operationRequestResult));
        srv->free(operReq);
        return;
    }

    TCPConnection::Pointer partnerConn = (*rsock)[operReq->RoleID];
    if((*sp)[partnerConn].m_interchage->inChange())             //对方正在交易中 由服务器发拒绝的包返回
    {
        operationRequestResult resp;
        resp.operType = operReq->operType;
        resp.operResult = Result_Reject;
        conn->Write_all((char*)&resp, sizeof(operationRequestResult));
        srv->free(operReq);
        return;
    }

    (*sp)[conn].m_interchage->goInChange();          //进入交易状态
    (*sp)[partnerConn].m_interchage->goInChange();          //服务器设定对方也进入交易状态 ，避免竞争
    (*sp)[conn].m_interchage->roleId = (*sp)[conn].m_roleid;  //保存自己的角色id
    (*sp)[partnerConn].m_interchage->roleId = (*sp)[partnerConn].m_roleid;  //对方保存自己的角色id
    (*sp)[conn].m_interchage->partnerConn = partnerConn; //互相保存对方的连接
    (*sp)[partnerConn].m_interchage->partnerConn = conn;  //互相保存对方的连接

    operReq->RoleID =  (*sp)[conn].m_roleid;                        // 向对方转发请求
    partnerConn->Write_all((char*)operReq, sizeof(operationRequest));
    srv->free(operReq);

}

void GameInterchange::operResponse(TCPConnection::Pointer conn,  operationRequestResult*  operReq)
{
    Server* srv = Server::GetInstance();
    SessionMgr::SessionPointer sp = SessionMgr::Instance()->GetSession();
    umap_roleSock rsock =  (SessionMgr::Instance()->GetRoleSock());
    auto iter = rsock->find(operReq->RoleID);
    if(iter == rsock->end())                    //对方掉线 由服务器发取消交易的包返回
    {
        if(operReq->operResult == Result_Accept)
        {
            interchangeOperPro resp;
            resp.operType = Operation_CancelChange;
            conn->Write_all((char*)&resp, sizeof(interchangeOperPro));
        }
        (*sp)[conn].m_interchage->clear();
        srv->free(operReq);
        return;
    }

    TCPConnection::Pointer partnerConn = (*rsock)[operReq->RoleID];
    operReq->RoleID =  (*sp)[conn].m_roleid;
    partnerConn->Write_all((char*)operReq, sizeof(operationRequestResult));       //转发答复的包

    if(operReq->operResult == Result_Accept)                                                                                   //如果答复同意交易，则设置交易标志
    {
//        (*sp)[conn].m_interchage->roleId = (*sp)[conn].m_roleid;  //保存自己的角色id
//        (*sp)[partnerConn].m_interchage->roleId = (*sp)[partnerConn].m_roleid;  //对方保存自己的角色id
//        (*sp)[conn].m_interchage->partnerConn = partnerConn; //互相保存对方的连接
//        (*sp)[partnerConn].m_interchage->partnerConn = conn;  //互相保存对方的连接
    }
    else if(operReq->operResult == Result_Reject || operReq->operResult == Result_RejectTimeOut)                           //拒绝
    {
        (*sp)[conn].m_interchage->clear();
        (*sp)[partnerConn].m_interchage->clear();
    }

   srv->free(operReq);
}

void GameInterchange::operMoneyChanges(TCPConnection::Pointer conn, interchangeMoney*  oper) //交易金钱变动
{

    Server* srv = Server::GetInstance();
    SessionMgr::SessionPointer sp = SessionMgr::Instance()->GetSession();
    boost::shared_ptr<Interchange> interchange = (*sp)[conn].m_interchage;
    TCPConnection::Pointer partnerConn = interchange->partnerConn;

    auto iter = sp->find(partnerConn);
    if(iter == sp->end())                        //对方离线 则发取消交易的包返回
    {
        interchangeOperPro resp;
        resp.operType = Operation_CancelChange;
        conn->Write_all((char*)&resp, sizeof(interchangeOperPro));
        for(auto iter = interchange->changes.begin(); iter != interchange->changes.end(); ++iter)    //背包位置信息恢复原来标志
        {
            (*sp)[conn].m_goodsPosition[iter->Position] = POS_NONEMPTY;
        }
        interchange->clear();           //清除交易状态
        srv->free(oper);
        return;
    }

    _umap_roleMoney::iterator iterMoney = (*sp)[conn].m_playerMoney->find(oper->MoneyType);
    if(iterMoney == (*sp)[conn].m_playerMoney->end()                                           //没有这样类型的金币
            || (*((*sp)[conn].m_playerMoney))[oper->MoneyType].Count < oper->MoneyCount)  //或者没有这样数量的金币
    {
        interchangeOperPro resp;
        resp.operType = Operation_MoneyNotEnough;         //由服务器发金币不足的包
        conn->Write_all((char*)&resp, sizeof(interchangeOperPro));
        srv->free(oper);
        return;
    }

    conn->Write_all((char*)oper, sizeof(interchangeMoney));
    oper->roleId = (*sp)[conn].m_roleid;
    partnerConn->Write_all((char*)oper, sizeof(interchangeMoney));

    interchange->money.MoneyCount = oper->MoneyCount;                      //缓存要交易的金币数量
    interchange->money.MoneyType = oper->MoneyType;

    srv->free(oper);
}

void GameInterchange::operChanges(TCPConnection::Pointer conn, interchangeOperGoods*  oper) //交易物品变动
{

    Server* srv = Server::GetInstance();

    SessionMgr::SessionPointer sp = SessionMgr::Instance()->GetSession();

    if(oper->goodsId>=min_EquipMentId && oper->goodsId <=max_EquipMentId)
    {
        _umap_roleEqu::iterator iterRoleGoods = (*sp)[conn].m_playerEqu->find(oper->goodsId);
        if(iterRoleGoods == (*sp)[conn].m_playerEqu->end())            //玩家选择的物品在服务器中不存在属于异常情况，忽略异常情况，服务器不做响应
        {
            return;
        }

        if(iterRoleGoods->second.goods.Position == oper->position&&iterRoleGoods->second.goods.Count == oper->goodsCount) //位置信息和数量同时对应，则是找到玩家选择的物品
        {
            (*sp)[conn].m_goodsPosition[oper->position] = POS_LOCKED;       //背包中找到该物品，则设定对该物品不能做其它操作
        }
        else
        {
            return;
        }
    }
    else
    {
        _umap_roleGoods::iterator iterRoleGoods = (*sp)[conn].m_playerGoods->find(oper->goodsId);
        if(iterRoleGoods == (*sp)[conn].m_playerGoods->end())            //玩家选择的物品在服务器中不存在属于异常情况，忽略异常情况，服务器不做响应
        {
            return;
        }
        vector<STR_Goods>::iterator iterGoods = iterRoleGoods->second.begin();
        for(; iterGoods != iterRoleGoods->second.end(); ++iterGoods)
        {
            if(iterGoods->Position == oper->position&&iterGoods->Count == oper->goodsCount) //位置信息和数量同时对应，则是找到玩家选择的物品
            {
                (*sp)[conn].m_goodsPosition[oper->position] = POS_LOCKED;       //背包中找到该物品，则设定对该物品不能做其它操作
                break;
            }
        }
        if(iterGoods == iterRoleGoods->second.end())                                    //背包中未找到该物品，属于异常情况，忽略异常情况
        {
            return;
        }
    }

    boost::shared_ptr<Interchange> interchange = (*sp)[conn].m_interchage;
    if(oper->operType == Goods_Add)                        //增加物品的操作
    {
        auto iter = interchange->changes.begin();
        for( ;iter != interchange->changes.end(); ++iter)
        {
            if(iter->Position == oper->position)
                break;
        }
        if(iter == interchange->changes.end())              //交易筐中还没添加该位置的物品
        {
            STR_Goods goods;
            goods.Count = oper->goodsCount;
            goods.Position = oper->position;
            goods.GoodsID = oper->goodsId;
            goods.TypeID = oper->goodsType;
            goods.Source = Source_Trade;
            interchange->changes.push_back(goods);
        }
        else
        {
            return;                //交易筐中已经添加该位置的物品，属于异常情况，则忽略该次请求
        }
    }
    else  if(oper->operType == Goods_Remove)                                                       //从交易篮中删除物品
    {
       int haveValue = 0;
       auto iter = interchange->changes.begin();
       for(;iter != interchange->changes.end();++iter)
       {
           if(iter->Count == oper->goodsCount&&iter->Position ==  oper->position&&iter->GoodsID == oper->goodsId)
           {
               haveValue = 1;
               iter = interchange->changes.erase(iter);
                (*sp)[conn].m_goodsPosition[oper->position] = POS_NONEMPTY;       //恢复物品原来标志
               break;
           }
       }
       if(iter == interchange->changes.end()&& haveValue == 0)
       {
           return;
       }
    }

    TCPConnection::Pointer partnerConn = interchange->partnerConn;
    auto iter = sp->find(partnerConn);
    if(iter == sp->end())                        //对方离线 则由服务器发取消交易的包返回
    {
        interchangeOperPro resp;
        resp.operType = Operation_CancelChange;
        conn->Write_all((char*)&resp, sizeof(interchangeOperPro));
        for(auto iter = interchange->changes.begin(); iter != interchange->changes.end(); ++iter)    //背包位置信息恢复原来标志
        {
            (*sp)[conn].m_goodsPosition[iter->Position] = POS_NONEMPTY;
        }
        interchange->clear();           //清除交易状态  对方离线所以不用处理其状态信息
        srv->free(oper);
        return;
    }

    conn->Write_all((char*)oper, sizeof(interchangeOperGoods));
    partnerConn->Write_all((char*)oper, sizeof(interchangeOperGoods));
    srv->free(oper);
}

void GameInterchange::operProLock(TCPConnection::Pointer conn,  interchangeOperPro*  oper)
{
    Server* srv = Server::GetInstance();
    SessionMgr::SessionPointer sp = SessionMgr::Instance()->GetSession();
    boost::shared_ptr<Interchange> interchange = (*sp)[conn].m_interchage;
    TCPConnection::Pointer partnerConn = interchange->partnerConn;


    interchange->lock();                  //进入锁定状态

    auto iter = sp->find(partnerConn);
    if(iter == sp->end())                        //对方离线 则由服务器发取消交易的包返回
    {
        interchangeOperPro resp;
        resp.operType = Operation_CancelChange;
        conn->Write_all((char*)&resp, sizeof(interchangeOperPro));
        for(auto iterIn = interchange->changes.begin(); iterIn != interchange->changes.end();++iterIn)
        {
              (*sp)[conn].m_goodsPosition[iterIn->Position] = POS_NONEMPTY;       //恢复物品原来标志
        }
        interchange->clear();           //清除交易状态  对方离线所以不用处理其状态信息

        srv->free(oper);
        return;
    }

    partnerConn->Write_all((char*)oper, sizeof(interchangeOperPro));
    srv->free(oper);
}

void GameInterchange::operProUnlock(TCPConnection::Pointer conn, interchangeOperPro*  oper)
{

    Server* srv = Server::GetInstance();
    SessionMgr::SessionPointer sp = SessionMgr::Instance()->GetSession();
    boost::shared_ptr<Interchange> interchange = (*sp)[conn].m_interchage;
    TCPConnection::Pointer partnerConn = interchange->partnerConn;

    auto iter = sp->find(partnerConn);
    if(iter == sp->end())    //对方离线 则由服务器发取消交易的包返回
    {
        interchangeOperPro resp;
        resp.operType = Operation_CancelChange;
        conn->Write_all((char*)&resp, sizeof(interchangeOperPro));

        for(auto iterIn = interchange->changes.begin(); iterIn != interchange->changes.end();++iterIn)
        {
              (*sp)[conn].m_goodsPosition[iterIn->Position] = POS_NONEMPTY;       //恢复物品原来标志
        }

        interchange->clear();           //清除交易状态  对方离线所以不用处理其状态信息

        srv->free(oper);
        return;
    }
    else                //对方在线
    {
        if((*sp)[partnerConn].m_interchage->isChangeChecked)  //对方已经点击确认交易时，如果解除锁定，则对方也退回到解除锁定状态
        {
            interchange->unlock();           //解除锁定状态
            (*sp)[partnerConn].m_interchage->unlock();        //对方解除锁定状态
            oper->operType = Operation_BothUnLock;  //对方已经点击确认交易时，如果解除锁定，则对方也退回到解除锁定状态
            conn->Write_all((char*)oper, sizeof(interchangeOperPro));  //这个包要向双方都发
            partnerConn->Write_all((char*)oper, sizeof(interchangeOperPro)); //这个包要向双方都发
        }
        else   //对方没有点击交易时，则向其转发取消锁定的包
        {
            oper->operType = Operation_UnLock;
            partnerConn->Write_all((char*)oper, sizeof(interchangeOperPro));
        }
        srv->free(oper);
    }
}

void GameInterchange::operProCheckChange(TCPConnection::Pointer conn,  interchangeOperPro*  oper)  //确认交易
{

    Server* srv = Server::GetInstance();
    SessionMgr::SessionPointer sp = SessionMgr::Instance()->GetSession();
    boost::shared_ptr<Interchange> interchange = (*sp)[conn].m_interchage;
    TCPConnection::Pointer partnerConn =  interchange->partnerConn;

    auto iter = sp->find(partnerConn);
    if(iter == sp->end())    //对方离线 则由服务器发取消交易的包返回
    {
        interchangeOperPro resp;
        resp.operType = Operation_CancelChange;
        conn->Write_all((char*)&resp, sizeof(interchangeOperPro));

        for(auto iterIn = interchange->changes.begin(); iterIn != interchange->changes.end();++iterIn)
        {
              (*sp)[conn].m_goodsPosition[iterIn->Position] = POS_NONEMPTY;       //恢复物品原来标志
        }
        interchange->clear();           //清除交易状态  对方离线所以不用处理其状态信息
        srv->free(oper);
        return;
    }

    boost::shared_ptr<Interchange> pInterchange = (*sp)[partnerConn].m_interchage;

    if(!(interchange->isLocked&&pInterchange->isLocked)) //双方没有全都锁定过时候，确认交易请求无效
    {
        interchangeOperPro resp;
        resp.operType = Operation_NotLockedCheckChange;  //回复双方没有全都锁定的包
        conn->Write_all((char*)&resp, sizeof(interchangeOperPro));
        srv->free(oper);
        return;
    }

    interchange->isChangeChecked = true;      //双方都已经锁定过，则置确认交易的标志

    if(!pInterchange->isChangeChecked)   //对方没有确认交易，则只转发包返回
    {
        partnerConn->Write_all((char*)oper, sizeof(interchangeOperPro));
        srv->free(oper);
        return;
    }

    partnerConn->Write_all((char*)oper, sizeof(interchangeOperPro)); //对方也已经确认交易 转发包
    srv->free(oper);

    //计算双方背包空闲位置个数，判断是否满足交换需求
    int emptyPartnerconn = 0;
    int emptyconn = 0;
    for(int i = 1; i <=BAGCAPACITY-1;++i)                     //查找空闲位置 计算空闲位置个数
    {
        if((*sp)[conn].m_goodsPosition[i] == POS_EMPTY)
        {
           emptyconn++;
        }
    }
    for(int i = 1; i <=BAGCAPACITY-1;++i)                     //查找空闲位置 计算空闲位置个数
    {
        if((*sp)[partnerConn].m_goodsPosition[i] == POS_EMPTY)
        {
           emptyPartnerconn++;
        }
    }
    if(interchange->changes.size() + emptyconn < pInterchange->changes.size()
            || pInterchange->changes.size() + emptyPartnerconn < interchange->changes.size())        //如果空闲位置个数不满足交换需求,则取消交易
    {
        interchangeOperPro resp;
        resp.operType = Operation_CancelChange;
        conn->Write_all((char*)&resp, sizeof(interchangeOperPro));
        partnerConn->Write_all((char*)&resp, sizeof(interchangeOperPro));

        for(auto iter = interchange->changes.begin(); iter != interchange->changes.end();++iter)
        {
              (*sp)[conn].m_goodsPosition[iter->Position] = POS_NONEMPTY;       //恢复物品原来标志
        }
        for(auto iter = pInterchange->changes.begin(); iter != pInterchange->changes.end();++iter)
        {
              (*sp)[partnerConn].m_goodsPosition[iter->Position] = POS_NONEMPTY;       //恢复物品原来标志
        }
        interchange->clear();           //清除交易状态
        pInterchange->clear(); //对方清除交易状态
        return;
    }

    operDoChange(conn);//如果空闲位置个数满足交换需求，则交换两个玩家物品和金钱
    operReport(conn);    //交换完成发送交易报告

    interchangeOperPro resp;
    resp.operType = Operation_Done;
    conn->Write_all((char*)&resp, sizeof(interchangeOperPro));
    partnerConn->Write_all((char*)&resp, sizeof(interchangeOperPro));

    GameTask* t_task = srv->GetGameTask();
    for(auto iter = interchange->changes.begin(); iter != interchange->changes.end();++iter)
    {
          t_task->UpdateCollectGoodsTaskProcess(conn,iter->TypeID);
    }
    for(auto iter = pInterchange->changes.begin(); iter != pInterchange->changes.end();++iter)
    {
          t_task->UpdateCollectGoodsTaskProcess(conn,iter->TypeID);
    }

    interchange->clear();                          //交易完毕 恢复到原来状态
    pInterchange->clear();  //交易完毕 恢复到原来状态
}

void GameInterchange::operDoChange(TCPConnection::Pointer conn)  //交换双方交易的物品
{

    Server* srv = Server::GetInstance();
    SessionMgr::SessionPointer sp = SessionMgr::Instance()->GetSession();
    boost::shared_ptr<Interchange> interchange = (*sp)[conn].m_interchage;
    TCPConnection::Pointer partnerConn =  interchange->partnerConn;
    boost::shared_ptr<Interchange> pInterchange = (*sp)[partnerConn].m_interchage;
    OperationPostgres*  opg =  srv->GetOperationPostgres();


    //把本方要交易的物品从本方背包删除
    for(auto iter = interchange->changes.begin(); iter != interchange->changes.end();++iter)
    {
        _umap_roleGoods::iterator iterGoods = (*sp)[conn].m_playerGoods->find(iter->GoodsID);
        for(auto iterIn = iterGoods->second.begin(); iterIn != iterGoods->second.end(); ++iterIn)
        {
            if(iterIn->Position == iter->Position)
            {
                if(iter->GoodsID>=min_EquipMentId && iter->GoodsID <=max_EquipMentId)   //装备
                {
                    (*sp)[conn].m_goodsPosition[iter->Position] = POS_EMPTY;
                    opg->PushUpdateGoods(interchange->roleId,&(*iter),PostDelete);
//                    (*sp)[conn].m_playerGoods->erase(iter->GoodsID);
                    break;
                }

                opg->PushUpdateGoods(interchange->roleId,&(*iter),PostDelete);
                 (*sp)[conn].m_goodsPosition[iter->Position] = POS_EMPTY;
                iterIn->Count -= iter->Count;
                if(iterIn->Count == 0)
                {
                    iterGoods->second.erase(iterIn);
                }
                break;
            }
        }
        if(0==iterGoods->second.size())
        {
            ((*sp)[conn].m_playerGoods)->erase(iter->GoodsID);
        }
    }

    //把对方要交易物品从对方背包中删除
    for(auto iter = pInterchange->changes.begin(); iter != pInterchange->changes.end();++iter)
    {
        _umap_roleGoods::iterator iterGoods = (*sp)[partnerConn].m_playerGoods->find(iter->GoodsID);
        for(auto iterIn = iterGoods->second.begin(); iterIn != iterGoods->second.end(); ++iterIn)
        {
            if(iterIn->Position == iter->Position)
            {
                if(iter->GoodsID>=min_EquipMentId && iter->GoodsID <=max_EquipMentId)   //装备
                {

                    (*sp)[partnerConn].m_goodsPosition[iter->Position] = POS_EMPTY;
                    opg->PushUpdateGoods(pInterchange->roleId,&(*iter),PostDelete);
//                    (*sp)[partnerConn].m_playerGoods->erase(iter->GoodsID);
                    break;
                }
                opg->PushUpdateGoods(pInterchange->roleId,&(*iter),PostDelete);

                (*sp)[partnerConn].m_goodsPosition[iter->Position] = POS_EMPTY;
                iterIn->Count -= iter->Count;
                if(iterIn->Count == 0)
                {
                    iterGoods->second.erase(iterIn);
                }
                break;
            }
        }
        if(0==iterGoods->second.size())
        {
            ((*sp)[partnerConn].m_playerGoods)->erase(iter->GoodsID);
        }
    }

    //将本方要交易的物品添加到对方背包
    for(auto iter = interchange->changes.begin(); iter != interchange->changes.end();++iter)
    {

        vector<STR_Goods> vec;
        STR_Goods goods;
        int i = 1;
        for(; i <=BAGCAPACITY-1;++i)                     //查找空闲位置
        {
            if((*sp)[partnerConn].m_goodsPosition[i] == POS_EMPTY)
            {
                (*sp)[partnerConn].m_goodsPosition[i] = POS_NONEMPTY;
                break;
            }
        }
        goods.Count = iter->Count;
        goods.GoodsID = iter->GoodsID;
        goods.Position = i;
        goods.TypeID = iter->TypeID;
        goods.Source = Source_Trade;
        vec.push_back(goods);
        iter->Position = i;
        opg->PushUpdateGoods(pInterchange->roleId,&goods,PostInsert);

        if(iter->GoodsID>=min_EquipMentId && iter->GoodsID <=max_EquipMentId)   //装备
        {

            STR_Equipment equAttr = (*((*sp)[conn].m_playerEqu))[iter->GoodsID].equAttr;
            interchange->vecEqui.push_back(equAttr);
            STR_PlayerEqu equ;
            equ.goods = goods;
            equ.equAttr = equAttr;
            (*((*sp)[partnerConn].m_playerEqu))[iter->GoodsID]  = equ;
            (*sp)[conn].m_playerEqu->erase(iter->GoodsID);
            opg->PushUpdateEquAttr(pInterchange->roleId,&equAttr,PostUpdate);
            continue;
        }

        _umap_roleGoods::iterator iterPartnerGoods = (*sp)[partnerConn].m_playerGoods->find(iter->GoodsID);
        if(iterPartnerGoods != (*sp)[partnerConn].m_playerGoods->end())
        {
            iterPartnerGoods->second.push_back(goods);
        }
        else
        {
            (*((*sp)[partnerConn].m_playerGoods))[iter->GoodsID] = vec;
        }
    }

     //将对方要交易的物品添加到本方背包
    for(auto iter = pInterchange->changes.begin(); iter != pInterchange->changes.end();++iter)
    {
        vector<STR_Goods> vec;
        STR_Goods goods;
        int i = 1;
        for(; i <=BAGCAPACITY-1;++i)
        {
            if((*sp)[conn].m_goodsPosition[i] == POS_EMPTY )
            {
                (*sp)[conn].m_goodsPosition[i] = POS_NONEMPTY;
                break;
            }

        }
        goods.Count = iter->Count;
        goods.GoodsID = iter->GoodsID;
        goods.Position = i;
        goods.TypeID = iter->TypeID;
        iter->Position = i;
        goods.Source = Source_Trade;
        vec.push_back(goods);
        opg->PushUpdateGoods(interchange->roleId,&goods,PostInsert);

        if(iter->GoodsID>=min_EquipMentId && iter->GoodsID <=max_EquipMentId)   //装备
        {
            STR_Equipment equAttr = (*((*sp)[partnerConn].m_playerEqu))[iter->GoodsID].equAttr;
            pInterchange->vecEqui.push_back(equAttr);
            STR_PlayerEqu equ;
            equ.goods = goods;
            equ.equAttr = equAttr;
            (*((*sp)[conn].m_playerEqu))[iter->GoodsID] = equ;
            (*sp)[partnerConn].m_playerEqu->erase(iter->GoodsID);
            opg->PushUpdateEquAttr(interchange->roleId,&equAttr,PostUpdate);
            continue;
        }

        _umap_roleGoods::iterator iterPartnerGoods = (*sp)[conn].m_playerGoods->find(iter->GoodsID);
        if(iterPartnerGoods != (*sp)[conn].m_playerGoods->end())
        {
            iterPartnerGoods->second.push_back(goods);
        }
        else
        {
            (*((*sp)[conn].m_playerGoods))[iter->GoodsID] = vec;
        }
    }

    //将本方交易的金钱添加到对方
    if(interchange->money.MoneyCount != 0)
    {
        _umap_roleMoney::iterator iterMoney = (*sp)[conn].m_playerMoney->find(interchange->money.MoneyType);
        iterMoney->second.Count -= interchange->money.MoneyCount;
        STR_PlayerMoney money;
        money.Count = iterMoney->second.Count;
        money.TypeID = interchange->money.MoneyType;
        opg->PushUpdateMoney(interchange->roleId,&money);

        if(iterMoney->second.Count == 0)
        {
            (*sp)[conn].m_playerMoney->erase(iterMoney);
        }
        _umap_roleMoney::iterator iterMoneyPartner = (*sp)[partnerConn].m_playerMoney->find(interchange->money.MoneyType);
        if(iterMoneyPartner != (*sp)[partnerConn].m_playerMoney->end())
        {
            iterMoneyPartner->second.Count += interchange->money.MoneyCount;
            money.Count = iterMoneyPartner->second.Count;
            opg->PushUpdateMoney(pInterchange->roleId,&money);
        }
        else
        {
            STR_PlayerMoney money;
            money.Count = interchange->money.MoneyCount ;
            money.TypeID = interchange->money.MoneyType;
            opg->PushUpdateMoney(pInterchange->roleId,&money);
            (*((*sp)[partnerConn].m_playerMoney))[money.TypeID] = money;
        }

    }

    //将对方要交易的金钱添加到本方
    if(pInterchange->money.MoneyCount != 0)
    {
        _umap_roleMoney::iterator iterMoney = (*sp)[partnerConn].m_playerMoney->find(pInterchange->money.MoneyType);
        iterMoney->second.Count -= pInterchange->money.MoneyCount;
        STR_PlayerMoney money;
        money.Count = iterMoney->second.Count;
        money.TypeID = pInterchange->money.MoneyType;
        opg->PushUpdateMoney(pInterchange->roleId,&money);

        if(iterMoney->second.Count == 0)
        {
            (*sp)[conn].m_playerMoney->erase(iterMoney);
        }
        _umap_roleMoney::iterator iterMoneyPartner = (*sp)[conn].m_playerMoney->find(pInterchange->money.MoneyType);
        if(iterMoneyPartner != (*sp)[partnerConn].m_playerMoney->end())
        {
            iterMoneyPartner->second.Count += pInterchange->money.MoneyCount;
            money.Count = iterMoneyPartner->second.Count;
            opg->PushUpdateMoney(interchange->roleId,&money);
        }
        else
        {
            STR_PlayerMoney money;
            money.Count = pInterchange->money.MoneyCount ;
            money.TypeID = pInterchange->money.MoneyType;
            opg->PushUpdateMoney(interchange->roleId,&money);
            (*((*sp)[conn].m_playerMoney))[money.TypeID] = money;
        }
    }
}

void GameInterchange::operReport(TCPConnection::Pointer conn)   //交易报告 交换双方物品后向双方发送交易的物品变动
{

    Server* srv = Server::GetInstance();
    SessionMgr::SessionPointer sp = SessionMgr::Instance()->GetSession();
    boost::shared_ptr<Interchange> interchange = (*sp)[conn].m_interchage;
    TCPConnection::Pointer partnerConn =  interchange->partnerConn;
    boost::shared_ptr<Interchange> pInterchange = (*sp)[partnerConn].m_interchage;

    STR_PackHead        head;
    char bufToConn[1024]={0};
    char bufToPartner[1024]={0};
    memset(&head,0,sizeof(STR_PackHead));

    //向双方发送本方交易物品（包括装备）的变动
    head.Len = sizeof(STR_Goods)*interchange->changes.size();
    if(head.Len != 0)
    {
        head.Flag = FLAG_BagGoods;
        memcpy(bufToPartner,&head,sizeof(head));
        for(int i = 0; i < interchange->changes.size(); ++i)
        {
            memcpy(bufToPartner+sizeof(STR_PackHead)+sizeof(STR_Goods)*i,&(interchange->changes[i]),sizeof(STR_Goods));
        }
        partnerConn->Write_all(bufToPartner, sizeof(STR_PackHead)+head.Len);  //向对方发送本方已经交易的物品
    }

    //向对方发送本方交易中装备的属性

    head.Len = interchange->vecEqui.size()*sizeof(STR_Equipment);
    if(head.Len != 0)       //如果交易中有装备
    {
        head.Flag = FLAG_EquGoodsAttr;
        memset(bufToPartner,0,1024);
        memcpy(bufToPartner,&head,sizeof(head));

        for(int i = 0; i < interchange->vecEqui.size(); ++i)
        {
            memcpy(bufToPartner+sizeof(STR_PackHead)+sizeof(STR_Equipment)*i,&(interchange->vecEqui[i]),sizeof(STR_Equipment));
        }
        partnerConn->Write_all(bufToPartner, sizeof(STR_PackHead)+head.Len);   //本方交易的物品是装备，要向对方发送属性信息
    }

    //向双方发送对方交易物品的情况
    head.Len = sizeof(STR_Goods)*pInterchange->changes.size();
    if(head.Len != 0)
    {
        head.Flag = FLAG_BagGoods;
        memset(bufToConn,0,1024);
        memcpy(bufToConn,&head,sizeof(head));
        for(int i = 0; i < pInterchange->changes.size(); ++i)
        {
            memcpy(bufToConn+sizeof(STR_PackHead)+sizeof(STR_Goods)*i,&(pInterchange->changes[i]),sizeof(STR_Goods));
        }
        conn->Write_all(bufToConn, sizeof(STR_PackHead)+head.Len);  //向本方发送对方已经交易的物品
    }

    //向本方发送对方交易中装备的属性
    head.Len = pInterchange->vecEqui.size()*sizeof(STR_Equipment);
    cout<<"                   head.len2    "<<head.Len<<endl;
    if(head.Len != 0)      //如果交易中有装备
    {
        memset(bufToConn,0,1024);
        head.Flag = FLAG_EquGoodsAttr;
        memcpy(bufToConn,&head,sizeof(head));
        for(int i = 0; i < pInterchange->vecEqui.size(); ++i)
        {
            memcpy(bufToConn+sizeof(STR_PackHead)+sizeof(STR_Equipment)*i,&(pInterchange->vecEqui[i]),sizeof(STR_Equipment));
        }
        conn->Write_all(bufToConn, sizeof(STR_PackHead)+head.Len);  //对方交易的如果是装备，则向本方发送装备属性信息
    }

    //发送双方交易的金钱类型在交易后的现状
    if(pInterchange->money.MoneyCount != 0)
    {
        char buf[1024] = {0};
        memset(&head,0,sizeof(STR_PackHead));
        head.Flag = FLAG_PlayerMoney;
        head.Len = sizeof(STR_PlayerMoney);
        memcpy(buf,&head,sizeof(head));

        _umap_roleMoney::iterator iterMoney = (*sp)[partnerConn].m_playerMoney->find(pInterchange->money.MoneyType);
        if (iterMoney != (*sp)[partnerConn].m_playerMoney->end())
        {
            memcpy(buf+sizeof(STR_PackHead),&(iterMoney->second),sizeof(STR_PlayerMoney));
        }
        else        //如果金钱中未找到这种类型，说明已经为0，发送数量为0的包
        {
            STR_PlayerMoney money;
            money.TypeID = pInterchange->money.MoneyType;
            money.Count = 0;
            memcpy(buf+sizeof(STR_PackHead),&money,sizeof(STR_PlayerMoney));
        }
        partnerConn->Write_all(buf, sizeof(STR_PackHead)+head.Len);   //向对方发送对方现在这种类型的金钱
        iterMoney = (*sp)[conn].m_playerMoney->find(pInterchange->money.MoneyType);
        {
            if (iterMoney != (*sp)[conn].m_playerMoney->end())
            {
                memset(buf+sizeof(STR_PackHead),0,sizeof(STR_PlayerMoney));
                memcpy(buf+sizeof(STR_PackHead),&(iterMoney->second),sizeof(STR_PlayerMoney));
                conn->Write_all(buf, sizeof(STR_PackHead)+head.Len);   //向本方发送本方现在这种类型的金钱
            }
        }
    }

    //发送双方交易中金钱类型在交易后的现状
    if(interchange->money.MoneyCount != 0)
    {
        char buf[1024] = {0};
        memset(&head,0,sizeof(STR_PackHead));
        head.Flag = FLAG_PlayerMoney;
        head.Len = sizeof(STR_PlayerMoney);
        memcpy(buf,&head,sizeof(head));

        _umap_roleMoney::iterator iterMoney = (*sp)[conn].m_playerMoney->find(interchange->money.MoneyType);
        if (iterMoney != (*sp)[conn].m_playerMoney->end())
        {
            memcpy(buf+sizeof(STR_PackHead),&(iterMoney->second),sizeof(STR_PlayerMoney));
        }
        else
        {
            STR_PlayerMoney money;
            money.TypeID = interchange->money.MoneyType;
            money.Count = 0;
            memcpy(buf+sizeof(STR_PackHead),&money,sizeof(STR_PlayerMoney));
        }
        conn->Write_all(buf, sizeof(STR_PackHead)+head.Len);  //向本方发送本方现在这种类型的金钱
        iterMoney = (*sp)[partnerConn].m_playerMoney->find(interchange->money.MoneyType);
        if (iterMoney != (*sp)[partnerConn].m_playerMoney->end())
        {
            memset(buf+sizeof(STR_PackHead),0,sizeof(STR_PlayerMoney));
            memcpy(buf+sizeof(STR_PackHead),&(iterMoney->second),sizeof(STR_PlayerMoney));
            partnerConn->Write_all(buf, sizeof(STR_PackHead)+head.Len); //向对方发送对方现在这种类型的金钱
        }
    }
}

void GameInterchange::operProCancelChange(TCPConnection::Pointer conn,interchangeOperPro*  oper)
{

    Server* srv = Server::GetInstance();
    SessionMgr::SessionPointer sp = SessionMgr::Instance()->GetSession();
    boost::shared_ptr<Interchange> interchange = (*sp)[conn].m_interchage;
    TCPConnection::Pointer partnerConn = interchange->partnerConn;

    auto iter = sp->find(partnerConn);

    if(iter == sp->end())    //对方离线
    {
        interchange->clear();           //清除交易状态
        srv->free(oper);
        return;
    }

    //对方在线
    boost::shared_ptr<Interchange> pInterchange = (*sp)[partnerConn].m_interchage;
    partnerConn->Write_all((char*)oper, sizeof(interchangeOperPro));   //转发取消交易的包

    for(auto iter = interchange->changes.begin(); iter != interchange->changes.end(); ++iter)    //背包位置信息恢复原来标志
    {
        (*sp)[conn].m_goodsPosition[iter->Position] = POS_NONEMPTY;
    }
    for(auto iter = pInterchange->changes.begin(); iter != pInterchange->changes.end(); ++iter)
    {
        (*sp)[partnerConn].m_goodsPosition[iter->Position] = POS_NONEMPTY;
    }
    interchange->clear();           //清除交易状态
    pInterchange->clear();             //对方清除交易状态
    srv->free(oper);
}

void GameInterchange::operProCancelRequest(TCPConnection::Pointer conn,interchangeOperPro*  oper)
{

    Server* srv = Server::GetInstance();
    SessionMgr::SessionPointer sp = SessionMgr::Instance()->GetSession();
    boost::shared_ptr<Interchange> interchange = (*sp)[conn].m_interchage;
    TCPConnection::Pointer partnerConn = interchange->partnerConn;

    auto iter = sp->find(partnerConn);

    if(iter == sp->end())    //对方离线
    {
        interchange->clear();           //清除交易状态
        srv->free(oper);
        return;
    }

    //对方在线
    boost::shared_ptr<Interchange> pInterchange = (*sp)[partnerConn].m_interchage;
    partnerConn->Write_all((char*)oper, sizeof(interchangeOperPro));   //转发取消交易的包

    interchange->clear();           //清除交易状态
    pInterchange->clear();             //对方清除交易状态
    srv->free(oper);
}
