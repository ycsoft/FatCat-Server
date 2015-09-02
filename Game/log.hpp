#ifndef LOG_HPP
#define LOG_HPP

#include <boost/noncopyable.hpp>

#include <ostream>
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>


#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/make_shared_object.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/support/date_time.hpp>

#include <boost/core/null_deleter.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;

using namespace boost::log::trivial;


///
/// \brief The Logger class
///
/// 服务端日志
///
class Logger:public boost::enable_shared_from_this<Logger>,boost::noncopyable
{
public:

    typedef boost::shared_ptr<Logger>   LoggerPointer;

    ///
    /// \brief init
    ///初始化，设置日志格式及存储方式
    ///
    void init()
    {
        typedef sinks::synchronous_sink <sinks::text_ostream_backend> text_sink;
        boost::shared_ptr<text_sink>    sink = boost::make_shared<text_sink>();

        boost::shared_ptr<std::ostream> stream2(&std::clog, boost::null_deleter());

        //Add strem to write log to
        sink->locked_backend()->add_stream(
//                    boost::make_shared<std::ofstream>("Log.log")
                    stream2
                    );

        sink->set_formatter
        (

                    expr::format(" %1% %2%: <%3%> %4%")
                        %expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S")
                        % expr::attr< unsigned int >("LineID")
                        % logging::trivial::severity
                        % expr::smessage
        );
        logging::core::get()->add_sink(sink);

        logging::add_common_attributes();
    }

    ///
    /// \brief Debug
    /// \param s
    /// \return
    ///打印调试级别日志
    ///
    LoggerPointer Debug( std::string s)
    {
        BOOST_LOG_SEV(lg, debug) << s;
        return shared_from_this();
    }

    ///
    /// \brief Trace
    /// \param s
    /// \return
    ///打印追踪信息
    ///
    LoggerPointer Trace( std::string s)
    {
        BOOST_LOG_SEV(lg, trace) << s;
        return shared_from_this();
    }

    ///
    /// \brief Info
    /// \param s
    /// \return
    ///打印普通信息
    ///
    LoggerPointer Info( std::string s)
    {
        BOOST_LOG_SEV(lg, info) << s;
        return shared_from_this();
    }

    ///
    /// \brief Warning
    /// \param s
    /// \return
    ///打印警告级别的日志
    ///
    LoggerPointer Warning(  std::string s)
    {
        BOOST_LOG_SEV(lg, warning) << s;
        return shared_from_this();
    }

    ///
    /// \brief Error
    /// \param s
    /// \return
    ///错误级别日志
    ///
    LoggerPointer Error( std::string s)
    {
        BOOST_LOG_SEV(lg, error) << s;
        return shared_from_this();
    }

    ///
    /// \brief Fatal
    /// \param s
    /// \return
    ///严重错误级日志
    ///
    LoggerPointer Fatal( std::string s)
    {
        BOOST_LOG_SEV(lg, fatal) << s;
        return shared_from_this();
    }

    static LoggerPointer   GetLogger()
    {
        static LoggerPointer ptr;
        if (  ptr.get() == NULL )
        {
            ptr = LoggerPointer(new Logger());//boost::make_shared<Logger>();
        }
        return ptr;
    }

private:
    Logger()
    {
        init();
    }
    src::severity_logger< severity_level > lg;
};


#endif // LOG_HPP

