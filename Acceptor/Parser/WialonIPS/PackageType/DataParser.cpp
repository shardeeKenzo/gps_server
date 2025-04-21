#include <boost/algorithm/string.hpp>

#include "DataParser.h"
#include "WialonIPS.h"

using namespace std;

DataParser::DataParser() {
    // EMPTY CONSTRUCTOR
}

DataParser::~DataParser() {

}

/*!
 * \brief DataParser::parseBase
 * \param theTokens
 * \return
 */
TokenMap
DataParser::parseBase(vector < string > theTokens) {
    TokenMap         tokenMap;
    int              pos
                   , len;
    string           date
                   , time
                   , lat
                   , lon;

    // after split it should be as follows(according to WialonIPS 1.1):
    // tokens[0]  - date
    // tokens[1]  - time
    // tokens[2]  - lat1
    // tokens[3]  - lat2
    // tokens[4]  - lon1
    // tokens[5]  - lon2
    // tokens[6]  - speed
    // tokens[7]  - course
    // tokens[8]  - height
    // tokens[9]  - sats

    // time --------------------------------------------------------------------
    date   = theTokens[0];
    time   = theTokens[1];

    tokenMap["time"] = to_string(WialonIPS::toPosixTime(date, time));

    // latlon ------------------------------------------------------------------
    lat = parseCoordinate(theTokens[2], LAT);
    // N = north = +
    // S = south = -
    if (!theTokens[3].compare("S")) lat.insert(0, "-");
    lon = parseCoordinate(theTokens[4], LON);
    // E = east = +
    // W = west = -
    if (!theTokens[5].compare("W")) lat.insert(0, "-");

    // an error has occured during coordinate parsing
    if (!lat.compare("0") || !lon.compare("0")) return tokenMap;
    // NOTREACHED

    tokenMap["lat"] = lat;
    tokenMap["lon"] = lon;
    // speed -------------------------------------------------------------------
    // km/h
    tokenMap["speed"]     = (theTokens[6].compare("NA"))
        ? theTokens[6]
        : "0";

    // course ------------------------------------------------------------------
    tokenMap["direction"] = (theTokens[7].compare("NA"))
        ? theTokens[7]
        : "0";

    // height ------------------------------------------------------------------
    tokenMap["alt"]       = (theTokens[8].compare("NA"))
        ? theTokens[8]
        : "0";

    // sats --------------------------------------------------------------------
    tokenMap["satcnt"]    = (theTokens[9].compare("NA"))
        ? theTokens[9]
        : "0";

    return tokenMap;
}

/*!
 * Parse full package of data(same as short, since we do
 * not use all its features)
 *
 * \param aString
 * \return
 */
TokenMap
DataParser::parseFull(string aString) {
    TokenMap         tokenMap;
    vector< string > tokens;
    int              pos
                   , len;

    // after split it should be as follows(according to WialonIPS 1.1):
    // tokens[0]  - date
    // tokens[1]  - time
    // tokens[2]  - lat1
    // tokens[3]  - lat2
    // tokens[4]  - lon1
    // tokens[5]  - lon2
    // tokens[6]  - speed
    // tokens[7]  - course
    // tokens[8]  - height
    // tokens[9]  - sats
    // tokens[10] - htop
    // tokens[11] - inputs
    // tokens[12] - outputs
    // tokens[13] - adc
    // tokens[14] - ibuttons
    // tokens[15] - params
    boost::split(
          tokens
        , aString
        , boost::is_any_of(";")
    );

     if (16 != tokens.size()) {
        tokenMap["error"] = "parsing";
        tokenMap["requestType"] = WialonIPS::reqTypeToString(
            WialonIPS::D_DATA
        );
        return tokenMap;
        // NOTREACHED
    }

    tokenMap = parseBase(tokens);

    if (7 == tokenMap.size()) {
        tokenMap["requestType"] = WialonIPS::reqTypeToString(
            WialonIPS::D_DATA
        );
    }

    return tokenMap;
}

/*!
 * \brief DataParser::parseShort
 * \param aString
 * \return
 */
TokenMap
DataParser::parseShort(string aString) {
    TokenMap         tokenMap;
    vector< string > tokens;
    int              pos
                   , len;

    // after split it should be as follows(according to WialonIPS 1.1):
    // tokens[0]  - date
    // tokens[1]  - time
    // tokens[2]  - lat1
    // tokens[3]  - lat2
    // tokens[4]  - lon1
    // tokens[5]  - lon2
    // tokens[6]  - speed
    // tokens[7]  - course
    // tokens[8]  - height
    // tokens[9]  - sats
    boost::split(
          tokens
        , aString
        , boost::is_any_of(";")
    );

    if (10 != tokens.size()) {
        tokenMap["requestType"] = WialonIPS::reqTypeToString(
            WialonIPS::SD_SHORT_DATA
        );
        tokenMap["error"] = "parsing";
        return tokenMap;
        // NOTREACHED
    }

    tokenMap = parseBase(tokens);

    if (7 == tokenMap.size()) {
        tokenMap["requestType"] = WialonIPS::reqTypeToString(
            WialonIPS::SD_SHORT_DATA
        );
    }

    return tokenMap;
}

/*!
 * \brief DataParser::parseCoordinate
 * \param aCoor
 * \param aType
 * \return
 */
string
DataParser::parseCoordinate(string aCoor, COORDINATE_TYPE aType) {
    if (!aCoor.compare("NA")) return "0";
    // NOTREACHED

    string sbuff;
    double dbuff;
    int    i, pos, len;

    // d - degree
    // m - minute
    // lat: ddmm.mmmmm   (ex: 5668.56745)
    // lon: dddmm.mmmmmm (ex: 03744.45623)
    // so we trim first two or three symbols depeds on coor type
    int toTrim = (LAT == aType) ? 2 : 3;

    sbuff  = aCoor.substr(toTrim, aCoor.length() - toTrim);

    aCoor.erase(toTrim, aCoor.length() - toTrim);
    //sbuff.erase(toTrim, 1);

    // consider leading zeros(0060000 -> xx600xx)
    //while ('0' == sbuff[0]) {
    //    sbuff.erase(0, 1);
    //    sbuff.erase(sbuff.length() - 1, 1);
    //}

    try {
        dbuff = stod(sbuff) / 60.;
    } catch(const std::invalid_argument e) {
        return "0";
        // NOTREACHED
    } catch(const std::out_of_range e) {
        return "0";
        // NOTREACHED
    }
    sbuff = to_string(dbuff);
    
    
    // removing dot and all before it
    while (sbuff[0] != '.') {
        sbuff.erase(0, 1);
    }
    // dot itself
    sbuff.erase(0, 1);
    
    // sbuff contains degrees / 100, so we need to make
    // sure that it is at least 3 characters long
    //while (sbuff.length() < 3) sbuff.insert(0, "0");
    
    // to keep 10E-6 precision we need to add zeros
    while (sbuff.length() < 6) sbuff.push_back('0');
    
    return aCoor + sbuff;
}

string
DataParser::answerShort(short aResult) {
    string answer;

    switch(aResult) {
        case 0:
            answer = "#ASD#-1\r\n";
            break;
        case 1:
            answer = "#ASD#1\r\n";
            break;
        default:
            answer = "#ASD#-1\r\n";
            break;
    }

    return answer;
}

string
DataParser::answerFull(short aResult) {
    string answer;

    switch(aResult) {
        case 0:
            answer = "#AD#-1\r\n";
            break;
        case 1:
            answer = "#AD#1\r\n";
            break;
        default:
            answer = "#AD#-1\r\n";
            break;
    }

    return answer;
}