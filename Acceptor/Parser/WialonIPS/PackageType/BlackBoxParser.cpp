#include <boost/algorithm/string.hpp>

#include "BlackBoxParser.h"
#include "DataParser.h"
#include "../../../Parser/WialonIPS/WialonIPS.h"

using namespace std;

BlackBoxParser::BlackBoxParser() {
    // EMPTY CONSTRUCTOR
}

BlackBoxParser::~BlackBoxParser() {

}

/*!
 * \brief BlackBoxParser::parse
 * \param aString
 * \return
 */
Maps
BlackBoxParser::parse(string aString) {
    TokenMap         buff;
    Maps             maps;
    vector< string > tokens;
    vector< string > subtokens;
    int              i
                   , pos
                   , len
                   , tokensCnt;
    string           lastToken
                   , token;

    // after split it should have short data packages:
    boost::split(
          tokens
        , aString
        , boost::is_any_of("|")
    );

    // last token would contain \r\n and we do not need it there
    tokensCnt = tokens.size();
    lastToken = tokens[tokensCnt - 1];

    pos       = lastToken.find("\r\n");
    len       = lastToken.length();
    lastToken = lastToken.substr(0, len - (len - pos));
    
    tokens[tokensCnt - 1] = lastToken;
    
    for (i = 0; i < tokensCnt; i++) {
        token = tokens[i];
        
        boost::split(
              subtokens
            , token
            , boost::is_any_of(";")
        );
        
        if (10 == subtokens.size())
            buff  = DataParser::parseShort(token);
        else
            buff  = DataParser::parseFull(token);
        
        buff["requestType"] = WialonIPS::reqTypeToString(
            WialonIPS::B_BLACK_BOX
        );
        
        maps.push_back(buff);
    }
    
    return maps;
}

/*!
 * \brief BlackBoxParser::answer
 * \param aResult
 * \return
 */
string
BlackBoxParser::answer(short aResult) {
    return string("#AB#" + to_string(aResult) + "\r\n");
}