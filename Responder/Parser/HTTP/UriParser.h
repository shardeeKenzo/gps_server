#ifndef URI_PARSER_H
#define URI_PARSER_H

#include <string>

#include "Parser.hpp"
#include "DataTypes.h"

using namespace std;

class UriParser : public Parser
{
public:
    UriParser();

    Maps   parse(const string& aString);
    string answer(int aReqType, short aResult);

    static
    void getStrToken(
          const string& str
        , const string& token
        , const string& delim
        ,       string& res
    );

    short parseGetDevices(
          const string& aString
        , Maps& theMaps
    );

    short parseGetTrack(
          const string& aString
        , Maps& theMaps
    );

    short parseGetAllTracks(
          const string& aString
        , Maps& theMaps
    );
};

#endif // URI_PARSER_HPP

//
//
//
