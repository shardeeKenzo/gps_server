#include "UriParser.h"

#include <boost/algorithm/string.hpp>
#include <time.h>
#include <iostream>

UriParser::UriParser() {
    // EMPTY CONSTRUCTOR
}


string
UriParser::answer(int /*aReqType*/, short /*aResult*/) {
    // PLACEHOLDER
    return string("");
}

Maps
UriParser::parse(const string& aString) {
    Maps             maps;
    vector< string > splitted;

    boost::split(
          splitted
        , aString
        , boost::is_any_of("?")
    );

    if (splitted.size() < 2) return maps;
    // NOTREACHED

    if (!splitted[0].compare("/GetDevices")) parseGetDevices(splitted[1], maps);
    else
    if (!splitted[0].compare("/GetTrack")) {
        parseGetTrack(splitted[1], maps);
        // if end of period wasn't given, we use current time
        if (!maps[0].count("finishTime")) {
            string buff = to_string(time(NULL) - timezone);
            maps[0]["finishTime"] = buff;
        }
    }
    else
    if (!splitted[0].compare("/GetAllTracks")) {
        parseGetAllTracks(splitted[1], maps);
    }

    return maps;
}

/*!
 * Get the value of parameter which looks like ".....param=value&..." in string
 * where token = "param=", delim = "&"
 *
 * \param str   - string we take "token" from
 * \param token - name of token we are looking for
 * \param len   - length of token in str
 * \param res   - resulting string
 * \throws    0 - token was successfully parsed
 *            1 - invalid inputs
 */
void
UriParser::getStrToken(
      const string& str
    , const string& token
    , const string& delim
    ,       string& res
)
{
    if (str.empty() || delim.empty() || token.empty()) {
        cerr << "\nERROR: parseStrToken: inputs are invalid\n";
        throw 1;
        // NOTREACHED
    }
    short  len    = 0;
    size_t pos    = 0;
    size_t delPos = 0;

    pos = str.find(token);
    if (string::npos != pos) {
        pos += token.length();
    } else {
        res = string("");
        return;
        // NOTREACHED
    }

    // looking for a delimeter
    delPos = str.find(delim, pos);
    // assuming it could be the last parameter
    if (-1 == pos)
        len = str.length() - pos;
    else
        len = delPos - pos;

    res = str.substr(pos, len);
}

/*!
 * \brief UriParser::parseGetDevices
 * \param aString - string to parse
 * \param theMaps - will contain the resulting vector of maps
 * \return 0 - success
 *         1 - error
 */
short
UriParser::parseGetDevices(const string& aString, Maps& theMaps) {
    TokenMap map;
    string   buff;

    try {
        getStrToken(aString, "login="     , "&", buff);
        map["login"]    = buff;

        getStrToken(aString, "password="  , "&", buff);
        map["password"] = buff;

        map["requestType"] = "GetDevices";

        theMaps.push_back(map);
    } catch(int e) { return 1; }
    // NOTREACHED

    return 0;
}

/*!
 * \brief UriParser::parseGetTrack
 * \param aString - string to parse
 * \param theMaps - will contain the resulting vector of maps
 * \return 0 - success
 *         1 - error
 */
short
UriParser::parseGetTrack(const string& aString, Maps& theMaps) {
    TokenMap map;
    string   buff;

    try {
        getStrToken(aString, "id="        , "&", buff);
        if (buff.length()) map["id"]         = buff;

        getStrToken(aString, "startTime=" , "&", buff);
        if (buff.length()) map["startTime"]  = buff;

        getStrToken(aString, "finishTime=", "&", buff);
        if (buff.length()) map["finishTime"] = buff;

        map["requestType"] = "GetTrack";

        theMaps.push_back(map);
    } catch(int e) { return 1; }
    // NOTREACHED

    return 0;
}


short
UriParser::parseGetAllTracks(const string& aString, Maps& theMaps) {
    TokenMap map;
    string   buff;

    try {
        getStrToken(aString, "idsAndStartTimes=" , "&", buff);
        if (buff.length()) map["idsAndStartTimes"]  = buff;

        map["requestType"] = "GetAllTracks";

        theMaps.push_back(map);
    } catch(int e) { return 1; }
    // NOTREACHED

    return 0;
}

//
//
//
