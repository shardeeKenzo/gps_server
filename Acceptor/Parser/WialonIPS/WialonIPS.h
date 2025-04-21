#ifndef WIALONIPS_H
#define WIALONIPS_H

#include <string>
#include <map>
#include <vector>

#include "Parser.hpp"

#include "DataTypes.h"

using namespace std;

static bool _S_initialized = false;

class WialonIPS : public Parser
{
public:
    WialonIPS();

    virtual ~WialonIPS();

    enum REQUEST {
          UNDEFINED_REQ
        , L_LOGIN
        , D_DATA
        , SD_SHORT_DATA
        , P_PING
        , B_BLACK_BOX
    };

    enum ANSWER {
          UNDEFINED_ANS
        , AL_LOGIN_ANSWER
        , AD_DATA_ANSWER
        , ASD_SHORT_DATA_ANSWER
        , AP_PING_ANSWER
        , AB_BLACK_BOX_ANSWER
    };

    static REQUEST reqStringToType(string aValue);
    static string  reqTypeToString(REQUEST aValue);

    static ANSWER ansStringToType(string aValue);
    static string ansTypeToString(ANSWER aValue);

    static long   toPosixTime(string aDate, string aTime);

    Maps     parse (string aMessage);
    string   answer(int    aReqType, short aResult);

private:
    static std::map<string , REQUEST> _S_requestTypes;
    static std::map<string , ANSWER > _S_answerTypes;
    static std::map<REQUEST, string > _S_requestStrings;
    static std::map<ANSWER , string > _S_answerStrings;
    int daylight;

    static void initialize();
};

#endif // WIALONIPS_H

//
//
//
