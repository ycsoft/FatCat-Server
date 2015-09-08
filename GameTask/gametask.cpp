#include "OperationPostgres/operationpostgres.h"
#include "memManage/diskdbmanager.h"
#include "Game/getdefinevalue.h"
#include "OperationGoods/operationgoods.h"
#include "utils/stringbuilder.hpp"
#include "Game/session.hpp"
#include "Game/log.hpp"
#include "gametask.h"
#include "server.h"


#define RESULT_SUCCESS         1      //成功
#define RESULT_PRE_TASK        2      //未接取条件任务
#define RESULT_CONDITION_TASK  3      //未接取条件任务
#define RESULT_TASK_GOODS      4      //未持有任务物品
#define RESULT_CONDITION_TITLE 5      //为获得任务条件称号
#define RESULT_CONDITION_COPY  6      //未完成条件副本
#define RESULT_SEX             7      //性别不符
#define RESULT_LEVEL           8      //等级不足
#define RESULT_PROFESSION      9      //职业不符


#define FINISH_TASKSUCCESS      1     //任务请求完成成功
#define FINISH_TASKFAIL         2     //任务请求完成失败
GameTask::GameTask()
    :m_dialogue(new umap_dialogue)
    ,m_taskDesc(new umap_taskDescription)
    ,m_taskAim(new umap_taskAim)
    ,m_taskReward(new umap_taskReward)
    ,m_goodsReward(new umap_goodsReward)
    ,m_taskProfile(new _umap_taskProfile)
    ,m_taskPremise(new umap_taskPremise)
{

}

GameTask::~GameTask()    
{
    delete m_dialogue;
    delete m_taskDesc;
    delete m_taskAim;
    delete m_taskReward;
    delete m_goodsReward;
    delete m_taskPremise;
}

//请求任务
void GameTask::AskTask(TCPConnection::Pointer conn, hf_uint32 taskid)
{
    //判断任务条件,暂时只判断等级
    SessionMgr::SessionMap* smap = SessionMgr::Instance()->GetSession().get();
    STR_RoleBasicInfo* t_RoleBaseInfo = &(*smap)[conn].m_RoleBaseInfo;

    umap_taskProcess playerAcceptTask = (*smap)[conn].m_playerAcceptTask;

    _umap_taskProcess::iterator it = playerAcceptTask->find(taskid);
    if(it != playerAcceptTask->end()) //已经接取当前任务
    {
        return;
    }

    //得到该任务的任务要求
    umap_taskPremise::iterator task_it = m_taskPremise->find(taskid);
    if(task_it == m_taskPremise->end()) //发送的任务编号错误
    {
        return;
    }
    STR_PackAskResult t_askResult;
    t_askResult.TaskID = taskid;
    //判断接取条件，暂时只判断等级
    if(t_RoleBaseInfo->Level < task_it->second.Level)  //等级不符合
    {
        t_askResult.Result = RESULT_LEVEL;
        conn->Write_all(&t_askResult, sizeof(STR_PackAskResult));
        return;
    }

    t_askResult.Result = RESULT_SUCCESS;   //请求任务成功
    conn->Write_all(&t_askResult, sizeof(STR_PackAskResult));

    STR_TaskProcess t_taskProcess;
    umap_taskAim::iterator iter= m_taskAim->find(taskid);
    if(iter != m_taskAim->end())
    {
        hf_char* buff = (hf_char*)Server::GetInstance()->malloc();
        t_taskProcess.TaskID = taskid;
        for(vector<STR_TaskAim>::iterator aim_it = iter->second.begin(); aim_it != iter->second.end(); aim_it++) //一个任务可能有多个任务目标
        {
            t_taskProcess.AimID = aim_it->AimID;
            t_taskProcess.AimAmount = aim_it->Amount;
            t_taskProcess.ExeModeID = aim_it->ExeModeID;
            if(aim_it->ExeModeID == EXE_collect_goods) //收集物品任务
            {
                //将此物品加到任务物品中
                 _umap_taskGoods::iterator taskGoods_it = (*smap)[conn].m_taskGoods->find(aim_it->AimID);
                 if(taskGoods_it != (*smap)[conn].m_taskGoods->end())
                 {
                    taskGoods_it->second.push_back(taskid);
                 }
                 else
                 {
                     vector<hf_uint32> t_vec;
                     t_vec.push_back(taskid);
                     (*(*smap)[conn].m_taskGoods)[aim_it->AimID] = t_vec;
                 }
                 //此数量从背包查得
                 t_taskProcess.FinishCount = OperationGoods::GetThisGoodsCount(conn,aim_it->AimID);
            }
            else if(aim_it->ExeModeID == EXE_upgrade) //升级任务
            {
                t_taskProcess.FinishCount = (*smap)[conn].m_RoleBaseInfo.Level;
            }
            else  //EXE_attack_monster
            {
                t_taskProcess.FinishCount = 0;
            }

            //现在为多目标第一个目标创建，第二个目标直接添加
             _umap_taskProcess::iterator process_it = playerAcceptTask->find(taskid);
             if(process_it != playerAcceptTask->end())
             {
                 process_it->second.push_back(t_taskProcess);
             }
             else
             {
                 vector<STR_TaskProcess> vec_process;
                 vec_process.push_back(t_taskProcess);
                 (*playerAcceptTask)[taskid] = vec_process;
             }
             Server::GetInstance()->GetOperationPostgres()->PushUpdateTask((*smap)[conn].m_roleid, &t_taskProcess, PostInsert); //将新任务添加到list
             memcpy(buff + sizeof(STR_PackHead), &t_taskProcess, sizeof(STR_TaskProcess));
        }
        STR_PackHead t_packHead;
        t_packHead.Flag = FLAG_TaskProcess;
        t_packHead.Len = sizeof(STR_TaskProcess) * iter->second.size();
        memcpy(buff, &t_packHead, sizeof(STR_PackHead));
        conn->Write_all(buff, sizeof(STR_PackHead) + t_packHead.Len);
        Server::GetInstance()->free(buff);
    }
}

 //放弃任务
