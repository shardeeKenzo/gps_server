#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <boost/shared_ptr.hpp>

#include "DataTypes.h"

using namespace std;

class Parser
{
public:
    Parser() {};
    virtual ~Parser() {};

    virtual Maps   parse(const string& aMessage)       = 0;
    virtual string answer(int aReqType, short aResult) = 0;
};

typedef boost::shared_ptr<Parser> Parser_ptr;

#endif // PARSER_H
//
//
//
