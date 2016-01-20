#!/bin/sh
INCLUDE="-I/mnt/boost/include"\
" -I/mnt/boost/include/boost/threadpool/boost"\
" -I/usr/local/hiredis/include"\
" -I/usr/local/PGSQL/include"\
" -I/usr/local/log4cpp/include"\
" -I../FatCat-Server" 

LIBS="/usr/local/hiredis/lib/libhiredis.a"\
" /usr/local/PGSQL/lib/libpq.so"\
" /mnt/boost/lib/libboost_system.a"\
" /mnt/boost/lib/libboost_thread.a"

FLAGS=

rm -fr *.o *.gch ./*/*.gch

echo g++  $FLAGS -c -g *.cpp  ./*/*.cpp ./*/*.hpp $INCLUDE -Wno-deprecated
g++  $FLAGS -c -g *.cpp  ./*/*.cpp ./*/*.hpp $INCLUDE -Wno-deprecated  --std=c++0x 

#g++  $FLAGS -c ./log/*.hpp $INCLUDE
#echo g++  $FLAGS -c ./log/*.hpp $INCLUDE
#
#g++  $FLAGS -c ./Game/*.cpp $INCLUDE
#echo g++  $FLAGS -c ./Game/*.cpp $INCLUDE
#
#g++  $FLAGS -c ./GameAttack/*.* $INCLUDE
#echo g++  $FLAGS -c ./GameAttack/*.c* $INCLUDE
#
#g++  $FLAGS -c ./GameChat/*.cpp $INCLUDE
#echo g++  $FLAGS -c ./GameChat/*.cpp $INCLUDE
#
#g++  $FLAGS -c ./GameInterchange/*.cpp $INCLUDE
#echo g++  $FLAGS -c ./GameInterchange/*.cpp $INCLUDE

#g++  $FLAGS -c ./GameTask/*.cpp $INCLUDE
#echo g++  $FLAGS -c ./GameTask/*.cpp $INCLUDE
#
#g++  $FLAGS -c ./memManage/*.cpp $INCLUDE
#echo g++  $FLAGS -c ./memManage/*.cpp $INCLUDE
#
#g++  $FLAGS -c ./Monster/* $INCLUDE
#echo g++  $FLAGS -c ./Monster/*.* $INCLUDE
#
#g++  $FLAGS -c ./NetWork/* $INCLUDE
#echo g++  $FLAGS -c ./NetWork/*.cpp $INCLUDE
#
#g++  $FLAGS -c ./OperationGoods/* $INCLUDE
#echo g++  $FLAGS -c ./OperationGoods/*.cpp $INCLUDE

#g++  $FLAGS -c ./OperationPostgres/*.cpp $INCLUDE
#echo g++  $FLAGS -c ./OperationPostgres/*.cpp $INCLUDE
#
#g++  $FLAGS -c ./PlayerLogin/*.cpp $INCLUDE
#echo g++  $FLAGS -c ./PlayerLogin/*.cpp $INCLUDE
#
#g++  $FLAGS -c ./TeamFriend/*.cpp $INCLUDE
#echo g++  $FLAGS -c ./TeamFriend/*.cpp $INCLUDE

#g++ *.o $LIBS -o Server  -lpthread -lrt
echo g++  *.o $LIBS -o Server  -lpthread -lrt -lexpat -L /usr/local/log4cpp/lib -llog4cpp
g++  *.o $LIBS -o Server  -lpthread -lrt -lexpat -L /usr/local/log4cpp/lib -llog4cpp

echo exit successfully!
