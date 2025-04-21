#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <string>

#include "Authorization.h"
#include "utils.hpp"
#include "Server.h"

namespace po = boost::program_options;

int main(int argc, char* argv[])
{
    ServerConfig            config;

    po::variables_map       map;
    po::options_description desc("Allowed options");

    desc.add_options()
    ("host"      , po::value< string >(&config.host)      , "host")
    ("port"      , po::value< string >(&config.port)      , "port")
    ("poolSize"  , po::value< size_t >(&config.poolSize)  , "poolSize")
    ("psqlHost"  , po::value< string >(&config.psqlHost)  , "psqlHost")
    ("psqlLogin" , po::value< string >(&config.psqlLogin) , "psqlLogin")
    ("psqlDbName", po::value< string >(&config.psqlDbName), "psqlDbName")
    ("psqlDbPass", po::value< string >(&config.psqlDbPass), "psqlDbPass")
    ;

    // openning file for logging
    freopen("responder_log.txt", "at", stdout);

    cout << getTimeString() << "Gps reponder started " << endl;

    readConfig(desc, map);

    cout << "Config read: " << endl
         << "host        : " << config.host       << endl
         << "port        : " << config.port       << endl
         << "psql host   : " << config.psqlHost   << endl
         << "psql dbname : " << config.psqlDbName << endl
         << "psql login  : " << config.psqlLogin  << endl
         << flush;

    Server s(config);

    // Run the server until stopped.
    s.run();

    return 0;
}