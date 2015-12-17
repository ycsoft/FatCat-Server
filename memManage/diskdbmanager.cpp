#include "diskdbmanager.h"
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include <boost/thread/mutex.hpp>


#include "Game/cmdtypes.h"
#include "Game/log.h"
#include "utils/stringbuilder.hpp"
#include "Game/getdefinevalue.h"
#include "server.h"
#include "Monster/monster.h"

using namespace std;
//static boost::mutex     mtx;

DiskDBManager::DiskDBManager()
{

}
DiskDBManager::~DiskDBManager()
{

}

bool DiskDBManager::Connect(Configuration con)
{
//    PGconn *PQconnectdb(const char *conninfo);
    m_PGconn = PQsetdbLogin(con.ip, con.port, NULL, NULL,con.dbName, con.user, con.password);
    if(PQstatus(m_PGconn) != CONNECTION_OK)
    {
        printf("PQconnectdb error\n");
        return false;
    }
    else
        return true;
}

bool DiskDBManager::Disconnect()
{
    PQfinish(m_PGconn);
    return true;
}


//从数据库中得到查询的数据
void* DiskDBManager::Get(const char* str)
{
    return NULL;
}


//执行不返回数据的命令  update insert delete move
hf_int32 DiskDBManager::Set(const char *str,...)
{
    m_mtx.lock();
    PGresult* t_PGresult = PQexec(m_PGconn, str);
    m_mtx.unlock();
    ExecStatusType t_ExecStatusType = PQresultStatus(t_PGresult);
    if(t_ExecStatusType != PGRES_COMMAND_OK) //成功完成一个不返回数据的命令
    {
        printf("%d\n", t_ExecStatusType);
        printf("PQexec error\n");
        return -1;
    }
    else
    {
        return atoi(PQcmdTuples(t_PGresult));
    }
}


//执行返回数据的命令  select
hf_int32 DiskDBManager::GetSqlResult(const hf_char* str)
{
    m_mtx.lock();
    PGresult* t_PGresult = PQexec(m_PGconn, str);
    m_mtx.unlock();
    ExecStatusType t_status = PQresultStatus(t_PGresult);
    if(t_status != PGRES_TUPLES_OK)   //执行一个返回数据的操作
    {
        return -1;
    }
    else
    {
        return PQntuples(t_PGresult);
    }
}
//得到玩家的登录信息
hf_int32 DiskDBManager::GetPlayerUserId(STR_PlayerLoginUserId* user,const char *str)
{
    m_mtx.lock();
    PGresult* t_PGresult = PQexec(m_PGconn, str);
    m_mtx.unlock();

    ExecStatusType t_ExecStatusType = PQresultStatus((t_PGresult));

    StringBuilder sbd;
    sbd<<"Function GetPlayerUserID :" << str;

    Logger::GetLogger()->Debug(sbd.str());

    if(t_ExecStatusType != PGRES_TUPLES_OK) // PGRES_TUPLES_OK表示成功执行一个返回数据的查询查询
    {
        printf("PQexec error\n");
        return -1;
    }
    else
    {
        int t_row = PQntuples(t_PGresult);   //行数
        for(int i = 0; i < t_row; i++)
        {
            memcpy(user->userName, PQgetvalue(t_PGresult, i, 0), PQgetlength(t_PGresult, i, 0));
            memcpy(user->password, PQgetvalue(t_PGresult, i, 1), PQgetlength(t_PGresult, i, 1));
        }
        return t_row;
    }
}

hf_int32 DiskDBManager::GetPlayerRoleList(ResRoleList* RoleList, const char *str)
{
    m_mtx.lock();
    PGresult* t_PGresult = PQexec(m_PGconn, str);
    m_mtx.unlock();

    ExecStatusType t_ExecStatusType = PQresultStatus((t_PGresult));

    StringBuilder sbd;
//    sbd<<"Function GetPlayerRoleList :" << str;

//    Logger::GetLogger()->Debug(sbd.str());
    if(t_ExecStatusType != PGRES_TUPLES_OK) // PGRES_TUPLES_OK表示成功执行一个返回数据的查询查询
    {
        printf("PQexec error\n");
        return -1;
    }
    else
    {
        int t_row = PQntuples(t_PGresult);   //行数
        STR_RoleBasicInfo t_RoleInfo;

        //打包数据
        for(int i = 0; i < t_row; i++)
        {
            memset(&t_RoleInfo, 0, sizeof(t_RoleInfo));
            memcpy(t_RoleInfo.Nick, PQgetvalue(t_PGresult, i, 0), PQgetlength(t_PGresult, i, 0));
            t_RoleInfo.RoleID = atoi(PQgetvalue(t_PGresult, i, 1));
            t_RoleInfo.Profession = atoi(PQgetvalue(t_PGresult, i, 2));
            t_RoleInfo.Level = atoi(PQgetvalue(t_PGresult, i, 3));
            t_RoleInfo.Sex = atoi(PQgetvalue(t_PGresult, i, 4));
            t_RoleInfo.Figure = atoi(PQgetvalue(t_PGresult, i, 5));
            t_RoleInfo.FigureColor = atoi(PQgetvalue(t_PGresult, i, 6));
            t_RoleInfo.Face = atoi(PQgetvalue(t_PGresult, i, 7));
            t_RoleInfo.Eye = atoi(PQgetvalue(t_PGresult, i, 8));
            t_RoleInfo.Hair = atoi(PQgetvalue(t_PGresult, i, 9));
            t_RoleInfo.HairColor = atoi(PQgetvalue(t_PGresult, i, 10));
            t_RoleInfo.ModeID = atoi(PQgetvalue(t_PGresult, i, 11));
            t_RoleInfo.SkirtID = atoi(PQgetvalue(t_PGresult, i, 12));

            RoleList->m_Role.push_back(t_RoleInfo);
        }
        return t_row;
    }
}


hf_int32 DiskDBManager::GetPlayerRegisterRoleInfo(STR_RoleBasicInfo* t_RoleInfo, const hf_char* str)
{
    m_mtx.lock();
    PGresult* t_PGresult = PQexec(m_PGconn, str);
    m_mtx.unlock();

    ExecStatusType t_ExecStatusType = PQresultStatus((t_PGresult));

    if(t_ExecStatusType != PGRES_TUPLES_OK) // PGRES_TUPLES_OK表示成功执行一个返回数据的查询查询
    {
        printf("PQexec error\n");
        return -1;
    }
    else
    {
        int t_row = PQntuples(t_PGresult);   //行数
        //打包数据
        if(t_row == 1)
        {
            memcpy(t_RoleInfo->Nick, PQgetvalue(t_PGresult, 0, 0), PQgetlength(t_PGresult, 0, 0));
            t_RoleInfo->RoleID = atoi(PQgetvalue(t_PGresult, 0, 1));
            t_RoleInfo->Profession = atoi(PQgetvalue(t_PGresult, 0, 2));
            t_RoleInfo->Level = atoi(PQgetvalue(t_PGresult, 0, 3));
            t_RoleInfo->Sex = atoi(PQgetvalue(t_PGresult, 0, 4));
            t_RoleInfo->Figure = atoi(PQgetvalue(t_PGresult, 0, 5));
            t_RoleInfo->FigureColor = atoi(PQgetvalue(t_PGresult, 0, 6));
            t_RoleInfo->Face = atoi(PQgetvalue(t_PGresult, 0, 7));
            t_RoleInfo->Eye = atoi(PQgetvalue(t_PGresult, 0, 8));
            t_RoleInfo->Hair = atoi(PQgetvalue(t_PGresult, 0, 9));
            t_RoleInfo->HairColor = atoi(PQgetvalue(t_PGresult, 0, 10));
            t_RoleInfo->ModeID = atoi(PQgetvalue(t_PGresult, 0, 11));
            t_RoleInfo->SkirtID = atoi(PQgetvalue(t_PGresult, 0, 12));
        }
        return t_row;
    }
}

bool DiskDBManager::IsConnected()
{
    if ( PQstatus(m_PGconn) != CONNECTION_OK )
        return false;
    else
        return true;
}

