#include "./../OperationPostgres/operationpostgres.h"
#include "./../PlayerLogin/playerlogin.h"
#include "./../memManage/diskdbmanager.h"
#include "./../Game/getdefinevalue.h"
#include "./../OperationGoods/operationgoods.h"
#include "./../utils/stringbuilder.hpp"
#include "./../Game/session.hpp"
#include "./../Game/log.h"
#include "gametask.h"
#include "./../server.h"


#define RESULT_SUCCESS         1      //成功
#define RESULT_PRE_TASK        2      //未完成前置任务
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
    ,m_exeDialogue( new umap_exeDialogue)
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
    delete m_exeDialogue;
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
    SessionMgr::SessionPointer smap = SessionMgr::Instance()->GetSession();
    hf_uint8 t_level = (*smap)[conn].m_roleExp.Level;

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
    if(t_level < task_it->second.Level)  //等级不符合
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
                umap_taskGoods taskGoods = (*smap)[conn].m_taskGoods;
                //将此任务加到物品任务中
                AddGoodsTask(taskGoods, aim_it->AimID, taskid);
                 //此数量从背包查得
                 hf_uint32 t_count = OperationGoods::GetThisGoodsCount(conn,aim_it->AimID);
                 if(t_count >= t_taskProcess.AimAmount)
                 {
                     t_taskProcess.FinishCount = t_taskProcess.AimAmount;
                 }
                 else
                 {
                     t_taskProcess.FinishCount = t_count;
                 }
            }
            else if(aim_it->ExeModeID == EXE_upgrade) //升级任务
            {
                if(t_level > t_taskProcess.AimAmount)
                {
                    t_taskProcess.FinishCount = t_taskProcess.AimAmount;
                }
                else
                {
                    t_taskProcess.FinishCount = t_level;
                }
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
    SessionMgr::SessionPointer smap = SessionMgr::Instance()->GetSession();
    umap_taskProcess playerAcceptTask = (*smap)[conn].m_playerAcceptTask;
    _umap_taskProcess::iterator it = playerAcceptTask->find(taskid);
    if(it == playerAcceptTask->end())
    {
        return;
    }

    umap_taskGoods taskGoods = (*smap)[conn].m_taskGoods;
    for(vector<STR_TaskProcess>::iterator process_it = it->second.begin(); process_it != it->second.end(); process_it++)
    {
        if(process_it->ExeModeID == EXE_collect_goods)
        {
            //从物品任务中删除该任务
            DeleteGoodsTask(taskGoods, process_it->AimID, taskid);
        }

        Server::GetInstance()->GetOperationPostgres()->PushUpdateTask((*smap)[conn].m_roleid, &(*process_it), PostDelete); //将任务从list中删除
    }
    playerAcceptTask->erase(taskid);
}

//请求完成任务
void GameTask::AskFinishTask(TCPConnection::Pointer conn, STR_FinishTask* finishTask)
{
    SessionMgr::SessionPointer smap = SessionMgr::Instance()->GetSession();
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
        {
            Server::GetInstance()->free(finishTask);
            return;
        }
    }

    vector<hf_uint32> reduceGoods;//用来保存任务完成时从背包删除的物品ID
    for(vector<STR_TaskProcess>::iterator process_it = it->second.begin(); process_it != it->second.end(); process_it++)
    {
        if(process_it->ExeModeID == EXE_collect_goods)
        {//在背包去掉目标数量的物品
            FinishCollectGoodsTask(conn, &(*process_it));
            reduceGoods.push_back(process_it->AimID);
        }
        if(!TaskFinishGoodsReward(conn, finishTask)) //物品奖励
        {
            Server::GetInstance()->free(finishTask);
            return;
        }
        TaskFinishTaskReward(conn, finishTask);  //任务奖励
        (*(*smap)[conn].m_completeTask)[finishTask->TaskID] = finishTask->TaskID;

        Server::GetInstance()->GetOperationPostgres()->PushUpdateTask((*smap)[conn].m_roleid, &(*process_it), PostDelete); //将任务从list中删除
        Server::GetInstance()->GetOperationPostgres()->PushUpdateCompleteTask((*smap)[conn].m_roleid, finishTask->TaskID);
        SendPlayerViewTask(conn);
    }

    for(vector<hf_uint32>::iterator reduce_it = reduceGoods.begin(); reduce_it != reduceGoods.end(); reduce_it++)
    {
        UpdateCollectGoodsTaskProcess(conn, *reduce_it);
    }
    (*smap)[conn].m_playerAcceptTask->erase(it);
    Server::GetInstance()->free(finishTask);
}

