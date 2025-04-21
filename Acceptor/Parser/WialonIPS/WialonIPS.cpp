#include <iostream>
#include <boost/array.hpp>
#include <boost/algorithm/string.hpp>

#include <time.h>

#include "WialonIPS.h"
#include "LoginParser.h"
#include "PingParser.h"
#include "DataParser.h"
#include "BlackBoxParser.h"

using namespace std;

map<string, WialonIPS::REQUEST> WialonIPS::_S_requestTypes;
map<string, WialonIPS::ANSWER > WialonIPS::_S_answerTypes;
map<WialonIPS::REQUEST, string> WialonIPS::_S_requestStrings;
map<WialonIPS::ANSWER , string> WialonIPS::_S_answerStrings;

WialonIPS::WialonIPS() {
    // EMPTY CONSTRUCTOR
}

WialonIPS::~WialonIPS() {

}

/*!
 * \brief WialonIPS::parse
 * \param aMessage
 * \return
 */
Maps
WialonIPS::parse(string aMessage) {
    Maps maps;

    if (!aMessage.length()) return maps;
    // NOTREACHED

    vector< string > splitted;

    // after split it should be as follows:
    // splitted[0] - nothing, since package starts with #
    // splitted[1] - package type /see WialonIPS::REQUEST
    // splitted[2] - package content. Depends on package type
    boost::split(
          splitted
        , aMessage
        , boost::is_any_of("#")
    );

    if (splitted.size() < 2) return maps;
    // NOTREACHED

    REQUEST id = reqStringToType(
        splitted[1]
    );

    switch(id) {
        case P_PING :
            maps.push_back(PingParser::parse(splitted[2]));
            break;
        case L_LOGIN :
            maps.push_back(LoginParser::parse(splitted[2]));
            break;
        case SD_SHORT_DATA :
            maps.push_back(DataParser::parseShort(splitted[2]));
            break;
        case D_DATA :
            maps.push_back(DataParser::parseFull(splitted[2]));
            break;
        case B_BLACK_BOX:
            maps = BlackBoxParser::parse(splitted[2]);
            break;
        case UNDEFINED_REQ :
            cerr << "WialonIPS::parseMessage: Undefined message type, message:"
                 << aMessage << endl;
        default:
            cerr << "Unpredicted situation has occurred "
                 << "in WialonIPS::parseMessage!" << endl;
            break;
    }

    return maps;
}

/*!
 * \brief WialonIPS::answer
 *
 * \param aReqType request type /see WialonIPS::REQUEST
 * \param aResult  0 - wrong password
 *                 1 - successful authorization
 *                 2 - deny connection
 * \return         formatted std::string
 */
string
WialonIPS::answer(int aReqType, short aResult) {
    string answer;

    switch (aReqType) {
        case L_LOGIN:
            answer = LoginParser::answer(aResult);
            break;
        case P_PING:
            answer = PingParser::answer(aResult);
            break;
        case D_DATA:
            answer = DataParser::answerFull(aResult);
            break;
        case SD_SHORT_DATA:
            answer = DataParser::answerShort(aResult);
            break;
        case B_BLACK_BOX:
            answer = BlackBoxParser::answer(aResult);
            break;
        default:
            answer = "";
            break;
    }

    return answer;
}

/*!
 * \brief WialonIPS::initialize
 */
void
WialonIPS::initialize() {
    if (_S_initialized) return;
    // NOTREACHED

    _S_requestTypes["L"]  = L_LOGIN;
    _S_requestTypes["D"]  = D_DATA;
    _S_requestTypes["SD"] = SD_SHORT_DATA;
    _S_requestTypes["B"]  = B_BLACK_BOX;
    _S_requestTypes["P"]  = P_PING;

    _S_answerTypes["AL"]  = AL_LOGIN_ANSWER;
    _S_answerTypes["AD"]  = AD_DATA_ANSWER;
    _S_answerTypes["ASD"] = ASD_SHORT_DATA_ANSWER;
    _S_answerTypes["AB"]  = AB_BLACK_BOX_ANSWER;
    _S_answerTypes["AP"]  = AP_PING_ANSWER;

    _S_requestStrings[L_LOGIN]       = "L";
    _S_requestStrings[D_DATA]        = "D";
    _S_requestStrings[SD_SHORT_DATA] = "SD";
    _S_requestStrings[B_BLACK_BOX]   = "B";
    _S_requestStrings[P_PING]        = "P";

    _S_answerStrings[AL_LOGIN_ANSWER]       = "AL";
    _S_answerStrings[AD_DATA_ANSWER]        = "AD";
    _S_answerStrings[ASD_SHORT_DATA_ANSWER] = "ASD";
    _S_answerStrings[AB_BLACK_BOX_ANSWER]   = "AB";
    _S_answerStrings[AP_PING_ANSWER]        = "AP";

    _S_initialized = true;
}