hf_int32 DiskDBManager::GetPlayerInitPos(STR_PackPlayerPosition *pos, const char *sql)
{
        if ( ! IsConnected()) return -1;

        m_mtx.lock();
        PGresult* t_PGresult = PQexec(m_PGconn, sql);
        m_mtx.unlock();

        ExecStatusType t_ExecStatusType = PQresultStatus((t_PGresult));
        if(t_ExecStatusType != PGRES_TUPLES_OK) // PGRES_TUPLES_OK表示成功执行一个返回数据的查询查询
        {
            std::ostringstream  os;

            os<<"SQL:"<<sql<<" Execute error";

            Logger::GetLogger()->Error(os.str().c_str());
            return -1;
        }
        else
        {
             int t_row = PQntuples(t_PGresult);   //行数
         //打包数据
             if(t_row == 1)
             {
                 istringstream( PQgetvalue(t_PGresult,0,0) ) >> pos->Pos_x;
                 istringstream( PQgetvalue(t_PGresult,0,1) ) >> pos->Pos_y;
                 istringstream( PQgetvalue(t_PGresult,0,2) ) >> pos->Pos_z;
                 istringstream( PQgetvalue(t_PGresult,0,3) ) >> pos->Direct;
                 istringstream( PQgetvalue(t_PGresult,0,4) ) >> pos->MapID;
                 pos->ActID = atoi(PQgetvalue(t_PGresult, 0, 5));
//                 istringstream( PQgetvalue(t_PGresult,0,5) ) >> pos->ActID;
             }
             return t_row;
        }

}


//从数据库中查询怪物条目
hf_int32 DiskDBManager:: GetMonsterSpawns(umap_monsterSpawns* monsterSpawns)
{
    const hf_char* str = "select * from T_MonsterSpawns;";
    m_mtx.lock();
    PGresult* t_PGresult = PQexec(m_PGconn, str);
    m_mtx.unlock();
    ExecStatusType t_ExecStatusType = PQresultStatus(t_PGresult);
    if(t_ExecStatusType != PGRES_TUPLES_OK) //成功执行一个返回数据的查询查询
    {
        std::ostringstream  os;
        os<<"Function : GetMonsterSpawns SQL: '"<<str<<"'' Execute Error";
//        Logger::GetLogger()->Error(os.str());
        return -1;
    }
    else
    {
        int t_row = PQntuples(t_PGresult);   //行数
        //为结果分配内存
        STR_MonsterSpawns t_monsterSpawns;
        //打包数据
        for(int i = 0; i < t_row; i++)
        {
            t_monsterSpawns.MonsterTypeID = atoi(PQgetvalue(t_PGresult, i, 0));
            t_monsterSpawns.SpawnsPosID = atoi(PQgetvalue(t_PGresult, i,1));
            t_monsterSpawns.Pos_x = atof(PQgetvalue(t_PGresult, i,2));
            t_monsterSpawns.Pos_y = atof(PQgetvalue(t_PGresult, i, 3));
            t_monsterSpawns.Pos_z = atof(PQgetvalue(t_PGresult, i, 4));
            t_monsterSpawns.Boundary = atof(PQgetvalue(t_PGresult, i,5));
            t_monsterSpawns.MapID = atoi(PQgetvalue(t_PGresult, i, 6));
            t_monsterSpawns.Amount = atoi(PQgetvalue(t_PGresult, i, 7));

            (*monsterSpawns)[t_monsterSpawns.SpawnsPosID] = t_monsterSpawns;
        }
        return t_row;
    }
}

//从数据库中查询怪物属性
hf_int32 DiskDBManager::GetMonsterType(umap_monsterType* monsterType)
{
    const hf_char* str = "select * from T_MonsterType;";
    m_mtx.lock();
    PGresult* t_PGresult = PQexec(m_PGconn, str);
    m_mtx.unlock();
    ExecStatusType t_status = PQresultStatus(t_PGresult);
    if(t_status != PGRES_TUPLES_OK)
    {
        Logger::GetLogger()->Error("select MosnterType error");
        return -1;
    }
    else
    {
        int t_row = PQntuples(t_PGresult);
        STR_MonsterType t_monsterType;
//        打包数据
        for(int i = 0; i < t_row; i++)
        {
            memset(&t_monsterType, 0, sizeof(STR_MonsterType));
            t_monsterType.MonsterTypeID = atoi(PQgetvalue(t_PGresult, i, 0));
            memcpy(t_monsterType.MonsterName, PQgetvalue(t_PGresult, i, 1), PQgetlength(t_PGresult, i, 1));
            t_monsterType.HP = atoi(PQgetvalue(t_PGresult, i, 2));
            t_monsterType.PhysicalAttack = atoi(PQgetvalue(t_PGresult, i, 3));
            t_monsterType.MagicAttack = atoi(PQgetvalue(t_PGresult, i, 4));
            t_monsterType.PhysicalDefense = atoi(PQgetvalue(t_PGresult, i, 5));
            t_monsterType.MagicDefense = atoi(PQgetvalue(t_PGresult, i, 6));
            t_monsterType.Attackrate = atoi(PQgetvalue(t_PGresult, i, 7));
            t_monsterType.MoveRate = atoi(PQgetvalue(t_PGresult, i, 8));
            t_monsterType.Crit_Rate = atof(PQgetvalue(t_PGresult, i, 9));
            t_monsterType.Dodge_Rate = atof(PQgetvalue(t_PGresult, i, 10));
            t_monsterType.Hit_Rate = atof(PQgetvalue(t_PGresult, i, 11));
            t_monsterType.RankID = atoi(PQgetvalue(t_PGresult, i, 12));
            t_monsterType.Level = atoi(PQgetvalue(t_PGresult, i, 13));
            t_monsterType.AttackTypeID = atoi(PQgetvalue(t_PGresult, i, 14));
//            t_monsterType.AttackRange = atoi(PQgetvalue(t_PGresult, i, 15));

            (*monsterType)[t_monsterType.MonsterTypeID] = t_monsterType;
        }
        return t_row;
    }
}

hf_int32 DiskDBManager::GetTaskProfile(umap_taskProfile TaskProfile)
{
    const hf_char* str = "select * from t_taskprofile;";
    m_mtx.lock();
    PGresult* t_PGresult = PQexec(m_PGconn, str);
    m_mtx.unlock();
    ExecStatusType t_status = PQresultStatus(t_PGresult);
    if(t_status != PGRES_TUPLES_OK)
    {
        Logger::GetLogger()->Error("select t_taskprofile error");
        return -1;
    }
    else
    {
        int t_row = PQntuples(t_PGresult);
        STR_TaskProfile t_profile;
        for(int i = 0; i < t_row; i++)
        {
            memset(&t_profile, 0, sizeof(STR_TaskProfile));
            t_profile.TaskID = atoi(PQgetvalue(t_PGresult, i, 0));
            memcpy(t_profile.TaskName, PQgetvalue(t_PGresult, i, 1), PQgetlength(t_PGresult, i, 1));
            t_profile.StartNPCID = atoi(PQgetvalue(t_PGresult, i, 2));
            t_profile.FinishNPCID = atoi(PQgetvalue(t_PGresult, i, 3));
            t_profile.AcceptModeID = atoi(PQgetvalue(t_PGresult, i, 4));
            t_profile.FinishModeID = atoi(PQgetvalue(t_PGresult, i, 5));
            t_profile.Status = 1; //未接取
            (*TaskProfile)[t_profile.TaskID] = t_profile;
        }
        return t_row;
    }
}


hf_int32 DiskDBManager::GetTaskDialogue(umap_dialogue* TaskDialogue)
{
    const hf_char* str = "select * from t_taskdialogue;";
    m_mtx.lock();
    PGresult* t_PGresult = PQexec(m_PGconn, str);
    m_mtx.unlock();
    ExecStatusType t_status = PQresultStatus(t_PGresult);
    if(t_status != PGRES_TUPLES_OK)
    {
        Logger::GetLogger()->Error("select t_taskDialogue error");
        return -1;
    }
    else
    {
        int t_row = PQntuples(t_PGresult);
        STR_TaskDlg t_dialogue;
        for(int i = 0; i < t_row; i++)
        {
            memset(&t_dialogue, 0, sizeof(STR_TaskDlg));
            t_dialogue.TaskID = atoi(PQgetvalue(t_PGresult, i, 0));
            t_dialogue.StartLen = PQgetlength(t_PGresult, i, 1) + 1;
//            t_dialogue.ExeLen = PQgetlength(t_PGresult, i, 2) + 1;
            t_dialogue.FinishLen = PQgetlength(t_PGresult, i, 2) + 1;
            memcpy(t_dialogue.StartDialogue, PQgetvalue(t_PGresult, i,1), t_dialogue.StartLen - 1);
//            memcpy(t_dialogue.ExeDialogue, PQgetvalue(t_PGresult, i, 2), t_dialogue.ExeLen - 1);
            memcpy(t_dialogue.FinishDialogue, PQgetvalue(t_PGresult, i, 2), t_dialogue.FinishLen - 1);
            (*TaskDialogue).insert(make_pair(t_dialogue.TaskID,t_dialogue));

        }
        return t_row;
    }
}