bool GameTask::TaskFinishGoodsReward(TCPConnection::Pointer conn, STR_FinishTask* finishTask)
{
    SessionMgr::SessionPointer smap = SessionMgr::Instance()->GetSession();
    umap_goodsReward::iterator goodsReward_it = m_goodsReward->find(finishTask->TaskID);
    if(goodsReward_it == m_goodsReward->end()) //没有物品奖励
    {
        return true;
    }
    hf_uint32 roleid = (*smap)[conn].m_roleid;
    hf_uint8   PosCount = 0;  //需要的空格子
    //判断能否放下
//    PosCount = goodsReward_it->second.size();
    for(vector<STR_GoodsReward>::iterator good_it = goodsReward_it->second.begin(); good_it != goodsReward_it->second.end(); good_it++)
    {
        if(EquTypeMinValue <= good_it->GoodsID && good_it->GoodsID <= EquTypeMaxValue) //装备
        {
            PosCount++;
        }
        else
        {
            PosCount = PosCount + good_it->Count/GOODSMAXCOUNT + 1;
        }
    }
    if(OperationGoods::JudgeEmptyPos(conn, PosCount) > 0) //空格子不够
    {
        STR_PackFinishTaskResult t_taskResult;
        t_taskResult.TaskID = finishTask->TaskID;
        t_taskResult.Result = FINISH_TASKFAIL;
        conn->Write_all(&t_taskResult, sizeof(STR_PackFinishTaskResult));
        return false;
    }

    hf_char* newGoodsBuff = (hf_char*)Server::GetInstance()->malloc();
    hf_char* equAttrBuff = (hf_char*)Server::GetInstance()->malloc();
    hf_uint8 goodsCount = 0;
    hf_uint8 equCount = 0;
    umap_roleGoods  playerGoods = (*smap)[conn].m_playerGoods;
    umap_roleEqu    playerEqu = (*smap)[conn].m_playerEqu;

    OperationPostgres* t_post = Server::GetInstance()->GetOperationPostgres();

    for(vector<STR_GoodsReward>::iterator good_it = goodsReward_it->second.begin(); good_it != goodsReward_it->second.end(); good_it++)
    {
        if(good_it->Type == DefaultGoods || finishTask->SelectGoodsID == good_it->GoodsID)
        {
            if(EquTypeMinValue <= good_it->GoodsID && good_it->GoodsID <= EquTypeMaxValue) //装备
            {
                STR_PlayerEqu t_equ;
                t_equ.goods.Count = 1;
                t_equ.goods.Position = OperationGoods::GetEmptyPos(conn);
                t_equ.goods.Source = Source_Task;
                t_equ.goods.GoodsID = OperationGoods::GetEquipmentID();
                t_equ.goods.TypeID = good_it->GoodsID; //可选装备奖励只有一件
                Server::GetInstance()->GetOperationGoods()->SetEquAttr(&t_equ.equAttr, t_equ.goods.TypeID);   //给新捡装备属性附初值
                t_equ.equAttr.EquID = t_equ.goods.GoodsID;

               (*playerEqu)[t_equ.goods.GoodsID] = t_equ;

                 memcpy(newGoodsBuff + sizeof(STR_PackHead) + goodsCount*sizeof(STR_Goods), &t_equ.goods, sizeof(STR_Goods));
                 goodsCount++;
                  t_post->PushUpdateGoods(roleid, &t_equ.goods, PostInsert); //将新买的物品添加到list
                 memcpy(equAttrBuff + sizeof(STR_PackHead) + equCount*sizeof(STR_EquipmentAttr), &t_equ.equAttr, sizeof(STR_EquipmentAttr));
                 equCount++;
                 t_post->PushUpdateEquAttr(roleid, &t_equ.equAttr, PostInsert); //将新买的物品添加到list
                 UpdateCollectGoodsTaskProcess(conn, t_equ.goods.TypeID);
            }
            else
            {
                STR_Goods t_goods;
                if(good_it->Type == DefaultGoods)
                {
                    t_goods.GoodsID = good_it->GoodsID;
                    t_goods.TypeID = good_it->GoodsID;
                }
                else
                {
                    t_goods.GoodsID = finishTask->SelectGoodsID;
                    t_goods.TypeID = finishTask->SelectGoodsID;
                }
                t_goods.Position = OperationGoods::GetEmptyPos(conn);
                t_goods.Source = Source_Task;
                for(hf_uint8 i = 0; i < good_it->Count/GOODSMAXCOUNT + 1; i++)
                {
                    if(good_it->Count - i*GOODSMAXCOUNT >= GOODSMAXCOUNT)
                        t_goods.Count = GOODSMAXCOUNT;
                    else
                        t_goods.Count = good_it->Count - i*GOODSMAXCOUNT;

                   memcpy(newGoodsBuff + sizeof(STR_PackHead) + goodsCount*sizeof(STR_Goods), &t_goods, sizeof(STR_Goods));
                   goodsCount++;
                   t_post->PushUpdateGoods(roleid, &t_goods, PostInsert); //将新买的物品添加到list

                   _umap_roleGoods::iterator playerGoods_it = playerGoods->find(t_goods.GoodsID);
                   if(playerGoods_it == playerGoods->end())
                   {
                       vector<STR_Goods> vec;
                       vec.push_back(t_goods);
                       (*playerGoods)[t_goods.GoodsID] = vec;
                   }
                   else
                   {
                       playerGoods_it->second.push_back(t_goods);
                   }
                   UpdateCollectGoodsTaskProcess(conn, t_goods.TypeID);
                }
            }
        }
    }
    STR_PackHead t_packHead;
    if(goodsCount)
    {
        t_packHead.Len = sizeof(STR_Goods) * goodsCount;
        t_packHead.Flag = FLAG_BagGoods;
        memcpy(newGoodsBuff, &t_packHead, sizeof(STR_PackHead));
        conn->Write_all(newGoodsBuff, sizeof(STR_PackHead) + t_packHead.Len);
    }
    if(equCount)
    {
        t_packHead.Len = sizeof(STR_EquipmentAttr) * equCount;
        t_packHead.Flag = FLAG_EquGoodsAttr;
        memcpy(equAttrBuff, &t_packHead, sizeof(STR_PackHead));
        conn->Write_all(equAttrBuff, sizeof(STR_PackHead) + t_packHead.Len);
    }
    Server::GetInstance()->free(newGoodsBuff);
    Server::GetInstance()->free(equAttrBuff);
    return true;
}

