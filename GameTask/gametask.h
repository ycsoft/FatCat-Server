#ifndef GAMETASK_H
#define GAMETASK_H

#include "Game/postgresqlstruct.h"
#include "NetWork/tcpconnection.h"
#include "Game/cmdtypes.h"

/**
 * @brief The GameTask class
 * 主要完成任务相关交互
 */
class GameTask
{
public:
    GameTask();
    ~GameTask();

     //请求任务
     void AskTask(TCPConnection::Pointer conn, hf_uint32 taskid);
     //放弃任务
     void QuitTask(TCPConnection::Pointer conn, hf_uint32 taskid);
     //请求完成任务
     void AskFinishTask(TCPConnection::Pointer conn, STR_FinishTask* finishTask);
     //请求任务对话
     void StartTaskDlg(TCPConnection::Pointer conn, hf_uint32 taskid);
     //请求任务结束对话
     void FinishTaskDlg(TCPConnection::Pointer conn, hf_uint32 taskid);
     //请求任务描述
     void TaskDescription(TCPConnection::Pointer conn, hf_uint32 taskid);
     //请求任务目标
     void TaskAim(TCPConnection::Pointer conn, hf_uint32 taskid);
     //请求任务奖励
     void TaskReward(TCPConnection::Pointer conn, hf_uint32 taskid);


    void FinishCollectGoodsTask(TCPConnection::Pointer conn, STR_TaskProcess* taskProcess);  //完成收集物品任务

    //
    bool TaskFinishGoodsReward(TCPConnection::Pointer conn, STR_FinishTask* finishTask);
    void TaskFinishTaskReward(TCPConnection::Pointer conn, STR_FinishTask* finishTask);


     //发送已接取的任务进度
     void SendPlayerTaskProcess(TCPConnection::Pointer conn);
     //发送玩家所在地图的任务
     void SendPlayerViewTask(TCPConnection::Pointer conn);

     //查找此任务是否为任务进度里收集物品，如果是，更新任务进度
     void UpdateCollectGoodsTaskProcess(TCPConnection::Pointer conn, hf_uint32 goodsType);

     //查找此任务是否为任务进度里打怪任务，如果是，更新任务进度
     void UpdateAttackMonsterTaskProcess(TCPConnection::Pointer conn, hf_uint32 monstertypeID);

     //将任务相关数据保存到boost::unordered_map结构中，键值为任务编号，值为该任务的数据包，客户端查询某任务数据包时用任务编号查询相关数据包发送给客户端
     void QueryTaskData();

     umap_dialogue*  Getdialogue()
     {
         return m_dialogue;
     }

     umap_taskDescription* GetDesc()
     {
         return m_taskDesc;
     }

     umap_taskAim* GetAim()
     {
         return m_taskAim;
     }

     umap_taskReward* GetReward()
     {
         return m_taskReward;
     }

     umap_goodsReward* GetgoodsReward()
     {
         return m_goodsReward;
     }

     umap_taskProfile GettaskProfile()
     {
         return m_taskProfile;
     }

     umap_taskPremise* GettaskPremise()
     {
         return m_taskPremise;
     }


 private:
     umap_dialogue*             m_dialogue;     //任务对话
     umap_taskDescription*      m_taskDesc;     //任务描述
     umap_taskAim*              m_taskAim;      //任务目标
     umap_taskReward*           m_taskReward;   //任务奖励
     umap_goodsReward*          m_goodsReward;  //物品奖励
     umap_taskProfile           m_taskProfile;  //保存任务概述
     umap_taskPremise*          m_taskPremise;  //任务要求

};

#endif // GAMETASK_H