void GameTask::QuitTask(TCPConnection::Pointer conn, hf_uint32 taskid)
{
     //将任务添加到该角色的任务列表里，退出时将未完成的任务写进数据库。
    SessionMgr::SessionMap* smap = SessionMgr::Instance()->GetSession().get();
    umap_taskProcess playerAcceptTask = (*smap)[conn].m_playerAcceptTask;
    _umap_taskProcess::iterator it = playerAcceptTask->find(taskid);
    if(it != playerAcceptTask->end())
    {
        umap_taskGoods t_taskGoods = (*smap)[conn].m_taskGoods;
        for(vector<STR_TaskProcess>::iterator process_it = it->second.begin(); process_it != it->second.end(); process_it++)
        {
            //如果要删除任务的任务的执行方式为收集物品，则删除任务时需要更新玩家任务物品
            if(process_it->ExeModeID == EXE_collect_goods)
            {
                _umap_taskGoods::iterator taskGoods_it = t_taskGoods->find(process_it->AimID);
                if(taskGoods_it != t_taskGoods->end())
                {
                    for(vector<hf_uint32>::iterator iter = taskGoods_it->second.begin(); iter != taskGoods_it->second.end(); )
                    {
                        if(*iter == taskid)
                        {
                            vector<hf_uint32>::iterator _iter = iter;
                            iter++;
                            taskGoods_it->second.erase(_iter);
                        }
                    }
                    if(taskGoods_it->second.size() == 0)
                    {
                        t_taskGoods->erase(taskGoods_it);
                    }
                }
            }

            Server::GetInstance()->GetOperationPostgres()->PushUpdateTask((*smap)[conn].m_roleid, &(*process_it), PostDelete); //将任务从list中删除
        }
        playerAcceptTask->erase(it);
    }
}

//请求完成任务
void GameTask::AskFinishTask(TCPConnection::Pointer conn, STR_FinishTask* finishTask)
{
    SessionMgr::SessionMap* smap = SessionMgr::Instance()->GetSession().get();
    umap_taskProcess playerAcceptTask = (*smap)[conn].m_playerAcceptTask;
    _umap_taskProcess::iterator it = playerAcceptTask->find(finishTask->TaskID);
    if(it == playerAcceptTask->end()) //没接取当前任务
    {
        Server::GetInstance()->free(finishTask);
        return;
    }

    for(vector<STR_TaskProcess>::iterator process_it = it->second.begin(); process_it != it->second.end(); process_it++)
    {
        if(process_it->AimAmount != process_it->FinishCount)
            return;
    }

    for(vector<STR_TaskProcess>::iterator process_it = it->second.begin(); process_it != it->second.end(); process_it++)
    {
        if(process_it->ExeModeID == EXE_collect_goods)
        {//在背包去掉目标数量的物品

        }
         Server::GetInstance()->GetOperationPostgres()->PushUpdateTask((*smap)[conn].m_roleid, &(*process_it), PostDelete); //将任务从list中删除
    }

    if(!TaskFinishGoodsReward(conn, finishTask)) //物品奖励
    {
        Server::GetInstance()->free(finishTask);
        return;
    }
    if(TaskFinishTaskReward(conn, finishTask))  //任务奖励
    {

        (*smap)[conn].m_playerAcceptTask->erase(it);
    }
    Server::GetInstance()->free(finishTask);
}

