#ifndef LOGIN_PARSER_H
#define LOGIN_PARSER_H

#include "DataTypes.h"

using namespace std;

class LoginParser
{
public:
    LoginParser();
    ~LoginParser();

    static TokenMap parse(string aString);
    static string   answer(short aResult);
};

#endif // LOGIN_PARSER_H

//
//
//
