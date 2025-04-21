#ifndef BLACK_BOX_PARSER_H
#define BLACK_BOX_PARSER_H

#include "DataTypes.h"

using namespace std;

class BlackBoxParser
{
public:

    BlackBoxParser();
    virtual ~BlackBoxParser();

    static Maps     parse(string aString);
    static string   answer(short aResult);
};

#endif // BLACK_BOX_PARSER_H

//
//
//