bool GameTask::TaskFinishGoodsReward(TCPConnection::Pointer conn, STR_FinishTask* finishTask)
{
//    SessionMgr::SessionMap* smap = SessionMgr::Instance()->GetSession().get();
//    umap_goodsReward::iterator goodsReward_it = m_goodsReward->find(finishTask->TaskID);
//    if(goodsReward_it == m_goodsReward->end()) //没有物品奖励
//    {
//        return true;
//    }
//    hf_uint32 roleid = (*smap)[conn].m_roleid;
//    hf_uint8   PosCount = 0;  //需要的空格子
//    //判断能否放下
////    PosCount = goodsReward_it->second.size();
//    for(vector<STR_PackGoodsReward>::iterator good_it = goodsReward_it->second.begin(); good_it != goodsReward_it->second.end(); good_it++)
//    {
//        if(EquTypeMinValue <= good_it->GoodsID && good_it->GoodsID <= EquTypeMaxValue) //装备
//        {
//            PosCount++;
//        }
//        else
//        {
//            PosCount = PosCount + good_it->Count/GOODSMAXCOUNT + 1;
//        }
//    }
//    if(OperationGoods::UseEmptyPos(conn, PosCount) > 0) //空格子不够
//    {
//        STR_PackFinishTaskResult t_taskResult;
//        t_taskResult.TaskID = finishTask->TaskID;
//        t_taskResult.Result = FINISH_TASKFAIL;
//        conn->Write_all(&t_taskResult, sizeof(STR_PackFinishTaskResult));
//        return false;
//    }

//    hf_char* newGoodsBuff = (hf_char*)Server::GetInstance()->malloc();
//    hf_char* equAttrBuff = (hf_char*)Server::GetInstance()->malloc();
//    hf_uint8 goodsCount = 0;
//    hf_uint8 equCount = 0;
//    umap_roleGoods  playerBagGoods = (*smap)[conn].m_playerGoods;
//    umap_roleEquAttr    playerEquAttr = (*smap)[conn].m_playerEquAttr;

//    OperationPostgres* t_post = Server::GetInstance()->GetOperationPostgres();

//    for(vector<STR_PackGoodsReward>::iterator good_it = goodsReward_it->second.begin(); good_it != goodsReward_it->second.end(); good_it++)
//    {
//        if(good_it->Type == DefaultGoods || finishTask->SelectGoodsID == good_it->GoodsID)
//        {
//            if(EquTypeMinValue <= good_it->GoodsID && good_it->GoodsID <= EquTypeMaxValue) //装备
//            {
//                STR_Goods t_goods;
//                t_goods.GoodsID = OperationGoods::GetEquipmentID();

//                if(good_it->Type == DefaultGoods)
//                {
//                    t_goods.TypeID = good_it->GoodsID;
//                }
//                else
//                {
//                    t_goods.TypeID = finishTask->SelectGoodsID;
//                }
//                t_goods.Count = 1;
//                t_goods.Position = OperationGoods::GetEmptyPos(conn);
//                t_goods.Source = Source_Task;

//                STR_Equipment t_equipment;

//                Server::GetInstance()->GetOperationGoods()->SetEquAttr(&t_equipment, t_goods.TypeID);   //给新捡装备属性附初值
//                t_equipment.EquID = t_goods.GoodsID;

//                vector<STR_Goods> t_vec;
//                t_vec.push_back(t_goods);
//               (*playerBagGoods)[t_goods.GoodsID] = t_vec;

//                (*playerEquAttr)[t_equipment.EquID] = t_equipment;
//                 memcpy(newGoodsBuff + sizeof(STR_PackHead) + goodsCount*sizeof(STR_Goods), &t_goods, sizeof(STR_Goods));
//                 goodsCount++;
//                  t_post->PushUpdateGoods(roleid, &t_goods, PostInsert); //将新买的物品添加到list
//                 memcpy(equAttrBuff + sizeof(STR_PackHead) + equCount*sizeof(STR_Equipment), &t_equipment, sizeof(STR_Equipment));
//                 equCount++;
//                 t_post->PushUpdateEquAttr(roleid, &t_equipment, PostInsert); //将新买的物品添加到list
//                 UpdateCollectGoodsTaskProcess(conn, t_goods.TypeID, t_goods.Count);
//            }
//            else
//            {
//                STR_Goods t_goods;
//                if(good_it->Type == DefaultGoods)
//                {
//                    t_goods.GoodsID = good_it->GoodsID;
//                    t_goods.TypeID = good_it->GoodsID;
//                }
//                else
//                {
//                    t_goods.GoodsID = finishTask->SelectGoodsID;
//                    t_goods.TypeID = finishTask->SelectGoodsID;
//                }
//                t_goods.Position = OperationGoods::GetEmptyPos(conn);
//                t_goods.Source = Source_Task;
//                for(hf_uint8 i = 0; i < good_it->Count/GOODSMAXCOUNT + 1; i++)
//                {
//                    if(good_it->Count - i*GOODSMAXCOUNT >= GOODSMAXCOUNT)
//                        t_goods.Count = GOODSMAXCOUNT;
//                    else
//                        t_goods.Count = good_it->Count - i*GOODSMAXCOUNT;

//                   memcpy(newGoodsBuff + sizeof(STR_PackHead) + goodsCount*sizeof(STR_Goods), &t_goods, sizeof(STR_Goods));
//                   goodsCount++;
//                   t_post->PushUpdateGoods(roleid, &t_goods, PostInsert); //将新买的物品添加到list
//                   UpdateCollectGoodsTaskProcess(conn, t_goods.TypeID, t_goods.Count);
//                }
//            }
//        }
//    }
//    STR_PackHead t_packHead;
//    if(goodsCount)
//    {
//        t_packHead.Len = sizeof(STR_Goods) * goodsCount;
//        t_packHead.Flag = FLAG_BagGoods;
//        memcpy(newGoodsBuff, &t_packHead, sizeof(STR_PackHead));
//        conn->Write_all(newGoodsBuff, sizeof(STR_PackHead) + t_packHead.Len);
//    }
//    if(equCount)
//    {
//        t_packHead.Len = sizeof(STR_Equipment) * equCount;
//        t_packHead.Flag = FLAG_EquGoodsAttr;
//        memcpy(equAttrBuff, &t_packHead, sizeof(STR_PackHead));
//        conn->Write_all(equAttrBuff, sizeof(STR_PackHead) + t_packHead.Len);
//    }
//    Server::GetInstance()->free(newGoodsBuff);
//    Server::GetInstance()->free(equAttrBuff);
//    return true;
}

