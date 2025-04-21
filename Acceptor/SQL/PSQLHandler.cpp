#include <pqxx/pqxx>
#include <iostream>

#include "PSQLHandler.h"
#include "utils.hpp"
#include "Logger.hpp"

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

/*!
 * \brief PSQLHandler::getCryptedPassword
 * \param anImei
 * \param xact
 * \return hash of user password and his id in db
 */
AuthData PSQLHandler::getCryptedPassword(string anImei, work &xact) {
    stringstream           query;
    result                 res;
    result::const_iterator row;
    int                    transportID;
    AuthData               toReturn;

    toReturn.transportID       = -1;
    toReturn.passwordHash = "";

    // first - get account id from sensors table according to anImei
    query << "SELECT globalid FROM transports WHERE gd = '" << anImei << "'"
          << endl << "ORDER BY registration_time DESC"
          << endl << "LIMIT 1";

    try                        { res = xact.exec(query);   }
    catch (const sql_error &e) { cerr << e.what() << endl; }

    if (!res.size()) return toReturn;
    // NOTREACHED

    row    = res.begin();
//    accID  = row["accid"].as<int>();
    transportID = row["globalid"].as<int>();

//    query.str("");

    // second - get password from accounts table if imei was registered at all
//    query << "SELECT password FROM accounts WHERE ID = " << accID;

//    try                        { res = xact.exec(query);   }
//    catch (const sql_error &e) { cerr << e.what() << endl; }

//    if (!res.size()) return toReturn;
    // NOTREACHED

//    row = res.begin();

    toReturn.transportID       = transportID;
    toReturn.passwordHash = "";

    BOOST_LOG(logger) << "PSQLHandler::getCryptedPassword transportID " << transportID;

    return toReturn;
}

AuthData PSQLHandler::addSensor(string anImei, work &xact) {
    AuthData toReturn;
    
//    toReturn.userID = accID;
    toReturn.passwordHash = "";

    long now = time(0);
    
    stringstream query;
    query << " INSERT INTO transports(compname, id, gd, name, last_time, lat, lon, registration_time) ";
    query << " VALUES ('logiston', 0, '" << anImei << "', '', 0, 0, 0, " << now << ") ";
//    query << " INSERT INTO transports(compname, id, gd, name, last_time, lat, lon) ";
//    query << " VALUES ('logiston', 0, '" << anImei << "', '', 0, 0, 0 ) ";
    query << " RETURNING ID;";
    
    try {
        
        pqxx::result res = xact.exec(query);
        toReturn.transportID = res[0][0].as<int>();
        xact.commit();
    } catch (const sql_error &e) {
        BOOST_LOG(logger) << "Error: " << e.what();
        BOOST_LOG(logger) << "   SQL occure error: " << query.str() << endl; 
    }
    BOOST_LOG(logger) << "PSQLHandler::addSensor toReturn.transportID " << toReturn.transportID;

    
    return toReturn;
}


int PSQLHandler::getTransportID(string anImei, work &xact) {
    result res;
    result::const_iterator row;

    stringstream query;
    query << "SELECT globalid FROM transports WHERE gd = '" << anImei << "'";

    try {
        res = xact.exec(query);
    }
    catch (const sql_error &e) {
        cerr << e.what() << endl;
    }

    row = res.begin();
    return row["globalid"].as<int>();
}


/*!
 * \brief PSQLHandler::uploadPoints
 * \param thePoints
 * \param xact
 * \return 0
 */
short
PSQLHandler::uploadPoints(vector< Point > thePoints, work &xact) {
    stringstream           query;
    result                 res;
    result::const_iterator row;
    int                    i, len;
    Point*                 p;

    query   << "INSERT INTO points("
    << endl << "transpid, lat, lon, alt, time, speed, direction, sc, signal"
    << endl << ") VALUES"
    << endl;

    len = thePoints.size();
    
//    long now = time(0);

    for (i = 0; i < len; i++) {
        p = &thePoints[i];

        if (0 != i) query << ",";

        query   << "("
        << endl << "  " << p->transportID
        << endl << ", " << p->lat
        << endl << ", " << p->lon
        << endl << ", " << p->alt
        << endl << ", " << p->time
        << endl << ", " << p->speed
        << endl << ", " << p->direction
        << endl << ", " << p->satcnt
        << endl << ", " << ((p->signal) ? "TRUE" : "FALSE")
//        << endl << ", " << now
        << endl << ")";
    }
    if (len > 0) {
        query << ";"
            << endl << "update transports set last_time=p.time, lat=p.lat, lon=p.lon, speed=p.speed, direction=p.direction "
            << endl << "from points p"
            << endl << "where transpid=globalid "
            << endl << "and p.time=(select time from points where transpid=globalid order by time desc limit 1)"
            << endl << "and p.time!=last_time;";
    }

    // DEBUG
    //cout << query.str() << endl;

    try { 
        res = xact.exec(query); 
        xact.commit();
    }
    catch (const sql_error &e) { 
        BOOST_LOG(logger) << getTimeString() << "Error: " << e.what();
        BOOST_LOG(logger) << "   SQL occure error: " << query.str() << endl;
    }

    return 0;
}

//
//
//