void GameTask::TaskFinishTaskReward(TCPConnection::Pointer conn, STR_FinishTask* finishTask)
{
    SessionMgr::SessionPointer smap = SessionMgr::Instance()->GetSession();
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

                hf_uint8 t_profession = (*smap)[conn].m_RoleBaseInfo.Profession;
                STR_RoleInfo* t_roleInfo = &(*smap)[conn].m_roleInfo;
                //更新玩家属性
                Server::GetInstance()->GetPlayerLogin()->UpdateJobAttr(t_profession, t_RoleExp->Level, t_roleInfo);
                UpdateAttackUpgradeTaskProcess(conn, t_RoleExp->Level);
                t_RoleExp->CurrentExp = t_RoleExp->CurrentExp + t_RewardExp.Experience - t_RoleExp->UpgradeExp;
                t_RoleExp->UpgradeExp = GetUpgradeExprience(t_RoleExp->Level);
            }
            else
            {
                t_RoleExp->CurrentExp = t_RoleExp->CurrentExp + t_RewardExp.Experience;
            }
            Server::GetInstance()->GetOperationPostgres()->PushUpdateExp((*smap)[conn].m_roleid, t_RoleExp->CurrentExp);
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
}

//完成收集物品任务
void GameTask::FinishCollectGoodsTask(TCPConnection::Pointer conn, STR_TaskProcess* taskProcess)
{
    SessionMgr::SessionPointer smap = SessionMgr::Instance()->GetSession();
    OperationPostgres* t_post = Server::GetInstance()->GetOperationPostgres();
    hf_char* buff = (hf_char*)Server::GetInstance()->malloc();

    hf_uint32 roleid = (*smap)[conn].m_roleid;
    STR_PackHead t_packHead;
    t_packHead.Flag = FLAG_TaskProcess;
    hf_uint32 i = 0;

    if(EquTypeMinValue <= taskProcess->AimID  && taskProcess->AimID <= EquTypeMaxValue) //装备
    {
        umap_roleEqu playerEqu = (*smap)[conn].m_playerEqu;
        for(_umap_roleEqu::iterator it = playerEqu->begin(); it != playerEqu->end(); it++)
        {
            if(it->second.goods.TypeID == taskProcess->AimID)
            {
                it->second.goods.Count = 0;
                memcpy(buff + sizeof(STR_PackHead) + i*sizeof(STR_Goods), &(it->second.goods), sizeof(STR_Goods));
                i++;
                t_post->PushUpdateGoods(roleid, &(it->second.goods), PostDelete);
                t_post->PushUpdateEquAttr(roleid, &(it->second.equAttr), PostDelete);
                _umap_roleEqu::iterator _it = it;
                playerEqu->erase(_it);
            }
            if(i == taskProcess->AimAmount)
            {
                break;
            }
        }
    }
    else  //其他物品
    {
        umap_roleGoods playerGoods = (*smap)[conn].m_playerGoods;
        _umap_roleGoods::iterator goods_it = playerGoods->find(taskProcess->AimID);
        if(goods_it != playerGoods->end())
        {
            hf_uint32 count = taskProcess->AimAmount;
            for(vector<STR_Goods>::iterator it = goods_it->second.begin(); it != goods_it->second.end(); it++)
            {
                if(it->Count > count)
                {
                    it->Count -= count;
                    memcpy(buff + sizeof(STR_PackHead) + i*sizeof(STR_Goods), &(*it), sizeof(STR_Goods));
                    i++;
                    t_post->PushUpdateGoods(roleid, &(*it), PostUpdate);
                    break;
                }
                else if(it->Count < count)
                {
                    it->Count = 0;
                    count -= it->Count;
                    memcpy(buff + sizeof(STR_PackHead) + i*sizeof(STR_Goods), &(*it), sizeof(STR_Goods));
                    i++;
                    t_post->PushUpdateGoods(roleid, &(*it), PostDelete);
                    vector<STR_Goods>::iterator _it = it;
                    goods_it->second.erase(_it);
                }
                else
                {
                    it->Count -= count;
                    memcpy(buff + sizeof(STR_PackHead) + i*sizeof(STR_Goods), &(*it), sizeof(STR_Goods));
                    i++;
                    t_post->PushUpdateGoods(roleid, &(*it), PostUpdate);
                    goods_it->second.erase(it);
                    break;
                }
            }
        }
        if(goods_it->second.size() == 0)
        {
            playerGoods->erase(goods_it);
        }
    }

    umap_taskGoods taskGoods = (*smap)[conn].m_taskGoods;
    _umap_taskGoods::iterator taskGoods_it = taskGoods->find(taskProcess->AimID);
    if(taskGoods_it != taskGoods->end())
    {
        for(vector<hf_uint32>::iterator taskID_it = taskGoods_it->second.begin(); taskID_it != taskGoods_it->second.end(); taskID_it++)
        {
            if(*taskID_it == taskProcess->TaskID)
            {
                taskGoods_it->second.erase(taskID_it);
                break;
            }
        }
        if(taskGoods_it->second.size() == 0)
        {
            taskGoods->erase(taskGoods_it);
        }
    }

    t_packHead.Len = sizeof(STR_TaskProcess)*i;
    memcpy(buff, &t_packHead, sizeof(STR_PackHead));
    conn->Write_all(buff, sizeof(STR_PackHead) + t_packHead.Len);
    Server::GetInstance()->free(buff);
}

