#ifndef CMDPARSE_H
#define CMDPARSE_H

#include "NetWork/tcpconnection.h"


void CommandParse(TCPConnection::Pointer conn, void* reg);

void CommandParseLogin(TCPConnection::Pointer conn, void* reg);

#endif  // CMDPARSE_H