hf_int32 DiskDBManager::GetTaskExeDialogue(umap_exeDialogue* TaskExeDialogue)
{
    const hf_char* str = "select * from t_taskexedialogue;";
    m_mtx.lock();
    PGresult* t_PGresult = PQexec(m_PGconn, str);
    m_mtx.unlock();
    ExecStatusType t_status = PQresultStatus(t_PGresult);
    if(t_status != PGRES_TUPLES_OK)
    {
        Logger::GetLogger()->Error("select t_taskDialogue error");
        return -1;
    }
    else
    {
        int t_row = PQntuples(t_PGresult);
        if(t_row == 0)
        {
            return 0;
        }
        STR_TaskExeDlg t_dialogue;
        for(int i = 0; i < t_row; i++)
        {
            memset(&t_dialogue, 0, sizeof(STR_TaskExeDlg));
            t_dialogue.TaskID = atoi(PQgetvalue(t_PGresult, i, 0));
            t_dialogue.AimID = atoi(PQgetvalue(t_PGresult, i, 1));
            t_dialogue.ExeLen = PQgetlength(t_PGresult, i, 2) + 1;
            memcpy(t_dialogue.ExeDialogue, PQgetvalue(t_PGresult, i, 2), t_dialogue.ExeLen - 1);
            umap_exeDialogue::iterator it = TaskExeDialogue->find(t_dialogue.TaskID);
            if(it == TaskExeDialogue->end())
            {
                vector<STR_TaskExeDlg> t_exeDlg;
                t_exeDlg.push_back(t_dialogue);
                (*TaskExeDialogue)[t_dialogue.TaskID] = t_exeDlg;
            }
            else
            {
                it->second.push_back(t_dialogue);
            }
        }
        return t_row;
    }
}

hf_int32 DiskDBManager::GetTaskDescription(umap_taskDescription* TaskDesc)
{
    const hf_char* str = "select * from t_taskdescription;";
    m_mtx.lock();
    PGresult* t_PGresult = PQexec(m_PGconn, str);
    m_mtx.unlock();
    ExecStatusType t_status = PQresultStatus(t_PGresult);
    if(t_status != PGRES_TUPLES_OK)
    {
        Logger::GetLogger()->Error("select t_taskdescrition error");
        return -1;
    }
    else
    {
        hf_int32 t_row = PQntuples(t_PGresult);
        STR_PackTaskDescription t_desc;
        for(int i = 0; i < t_row; i++)
        {
            memset(t_desc.Description, 0, sizeof(t_desc.Description));
            t_desc.TaskID = atoi(PQgetvalue(t_PGresult, i, 0));
            t_desc.TaskPropsID = atoi(PQgetvalue(t_PGresult, i, 1));
            t_desc.Time = atoi(PQgetvalue(t_PGresult, i, 2));
            memcpy(t_desc.Description, PQgetvalue(t_PGresult, i, 3), PQgetlength(t_PGresult, i, 3));
            (*TaskDesc).insert(make_pair(t_desc.TaskID,t_desc));
        }
        return t_row;
    }
}

hf_int32 DiskDBManager::GetTaskAim(umap_taskAim* TaskAim)
{
    const hf_char* str = "select * from t_taskaim;";
    m_mtx.lock();
    PGresult* t_PGresult = PQexec(m_PGconn, str);
    m_mtx.unlock();
    ExecStatusType t_status = PQresultStatus(t_PGresult);
    if(t_status != PGRES_TUPLES_OK)
    {
        Logger::GetLogger()->Error("select t_taskaim error");
        return -1;
    }
    else
    {
        hf_int32 t_row = PQntuples(t_PGresult);
        STR_TaskAim t_Aim;
        for(int i = 0; i < t_row; i++)
        {
            t_Aim.TaskID = atoi(PQgetvalue(t_PGresult, i, 0));
            t_Aim.AimID = atoi(PQgetvalue(t_PGresult, i, 1));
            t_Aim.Amount = atoi(PQgetvalue(t_PGresult, i, 2));
            t_Aim.ExeModeID = atoi(PQgetvalue(t_PGresult, i, 3));
            umap_taskAim::iterator it = TaskAim->find(t_Aim.TaskID);
            if(it == TaskAim->end())
            {
                vector<STR_TaskAim> vecAim;
                vecAim.push_back(t_Aim);
                (*TaskAim)[t_Aim.TaskID] = vecAim;
            }
            else
            {
                it->second.push_back(t_Aim);
            }
        }
        return t_row;
    }
}

hf_int32 DiskDBManager::GetTaskReward(umap_taskReward* TaskReward)
{
    const char* str = "select * from t_taskreward;";
    m_mtx.lock();
    PGresult* t_PGresult = PQexec(m_PGconn, str);
    m_mtx.unlock();
    ExecStatusType t_status = PQresultStatus(t_PGresult);
    if(t_status != PGRES_TUPLES_OK)
    {
        Logger::GetLogger()->Error("select t_taskaim error");
        return -1;
    }
    else
    {
        hf_int32 t_row = PQntuples(t_PGresult);
        STR_TaskReward t_reward;
        for(int i = 0; i < t_row; i++)
        {
            t_reward.TaskID = atoi(PQgetvalue(t_PGresult, i, 0));
            t_reward.Experience = atoi(PQgetvalue(t_PGresult, i, 1));
            t_reward.Money = atoi(PQgetvalue(t_PGresult, i, 2));
            t_reward.SkillID = atoi(PQgetvalue(t_PGresult, i, 3));
            t_reward.TitleID = atoi(PQgetvalue(t_PGresult, i, 4));
            t_reward.Attribute = atoi(PQgetvalue(t_PGresult, i, 5));
            (*TaskReward).insert(make_pair(t_reward.TaskID,t_reward));
        }
        return t_row;
    }
}

hf_int32 DiskDBManager::GetGoodsReward(umap_goodsReward* GoodsReward)
{
    const char* str = "select * from t_GoodsReward;";
    m_mtx.lock();
    PGresult* t_PGresult = PQexec(m_PGconn, str);
    m_mtx.unlock();
    ExecStatusType t_status = PQresultStatus(t_PGresult);
    if(t_status != PGRES_TUPLES_OK)
    {
        Logger::GetLogger()->Error("select t_GoodsReward error");
        return -1;
    }
    else
    {
        hf_int32 t_row = PQntuples(t_PGresult);
        STR_GoodsReward t_good;
        vector<STR_GoodsReward> v_goodReward;
        for(int i = 0; i < t_row; i++)
        {
            v_goodReward.clear();

            hf_uint32 TaskID = atoi(PQgetvalue(t_PGresult, i, 0));

            t_good.GoodsID = atoi(PQgetvalue(t_PGresult, i, 1));
            t_good.Count = atoi(PQgetvalue(t_PGresult, i, 2));
            t_good.Type = atoi(PQgetvalue(t_PGresult, i, 3));
            v_goodReward.push_back(t_good);

            umap_goodsReward::iterator it = GoodsReward->find(TaskID);
            if(it != GoodsReward->end())
            {
                it->second.push_back(t_good);
            }
            else
            {
                (*GoodsReward)[TaskID] = v_goodReward;
            }
        }
         return t_row;
    }
}

//查询任务条件
hf_int32 DiskDBManager::GetTaskPremise(umap_taskPremise* t_TaskPremise)
{
    const char* str = "select * from t_taskPremise;";
    m_mtx.lock();
    PGresult* t_PGresult = PQexec(m_PGconn, str);
    m_mtx.unlock();
    ExecStatusType t_status = PQresultStatus(t_PGresult);
    if(t_status != PGRES_TUPLES_OK)
    {
        return -1;
    }
    else
    {
        hf_int32 t_row = PQntuples(t_PGresult);
        STR_TaskPremise t_taskPremise;
        for(int i = 0; i < t_row; i++)
        {
            t_taskPremise.TaskID = atoi(PQgetvalue(t_PGresult, i, 0));
            t_taskPremise.PreTaskID = atoi(PQgetvalue(t_PGresult, i, 1));
            t_taskPremise.CrmtTaskID = atoi(PQgetvalue(t_PGresult, i, 2));
            t_taskPremise.GoodsID = atoi(PQgetvalue(t_PGresult, i, 3));
            t_taskPremise.TitleID = atoi(PQgetvalue(t_PGresult, i, 4));
            t_taskPremise.DungeonID = atoi(PQgetvalue(t_PGresult, i, 5));
            t_taskPremise.GenderID = atoi(PQgetvalue(t_PGresult, i, 6));
            t_taskPremise.Level = atoi(PQgetvalue(t_PGresult, i, 7));
            t_taskPremise.JobID = atoi(PQgetvalue(t_PGresult, i, 8));

            (*t_TaskPremise).insert(make_pair(t_taskPremise.TaskID, t_taskPremise));
        }
        return t_row;
    }
}

