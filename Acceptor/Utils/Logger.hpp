#pragma once

#define BOOST_ALL_DYN_LINK


#include <boost/log/sources/basic_logger.hpp>
#include <boost/log/sources/logger.hpp>
//#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
//#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>


namespace logging  = boost::log;
namespace src      = boost::log::sources;
namespace expr     = boost::log::expressions;
namespace keywords = boost::log::keywords;
namespace sinks    = boost::log::sinks;


static void initLogger()
{
    logging::add_file_log
    (
        keywords::file_name = "log/gps_tracker_%Y%m%d_%H%M%S.log",
        keywords::time_based_rotation = sinks::file::rotation_at_time_point(12, 0, 0)
    );
}


static src::logger_mt logger;

static void initLogger1() {
	boost::shared_ptr< logging::core > loggingCore = logging::core::get();

	boost::shared_ptr< sinks::text_file_backend > backend =
        boost::make_shared< sinks::text_file_backend >(
            // file name pattern
            keywords::file_name = "log/gps_tracker_%Y%m%d_%H%M%S.log",
            // ...or at midnight, whichever comes first
            keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0)                    
        );

    backend->auto_flush(true);

    // Wrap it into the frontend and register in the core.
    // The backend requires synchronization in the frontend.
    typedef sinks::synchronous_sink< sinks::text_file_backend > sink_t;
    boost::shared_ptr< sink_t > sink(boost::make_shared< sink_t >(backend));

    loggingCore->add_sink(sink);
}