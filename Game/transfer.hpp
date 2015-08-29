#ifndef TRANSFER
#define TRANSFER

#include <boost/asio.hpp>
#include <iostream>
#include <boost/thread/mutex.hpp>

using boost::asio::ip::tcp;

static boost::mutex  mtx;


class Transfer:public boost::noncopyable
{
public:
    static int write_n( tcp::socket *sk,char *buf, int size)
    {
        mtx.lock();
        int wrtn,total = size;
        boost::system::error_code error;
        //boost::asio::write(*sk,boost::asio::buffer(buf,size),boost::asio::transfer_exactly(size),error);



        wrtn = boost::asio::write(*sk,boost::asio::buffer(buf,total),boost::asio::transfer_all(),error);
        if ( error )
        {
            wrtn = -1;
            sk->close();
        }
//        while ( wrtn < total)
//        {
//            if ( error == boost::asio::error::eof || error == boost::asio::error::shut_down)
//            {
//                sk->close();
//                return -1;
//            }
//            wrtn += boost::asio::write(*sk,boost::asio::buffer(buf+wrtn,total-wrtn),error);
//        }
        mtx.unlock();
        //Locker::sock_lock.unlock();
        return wrtn;
    }
};

#endif // TRANSFER