//查询任务进度
hf_int32 DiskDBManager::GetPlayerTaskProcess(umap_taskProcess TaskProcess, const hf_char* str)
{
    m_mtx.lock();
    PGresult* t_PGresult = PQexec(m_PGconn, str);
    m_mtx.unlock();
    ExecStatusType t_status = PQresultStatus(t_PGresult);
    if(t_status != PGRES_TUPLES_OK)
    {
        return -1;
    }
    else
    {
        hf_int32 t_row = PQntuples(t_PGresult);
        STR_TaskProcess t_taskProcess;
        for(int i = 0; i < t_row; i++)
        {
            memset(&t_taskProcess, 0, sizeof(STR_TaskProcess));
            t_taskProcess.TaskID = atoi(PQgetvalue(t_PGresult, i, 1));
            t_taskProcess.AimID = atoi(PQgetvalue(t_PGresult, i, 2));
            t_taskProcess.FinishCount = atoi(PQgetvalue(t_PGresult, i, 3));
            t_taskProcess.AimAmount = atoi(PQgetvalue(t_PGresult, i, 4));
            t_taskProcess.ExeModeID = atoi(PQgetvalue(t_PGresult, i, 5));

            _umap_taskProcess::iterator it = TaskProcess->find(t_taskProcess.TaskID);
            if(it != TaskProcess->end())
            {
                it->second.push_back(t_taskProcess);
            }
            else
            {
                vector<STR_TaskProcess> vec_process;
                vec_process.push_back(t_taskProcess);
                (*TaskProcess)[t_taskProcess.TaskID] = vec_process;
            }
        }
        return t_row;
    }
}

//查询角色信息
hf_int32 DiskDBManager::GetRoleInfo(STR_RoleInfo* roleinfo, const hf_char* str)
{
    m_mtx.lock();
    PGresult* t_PGresult = PQexec(m_PGconn, str);
    m_mtx.unlock();
    ExecStatusType t_status = PQresultStatus(t_PGresult);
    if(t_status != PGRES_TUPLES_OK)
    {
        return -1;
    }
    else
    {
        hf_int32 t_row = PQntuples(t_PGresult);
        if(t_row == 1)
        {
            roleinfo->MaxHP = atoi(PQgetvalue(t_PGresult, 0, 1));
            roleinfo->HP = atoi(PQgetvalue(t_PGresult, 0, 2));
            roleinfo->MaxMagic = atoi(PQgetvalue(t_PGresult, 0, 3));
            roleinfo->Magic = atoi(PQgetvalue(t_PGresult, 0, 4));

            roleinfo->PhysicalDefense = atoi(PQgetvalue(t_PGresult, 0, 5));
            roleinfo->MagicDefense = atoi(PQgetvalue(t_PGresult, 0, 6));
            roleinfo->PhysicalAttack = atoi(PQgetvalue(t_PGresult, 0, 7));
            roleinfo->MagicAttack = atoi(PQgetvalue(t_PGresult, 0, 8));

            roleinfo->Crit_Rate = atof(PQgetvalue(t_PGresult, 0, 9));
            roleinfo->Dodge_Rate = atof(PQgetvalue(t_PGresult, 0, 10));
            roleinfo->Hit_Rate = atof(PQgetvalue(t_PGresult, 0, 11));
            roleinfo->Resist_Rate = atof(PQgetvalue(t_PGresult, 0, 12));
            roleinfo->Caster_Speed = atoi(PQgetvalue(t_PGresult, 0, 13));
            roleinfo->Move_Speed = atoi(PQgetvalue(t_PGresult, 0, 14));
            roleinfo->Hurt_Speed = atoi(PQgetvalue(t_PGresult, 0, 15));
            roleinfo->Small_Universe = atoi(PQgetvalue(t_PGresult, 0, 16));
            roleinfo->maxSmall_Universe = atoi(PQgetvalue(t_PGresult, 0, 17));
            roleinfo->RecoveryLife_Percentage = atof(PQgetvalue(t_PGresult, 0, 18));
            roleinfo->RecoveryLife_value = atoi(PQgetvalue(t_PGresult, 0, 19));
            roleinfo->RecoveryMagic_Percentage = atof(PQgetvalue(t_PGresult, 0, 20));
            roleinfo->RecoveryMagic_value = atoi(PQgetvalue(t_PGresult, 0, 21));
            roleinfo->MagicHurt_Reduction = atof(PQgetvalue(t_PGresult, 0, 22));
            roleinfo->PhysicalHurt_Reduction = atof(PQgetvalue(t_PGresult, 0, 23));
            roleinfo->CritHurt = atof(PQgetvalue(t_PGresult, 0, 24));
            roleinfo->CritHurt_Reduction = atof(PQgetvalue(t_PGresult, 0, 25));
            roleinfo->Magic_Pass = atof(PQgetvalue(t_PGresult, 0, 26));
            roleinfo->Physical_Pass = atof(PQgetvalue(t_PGresult, 0, 27));
            roleinfo->Rigorous = atoi(PQgetvalue(t_PGresult, 0, 28));
            roleinfo->Will = atoi(PQgetvalue(t_PGresult, 0, 29));
            roleinfo->Wise = atoi(PQgetvalue(t_PGresult, 0, 30));
            roleinfo->Mentality = atoi(PQgetvalue(t_PGresult, 0, 21));
            roleinfo->Physical_fitness = atoi(PQgetvalue(t_PGresult, 0, 32));
        }
        return t_row;
    }
}

//查询玩家经验
hf_int32 DiskDBManager::GetRoleExperience(STR_PackRoleExperience* RoleExp, const hf_char* str)
{
    m_mtx.lock();
    PGresult* t_PGresult = PQexec(m_PGconn, str);
    m_mtx.unlock();
    ExecStatusType t_status = PQresultStatus(t_PGresult);
    if(t_status != PGRES_TUPLES_OK)
    {
        return -1;
    }
    else
    {
        hf_int32 t_row = PQntuples(t_PGresult);
        if(t_row == 1)
        {
            RoleExp->Level = atoi(PQgetvalue(t_PGresult, 0, 0));
            RoleExp->CurrentExp = atoi(PQgetvalue(t_PGresult, 0, 1));
            RoleExp->UpgradeExp = GetUpgradeExprience(RoleExp->Level);           
        }
        return t_row;
    }
}


//查询好友列表
hf_int32 DiskDBManager::GetFriendList(umap_friendList t_friendList, const hf_char* str)
{
    m_mtx.lock();
    PGresult* t_PGresult = PQexec(m_PGconn, str);
    m_mtx.unlock();
    ExecStatusType t_status = PQresultStatus(t_PGresult);
    if(t_status != PGRES_TUPLES_OK)
    {
        return -1;
    }
    else
    {
        hf_int32 t_row = PQntuples(t_PGresult);
        STR_FriendInfo t_friendInfo;
        for(hf_int32 i = 0; i < t_row; i++)
        {
            memset(&t_friendInfo, 0, sizeof(STR_FriendInfo));
            t_friendInfo.RoleID = atoi(PQgetvalue(t_PGresult, i, 0));
            memcpy(t_friendInfo.Nick, PQgetvalue(t_PGresult, i, 1), PQgetlength(t_PGresult, i, 1));
            t_friendInfo.Status = 2; //默认不在线
            (*t_friendList)[t_friendInfo.RoleID] = t_friendInfo;
        }
        return t_row;
    }
}

//查询某个昵称的roleid
hf_int32 DiskDBManager::GetNickRoleid(hf_uint32* roleid, const hf_char* str)
{
    m_mtx.lock();
    PGresult* t_PGresult = PQexec(m_PGconn, str);
    m_mtx.unlock();
    ExecStatusType t_status = PQresultStatus(t_PGresult);
    if(t_status != PGRES_TUPLES_OK)
    {
        return -1;
    }
    else
    {
        hf_int32 t_row = PQntuples(t_PGresult);
        if(t_row == 1)
        {
            *roleid = atoi(PQgetvalue(t_PGresult, 0, 0));
        }
        return t_row;
    }
}