bool GameTask::TaskFinishTaskReward(TCPConnection::Pointer conn, STR_FinishTask* finishTask)
{
    SessionMgr::SessionMap* smap = SessionMgr::Instance()->GetSession().get();
    umap_taskReward::iterator taskReward_it = m_taskReward->find(finishTask->TaskID);
    if(taskReward_it != m_taskReward->end())  //任务奖励
    {
        if(taskReward_it->second.Experience != 0) //经验
        {
            STR_PackRewardExperience t_RewardExp;
            t_RewardExp.ID = finishTask->TaskID;
            t_RewardExp.Experience = taskReward_it->second.Experience;
            conn->Write_all(&t_RewardExp, sizeof(STR_PackRewardExperience));

            STR_PackRoleExperience* t_RoleExp = &(*smap)[conn].m_roleExp;

            //玩家升级
            if(t_RoleExp->CurrentExp + t_RewardExp.Experience >= t_RoleExp->UpgradeExp)
            {
                t_RoleExp->Level += 1;
                Server::GetInstance()->GetOperationPostgres()->PushUpdateLevel((*smap)[conn].m_roleid, t_RoleExp->Level);
                t_RoleExp->CurrentExp = t_RoleExp->CurrentExp + t_RewardExp.Experience - t_RoleExp->UpgradeExp;
                t_RoleExp->UpgradeExp = GetUpgradeExprience(t_RoleExp->Level);
            }
            else
            {
                t_RoleExp->CurrentExp = t_RoleExp->CurrentExp + t_RewardExp.Experience;
            }
            Server::GetInstance()->GetOperationPostgres()->PushUpdateExp((*smap)[conn].m_roleid, t_RoleExp->UpgradeExp);
            conn->Write_all(t_RoleExp, sizeof(STR_PackRoleExperience));
        }
        if(taskReward_it->second.Money != 0) //金钱
        {
            STR_PlayerMoney* playerMoney = &(*((*smap)[conn].m_playerMoney))[Money_1];
            playerMoney->Count += taskReward_it->second.Money;
            Server::GetInstance()->GetOperationPostgres()->PushUpdateMoney((*smap)[conn].m_roleid, playerMoney);
            STR_PackPlayerMoney t_money(playerMoney);
            conn->Write_all(&t_money, sizeof(STR_PackPlayerMoney));
        }
    }
     STR_PackFinishTaskResult t_taskResult;
     t_taskResult.TaskID = finishTask->TaskID;
     t_taskResult.Result = FINISH_TASKSUCCESS;
     conn->Write_all(&t_taskResult, sizeof(STR_PackFinishTaskResult));
     return true;
}

