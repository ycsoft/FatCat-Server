
#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <iostream>
#include <event.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <hash_map>
#include <postgresqlstruct.h>
#include "md5.h"

#include "unistd.h"
#include "errno.h"

#define PORT    7000
#define BACKLOG 5
#define MEMSIZE 1024
using namespace std;
static int Registered = 0;
static int logd = 0;

struct Register
{
    _STR_PackHead head;
    _STR_PackPlayerRegisterUserId reg;
    Register(const string& userName,const string& password,const string& email)
    {
        memset(&head,0,sizeof(Register));
        head.Len = htons(sizeof(_STR_PackPlayerRegisterUserId));
        head.Flag = htons(FLAG_PlayerRegisterUserId);
        strcpy((char*)reg.userName, userName.c_str());
        strcpy((char*)reg.password,password.c_str());
        strcpy((char*)reg.Email,email.c_str());
    }
};

struct RegisterRole
{
    _STR_PackHead head;
    _STR_PackPlayerRegisterRole reg;
    RegisterRole(const string& userName,hf_byte profession,hf_byte sex,const string& nickname)
    {
        memset(&head,0,sizeof(RegisterRole));
        head.Len = htons(sizeof(_STR_PackPlayerRegisterRole));
        head.Flag = htons(FLAG_PlayerRegisterRole);
        strcpy((char*)reg.userName, userName.c_str());
        reg.Profession = profession;
        reg.Sex = sex;
        strcpy((char*)reg.nickname,nickname.c_str());
    }
};

struct Login
{
    _STR_PackHead head;
    _STR_PackPlayerLoginUserId log;
    Login(const string& userName,const string& password)
    {
        memset(&head,0,sizeof(Login));
        head.Len = htons(sizeof(_STR_PackPlayerLoginUserId));
        head.Flag = htons(FLAG_PlayerLoginUserId);
        strcpy((char*)log.userName, userName.c_str());
        strcpy((char*)log.password,password.c_str());
    }
};

struct LoginRole
{
    _STR_PackHead head;
    _STR_PackPlayerRole log;
    LoginRole(hf_int32 roleNum)
    {
        memset(&head,0,sizeof(Login));
        head.Len = htons(sizeof(_STR_PackPlayerRole));
        head.Flag = htons(FLAG_PlayerLoginRole);
        log.buff = roleNum;
    }
};

ssize_t readn(int fd,void*vptr,size_t n);
ssize_t writen(int fd,const void*vptr,size_t n);

void handle(int connfd);
void handleFlagResult(int sockfd,void* bufReceive);

int main()
{
    struct sockaddr_in  servaddr;
    int connfd;
    connfd = socket(AF_INET,SOCK_STREAM,0);

    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
  //  servaddr.sin_addr.s_addr = inet_addr("61.185.224.60");
    servaddr.sin_addr.s_addr = inet_addr("192.168.1.111");

    if(connect(connfd,(struct sockaddr *)&servaddr,sizeof(servaddr))<0)
    {
        perror("connect error");
        return -1;
    }
    handle(connfd);
}

