#ifndef PSQLHANDLER_H
#define PSQLHANDLER_H

#include <string>
#include <vector>
//#include <tuple>
#include <pqxx/pqxx>

#include "DataTypes.h"

using namespace std;
using namespace pqxx;

#define PACKSIZE 1000

struct AuthData {
    int    userID
         , sensID;
    string passwordHash;
};

class PSQLHandler
{
public:
     PSQLHandler();
    ~PSQLHandler();

    static short connectToDB(
          const string& aHostName
        , const string& aUserName
        , const string& aDBName
        , const string& aDBPass
        , pqxx::connection** con
    );

    static long getUserID(const string& aLogin, work& xact);
    static string getCryptedPassword(const string& aLogin, work& xact);
    static Track getTrack(
          long  anID
        , long  aPeriodStart
        , long  aPeriodEnd
        , work& xact
    );
    static void getAllTracks(const vector<pair<long,long>> &idsAndStartTimes, vector<std::tuple<long,long,Track>> &tracks, work& xact);

    static Sensors getSensors(long anAccID, work& xact);
    static Point   getLastSensorData(long aSensID, work& xact);
};

#endif // PSQLHANDLER_H

//
//
//