//删除任务物品
void GameTask::DeleteTaskGoods(umap_taskGoods taskGoods, hf_uint32 GoodsID)
{
    _umap_taskGoods::iterator taskGoods_it = taskGoods->find(GoodsID);
    if(taskGoods_it != taskGoods->end())
    {
        taskGoods->erase(taskGoods_it);
    }
}

//增加任务物品
void GameTask::AddTaskGoods(umap_taskGoods taskGoods, hf_uint32 GoodsID)
{
    _umap_taskGoods::iterator taskGoods_it = taskGoods->find(GoodsID);
    if(taskGoods_it == taskGoods->end())
    {
        vector<hf_uint32> vec;
        vec.push_back(GoodsID);
        (*taskGoods)[GoodsID] = vec;
    }
    else
    {
        taskGoods_it->second.push_back(GoodsID);
    }
}

//删除物品任务
void GameTask::DeleteGoodsTask(umap_taskGoods taskGoods, hf_uint32 GoodsID, hf_uint32 taskID)
{
    _umap_taskGoods::iterator taskGoods_it = taskGoods->find(GoodsID);
    if(taskGoods_it != taskGoods->end())
    {
        for(vector<hf_uint32>::iterator iter = taskGoods_it->second.begin(); iter != taskGoods_it->second.end(); iter++)
        {
            if(*iter == taskID)
            {
                taskGoods_it->second.erase(iter);
                break;
            }
        }
        if(taskGoods_it->second.size() == 0)
        {
            taskGoods->erase(taskGoods_it);
        }
    }
}

