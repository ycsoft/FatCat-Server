#include "fileoperation.h"
#include "Monster/monsterstruct.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdio>

fileOperation::fileOperation()
{
    buffMap1 = NULL;
    buffMap2 = NULL;
    buffMap3 = NULL;
}

fileOperation::~fileOperation()
{
    if(buffMap1)
        delete buffMap1;
    if(buffMap2)
        delete buffMap2;
    if(buffMap3)
        delete buffMap3;
}

void fileOperation::ReadFile (hf_char* filePath,  hf_uint16 MapID)
{
    hf_int32 fd = open(filePath, O_RDONLY, 777);
    if(fd == -1)
    {
        printf("%s open failed\n", filePath);
        return;
    }
    hf_int32 fileLength = lseek (fd, 0, SEEK_END);
    if(fileLength == -1)
    {
        printf("lseek error\n");
        close(fd);
        return;
    }
    printf("fileLength = %d\n", fileLength);

    if(fileLength != 0)
    {
        hf_char* buffMap = NULL;
        switch(MapID)
        {
        case MAP1:
        {
            buffMap1 = new hf_char[fileLength];
            buffMap = buffMap1;
            break;
        }
        case MAP2:
        {
            buffMap2 = new hf_char[fileLength];
            buffMap = buffMap2;
            break;
        }
        case MAP3:
        {
            buffMap3 = new hf_char[fileLength];
            buffMap = buffMap3;
            break;
        }
        default:
        {
            printf("not MapFile\n");
            return;
        }
            break;

        }

        memset(buffMap, 0, fileLength);
        lseek (fd, 0, SEEK_SET);

        hf_uint32 num = 0;
        for(hf_uint32 i = 0; i < fileLength; i++)
        {
            if(read(fd, &num, 1) == 1)
            {
                printf("%d ", num);
                if(num > 100)
                {
                    buffMap[i] = 1;
                }
            }
            else
            {
                printf("%d read error\n", fileLength);
                return;
            }
        }
        printf("\n");
    }
    close(fd);
}


//判断移动方向
hf_uint8 fileOperation::JudgeMoveDirect(hf_float current_x, hf_float current_z, hf_uint32 MapID, hf_float target_x, hf_float target_z)
{
    hf_char* buffMap = NULL;
    switch(MapID)
    {
    case MAP1:
    {
        buffMap = buffMap1;
        break;
    }
    case MAP2:
    {
        buffMap = buffMap1;
        break;
    }
    case MAP3:
    {
        buffMap = buffMap1;
        break;
    }
    default:
        return 0;
    }
    //右1 右上2 上3 左上4 左5 左下6 下7 右下8
    hf_uint8 moveDirect = 0;
    if(target_x - current_x >= 1)
    {
        if(target_z - current_z >= 1)
            moveDirect = 2; //右上
        else if(target_z - current_z <= -1)
            moveDirect = 8; //右下
        else
            moveDirect = 1; //右
    }
    else if(target_x - current_x <= -1)
    {
        if(target_z - current_z >= 1)
            moveDirect = 4; //左上
        else if(target_z - current_z <= -1)
            moveDirect = 6; //左下
        else
            moveDirect = 5; //左
    }
    else
    {
        if(target_z - current_z >= 1)
            moveDirect = 3; //上
        else if(target_z - current_z <= -1)
            moveDirect = 7; //下
    }
    return EnsureMoveDirect (current_x, current_z, buffMap, moveDirect);
}


hf_uint8 fileOperation::EnsureMoveDirect(hf_float current_x, hf_float current_z,hf_char* buffMap, hf_uint8 direct)
{
    hf_int32 t_x = (hf_int32)current_x;
    hf_int32 t_z = (hf_int32)current_z;
    switch (direct)
    {
    case 1:
    {
        if(buffMap[t_z*MAP_X+t_x+1] == 0)
            return 1;
    }
    case 2:
    {
        if(buffMap[(t_z-1)*MAP_X+t_x+1] == 0)
            return 2;
    }
    case 3:
    {
        if(buffMap[(t_z-1)*MAP_X+t_x] == 0)
            return 3;
    }
    case 4:
    {
        if(buffMap[(t_z-1)*MAP_X+t_x-1] == 0)
            return 4;
    }
    case 5:
    {
        if(buffMap[(t_z)*MAP_X+t_x-1] == 0)
            return 5;
    }
    case 6:
    {
        if(buffMap[(t_z+1)*MAP_X+t_x-1] == 0)
            return 6;
    }
    case 7:
    {
        if(buffMap[(t_z+1)*MAP_X+t_x] == 0)
            return 7;
    }
    case 8:
        if(buffMap[(t_z+1)*MAP_X+t_x+1] == 0)
            return 8;
    default:
        return 0;
    }
}
