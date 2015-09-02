#ifndef RPC_H
#define RPC_H


/**
 * @brief The RPC class
 *
 *为了便于服务器之间的通信，设计了该类，使其他服务器可以调用本地的函数完成特定功能
 *通过这种设定，可以较为方便地实现分布式计算
 *
 *
 */

#define   RPC_FLAG      1101
#define   RPC_RES       1102

struct  rpc
{
    unsigned short len;
    unsigned short flag;
    char          fun[32];
    void          *arg;
};

struct rpc_res
{
  unsigned  short len;
  unsigned  short flag;
  unsigned  short flag2;
  void   *result;
};

class RPC
{
public:
  RPC();

  template < typename Return_Type, typename Arg_Type>

  Return_Type  Call( const char * host,const char *fun, Arg_Type *args , int argsize);


  ~RPC();
};

#endif // RPC_H