//请求完成收集物品任务
bool GameTask::AskFinishCollectGoodsTask(TCPConnection::Pointer conn, /*STR_FinishTask* finishTask,*/ STR_TaskProcess* taskProcess)
{
//    SessionMgr::SessionMap* smap = SessionMgr::Instance()->GetSession().get();

//    umap_goodsReward::iterator goodsReward_it = m_goodsReward->find(taskProcess->TaskID);
//    if(goodsReward_it == m_goodsReward->end()) //没有物品奖励
//    {
//        return true;
//    }
//    hf_uint32 roleid = (*smap)[conn].m_roleid;
//    hf_uint8   PosCount = 0;  //需要的空格子
//    //判断能否放下
//    for(vector<STR_PackGoodsReward>::iterator good_it = goodsReward_it->second.begin(); good_it != goodsReward_it->second.end(); good_it++)
//    {
//        if(EquTypeMinValue <= good_it->GoodsID && good_it->GoodsID <= EquTypeMaxValue) //装备
//        {
//            PosCount++;
//        }
//        else
//        {
//            PosCount = PosCount + good_it->Count/GOODSMAXCOUNT + 1;
//        }
//    }
//    if(OperationGoods::UseEmptyPos(conn, PosCount) > 0) //空格子不够
//    {
//        STR_PackFinishTaskResult t_taskResult;
//        t_taskResult.TaskID = taskProcess->TaskID;
//        t_taskResult.Result = FINISH_TASKFAIL;
//        conn->Write_all(&t_taskResult, sizeof(STR_PackFinishTaskResult));
//        return false;
//    }

//    OperationPostgres* t_post = Server::GetInstance()->GetOperationPostgres();
//    umap_roleEquAttr playerEquAttr = (*smap)[conn].m_playerEquAttr;
//    umap_roleGoods playerGoods = (*smap)[conn].m_playerGoods;
//    if(EquTypeMinValue <= taskProcess->AimID && taskProcess->AimID <= EquTypeMaxValue)
//    {
//        for(_umap_roleGoods::iterator goods_it = playerGoods->begin(); goods_it != playerGoods->end(); goods_it++)
//        {
//            vector<STR_Goods>::iterator iter= goods_it->second.begin();
//            if(iter->TypeID == taskProcess->AimID)
//            {
//                taskProcess->AimAmount -= 1;
//                t_post->PushUpdateGoods(roleid, &(*iter), PostDelete); //将任务从list中删除
//                _umap_roleEquAttr::iterator equ_it = playerEquAttr->find(iter->GoodsID);
//                t_post->PushUpdateEquAttr(roleid, &equ_it->second, PostDelete);
//                if(taskProcess->AimAmount == 0) //完成
//                {
//                    break;
//                }
//            }
//        }
//    }
//    else
//    {
//        _umap_roleGoods::iterator goods_it = playerGoods->find(taskProcess->AimID);
//        for(vector<STR_Goods>::iterator iter = goods_it->second.begin(); iter != goods_it->second.end();)
//        {
//            if(iter->Count == taskProcess->AimAmount)
//            {

//            }
//            else if(iter->Count > taskProcess->AimAmount)
//            {

//            }
//            else
//            {

//            }
//        }
//    }
}

 //请求任务对话
void GameTask::StartTaskDlg(TCPConnection::Pointer conn, hf_uint32 taskid)
{
    umap_dialogue::iterator it = (*m_dialogue).find(taskid);
    if(it != (*m_dialogue).end())
    {
        hf_char* buff = (hf_char*)Server::GetInstance()->malloc();
        STR_PackTaskDlg t_dlg= (*m_dialogue)[taskid];
        STR_PackHead t_packHead;
        t_packHead.Len =  t_dlg.StartLen + sizeof(t_dlg.TaskID);
        t_packHead.Flag = FLAG_StartTaskDlg;
        memcpy(buff, &t_packHead, sizeof(STR_PackHead));
        memcpy(buff + sizeof(STR_PackHead), &t_dlg.TaskID, sizeof(t_dlg.TaskID));
        memcpy(buff + sizeof(STR_PackHead) + sizeof(t_dlg.TaskID), t_dlg.StartDialogue, t_dlg.StartLen);

        conn->Write_all(buff, sizeof(STR_PackHead) + t_packHead.Len);
        Server::GetInstance()->free(buff);
    }
}

//请求任务结束对话
void GameTask::FinishTaskDlg(TCPConnection::Pointer conn, hf_uint32 taskid)
{
    umap_dialogue::iterator it = (*m_dialogue).find(taskid);
    if(it != (*m_dialogue).end())
    {
        hf_char* buff = (hf_char*)Server::GetInstance()->malloc();
        STR_PackTaskDlg t_dlg= (*m_dialogue)[taskid];
        STR_PackHead t_packHead;
        t_packHead.Len = t_dlg.FinishLen + sizeof(t_dlg.TaskID) ;
        t_packHead.Flag = FLAG_FinishTaskDlg;
        memcpy(buff, &t_packHead, sizeof(STR_PackHead));
        memcpy(buff + sizeof(STR_PackHead), &t_dlg.TaskID, sizeof(t_dlg.TaskID));
        memcpy(buff + sizeof(STR_PackHead) + sizeof(t_dlg.TaskID), t_dlg.FinishDialogue, t_dlg.FinishLen);

        conn->Write_all(buff, sizeof(STR_PackHead) + t_packHead.Len);
        Server::GetInstance()->free(buff);
    }
}
 //请求任务描述
