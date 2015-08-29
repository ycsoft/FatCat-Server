#!/bin/sh
INCLUDE="-I/home/yangchen/workspace/boost_1_58_0"\
" -I/home/yangchen/workspace/threadpool"\
" -I/usr/local/hiredis/include/hiredis"\
" -I/usr/include/postgresql"\
" -I../Server" 

LIBS="/usr/local/hiredis/lib/libhiredis.a"\
" /usr/lib/libpq.so"\
" /home/yangchen/workspace/boost_1_58_0/stage/lib/libboost_system.a"\
" /home/yangchen/workspace/boost_1_58_0/stage/lib/libboost_thread.a"


echo $INCLUDE
g++ -c *.cpp $INCLUDE
g++ -c ./Game/*.hpp $INCLUDE
g++ -c ./memManage/*.cpp $INCLUDE
g++ -c ./NetWork/*.cpp $INCLUDE

g++ *.o $LIBS -o Server  -lpthread

 