//查询添加好友请求
hf_int32 DiskDBManager::GetAskAddFriend(vector<STR_AddFriend>& addFriend, const hf_char* str)
{
    m_mtx.lock();
    PGresult* t_PGresult = PQexec(m_PGconn, str);
    m_mtx.unlock();
    ExecStatusType t_status = PQresultStatus(t_PGresult);
    if(t_status != PGRES_TUPLES_OK)
    {
        return -1;
    }
    else
    {
        hf_int32 t_row = PQntuples(t_PGresult);
        STR_AddFriend t_addFriend;
        for(hf_int32 i = 0; i < t_row; i++)
        {
            memset(&t_addFriend, 0, sizeof(STR_AddFriend));
            t_addFriend.RoleID = atoi(PQgetvalue(t_PGresult, i, 0));
            memcpy(t_addFriend.Nick,PQgetvalue(t_PGresult, i, 1), PQgetlength(t_PGresult, i, 1));
            addFriend.push_back(t_addFriend);
        }
        return t_row;
    }
}


//查询NPC信息
hf_int32 DiskDBManager::GetNPCInfo(umap_npcInfo* npcInfo)
{
    const hf_char* str = "select * from t_npcinfo;";
    m_mtx.lock();
    PGresult* t_PGresult = PQexec(m_PGconn, str);
    m_mtx.unlock();
    ExecStatusType t_status = PQresultStatus(t_PGresult);
    if(t_status != PGRES_TUPLES_OK)
    {
        return -1;
    }
    else
    {
        hf_int32 t_row = PQntuples(t_PGresult);
        NPCInfo t_NPCInfo;
        for(hf_int32 i = 0; i < t_row; i++)
        {
            t_NPCInfo.NpcID = atoi(PQgetvalue(t_PGresult, i, 0));
            t_NPCInfo.Mapid = atoi(PQgetvalue(t_PGresult, i, 1));
            t_NPCInfo.Pos_x = atof(PQgetvalue(t_PGresult, i, 2));
            t_NPCInfo.Pos_y = atof(PQgetvalue(t_PGresult, i, 3));
            t_NPCInfo.Pos_z = atof(PQgetvalue(t_PGresult, i, 4));
            (*npcInfo)[t_NPCInfo.NpcID] = t_NPCInfo;
        }
        return t_row;
    }
}

//查询怪物掉落物品
hf_int32 DiskDBManager::GetMonsterLoot(umap_monsterLoot* monsterLoot)
{
    const hf_char* str = "select * from t_monsterloot;";
    m_mtx.lock();
    PGresult* t_PGresult = PQexec(m_PGconn, str);
    m_mtx.unlock();
    ExecStatusType t_status = PQresultStatus(t_PGresult);
    if(t_status != PGRES_TUPLES_OK)
    {
        return -1;
    }
    else
    {
        hf_int32 t_row = PQntuples(t_PGresult);
        STR_MonsterLoot t_monsterLoot;
        vector<STR_MonsterLoot> v_monsterLoot;

        for(hf_int32 i = 0; i < t_row; i++)
        {
            v_monsterLoot.clear();
            t_monsterLoot.MonsterTypeID = atoi(PQgetvalue(t_PGresult, i, 0));
            t_monsterLoot.PreCondition = atoi(PQgetvalue(t_PGresult, i, 1));
            t_monsterLoot.LootGoodsID = atoi(PQgetvalue(t_PGresult, i, 2));
            t_monsterLoot.LootProbability = atof(PQgetvalue(t_PGresult, i, 3));
            t_monsterLoot.Count = atoi(PQgetvalue(t_PGresult, i, 4));
            v_monsterLoot.push_back(t_monsterLoot);


            umap_monsterLoot::iterator it = monsterLoot->find(t_monsterLoot.MonsterTypeID);
            if(it != monsterLoot->end())
            {
                it->second.push_back(t_monsterLoot);
            }
            else
            {
                (*monsterLoot)[t_monsterLoot.MonsterTypeID] = v_monsterLoot;
            }
        }
        return t_row;
    }
}

//查询技能信息
hf_int32 DiskDBManager::GetSkillInfo(umap_skillInfo* skillInfo)
{
    const hf_char* str = "select * from t_skillinfo;";
    m_mtx.lock();
    PGresult* t_PGresult = PQexec(m_PGconn, str);
    m_mtx.unlock();
    ExecStatusType t_status = PQresultStatus(t_PGresult);
    if(t_status != PGRES_TUPLES_OK)
    {
        return -1;
    }
    else
    {
        hf_int32 t_row = PQntuples(t_PGresult);
        STR_PackSkillInfo t_skill;
        for(hf_int32 i = 0; i < t_row; i++)
        {
            t_skill.SkillID = atoi(PQgetvalue(t_PGresult, i, 0));
            t_skill.UseGoodsID = atoi(PQgetvalue(t_PGresult, i, 1));
            t_skill.UseMagic = atoi(PQgetvalue(t_PGresult, i, 2));
            t_skill.CoolTime = atof(PQgetvalue(t_PGresult, i, 3));
            t_skill.LeadTime = atof(PQgetvalue(t_PGresult, i, 4));
            t_skill.PhysicalHurt = atoi(PQgetvalue(t_PGresult, i, 5));
            t_skill.PhyPlus = atof(PQgetvalue(t_PGresult, i, 6));
            t_skill.MagicHurt = atoi(PQgetvalue(t_PGresult, i, 7));
            t_skill.MagPlus = atof(PQgetvalue(t_PGresult, i, 8));
            t_skill.UseGoodsCount = atoi(PQgetvalue(t_PGresult, i, 9));
            t_skill.FarDistance = atoi(PQgetvalue(t_PGresult, i, 10));
            t_skill.NearlyDistance = atoi(PQgetvalue(t_PGresult, i, 11));
            t_skill.TriggerID = atoi(PQgetvalue(t_PGresult, i, 12));
            t_skill.SkillRangeID = atoi(PQgetvalue(t_PGresult, i, 13));
            t_skill.UseAnger = atoi(PQgetvalue(t_PGresult, i, 14));
            t_skill.CastingTime = atoi(PQgetvalue(t_PGresult, i, 15));
            t_skill.CasterNumber = atoi(PQgetvalue(t_PGresult, i, 16));
            (*skillInfo)[t_skill.SkillID] = t_skill;
        }
        return t_row;
    }
}

//查询玩家金币
hf_int32 DiskDBManager::GetPlayerMoney(umap_roleMoney playerMoney, const hf_char* str)
{
    m_mtx.lock();
    PGresult* t_PGresult = PQexec(m_PGconn, str);
    m_mtx.unlock();
    ExecStatusType t_status = PQresultStatus(t_PGresult);
    if(t_status != PGRES_TUPLES_OK)
    {
        return -1;
    }
    else
    {
        hf_int32 t_row = PQntuples(t_PGresult);
        STR_PlayerMoney t_money;
        for(hf_int32 i = 0; i < t_row; i++)
        {
            t_money.Count = atoi(PQgetvalue(t_PGresult, i, 0));
            t_money.TypeID = atoi(PQgetvalue(t_PGresult, i, 1));
            (*playerMoney)[t_money.TypeID] = t_money;
        }
        return t_row;
    }
}

//查询玩家物品
hf_int32 DiskDBManager::GetPlayerGoods(umap_roleGoods playerGoods, umap_roleEqu playerEqu, const hf_char* str)
{
    m_mtx.lock();
    PGresult* t_PGresult = PQexec(m_PGconn, str);
    m_mtx.unlock();
    ExecStatusType t_status = PQresultStatus(t_PGresult);
    if(t_status != PGRES_TUPLES_OK)
    {
        return -1;
    }
    else
    {
        hf_int32 t_row = PQntuples(t_PGresult);
        STR_Goods t_goods;
        STR_PlayerEqu t_equ;
        t_goods.Source = Source_Bag;
        vector<STR_Goods> t_vec;
        for(hf_int32 i = 0; i < t_row; i++)
        {
            t_goods.GoodsID = atoi(PQgetvalue(t_PGresult, i, 0));
            t_goods.TypeID = atoi(PQgetvalue(t_PGresult, i, 1));
            t_goods.Count = atoi(PQgetvalue(t_PGresult, i, 2));
            t_goods.Position = atoi(PQgetvalue(t_PGresult, i, 3));

            if(EquTypeMinValue <= t_goods.TypeID && t_goods.TypeID <= EquTypeMaxValue) //装备
            {
                memcpy(&t_equ.goods, &t_goods, sizeof(STR_Goods));
                (*playerEqu)[t_goods.GoodsID] = t_equ;
                continue;
            }
            _umap_roleGoods::iterator it = playerGoods->find(t_goods.GoodsID);
            if(it != playerGoods->end())
            {
                it->second.push_back(t_goods);
            }
            else
            {
                t_vec.clear();
                t_vec.push_back(t_goods);
                (*playerGoods)[t_goods.GoodsID] = t_vec;
            }
        }
        return t_row;
    }
}

