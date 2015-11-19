#include "fileoperation.h"


#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdio>

fileOperation::fileOperation()
{

}

fileOperation::~fileOperation()
{

}

//fileOperation::OpenFile (hf_char *filePath)
//{
//    if((fdr=open(filePath, O_RDONLY))==-1)
//    {
//        printf("file not found,open failed!");
//        return -1;
//    }
//    else
//        return 0;
//}

void fileOperation::ReadFile (hf_char* filePath)
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
        lseek (fd, 0, SEEK_SET);

        hf_int32 readLen = 0;
        hf_uint32 num = 0;
        for(hf_uint32 i = 0; i < fileLength; i++)
        {
            readLen = read(fd, &num, 1);
            if(readLen > 0)
            {
                printf("%d ", num);
            }
        }
        printf("\n");
    }
    close(fd);
}
