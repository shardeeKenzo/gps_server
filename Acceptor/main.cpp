#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <string>

#include "Utils/Logger.hpp"

#include "Authorization.h"
#include "utils.hpp"
#include "Server.h"

namespace po = boost::program_options;

#include <time.h>

int main(int argc, char* argv[]) {    
    
    initLogger1();

    ServerConfig            config;
    
    po::variables_map       map;
    po::options_description desc("Allowed options");
    
    desc.add_options()
    ("host"      , po::value< string >(&config.host      ), "host"      )
    ("port"      , po::value< string >(&config.port      ), "port"      )
    ("poolSize"  , po::value< size_t >(&config.poolSize  ), "poolSize"  )
    ("psqlHost"  , po::value< string >(&config.psqlHost  ), "psqlHost"  )
    ("psqlLogin" , po::value< string >(&config.psqlLogin ), "psqlLogin" )
    ("psqlDbName", po::value< string >(&config.psqlDbName), "psqlDbName")
    ("psqlDbPass", po::value< string >(&config.psqlDbPass), "psqlDbPass")
    ;
    
    readConfig(desc, map);

    // openning file for logging
    //freopen("acceptor_log.txt", "at", stdout);
    //freopen("acceptor_err.txt", "at", stderr);
    
    BOOST_LOG(logger) << getTimeString() << "Gps acceptor started " << endl << endl;
    
    Server s(config);
    
    s.run();

    return 0;
}