void GameTask::TaskDescription(TCPConnection::Pointer conn, hf_uint32 taskid)
{
    umap_taskDescription::iterator it = (*m_taskDesc).find(taskid);
    if(it != (*m_taskDesc).end())
    {
        STR_PackTaskDescription t_desc= (*m_taskDesc)[taskid];
        conn->Write_all(&t_desc, sizeof(STR_PackTaskDescription));
    }
}

 //请求任务目标
void GameTask::TaskAim(TCPConnection::Pointer conn, hf_uint32 taskid)
{
    umap_taskAim::iterator it = m_taskAim->find(taskid);
    if(it != m_taskAim->end())
    {
        hf_char* buff = (hf_char*)Server::GetInstance()->malloc();
        for(vector<STR_TaskAim>::iterator iter = it->second.begin(); iter != it->second.end(); iter++)
        {
            memcpy(buff + sizeof(STR_PackHead), &(*iter), sizeof(STR_TaskAim));
        }
        STR_PackHead t_packHead;
        t_packHead.Flag = FLAG_TaskAim;
        t_packHead.Len = it->second.size();
        conn->Write_all(buff, sizeof(STR_PackHead) + t_packHead.Len);
        Server::GetInstance()->free(buff);
    }
}

 //请求任务奖励
void GameTask::TaskReward(TCPConnection::Pointer conn, hf_uint32 taskid)
{
    Server* srv = Server::GetInstance();
    hf_char* buff = (hf_char*)srv->malloc();
    STR_PackHead        t_packHead;
    t_packHead.Flag = FLAG_TaskReward;
    umap_taskReward::iterator it = (*m_taskReward).find(taskid);  //其他奖励
    if(it != (*m_taskReward).end())
    {
        memcpy(buff + sizeof(STR_PackHead), &(*m_taskReward)[taskid], sizeof(STR_PackTaskReward));
    }

    umap_goodsReward::iterator iter = (*m_goodsReward).find(taskid);  //奖励物品
    hf_uint8 i = 0;
    if(iter != (*m_goodsReward).end())
    {
        for(vector<STR_PackGoodsReward>::iterator itt = iter->second.begin(); itt != iter->second.end(); itt++)
        {
            memcpy(buff + sizeof(STR_PackHead) + sizeof(STR_PackTaskReward) + i*sizeof(STR_PackGoodsReward), &itt->GoodsID, sizeof(STR_PackGoodsReward));
            i++;
        }
    }
    t_packHead.Len = sizeof(STR_PackTaskReward) + i*sizeof(STR_PackGoodsReward);
    memcpy(buff, &t_packHead, sizeof(STR_PackHead));
    conn->Write_all(buff, sizeof(STR_PackHead) + t_packHead.Len);
    srv->free(buff);
}

 //发送已接取的任务进度,和任务概述
void GameTask::SendPlayerTaskProcess(TCPConnection::Pointer conn)
{
    Server* srv = Server::GetInstance();
     //根据任务条件和玩家信息判断玩家可接取的任务
    SessionMgr::SessionMap* smap = SessionMgr::Instance()->GetSession().get();
    umap_taskProcess playerAcceptTask = ((*smap)[conn]).m_playerAcceptTask;
    //查询任务进度
    StringBuilder builder;
    builder << "select * from t_playertaskprocess where roleid = " << (*smap)[conn].m_roleid << ";";
    Logger::GetLogger()->Debug(builder.str());
    if ( srv->getDiskDB()->GetPlayerTaskProcess(playerAcceptTask, (const hf_char*)builder.str()) < 0 )
    {
        Logger::GetLogger()->Error("Query playerAcceptTask error");
        return;
    }

    //发送已接取的任务进度
    if(playerAcceptTask->size() > 0)
    {
        hf_char* buff = (hf_char*)srv->malloc();
        hf_char* probuff = (hf_char*)srv->malloc();
        hf_uint32 i = 0;
        hf_uint32 j = 0;
        for(_umap_taskProcess::iterator it = playerAcceptTask->begin();it != playerAcceptTask->end(); it++)
        {
            (*m_taskProfile)[it->first].Status = 2;
            memcpy(probuff + sizeof(STR_PackHead) + i*sizeof(STR_TaskProfile), &(*m_taskProfile)[it->first], sizeof(STR_TaskProfile));

            for(vector<STR_TaskProcess>::iterator iter = it->second.begin(); iter != it->second.end(); iter++)
            {
                memcpy(buff + sizeof(STR_PackHead) + j*sizeof(STR_TaskProcess), &(*iter), sizeof(STR_TaskProcess));
                j++;
            }
            i++;
        }

        STR_PackHead t_packHead;
        //发送玩家已经接取的任务的任务概述
        t_packHead.Flag = FLAG_TaskProfile;
        t_packHead.Len = sizeof(STR_TaskProfile) * i;
        memcpy(probuff, &t_packHead, sizeof(STR_PackHead));
        conn->Write_all(probuff, sizeof(STR_PackHead) + t_packHead.Len);

        t_packHead.Flag = FLAG_TaskProcess;
        t_packHead.Len = sizeof(STR_TaskProcess) * j;
        memcpy(buff, &t_packHead, sizeof(STR_PackHead));
        conn->Write_all(buff, sizeof(STR_PackHead) + t_packHead.Len);

        srv->free(buff);
        srv->free(probuff);
    } 
}