//查询玩家装备属性
hf_int32 DiskDBManager::GetPlayerEqu(umap_roleEqu playerEqu, const hf_char* str)
{
    m_mtx.lock();
    PGresult* t_PGresult = PQexec(m_PGconn, str);
    m_mtx.unlock();
    ExecStatusType t_status = PQresultStatus(t_PGresult);
    if(t_status != PGRES_TUPLES_OK)
    {
        return -1;
    }
    else
    {
        hf_int32 t_row = PQntuples(t_PGresult);
        STR_EquipmentAttr t_equAttr;
        for(hf_int32 i = 0; i < t_row; i++)
        {
            t_equAttr.EquID = atoi(PQgetvalue(t_PGresult, i, 1));
            t_equAttr.TypeID = atoi(PQgetvalue(t_PGresult, i, 2));
            t_equAttr.Crit_Rate = atof(PQgetvalue(t_PGresult, i, 3));
            t_equAttr.Dodge_Rate = atof(PQgetvalue(t_PGresult, i, 4));
            t_equAttr.Hit_Rate = atof(PQgetvalue(t_PGresult, i, 5));
            t_equAttr.Resist_Rate = atof(PQgetvalue(t_PGresult, i, 6));
            t_equAttr.Caster_Speed = atof(PQgetvalue(t_PGresult, i, 7));
            t_equAttr.Move_Speed = atof(PQgetvalue(t_PGresult, i, 8));
            t_equAttr.Hurt_Speed = atof(PQgetvalue(t_PGresult, i, 9));
            t_equAttr.RecoveryLife_Percentage = atof(PQgetvalue(t_PGresult, i, 10));
            t_equAttr.RecoveryLife_value = atoi(PQgetvalue(t_PGresult, i, 11));
            t_equAttr.RecoveryMagic_Percentage = atof(PQgetvalue(t_PGresult, i, 12));
            t_equAttr.RecoveryMagic_value = atoi(PQgetvalue(t_PGresult, i, 13));
            t_equAttr.MagicHurt_Reduction = atof(PQgetvalue(t_PGresult, i, 14));
            t_equAttr.PhysicalHurt_Reduction = atof(PQgetvalue(t_PGresult, i, 15));
            t_equAttr.CritHurt = atof(PQgetvalue(t_PGresult, i, 16));
            t_equAttr.CritHurt_Reduction = atof(PQgetvalue(t_PGresult, i, 17));
            t_equAttr.Magic_Pass = atof(PQgetvalue(t_PGresult, i, 18));
            t_equAttr.Physical_Pass = atof(PQgetvalue(t_PGresult, i, 19));
            t_equAttr.SuitSkillID = atoi(PQgetvalue(t_PGresult, i, 20));
            t_equAttr.HP = atoi(PQgetvalue(t_PGresult, i, 21));
            t_equAttr.Magic = atoi(PQgetvalue(t_PGresult, i, 22));
            t_equAttr.PhysicalDefense = atoi(PQgetvalue(t_PGresult, i, 23));
            t_equAttr.MagicDefense = atoi(PQgetvalue(t_PGresult, i, 24));
            t_equAttr.PhysicalAttack = atoi(PQgetvalue(t_PGresult, i, 25));
            t_equAttr.MagicAttack = atoi(PQgetvalue(t_PGresult, i, 26));
            t_equAttr.Rigorous = atoi(PQgetvalue(t_PGresult, i, 27));
            t_equAttr.Will = atoi(PQgetvalue(t_PGresult, i, 28));
            t_equAttr.Wise = atoi(PQgetvalue(t_PGresult, i, 29));
            t_equAttr.Mentality = atoi(PQgetvalue(t_PGresult, i, 30));
            t_equAttr.Physical_fitness = atoi(PQgetvalue(t_PGresult, i, 31));
            t_equAttr.JobID = atoi(PQgetvalue(t_PGresult, i, 32));
            t_equAttr.BodyPos = atoi(PQgetvalue(t_PGresult, i, 33));
            t_equAttr.Grade = atoi(PQgetvalue(t_PGresult, i, 34));
            t_equAttr.Level = atoi(PQgetvalue(t_PGresult, i, 35));
            t_equAttr.StrengthenLevel = atoi(PQgetvalue(t_PGresult, i, 36));
            t_equAttr.MaxDurability = atoi(PQgetvalue(t_PGresult, i, 37));
            t_equAttr.Durability = atoi(PQgetvalue(t_PGresult, i, 38));

            memcpy(&((*playerEqu)[t_equAttr.EquID].equAttr), &t_equAttr, sizeof(STR_EquipmentAttr));
        }
        return t_row;
    }
}


//查询玩家未捡取的物品位置
hf_int32 DiskDBManager::GetNotPickGoodsPosition(umap_lootPosition lootPosition, const hf_char* str)
{
    time_t timep;
    time(&timep);
    m_mtx.lock();
    PGresult* t_PGresult = PQexec(m_PGconn, str);
    m_mtx.unlock();
    ExecStatusType t_status = PQresultStatus(t_PGresult);
    if(t_status != PGRES_TUPLES_OK)
    {
        return -1;
    }
    else
    {
        hf_int32 t_row = PQntuples(t_PGresult);
        LootPositionTime t_loot;
        for(hf_int32 i = 0; i < t_row; i++)
        {
            t_loot.timep = (hf_uint32)timep;
            t_loot.continueTime = atoi(PQgetvalue(t_PGresult, i, 0));
            t_loot.goodsPos.GoodsFlag = atoi(PQgetvalue(t_PGresult, i, 1));
            t_loot.goodsPos.Pos_x = atoi(PQgetvalue(t_PGresult, i, 2));
            t_loot.goodsPos.Pos_y = atoi(PQgetvalue(t_PGresult, i, 3));
            t_loot.goodsPos.Pos_z = atoi(PQgetvalue(t_PGresult, i, 4));
            t_loot.goodsPos.MapID = atoi(PQgetvalue(t_PGresult, i, 5));

            (*lootPosition)[t_loot.goodsPos.GoodsFlag] = t_loot;
        }
        return t_row;
    }
}

//查询玩家未捡取的物品
hf_int32 DiskDBManager::GetNotPickGoods(umap_lootGoods lootGoods, const hf_char* str)
{
    m_mtx.lock();
    PGresult* t_PGresult = PQexec(m_PGconn, str);
    m_mtx.unlock();
    ExecStatusType t_status = PQresultStatus(t_PGresult);
    if(t_status != PGRES_TUPLES_OK)
    {
        return -1;
    }
    else
    {
        hf_int32 t_row = PQntuples(t_PGresult);
        STR_LootGoods t_goods;
        vector<STR_LootGoods> vec;
        for(hf_int32 i = 0; i < t_row; i++)
        {
            hf_uint32 lootID = atoi(PQgetvalue(t_PGresult, i, 0));
            t_goods.LootGoodsID = atoi(PQgetvalue(t_PGresult, i, 1));
            t_goods.Count = atoi(PQgetvalue(t_PGresult, i, 2));

            _umap_lootGoods::iterator it = lootGoods->find(lootID);
            if(it != lootGoods->end())
            {
                it->second.push_back(t_goods);
            }
            else
            {
                vec.clear();
                vec.push_back(t_goods);
                (*lootGoods)[lootID] = vec;
            }
        }
        return t_row;
    }
}

//查询物品价格
hf_int32 DiskDBManager::GetGoodsPrice(umap_goodsPrice* goodsPrice, const hf_char* str)
{
    m_mtx.lock();
    PGresult* t_PGresult = PQexec(m_PGconn, str);
    m_mtx.unlock();
    ExecStatusType t_status = PQresultStatus(t_PGresult);
    if(t_status != PGRES_TUPLES_OK)
    {
        return -1;
    }
    else
    {
        hf_int32 t_row = PQntuples(t_PGresult);
        STR_GoodsPrice t_goodsPrice;
        for(hf_int32 i = 0; i < t_row; i++)
        {
            t_goodsPrice.GoodsID = atoi(PQgetvalue(t_PGresult, i, 0));
            t_goodsPrice.BuyPrice = atoi(PQgetvalue(t_PGresult, i, 1));
            t_goodsPrice.SellPrice = atoi(PQgetvalue(t_PGresult, i, 2));
            (*goodsPrice)[t_goodsPrice.GoodsID] = t_goodsPrice;
        }
        return t_row;
    }
}

