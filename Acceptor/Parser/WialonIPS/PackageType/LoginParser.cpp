#include <vector>
#include <boost/algorithm/string.hpp>

#include "WialonIPS.h"
#include "LoginParser.h"

#include "utils.hpp"

using namespace std;

LoginParser::LoginParser() {

}

LoginParser::~LoginParser() {

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
LoginParser::parse(string aString) {
    TokenMap         tokenMap;
    vector< string > tokens;
    int              pos
                   , len;
    string           imei
                   , password;

    // after split it should be as follows:
    // tokens[0] - imei
    // tokens[1] - password
    boost::split(
          tokens
        , aString
        , boost::is_any_of(";")
    );

    imei     = tokens[0];
    password = tokens[1];

    pos      = password.find("\r\n");
    len      = password.length();
    password = password.substr(0, len - (len - pos));

    if (!tokens[0].length()) pushError(EMPTY_IMEI    , "LoginParser::parse");
    if (!tokens[1].length()) pushError(EMPTY_PASSWORD, "LoginParser::parse");

    if (!tokens[0].length() || !tokens[1].length()) {
        return tokenMap;
        // NOTREACHED
    }

    tokenMap["imei"]        = tokens[0];
    tokenMap["password"]    = password;
    tokenMap["requestType"] = WialonIPS::reqTypeToString(
        WialonIPS::L_LOGIN
    );

    return tokenMap;
}

/*!
 * \brief LoginParser::answer
 * \param aResult
 * \return
 */
string
LoginParser::answer(short aResult) {
    string answer;
    switch(aResult) {
        case 0:
            answer = "#AL#01\r\n";
            break;
        case 1:
            answer = "#AL#1\r\n";
            break;
        case 2:
            answer = "#AL#0\r\n";
            break;
        default:
            answer = "#AL#0\r\n";
            break;
    }

    return answer;
}

//
//
//
