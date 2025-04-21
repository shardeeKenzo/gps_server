#ifndef UTILS_H
#define UTILS_H

#include <fstream>
#include <sstream>
#include <time.h>
#include <stdlib.h>
#include <boost/program_options.hpp>
#include "Connection.h"

#include "Globals.hpp"

using namespace std;
namespace po = boost::program_options;

inline
void
pushError(ERROR_CODE aCode, string aString) {
    _G_errors.push_back(
        make_tuple(
              aCode
            , aString
            , time(NULL)
        )
    );
}

inline
void
fillErrorMap() {
    _G_errorMap[NO_ERROR]       = "No error occurred";
    _G_errorMap[UNKNOWN_ERROR]  = "Unknown error";
    _G_errorMap[EMPTY_IMEI]     = "given IMEI is empty";
    _G_errorMap[WRONG_IMEI]     = "IMEI doesn't match any entry in database";
    _G_errorMap[EMPTY_PASSWORD] = "given password is empty";
    _G_errorMap[WRONG_PASSWORD] = "password doesnt match given IMEI";
}

inline
void
readConfig(
      po::options_description& aDesc
    , po::variables_map& aMap
)
{
    std::ifstream file("config.ini");

    if (!file.is_open()) return;
    // NOTREACHED

    // Clear the map.
    //aMap = po::variables_map();

    po::store(po::parse_config_file(file, aDesc, true), aMap);
    file.close();
    po::notify(aMap);
}

inline
string
getTimeString() {
    char strtime[30];

    time_t rawtime; time( &rawtime );
    struct tm * timeinfo = localtime ( &rawtime );
    strftime (strtime, 30, "[%d.%m.%y %H:%M:%S] ", timeinfo);

    return string(strtime);
}

inline
string
getTimeString(time_t rawtime) {
    char strtime[30];

    struct tm * timeinfo = localtime ( &rawtime );
    strftime (strtime, 30, "[%d.%m.%y %H:%M:%S] ", timeinfo);

    return string(strtime);
}

#endif // UTILS_H

//
//
//