//增加物品任务
void GameTask::AddGoodsTask(umap_taskGoods taskGoods, hf_uint32 GoodsID, hf_uint32 taskID)
{
    _umap_taskGoods::iterator taskGoods_it = taskGoods->find(GoodsID);
    if(taskGoods_it != taskGoods->end())
    {
       taskGoods_it->second.push_back(taskID);
    }
    else
    {
        vector<hf_uint32> t_vec;
        t_vec.push_back(taskID);
        (*taskGoods)[GoodsID] = t_vec;
    }
}
 //请求任务对话
void GameTask::StartTaskDlg(TCPConnection::Pointer conn, hf_uint32 taskid)
{
    umap_dialogue::iterator it = (*m_dialogue).find(taskid);
    if(it != (*m_dialogue).end())
    {
        hf_char* buff = (hf_char*)Server::GetInstance()->malloc();
        STR_TaskDlg t_dlg= (*m_dialogue)[taskid];
        STR_PackHead t_packHead;
        t_packHead.Len =  t_dlg.StartLen + sizeof(t_dlg.TaskID);
        t_packHead.Flag = FLAG_TaskStartDlg;
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
        STR_TaskDlg t_dlg= (*m_dialogue)[taskid];
        STR_PackHead t_packHead;
        t_packHead.Len = t_dlg.FinishLen + sizeof(t_dlg.TaskID) ;
        t_packHead.Flag = FLAG_TaskFinishDlg;
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
        hf_uint32 i = 0;
        for(vector<STR_TaskAim>::iterator iter = it->second.begin(); iter != it->second.end(); iter++)
        {
            memcpy(buff + sizeof(STR_PackHead) + i*sizeof(STR_TaskAim), &(*iter), sizeof(STR_TaskAim));
            i++;
        }
        STR_PackHead t_packHead;
        t_packHead.Flag = FLAG_TaskAim;
        t_packHead.Len = sizeof(STR_TaskAim)*i;
        memcpy(buff, &t_packHead, sizeof(STR_PackHead));
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
        memcpy(buff + sizeof(STR_PackHead), &(*m_taskReward)[taskid], sizeof(STR_TaskReward));
    }

    umap_goodsReward::iterator iter = (*m_goodsReward).find(taskid);  //奖励物品
    hf_uint8 i = 0;
    if(iter != m_goodsReward->end())
    {
        for(vector<STR_GoodsReward>::iterator itt = iter->second.begin(); itt != iter->second.end(); itt++)
        {
            memcpy(buff + sizeof(STR_PackHead) + sizeof(STR_TaskReward) + i*sizeof(STR_GoodsReward), &itt->GoodsID, sizeof(STR_GoodsReward));
            i++;
        }
    }
    t_packHead.Len = sizeof(STR_TaskReward) + i*sizeof(STR_GoodsReward);
    memcpy(buff, &t_packHead, sizeof(STR_PackHead));
    conn->Write_all(buff, sizeof(STR_PackHead) + t_packHead.Len);
    srv->free(buff);
}

void GameTask::AskTaskExeDialog(TCPConnection::Pointer conn, STR_AskTaskExeDlg* exeDlg)
{
    umap_exeDialogue::iterator it = (*m_exeDialogue).find(exeDlg->TaskID);
    if(it == m_exeDialogue->end())
    {
        Server::GetInstance()->free(exeDlg);
        return;
    }
    for(vector<STR_TaskExeDlg>::iterator iter = it->second.begin(); iter != it->second.end(); iter++)
    {
        if(exeDlg->AimID == iter->AimID)
        {
            hf_char* buff = (hf_char*)Server::GetInstance()->malloc();
            STR_PackHead t_packHead;
            t_packHead.Len =  iter->ExeLen + sizeof(iter->TaskID) + sizeof(iter->AimID);
            t_packHead.Flag = FLAG_TaskExeDlg;

            memcpy(buff, &t_packHead, sizeof(STR_PackHead));
            memcpy(buff + sizeof(STR_PackHead), &iter->TaskID, sizeof(iter->TaskID));
            memcpy(buff + sizeof(STR_PackHead) + sizeof(iter->TaskID), &iter->AimID, sizeof(iter->AimID));
            memcpy(buff + sizeof(STR_PackHead)  + sizeof(iter->TaskID) + sizeof(iter->AimID), iter->ExeDialogue, iter->ExeLen);

            cout << "taskid" << iter->TaskID << "aimid" << iter->AimID <<  "ExeDialogue:" << iter->ExeDialogue << endl;
            conn->Write_all(buff, sizeof(STR_PackHead) + t_packHead.Len);
            Server::GetInstance()->free(buff);
        }
    }
    Server::GetInstance()->free(exeDlg);
}