void handle(int sockfd)
{
    Register regist("yangyaanaga",MD5(string("123dssa45")).toString(),"adsf");
    writen(sockfd,(const void*)&regist,sizeof(regist));

    for(;;)
    {
        STR_PackHead recv;
        char bufReceive[BLOCK_SIZE_MID];
        bzero(&bufReceive,sizeof(bufReceive));
        bzero(&recv,sizeof(recv));
        readn(sockfd,(void*)&recv,sizeof(recv));
        readn(sockfd,(void*)&bufReceive,ntohs(recv.Len));

        int flag = ntohs(recv.Flag);
        switch(flag)
        {
            case FLAG_Result:
            {
                handleFlagResult(sockfd,&bufReceive);
            }
            case FLAG_PlayerLoginUserId:
            {
                int roleCount = ntohs(recv.Len)/4;
                int i = 0;
                vector<hf_int32> roleList;
                while(i < roleCount)
                {
                    roleList.push_back(htonl(*((hf_int32*)&bufReceive)+i));
                    i++;
                }
                LoginRole loginRole(roleList[roleList.size()-1]);
                writen(sockfd,(const void*)&loginRole,sizeof(loginRole));
            }
            case FLAG_MonsterInfo:
            {
                for(int j = 0; j < recv.Len/sizeof(_STR_PackMonsterPosition); j++)
                {
                    cout<<"MonsterID"<<ntohl((j + (_STR_PackMonsterPosition*)&bufReceive)->MonsterID)<<endl;
                    cout<<"Pos_x"<<ntohl((j + (_STR_PackMonsterPosition*)&bufReceive)->Pos_x)<<endl;
                    cout<<"Pos_y"<<ntohl((j + (_STR_PackMonsterPosition*)&bufReceive)->Pos_y)<<endl;
                    cout<<"Pos_z"<<ntohl((j + (_STR_PackMonsterPosition*)&bufReceive)->Pos_z)<<endl;
                    cout<<"Pos_x"<<ntohs((j + (_STR_PackMonsterPosition*)&bufReceive)->Direct)<<endl;
                    cout<<"ActID"<<ntohl((j + (_STR_PackMonsterPosition*)&bufReceive)->ActID)<<endl;
                }
            }
            case FLAG_MonsterAttrbt:
            {
                cout<<"MonsterID"<<ntohl(((_STR_PackMonsterAttrbt*)&bufReceive)->MonsterID)<<"\n"<<endl;
                cout<<"HP"<<ntohl(((_STR_PackMonsterAttrbt*)&bufReceive)->HP)<<"\n\n\n"<<endl;
            }
            default:
                break;
        }

    }
}

ssize_t readn(int fd,void*vptr,size_t n)
{
    size_t nleft;
    ssize_t nread;
    char *ptr;

    ptr = (char*)vptr;
    nleft = n;
    while(nleft > 0)
    {
        if((nread = read(fd,ptr,nleft))<0)
        {
            if(errno==EINTR)
                nread=0;
            else
                return (-1);
        }
        else if(nread == 0)
            break;
        nleft -= nread;
        ptr += nread;
    }
    return (n-nleft);
}

ssize_t writen(int fd,const void*vptr,size_t n)
{
    size_t nleft;
    ssize_t nwritten;
    const char *ptr;
    ptr = (const char*)vptr;
    nleft = n;
    while(nleft>0)
    {
        if((nwritten = write(fd,ptr,nleft))<=0)
        {
            if(nwritten<0&&errno==EINTR)
                nwritten = 0;
            else
                return (-1);
        }
        nleft -= nwritten;
        ptr += nwritten;
    }
    return n;
}

void handleFlagResult(int sockfd,void* bufReceive)
{
    _STR_PackResult* result = (_STR_PackResult*)((char*)bufReceive-sizeof(STR_PackHead));

    iint flag = ntohs(result->Flag);
    switch(flag)
    {
        case FLAG_PlayerRegisterUserId:
        {
            if(ntohs(result->result) == 0)
            {
                cout <<"register success"<<endl;
                RegisterRole regRole("yangyaanaga",1,1,"adsf@asdf");
                writen(sockfd,(const void*)&regRole,sizeof(regRole));
            }
            else if(ntohs(result->result) == 1)
            {
                cout << "用户名已经存在"<<endl;
            }
            else if(ntohs(result->result) == 2)
            {
                cout << "邮箱已经" <<endl;
            }
        }
        case FLAG_PlayerRegisterRole:
        {

            if(ntohs(result->result) == 0)
            {
                cout <<"register role sucess"<<endl;
                Login login("yangyaanaga",MD5(string("123dssa45")).toString());
                writen(sockfd,(const void*)&login,sizeof(login));
            }
            else
            {
                cout <<"register role fail"<<endl;
            }

        }
        case FLAG_PlayerLoginRole:
        {
            if(ntohs(result->result) == 0)
            {
                cout<<"角色登录成功"<<endl;
            }
            else
            {
                cout<<"角色登录失败"<<endl;
            }
        }
        default:
            break;
    }
}