//发送玩家可视范围内的任务
void GameTask::SendPlayerViewTask(TCPConnection::Pointer conn)
{
    //根据任务条件和玩家信息判断玩家可接取的任务
    SessionMgr::SessionMap* smap = SessionMgr::Instance()->GetSession().get();
    STR_RoleBasicInfo* t_RoleBaseInfo = &(*smap)[conn].m_RoleBaseInfo; //得到玩家信息
    umap_taskProcess playerAcceptTask = ((*smap)[conn]).m_playerAcceptTask;

    hf_int32 size = 0;
    Server* srv = Server::GetInstance();
    hf_char* buff = (hf_char*)srv->malloc();
    STR_PackHead t_packHead;
    //发送玩家所在地图上的任务
    for(_umap_taskProfile::iterator it = m_taskProfile->begin(); it != m_taskProfile->end(); it++)
    {
        _umap_taskProcess::iterator iter = playerAcceptTask->find(it->first);
        if(iter == playerAcceptTask->end()) //是否已接取
        {
            STR_TaskPremise t_taskpremise = (*m_taskPremise)[it->first];
            if(t_RoleBaseInfo->Level >= t_taskpremise.Level)  //等级符合
            {
                it->second.Status = 1;
                memcpy(buff + sizeof(STR_PackHead) + size*sizeof(STR_TaskProfile), &(*m_taskProfile)[it->first], sizeof(STR_TaskProfile));
                size++;
            }
        }
        if(size == (CHUNK_SIZE - sizeof(STR_PackHead))/sizeof(STR_TaskProfile))
        {
            t_packHead.Flag = FLAG_TaskProfile;
            t_packHead.Len = sizeof(STR_TaskProfile) * size;

            memcpy(buff, (hf_char*)&t_packHead, sizeof(STR_PackHead));
            //发送可接取的任务
            conn->Write_all(buff, sizeof(STR_PackHead) + t_packHead.Len);
            size = 0;
        }
    }

    if(size != (CHUNK_SIZE - sizeof(STR_PackHead))/sizeof(STR_TaskProfile) && size != 0)
    {
        t_packHead.Flag = FLAG_TaskProfile;
        t_packHead.Len = sizeof(STR_TaskProfile) * size;

        memcpy(buff, (hf_char*)&t_packHead, sizeof(STR_PackHead));
        //发送可接取的任务
        conn->Write_all(buff, sizeof(STR_PackHead) + t_packHead.Len);
    }
    srv->free(buff);
}

