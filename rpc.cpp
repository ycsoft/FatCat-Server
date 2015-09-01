
#include <string.h>

#include "rpc.h"
#include "Game/transfer.hpp"

RPC::RPC()
{

}

RPC::~RPC()
{

}


template<typename Return_Type, typename Arg_Type>

Return_Type RPC::Call(  const char * host, const char *fun, Arg_Type *args , int argsize  )
{
  struct rpc   rpcpack;

  rpcpack.flag = RPC_FLAG;
  memcpy(rpcpack.fun,fun,strlen(fun));
  memcpy(rpcpack.arg, args, argsize);
  rpcpack.len = sizeof( unsigned short) + 32 + argsize;

}
