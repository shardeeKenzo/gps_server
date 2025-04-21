#ifndef PING_PARSER_H
#define PING_PARSER_H

#include <string>

#include "DataTypes.h"

class PingParser
{
public:
    PingParser();
    virtual ~PingParser();

    static TokenMap parse(string aString);
    static string   answer(short);
};

#endif // PING_PACKAGE_H

//
//
//
