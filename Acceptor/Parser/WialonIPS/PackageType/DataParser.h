#ifndef DATA_PARSER_H
#define DATA_PARSER_H

#include "DataTypes.h"

using namespace std;

class DataParser
{
public:
    enum COORDINATE_TYPE {
          NO_TYPE
        , LAT
        , LON
    };

    DataParser();
    virtual ~DataParser();

    static TokenMap parseFull(string aString);
    static TokenMap parseShort(string aString);
    static TokenMap parseBase(vector < string > theTokens);
    static string   answerShort(short aResult);
    static string   answerFull(short aResult);

private:
    static string parseCoordinate(
          string          aCoordinate
        , COORDINATE_TYPE aType
    );
};

#endif // DATA_PARSER_H

//
//
//