//查找此任务是否为任务进度里收集物品，如果是，更新任务进度
void GameTask::UpdateCollectGoodsTaskProcess(TCPConnection::Pointer conn, hf_uint32 goodsID, hf_uint32 goodstype)
{
    SessionMgr::SessionMap* smap = SessionMgr::Instance()->GetSession().get();    
    umap_taskGoods taskGoods = (*smap)[conn].m_taskGoods;
    _umap_taskGoods::iterator it = taskGoods->find(goodstype);
    if(it == taskGoods->end())
    {
        return;
    }
    umap_taskProcess playerAcceptTask = ((*smap)[conn]).m_playerAcceptTask;
    hf_char* buff = (hf_char*)Server::GetInstance()->malloc();
    STR_PackHead t_packHead;
    t_packHead.Flag = FLAG_TaskProcess;
    t_packHead.Len = 0;

    for(vector<hf_uint32>::iterator task_it = it->second.begin(); task_it != it->second.end(); task_it++)
    {
        _umap_taskProcess::iterator process_it = playerAcceptTask->find(*task_it);
        if(process_it == playerAcceptTask->end())  //基本不会找不到，除非程序出错
        {
            continue;
        }
        hf_uint32 count = OperationGoods::GetThisGoodsCount(conn, goodstype);

        for(vector<STR_TaskProcess>::iterator iter = process_it->second.begin(); iter != process_it->second.end(); iter++)
        {
            if(iter->AimID == goodsID && iter->ExeModeID == EXE_collect_goods)
            {
                if(iter->FinishCount == iter->AimAmount && iter->FinishCount <= count)
                {
                    continue;
                }
                else if(iter->FinishCount > count)
                {
                    iter->FinishCount = iter->AimAmount;
                    Server::GetInstance()->GetOperationPostgres()->PushUpdateTask((*smap)[conn].m_roleid, &(*iter), PostUpdate);
                    memcpy(buff + sizeof(STR_PackHead), &(*iter), sizeof(STR_TaskProcess));
                    t_packHead.Len += sizeof(STR_TaskProcess);
                }
                else
                {
                    iter->FinishCount = count;
                     Server::GetInstance()->GetOperationPostgres()->PushUpdateTask((*smap)[conn].m_roleid, &(*iter), PostUpdate);
                    memcpy(buff + sizeof(STR_PackHead), &(*iter), sizeof(STR_TaskProcess));
                    t_packHead.Len += sizeof(STR_TaskProcess);
                }
            }
        }
    }
    conn->Write_all(buff, sizeof(STR_PackHead) + t_packHead.Len);
    Server::GetInstance()->free(buff);
}

//查找此任务是否为任务进度里打怪任务，如果是，更新任务进度
void GameTask::UpdateAttackMonsterTaskProcess(TCPConnection::Pointer conn, hf_uint32 monstertypeID)
{
    SessionMgr::SessionPointer smap =  SessionMgr::Instance()->GetSession();
    umap_taskProcess playerAcceptTask = ((*smap)[conn]).m_playerAcceptTask;
    hf_char* buff = (hf_char*)Server::GetInstance()->malloc();
    STR_PackHead t_packHead;
    t_packHead.Flag = FLAG_TaskProcess;
    t_packHead.Len = 0;
    for(_umap_taskProcess::iterator t_task = playerAcceptTask->begin(); t_task != playerAcceptTask->end(); t_task++)
    {
        for(vector<STR_TaskProcess>::iterator iter = t_task->second.begin(); iter != t_task->second.end(); iter++)
        {
            if(iter->AimID == monstertypeID && iter->ExeModeID == EXE_attack_monster && iter->FinishCount < iter->AimAmount)
            {
                if(iter->FinishCount == iter->AimAmount)
                {
                    break;
                }
                iter->FinishCount = iter->FinishCount + 1;
                 Server::GetInstance()->GetOperationPostgres()->PushUpdateTask((*smap)[conn].m_roleid, &(*iter), PostUpdate);
                memcpy(buff + sizeof(STR_PackHead), &(*iter), sizeof(STR_TaskProcess));
                t_packHead.Len += sizeof(STR_TaskProcess);
            }
        }
        memcpy(buff, &t_packHead, sizeof(STR_PackHead));
        conn->Write_all(buff, sizeof(STR_PackHead) + t_packHead.Len);
    }
    Server::GetInstance()->free(buff);
}

//将任务相关数据查虚保存到boost::unordered_map结构中，键值为任务编号，值为该任务的数据包，客户端查询某任务数据包时用任务编号查询相关数据包发送给客户端
void GameTask::QueryTaskData()
{
    DiskDBManager* t_db = Server::GetInstance()->getDiskDB();
    //查询任务对话
    if ( t_db->GetTaskDialogue(m_dialogue) < 0 )
    {
        Logger::GetLogger()->Error("Query TaskDialogue error");
        return;
    }

    //查询任务描述
    if ( t_db->GetTaskDescription(m_taskDesc) < 0 )
    {
        Logger::GetLogger()->Error("Query TaskDescription error");
        return;
    }

    //查询任务目标
    if ( t_db->GetTaskAim(m_taskAim) < 0 )
    {
        Logger::GetLogger()->Error("Query TaskAim error");
        return;
    }

    //查询任务奖励
    if ( t_db->GetTaskReward(m_taskReward) < 0 )
    {
        Logger::GetLogger()->Error("Query TaskReward error");
        return;
    }

    //查询物品奖励
    if ( t_db->GetGoodsReward(m_goodsReward) < 0 )
    {
        Logger::GetLogger()->Error("Query GoodsReward error");
        return;
    }

    //查询任务概述
    if ( t_db->GetTaskProfile(m_taskProfile) < 0 )
    {
        Logger::GetLogger()->Error("Query TaskProfile error");
        return;
    }

    //查询任务条件
    if( t_db->GetTaskPremise(m_taskPremise) < 0)
    {
        Logger::GetLogger()->Error("Query TaskPremise error");
    }
}




