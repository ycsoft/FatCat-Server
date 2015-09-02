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


#echo $INCLUDE
rm -fr *.o
rm -fr Server
rm -fr *.gch
rm -fr ./Game/*.gch
 
