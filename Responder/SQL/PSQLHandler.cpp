#include <pqxx/pqxx>
#include <iostream>
#include <assert.h>
#include <time.h>
#include <set>

#include "PSQLHandler.h"
#include "utils.hpp"
#include "DataTypes.h"

using namespace std;
using namespace pqxx;


PSQLHandler::PSQLHandler() {
    // EMPTY CONSTRUCTOR
}

PSQLHandler::~PSQLHandler() {
    // EMPTY
}

/*!
 * \brief PSQLHandler::connectToDB
 * \param aHostName
 * \param aUserName
 * \param aDBName
 * \param aDBPass
 * \return
 */
short
PSQLHandler::connectToDB(
      const string& aHostName
    , const string& aUserName
    , const string& aDBName
    , const string& aDBPass
    , pqxx::connection** aCon
)
{
    if (aDBName.empty()) {
        cerr << "PSQLHandler::connectToDB: aDBName can not be empty" << endl;
        return 1;
        // NOTREACHED
    }

    string conStr("dbname=" + aDBName);
    if (!aHostName.empty()) conStr.append(" host="     + aHostName);
    if (!aUserName.empty()) conStr.append(" user="     + aUserName);
    if (!aDBPass.empty())   conStr.append(" password=" + aDBPass  );

    try {
        *aCon = new connection(conStr);
    } catch (const sql_error &e) {
        cerr << e.what() << endl;
        return 2;
        // NOTREACHED
    }

    return 0;
}

long
PSQLHandler::getUserID(const string& aLogin, work& xact) {
    stringstream           query;
    result                 res;
    result::const_iterator row;
    long                   id = -1;

    query << "SELECT ID FROM accounts WHERE login = '" << aLogin << "';";

    try                        { res = xact.exec(query);   }
    catch (const sql_error &e) { cerr << e.what() << endl; }

    if (!res.size()) return id;
    // NOTREACHED

    row = res.begin();

    id  = row["ID"].as<long>();

    return id;
}

/*!
 * \brief PSQLHandler::getCryptedPassword
 * \param anImei
 * \param xact
 * \return hash of user password and his id in db
 */
string
PSQLHandler::getCryptedPassword(const string& aLogin, work &xact)
{
    stringstream           query;
    result                 res;
    result::const_iterator row;
    string                 passwordHash;

    query << "SELECT password FROM accounts WHERE login = '" << aLogin << "';";

    try                        { res = xact.exec(query);   }
    catch (const sql_error &e) { cerr << e.what() << endl; }

    if (!res.size()) return passwordHash;
    // NOTREACHED

    row = res.begin();

    passwordHash = row["password"].as<string>();

    return passwordHash;
}

/*!
 * \brief PSQLHandler::getTrack
 * \param anImei
 * \param aPeriodStart
 * \param aPeriodEnd
 * \return empty vector on error or filled vector of points
 */
Track
PSQLHandler::getTrack(
      long  anID
    , long  aPeriodStart
    , long  aPeriodEnd
    , work& xact
)
{
    stringstream           query;
    result                 res;
    result::const_iterator row;
    Track                  track;
    Point                  p;

    query   << "SELECT lat, lon, alt, time, speed, direction, satcnt, signal "
    << endl << "FROM data WHERE "
    << endl << "sensID = "     << anID         << " AND "
    << endl << "time BETWEEN " << aPeriodStart << " AND " << aPeriodEnd << ";";

    try                        { res = xact.exec(query);   }
    catch (const sql_error &e) { cerr << e.what() << endl; }

    for (row = res.begin(); row != res.end(); ++row) {
        p.lat       = row["lat"].as<int>();
        p.lon       = row["lon"].as<int>();
        p.alt       = row["alt"].as<int>();
        p.direction = row["direction"].as<int>();
        p.speed     = row["speed"].as<int>();
        p.time      = row["time"].as<int>();
        p.satcnt    = row["satcnt"].as<int>();
        p.signal    = row["signal"].as<bool>();

        track.push_back(p);
    }

    return track;
}

void 
PSQLHandler::getAllTracks(const vector<pair<long,long>> &idsAndStartTimes, vector<std::tuple<long,long,Track>> &tracks, work& xact)
{
    stringstream           query;
    result                 res;
    //result::const_iterator row;
    Track                  track;
    Point                  p;
    long                   curID=0;
    long                   curTimeUpl;

    query   << "SELECT sensID, lat, lon, alt, time, speed, direction, satcnt, signal,timeupl "
    << endl << "FROM data WHERE false ";
    for(const auto &tm : idsAndStartTimes)
        query << endl << "OR (sensID = " << tm.first << " AND timeupl > " << tm.second << ") ";
    query << endl << "ORDER BY sensID, timeupl;";

    
    try                        { res = xact.exec(query);  }
    catch (const sql_error &e) { cout << e.what() << endl; } 
    cout << "query ok" << endl;
    try{
    for(const auto row : res)
    {
        if(!curID)
            curID=row["sensID"].as<long>();
        else if(curID!=row["sensID"].as<long>())
        {
            tracks.emplace_back(curID, curTimeUpl, track);
            track = Track();
            curID=row["sensID"].as<long>();
        }
        p.lat       = row["lat"].as<int>();
        p.lon       = row["lon"].as<int>();
        p.alt       = row["alt"].as<int>();
        p.direction = row["direction"].as<int>();
        p.speed     = row["speed"].as<int>();
        p.time      = row["time"].as<int>();
        p.satcnt    = row["satcnt"].as<int>();
        p.signal    = row["signal"].as<bool>();
        curTimeUpl  = row["timeupl"].as<long>();
        track.push_back(p);
    }
    tracks.emplace_back(curID, curTimeUpl, track);
        
    }
    catch (const exception &e) { cout << e.what() << endl; }
    cout << tracks.size()<<"tracks" << endl;
}

Sensors
PSQLHandler::getSensors(
      long  anAccID
    , work& xact
)
{
    stringstream           query;
    result                 res;
    result::const_iterator row;
    Sensors                sensors;
    Sensor                 sensor;

    query   << "SELECT ID, imei  FROM sensors WHERE accID = "
            << anAccID << ";";

    try                        { res = xact.exec(query);   }
    catch (const sql_error &e) { cerr << e.what() << endl; }

    for (row = res.begin(); row != res.end(); ++row) {
        sensor.id   = row["ID"].as<long>();
        sensor.imei = row["imei"].as<string>();

        sensors.push_back(sensor);
    }

    return sensors;
}

Point
PSQLHandler::getLastSensorData(
      long  aSensID
    , work& xact
)
{
    stringstream           query;
    result                 res;
    result::const_iterator row;
    Point                  p;

    query << " SELECT lat, lon, alt, time, speed, direction, satcnt FROM data ";
    query << " WHERE (lat != 0 OR lon != 0) AND sensID = " << aSensID;
    //query << " WHERE sensID = " << aSensID;
    query << " ORDER BY time DESC LIMIT 1;";

    try                        { res = xact.exec(query);   }
    catch (const sql_error &e) { cerr << e.what() << endl; }

    if (!res.size()) return p;
    // NOTREACHED

    row = res.begin();

    p.lat       = row["lat"].as<int>();
    p.lon       = row["lon"].as<int>();
    p.alt       = row["alt"].as<int>();
    p.direction = row["direction"].as<int>();
    p.speed     = row["speed"].as<int>();
    p.time      = row["time"].as<int>();
    p.satcnt    = row["satcnt"].as<int>();

    return p;
}