void GameTask::TaskExeDialogFinish(TCPConnection::Pointer conn, STR_AskTaskExeDlg* exeDlg)
{
    SessionMgr::SessionPointer smap = SessionMgr::Instance()->GetSession();
    umap_taskProcess t_task = (*smap)[conn].m_playerAcceptTask;
    _umap_taskProcess::iterator it = t_task->find(exeDlg->TaskID);
    if(it == t_task->end())
    {
        Server::GetInstance()->free(exeDlg);
        return;
    }
    for(vector<STR_TaskProcess>::iterator iter = it->second.begin(); iter != it->second.end(); iter++)
    {
        if(exeDlg->AimID == iter->AimID)
        {
            if(iter->FinishCount == 1)
            {
                Server::GetInstance()->free(exeDlg);
                return;
            }
            iter->FinishCount = 1;
            STR_PackTaskProcess t_taskProcess(&(*iter));
            conn->Write_all(&t_taskProcess, sizeof(STR_PackTaskProcess));
        }
    }
    Server::GetInstance()->free(exeDlg);
}

 //发送已接取的任务进度,和任务概述
void GameTask::SendPlayerTaskProcess(TCPConnection::Pointer conn)
{
    Server* srv = Server::GetInstance();
     //根据任务条件和玩家信息判断玩家可接取的任务
    SessionMgr::SessionPointer smap = SessionMgr::Instance()->GetSession();
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
        umap_taskGoods taskGoods = (*smap)[conn].m_taskGoods;
        hf_char* buff = (hf_char*)srv->malloc();
        hf_char* proBuff = (hf_char*)srv->malloc();
        hf_char* descBuff = (hf_char*)srv->malloc();
        hf_char* rewardBuff = (hf_char*)srv->malloc();
        hf_uint32 i = 0;
        hf_uint32 j = 0;
        hf_uint32 rewardLen = sizeof(STR_PackHead);
        STR_PackHead t_packHead;
        for(_umap_taskProcess::iterator it = playerAcceptTask->begin();it != playerAcceptTask->end(); it++)
        {
            //任务概述
            (*m_taskProfile)[it->first].Status = 2;
            memcpy(proBuff + sizeof(STR_PackHead) + i*sizeof(STR_TaskProfile), &(*m_taskProfile)[it->first], sizeof(STR_TaskProfile));

            //任务描述
            memset(descBuff, 0, CHUNK_SIZE);
            memcpy(descBuff, &(*m_taskDesc)[it->first], sizeof(STR_PackTaskDescription));
            conn->Write_all(descBuff,  sizeof(STR_PackTaskDescription));

            TaskReward(conn, it->first);

            for(vector<STR_TaskProcess>::iterator iter = it->second.begin(); iter != it->second.end(); iter++)
            {
                memcpy(buff + sizeof(STR_PackHead) + j*sizeof(STR_TaskProcess), &(*iter), sizeof(STR_TaskProcess));
                j++;
                if(iter->ExeModeID == EXE_collect_goods)
                {
                    _umap_taskGoods::iterator taskGoods_it = taskGoods->find(iter->AimID);
                    if(taskGoods_it == taskGoods->end())
                    {
                        vector<hf_uint32> vec;
                        vec.push_back(iter->TaskID);
                        (*taskGoods)[iter->AimID] = vec;
                    }
                    else
                    {
                        taskGoods_it->second.push_back(iter->TaskID);
                    }
                }
            }
            i++;
        }


        //发送玩家已经接取的任务的任务概述
        t_packHead.Flag = FLAG_TaskProfile;
        t_packHead.Len = sizeof(STR_TaskProfile)*i;
        memcpy(proBuff, &t_packHead, sizeof(STR_PackHead));
        conn->Write_all(proBuff, sizeof(STR_PackHead) + t_packHead.Len);


//        if(rewardLen != 0)
//        {
//            conn->Write_all(rewardBuff, rewardLen);
//        }

        t_packHead.Flag = FLAG_TaskProcess;
        t_packHead.Len = sizeof(STR_TaskProcess) * j;
        memcpy(buff, &t_packHead, sizeof(STR_PackHead));
        conn->Write_all(buff, sizeof(STR_PackHead) + t_packHead.Len);

        srv->free(buff);
        srv->free(proBuff);
        srv->free(descBuff);
        srv->free(rewardBuff);
    } 
}