//查询消耗品价格
hf_int32 DiskDBManager::GetConsumableAttr(umap_consumable* consumable, const hf_char* str)
{
    m_mtx.lock();
    PGresult* t_PGresult = PQexec(m_PGconn, str);
    m_mtx.unlock();
    ExecStatusType t_status = PQresultStatus(t_PGresult);
    if(t_status != PGRES_TUPLES_OK)
    {
        return -1;
    }
    else
    {
        hf_int32 t_row = PQntuples(t_PGresult);
        STR_Consumable t_consumable;
        for(hf_int32 i = 0; i < t_row; i++)
        {
            t_consumable.GoodsID = atoi(PQgetvalue(t_PGresult, i, 0));
            t_consumable.HP = atoi(PQgetvalue(t_PGresult, i, 1));
            t_consumable.Magic = atoi(PQgetvalue(t_PGresult, i, 2));
            t_consumable.ColdTime = atoi(PQgetvalue(t_PGresult, i, 3));
            t_consumable.StackNumber = atoi(PQgetvalue(t_PGresult, i, 4));
            t_consumable.PersecondHP = atoi(PQgetvalue(t_PGresult, i, 5));
            t_consumable.PersecondMagic = atoi(PQgetvalue(t_PGresult, i, 6));
            t_consumable.UserLevel = atoi(PQgetvalue(t_PGresult, i, 7));
            t_consumable.ContinueTime = atoi(PQgetvalue(t_PGresult, i, 8));
            t_consumable.Type = atoi(PQgetvalue(t_PGresult, i, 9));

            (*consumable)[t_consumable.GoodsID] = t_consumable;
        }
        return t_row;
    }
}

//查询装备属性
hf_int32 DiskDBManager::GetEquAttr(umap_equAttr* equAttr, const hf_char* str)
{
    m_mtx.lock();
    PGresult* t_PGresult = PQexec(m_PGconn, str);
    m_mtx.unlock();
    ExecStatusType t_status = PQresultStatus(t_PGresult);
    if(t_status != PGRES_TUPLES_OK)
    {
        return -1;
    }
    else
    {
        hf_int32 t_row = PQntuples(t_PGresult);
        STR_EquipmentAttr t_equAttr;
        for(hf_int32 i = 0; i < t_row; i++)
        {
            t_equAttr.TypeID = atoi(PQgetvalue(t_PGresult, i, 0));
            t_equAttr.Crit_Rate = atof(PQgetvalue(t_PGresult, i, 2));
            t_equAttr.Dodge_Rate = atof(PQgetvalue(t_PGresult, i, 3));
            t_equAttr.Hit_Rate = atof(PQgetvalue(t_PGresult, i, 4));
            t_equAttr.Resist_Rate = atof(PQgetvalue(t_PGresult, i, 5));
            t_equAttr.Caster_Speed = atof(PQgetvalue(t_PGresult, i, 6));
            t_equAttr.Move_Speed = atof(PQgetvalue(t_PGresult, i, 7));
            t_equAttr.Hurt_Speed = atof(PQgetvalue(t_PGresult, i, 8));
            t_equAttr.RecoveryLife_Percentage = atof(PQgetvalue(t_PGresult, i, 9));
            t_equAttr.RecoveryLife_value = atoi(PQgetvalue(t_PGresult, i, 10));
            t_equAttr.RecoveryMagic_Percentage = atof(PQgetvalue(t_PGresult, i, 11));
            t_equAttr.RecoveryMagic_value = atoi(PQgetvalue(t_PGresult, i, 12));
            t_equAttr.MagicHurt_Reduction = atof(PQgetvalue(t_PGresult, i, 13));
            t_equAttr.PhysicalHurt_Reduction = atof(PQgetvalue(t_PGresult, i, 14));
            t_equAttr.CritHurt = atof(PQgetvalue(t_PGresult, i, 15));
            t_equAttr.CritHurt_Reduction = atof(PQgetvalue(t_PGresult, i, 16));
            t_equAttr.Magic_Pass = atof(PQgetvalue(t_PGresult,i, 17));
            t_equAttr.Physical_Pass = atof(PQgetvalue(t_PGresult,i, 18));
            t_equAttr.SuitSkillID = atoi(PQgetvalue(t_PGresult,i, 19));
            t_equAttr.HP = atoi(PQgetvalue(t_PGresult, i, 20));
            t_equAttr.Magic = atoi(PQgetvalue(t_PGresult, i, 21));
            t_equAttr.PhysicalDefense = atoi(PQgetvalue(t_PGresult, i, 22));
            t_equAttr.MagicDefense = atoi(PQgetvalue(t_PGresult, i, 23));
            t_equAttr.PhysicalAttack = atoi(PQgetvalue(t_PGresult, i, 24));
            t_equAttr.MagicAttack = atoi(PQgetvalue(t_PGresult, i, 25));
            t_equAttr.Rigorous = atoi(PQgetvalue(t_PGresult, i, 26));
            t_equAttr.Will = atoi(PQgetvalue(t_PGresult, i, 27));
            t_equAttr.Wise = atoi(PQgetvalue(t_PGresult, i, 28));
            t_equAttr.Mentality = atoi(PQgetvalue(t_PGresult, i, 29));
            t_equAttr.Physical_fitness = atoi(PQgetvalue(t_PGresult, i, 30));
            t_equAttr.JobID = atoi(PQgetvalue(t_PGresult, i ,31));
            t_equAttr.BodyPos = atoi(PQgetvalue(t_PGresult, i, 32));
            t_equAttr.Grade = atoi(PQgetvalue(t_PGresult, i, 33));
            t_equAttr.Level = atoi(PQgetvalue(t_PGresult, i, 34));
            t_equAttr.StrengthenLevel = atoi(PQgetvalue(t_PGresult, i, 35));
            t_equAttr.MaxDurability = atoi(PQgetvalue(t_PGresult, i, 36));
            t_equAttr.Durability = t_equAttr.MaxDurability;

            (*equAttr)[t_equAttr.TypeID] = t_equAttr;
        }
        return t_row;
    }
}


//查询数据库中装备现在的最大值
hf_int32 DiskDBManager::GetEquIDMaxValue()
{
    const hf_char* str = "select max(equid) from t_playerequattr;";
    Logger::GetLogger()->Debug(str);
    m_mtx.lock();
    PGresult* t_PGresult = PQexec(m_PGconn, str);
    m_mtx.unlock();
    ExecStatusType t_status = PQresultStatus(t_PGresult);
    if(t_status != PGRES_TUPLES_OK)
    {
        return -1;
    }
    hf_int32 t_row = PQntuples(t_PGresult);
    if(t_row == 0)
    {
        return EquipMentID;
    }
    else
    {
        hf_uint32 equid = atoi(PQgetvalue(t_PGresult, 0, 0));
        if(equid < EquipMentID)
        {
            return EquipMentID;
        }
        else
        {
            return equid;
        }
    }
}

//查询用户身上穿戴的装备
hf_int32 DiskDBManager::GetUserBodyEqu(hf_char* buff, hf_char* str)
{
    m_mtx.lock();
    PGresult* t_PGresult = PQexec(m_PGconn, str);
    m_mtx.unlock();
    ExecStatusType t_status = PQresultStatus(t_PGresult);
    if(t_status != PGRES_TUPLES_OK)
    {
        return -1;
    }
    else
    {
        STR_BodyEquipment bodyEqu;
        hf_int32 t_row = PQntuples(t_PGresult);
        for(hf_int32 i = 0; i < t_row; i++)
        {
            bodyEqu.roleid = atoi(PQgetvalue(t_PGresult, 0, 0));
            bodyEqu.Head = atoi(PQgetvalue(t_PGresult, 0, 1));
            bodyEqu.HeadType = atoi(PQgetvalue(t_PGresult, 0, 2));
            bodyEqu.UpperBody = atoi(PQgetvalue(t_PGresult, 0, 3));
            bodyEqu.UpperBodyType = atoi(PQgetvalue(t_PGresult, 0, 4));
            bodyEqu.Pants = atoi(PQgetvalue(t_PGresult, 0, 5));
            bodyEqu.PantsType = atoi(PQgetvalue(t_PGresult, 0, 6));
            bodyEqu.Shoes = atoi(PQgetvalue(t_PGresult, 0, 7));
            bodyEqu.ShoesType = atoi(PQgetvalue(t_PGresult, 0, 8));
            bodyEqu.Belt = atoi(PQgetvalue(t_PGresult, 0, 9));
            bodyEqu.BeltType = atoi(PQgetvalue(t_PGresult, 0, 10));
            bodyEqu.Neaklace = atoi(PQgetvalue(t_PGresult, 0, 11));
            bodyEqu.NeaklaceType = atoi(PQgetvalue(t_PGresult, 0, 12));
            bodyEqu.Bracelet = atoi(PQgetvalue(t_PGresult, 0, 13));
            bodyEqu.BraceletType = atoi(PQgetvalue(t_PGresult, 0, 14));
            bodyEqu.LeftRing = atoi(PQgetvalue(t_PGresult, 0, 15));
            bodyEqu.LeftRingType = atoi(PQgetvalue(t_PGresult, 0, 16));
            bodyEqu.RightRing = atoi(PQgetvalue(t_PGresult, 0, 17));
            bodyEqu.RightRingType = atoi(PQgetvalue(t_PGresult, 0, 18));
            bodyEqu.Phone = atoi(PQgetvalue(t_PGresult, 0, 19));
            bodyEqu.PhoneType = atoi(PQgetvalue(t_PGresult, 0, 20));
            bodyEqu.Weapon = atoi(PQgetvalue(t_PGresult, 0, 21));
            bodyEqu.WeaponType = atoi(PQgetvalue(t_PGresult, 0, 22));
            memcpy(buff + sizeof(STR_PackHead) + i*sizeof(STR_BodyEquipment), &bodyEqu, sizeof(STR_BodyEquipment));
        }
        return t_row;
    }
}