/*!
 * \brief WialonIPS::reqStringToType
 * \param aValue
 * \return
 */
WialonIPS::REQUEST
WialonIPS::reqStringToType(string aValue) {
    if(!_S_initialized) initialize();

    try {
        return _S_requestTypes.at(aValue);
        // NOTREACHED
    }
    catch(const out_of_range& oor) {
        return UNDEFINED_REQ;
        // NOTREACHED
    }
}

/*!
 * \brief WialonIPS::reqTypeToString
 * \param aValue
 * \return
 */
string
WialonIPS::reqTypeToString(REQUEST aValue) {
    if(!_S_initialized) initialize();

    try {
        return _S_requestStrings.at(aValue);
        // NOTREACHED
    }
    catch(const std::out_of_range& oor) {
        return "UNDEFINED_REQ";
        // NOTREACHED
    }
}

/*!
 * \brief WialonIPS::ansStringToType
 * \param aValue
 * \return
 */
WialonIPS::ANSWER
WialonIPS::ansStringToType(string aValue) {
    if(!_S_initialized) initialize();

    try {
        return _S_answerTypes.at(aValue);
        // NOTREACHED
    }
    catch(const std::out_of_range& oor) {
        return UNDEFINED_ANS;
        // NOTREACHED
    }
}

/*!
 * \brief WialonIPS::ansTypeToString
 * \param aValue
 * \return
 */
string
WialonIPS::ansTypeToString(ANSWER aValue) {
    if(!_S_initialized) initialize();

    try {
        return _S_answerStrings.at(aValue);
        // NOTREACHED
    }
    catch(const std::out_of_range& oor) {
        return "UNDEFINED_ANS";
        // NOTREACHED
    }
}

/*!
 * \brief WialonIPS::toPosixTime
 * \param aDate
 * \param aTime
 * \return
 */
long WialonIPS::toPosixTime(string aDate, string aTime) {
    time_t    curTime    = time(NULL);

    if (!aDate.compare("NA") || 6 != aDate.length()) {
        return curTime;
        // NOTREACHED
    }


    if (!aTime.compare("NA") || aTime.empty()) {
        aTime = "000000";
    }
    
    string    givenYear    = aDate.substr(4, 2);
    string    givenMonth   = aDate.substr(2, 2);
    string    givenDay     = aDate.substr(0, 2);

    string    givenHours   = aTime.substr(0, 2);
    string    givenMinutes = aTime.substr(2, 2);
    string    givenSeconds = aTime.substr(4, 2);
    
    struct tm curTimeS   = *localtime(&curTime);
    string year = std::to_string(curTimeS.tm_year + 1900);
    givenYear = year.substr(0, 2) + givenYear;
    
    
    struct tm t;
    t.tm_year  = stoi(givenYear)  - 1900; // starts with 1900
    t.tm_mon   = stoi(givenMonth) -    1; // starts with 0
    t.tm_mday  = stoi(givenDay);
    t.tm_hour  = stoi(givenHours)  ;      // starts with 0
    t.tm_min   = stoi(givenMinutes);      // starts with 0
    t.tm_sec   = stoi(givenSeconds);      // starts with 0
    
    // t.tm_isdst = 0;                    // Is DST on? 1 = yes, 0 = no, -1 = unknown

//     char *tz;
// 
//     tz = getenv("TZ");
//     setenv("TZ", "", 1);
//     tzset();
    
    // long res = mktime(&t);
    long res = timegm(&t);
    
//     if (tz)
//        setenv("TZ", tz, 1);
//     else
//        unsetenv("TZ");
//     tzset();
    
    return res;
}