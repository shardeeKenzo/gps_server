#include <vector>
#include <boost/algorithm/string.hpp>

#include "PingParser.h"
#include "WialonIPS.h"

#include "utils.hpp"

using namespace std;

PingParser::PingParser() {
    // EMPTY CONSTRUCTOR
}

PingParser::~PingParser() {

}

/*!
 * \brief LoginParser::parse
 * \param aString
 * \return map of tokens parsed from aString or an empty map in case of error
 *
 * map keys:   imei
 *           , password
 *           , requestType /see WialonIPS::REQUEST
 */
TokenMap
PingParser::parse(string aString) {
    TokenMap         tokenMap;
    vector< string > tokens;

    // after split it should be as follows:
    boost::split(
          tokens
        , aString
        , boost::is_any_of(";")
    );

    tokenMap["requestType"] = WialonIPS::reqTypeToString(
        WialonIPS::P_PING
    );

    return tokenMap;
}

/*!
 * \brief PingParser::answer
 * \return
 */
string
PingParser::answer(short /* aResult */) {
    return "#AP#\r\n";
}

//
//
//
