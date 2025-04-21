#ifndef PSQLHANDLER_H
#define PSQLHANDLER_H

#include <string>
#include <vector>
#include <pqxx/pqxx>

#include "DataTypes.h"

using namespace std;
using namespace pqxx;

#define PACKSIZE 1000

struct AuthData {
    int   transportID;
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

    static AuthData getCryptedPassword(string anImei, work &xact);
    static AuthData addSensor(string anImei, work &xact);
    static int      getTransportID(string anImei, work &xact);
    static short    uploadPoints(vector< Point > thePoints, work &xact);
};

#endif // PSQLHANDLER_H

//
//
//
