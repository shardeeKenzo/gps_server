#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include <map>
#include <vector>
#include <utility>
#include <tuple>
#include <string>
#include <time.h>

#include "ErrorCodes.h"

using namespace std;

typedef map   < string    , string >         TokenMap;
typedef vector< TokenMap >                   Maps;
typedef map   < ERROR_CODE, string >         ErrorMap;
// key - code, value - method where it happened
typedef tuple < ERROR_CODE, string, time_t > Error;
typedef vector< Error >                      Errors;

struct Point
{
    int   userID
        , transportID;

    int   lat
        , lon
        , alt
        , speed
        , direction
        , satcnt;

    long time;

    bool signal;

    Point() {
        userID    = 0;
        transportID    = 0;
        lat       = 0;
        lon       = 0;
        alt       = 0;
        speed     = 0;
        direction = 0;
        satcnt    = 0;
        time      = 0;
        signal    = 0;
    }
};

//struct Transport
//{
//    int globalID,
//        lat,
//        lon,
//        speed,
//        direction;
//
//    long last_time;
//
//    Transport(){
//        globalID = 0;
//        lat = 0;
//        lon = 0;
//        speed = 0;
//        direction = 0;
//        last_time = 0;
//    }
//};

#endif // DATA_TYPES_H

//
//
//