//查询玩家身上穿戴的装备
hf_int32 DiskDBManager::GetRoleBodyEqu(STR_BodyEquipment* bodyEqu, hf_char* str)
{
    m_mtx.lock();
    PGresult* t_PGresult = PQexec(m_PGconn, str);
    m_mtx.unlock();
    ExecStatusType t_status = PQresultStatus(t_PGresult);
    if(t_status != PGRES_TUPLES_OK)
    {
        return -1;
    }
    else
    {
        hf_int32 t_row = PQntuples(t_PGresult);
        if(t_row == 1)
        {
            bodyEqu->roleid = atoi(PQgetvalue(t_PGresult, 0, 0));
            bodyEqu->Head = atoi(PQgetvalue(t_PGresult, 0, 1));
            bodyEqu->HeadType = atoi(PQgetvalue(t_PGresult, 0, 2));
            bodyEqu->UpperBody = atoi(PQgetvalue(t_PGresult, 0, 3));
            bodyEqu->UpperBodyType = atoi(PQgetvalue(t_PGresult, 0, 4));
            bodyEqu->Pants = atoi(PQgetvalue(t_PGresult, 0, 5));
            bodyEqu->PantsType = atoi(PQgetvalue(t_PGresult, 0, 6));
            bodyEqu->Shoes = atoi(PQgetvalue(t_PGresult, 0, 7));
            bodyEqu->ShoesType = atoi(PQgetvalue(t_PGresult, 0, 8));
            bodyEqu->Belt = atoi(PQgetvalue(t_PGresult, 0, 9));
            bodyEqu->BeltType = atoi(PQgetvalue(t_PGresult, 0, 10));
            bodyEqu->Neaklace = atoi(PQgetvalue(t_PGresult, 0, 11));
            bodyEqu->NeaklaceType = atoi(PQgetvalue(t_PGresult, 0, 12));
            bodyEqu->Bracelet = atoi(PQgetvalue(t_PGresult, 0, 13));
            bodyEqu->BraceletType = atoi(PQgetvalue(t_PGresult, 0, 14));
            bodyEqu->LeftRing = atoi(PQgetvalue(t_PGresult, 0, 15));
            bodyEqu->LeftRingType = atoi(PQgetvalue(t_PGresult, 0, 16));
            bodyEqu->RightRing = atoi(PQgetvalue(t_PGresult, 0, 17));
            bodyEqu->RightRingType = atoi(PQgetvalue(t_PGresult, 0, 18));
            bodyEqu->Phone = atoi(PQgetvalue(t_PGresult, 0, 19));
            bodyEqu->PhoneType = atoi(PQgetvalue(t_PGresult, 0, 20));
            bodyEqu->Weapon = atoi(PQgetvalue(t_PGresult, 0, 21));
            bodyEqu->WeaponType = atoi(PQgetvalue(t_PGresult, 0, 22));
        }
        return t_row;
    }
}


//查询职业属性
hf_int32 DiskDBManager::GetJobAttribute(STR_RoleJobAttribute* jobAttr, hf_char* str)
{
    m_mtx.lock();
    PGresult* t_PGresult = PQexec(m_PGconn, str);
    m_mtx.unlock();
    ExecStatusType t_status = PQresultStatus(t_PGresult);
    if(t_status != PGRES_TUPLES_OK)
    {
        return -1;
    }
    else
    {
        jobAttr++;
        hf_int32 t_row = PQntuples(t_PGresult);
        for(hf_int32 i = 0; i < t_row; i++)
        {
            jobAttr->MaxHP = atoi(PQgetvalue(t_PGresult, i, 0));;
            jobAttr->MaxMagic = atoi(PQgetvalue(t_PGresult, i, 1));;
            jobAttr->PhysicalDefense = atoi(PQgetvalue(t_PGresult, i, 2));;
            jobAttr->MagicDefense = atoi(PQgetvalue(t_PGresult, i, 3));;
            jobAttr->PhysicalAttack = atoi(PQgetvalue(t_PGresult, i, 4));
            jobAttr->PhysicalDefense = atoi(PQgetvalue(t_PGresult, i, 5));
            jobAttr->Rigorous = atoi(PQgetvalue(t_PGresult, i, 6));
            jobAttr->Will = atoi(PQgetvalue(t_PGresult, i, 7));
            jobAttr->Wise = atoi(PQgetvalue(t_PGresult, i, 8));
            jobAttr->Mentality = atoi(PQgetvalue(t_PGresult, i, 9));
            jobAttr->Physical_fitness = atoi(PQgetvalue(t_PGresult, i, 10));
            jobAttr++;
        }
        return t_row;
    }
}


//查询玩家基本信息
hf_int32 DiskDBManager::GetRoleBasicInfo(STR_RoleBasicInfo* roleInfo, hf_char* str)
{
    m_mtx.lock();
    PGresult* t_PGresult = PQexec(m_PGconn, str);
    m_mtx.unlock();

    ExecStatusType t_ExecStatusType = PQresultStatus((t_PGresult));
    if(t_ExecStatusType != PGRES_TUPLES_OK)
    {
        printf("PQexec error\n");
        return -1;
    }
    else
    {
        hf_int32 t_row = PQntuples(t_PGresult);   //行数
        if(t_row == 1)
        {
            memcpy(roleInfo->Nick, PQgetvalue(t_PGresult, 0, 1), PQgetlength(t_PGresult, 0, 1));
            roleInfo->RoleID = atoi(PQgetvalue(t_PGresult, 0, 2));
            roleInfo->Profession = atoi(PQgetvalue(t_PGresult, 0, 3));
            roleInfo->Level = atoi(PQgetvalue(t_PGresult, 0, 4));
            roleInfo->Sex = atoi(PQgetvalue(t_PGresult, 0, 5));
            roleInfo->Figure = atoi(PQgetvalue(t_PGresult, 0, 6));
            roleInfo->FigureColor = atoi(PQgetvalue(t_PGresult, 0, 7));
            roleInfo->Face = atoi(PQgetvalue(t_PGresult, 0, 8));
            roleInfo->Eye = atoi(PQgetvalue(t_PGresult, 0, 9));
            roleInfo->Hair = atoi(PQgetvalue(t_PGresult, 0, 10));
            roleInfo->HairColor = atoi(PQgetvalue(t_PGresult, 0, 11));
            roleInfo->ModeID = atoi(PQgetvalue(t_PGresult, 0, 13));
            roleInfo->SkirtID = atoi(PQgetvalue(t_PGresult, 0, 14));
        }
        return t_row;
    }
}

//查询玩家已经完成的任务
hf_int32 DiskDBManager::GetPlayerCompleteTask(umap_completeTask completeTask, hf_char* str)
{
    m_mtx.lock();
    PGresult* t_PGresult = PQexec(m_PGconn, str);
    m_mtx.unlock();

    ExecStatusType t_ExecStatusType = PQresultStatus((t_PGresult));
    if(t_ExecStatusType != PGRES_TUPLES_OK)
    {
        printf("PQexec error\n");
        return -1;
    }
    else
    {
        hf_int32 t_row = PQntuples(t_PGresult);   //行数
        hf_uint32 taskid = 0;
        for(hf_int32 i = 0; i < t_row; i++)
        {
            taskid = atoi(PQgetvalue(t_PGresult, i, 0));
            (*completeTask)[taskid] = taskid;
        }
        return t_row;
    }
}