//发送玩家可视范围内的任务
void GameTask::SendPlayerViewTask(TCPConnection::Pointer conn)
{
    //根据任务条件和玩家信息判断玩家可接取的任务
    SessionMgr::SessionPointer smap = SessionMgr::Instance()->GetSession();
    hf_uint8 t_level = (*smap)[conn].m_roleExp.Level; //得到玩家等级
    umap_taskProcess playerAcceptTask = (*smap)[conn].m_playerAcceptTask;
    umap_completeTask playerCompleteTask = (*smap)[conn].m_completeTask;

    hf_int32 size = 0;
    Server* srv = Server::GetInstance();
    hf_char* buff = (hf_char*)srv->malloc();
    STR_PackHead t_packHead;
    //发送玩家所在地图上的任务
    for(_umap_taskProfile::iterator it = m_taskProfile->begin(); it != m_taskProfile->end(); it++)
    {
        _umap_taskProcess::iterator iter = playerAcceptTask->find(it->first);
        //是否已经完成过了，暂时判断完成了就不再发送，以后根据任务是否可重复接取判断
        if(iter != playerAcceptTask->end()) //已接取
            continue;
        _umap_completeTask::iterator com_it = playerCompleteTask->find(it->first);
        if(com_it != playerCompleteTask->end())
            continue;



        STR_TaskPremise t_taskpremise = (*m_taskPremise)[it->first];
        if(t_level < t_taskpremise.Level)  //等级符合
        {
            continue;
        }
        if(t_taskpremise.PreTaskID != 0) //前置任务不为0
        {
            _umap_completeTask::iterator completeTask_it = playerCompleteTask->find(t_taskpremise.PreTaskID);
            if(completeTask_it == playerCompleteTask->end())
            {
                continue;
            }
        }

        memcpy(buff + sizeof(STR_PackHead) + size*sizeof(STR_TaskProfile), &(*m_taskProfile)[it->first], sizeof(STR_TaskProfile));
        size++;
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
void GameTask::UpdateCollectGoodsTaskProcess(TCPConnection::Pointer conn, hf_uint32 goodstype)
{
    SessionMgr::SessionPointer smap = SessionMgr::Instance()->GetSession();
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
            if(iter->AimID == goodstype && iter->ExeModeID == EXE_collect_goods)
            {
                if(iter->FinishCount > count)
                {
                    iter->FinishCount = count;
                    Server::GetInstance()->GetOperationPostgres()->PushUpdateTask((*smap)[conn].m_roleid, &(*iter), PostUpdate);
                    memcpy(buff + sizeof(STR_PackHead), &(*iter), sizeof(STR_TaskProcess));
                    t_packHead.Len += sizeof(STR_TaskProcess);
                }
                else
                {
                    if(iter->AimAmount <= count)
                    {
                        if(iter->FinishCount == iter->AimAmount)
                        {
                            continue;
                        }
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
    }
    memcpy(buff, &t_packHead, sizeof(STR_PackHead));
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


//查找此任务是否为任务进度里升级任务，如果是，更新任务进度
void GameTask::UpdateAttackUpgradeTaskProcess(TCPConnection::Pointer conn, hf_uint32 Level)
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
            if(iter->ExeModeID == EXE_upgrade && iter->AimAmount >= iter->AimAmount)
            {
                iter->FinishCount = Level;
                 Server::GetInstance()->GetOperationPostgres()->PushUpdateLevel((*smap)[conn].m_roleid, Level);
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

    //查询任务执行对话
    if(t_db->GetTaskExeDialogue(m_exeDialogue) < 0)
    {
        Logger::GetLogger()->Error("Query TaskExeDialogue error");
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




