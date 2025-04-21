#include <crypt.h>
#include <time.h>
#include <iostream>

#include "rapidjson/document.h"     // rapidjson's DOM-style API
#include "rapidjson/prettywriter.h" // for stringify JSON
#include "rapidjson/filestream.h"
#include "rapidjson/stringwriter.h"

#include "Authorization.h"
#include "PSQLHandler.h"

#include <boost/thread/mutex.hpp>

#include "utils.hpp"

Authorization::Authorization(
    pqxx::connection* aReadCon,
    pqxx::connection* aWriteCon,
    boost::mutex* aMutex
)
{
    if (!aReadCon || !aWriteCon) {
        cerr << "    !Connection::Connection: An attempt to make a connection "
             << "with null pointer to pqxx::connection was made" << endl;
        exit(1);
        // TERMINATION
    }

    _readCon    = aReadCon;
    _writeCon   = aWriteCon;
    _mutex      = aMutex;
}

Authorization::~Authorization() {
    // EMPTY
}

/*!
 * \brief Authorization::authorize
 * \param anImei
 * \param aPassword
 * \return
 */
bool Authorization::authorize(const string& aLogin, const string& aPassword) {
    bool res;

//     boost::mutex::scoped_lock lock(*_mutex);
// 
//     if (string::npos    != aLogin.find(";")
//         || string::npos != aLogin.find("(")
//         || string::npos != aPassword.find(";")
//         || string::npos != aPassword.find("("))
//     {
//         cout << endl << " -- " << endl << " -- "
//         "    !possible hack attempt: " << aLogin << " " << aPassword << endl;
// 
//         return false;
//         // NOTREACHED
//     }

//     work xact(*_readCon);    
//     string dbHash = PSQLHandler::getCryptedPassword(aLogin, xact);
//     if (!dbHash.length()) return false;
//     string resultHash = crypt(aPassword.c_str(), dbHash.c_str());
//     res        = !dbHash.compare(resultHash);
    
    res = true; // -- ЗАГЛУШКА
    
    // DEBUG
    if (res) cout << endl << getTimeString() << " user " << aLogin
                  << " registered successfully"     << endl;
    else     cout << endl << getTimeString() << " user " << aLogin
                  << " provided wrong account data" << endl;
    
    return res;
}

/*!
 * \brief Authorization::generateSalt
 * \return
 */
string
Authorization::generateSalt() {
    // TODO: make more random salt!
    unsigned long seed[2];
    char salt[] = "$1$........";

    const char *const seedchars =
        "./0123456789ABCDEFGHIJKLMNOPQRST"
        "UVWXYZabcdefghijklmnopqrstuvwxyz";
    string passwordHash, anotherHash;

    int i;

    seed[0] = time(NULL);
    seed[1] = getpid() ^ (seed[0] >> 14 & 0x30000);

    /* Turn it into printable characters from `seedchars'. */
    for (i = 0; i < 8; i++)
        salt[3+i] = seedchars[(seed[i/5] >> (i%5)*6) & 0x3f];

    string res(salt);

    return res;
}

/*!
 * \param anAccID - account identifier
 * \return
 */
string
Authorization::generateGeodinAnswer(long anAccID, string aLogin) {
    using namespace rapidjson;

    boost::mutex::scoped_lock lock(*_mutex);

    work     xact(*_readCon);
    Sensors  sensors = PSQLHandler::getSensors(anAccID, xact);

    Document d;
    Value    items;
    int      i
           , len = sensors.size();
    Point    p;

    d.SetObject();
    items.SetArray();

    for (i = 0; i < len; i++) {
        p = PSQLHandler::getLastSensorData(sensors[i].id, xact);
        Value   obj
              , pos;

        obj.SetObject();

        // TODO: make it arch dependable
        obj.AddMember("id" , (int)sensors[i].id      , d.GetAllocator());
        obj.AddMember("gd" , sensors[i].imei.c_str() , d.GetAllocator());

        pos.SetObject();

        // TODO: make it arch dependable
        pos.AddMember("t"  , (int)p.time     , d.GetAllocator());
        pos.AddMember("s"  , p.speed         , d.GetAllocator());
        pos.AddMember("x"  , p.lat / 1000000., d.GetAllocator());
        pos.AddMember("y"  , p.lon / 1000000., d.GetAllocator());
        pos.AddMember("z"  , p.alt           , d.GetAllocator());
        pos.AddMember("sc" , p.satcnt        , d.GetAllocator());
        pos.AddMember("c"  , p.direction     , d.GetAllocator());

        obj.AddMember("pos", pos             , d.GetAllocator());

        items.PushBack(obj, d.GetAllocator());
    }

    d.AddMember("items", items          , d.GetAllocator());
    d.AddMember("login", aLogin.c_str() , d.GetAllocator());

    // Stringify json
    ostringstream outStream;
    StreamWriter<ostringstream> writer(outStream);
    d.Accept(writer);

    // Extract the message from the stream and format it.
    return outStream.str();
}

long
Authorization::getUserID(const string& aLogin) {
    boost::mutex::scoped_lock lock(*_mutex);

    work xact(*_readCon);
    return PSQLHandler::getUserID(aLogin, xact);
}

void
Authorization::runCookieCleaner() {
    for (;;) {
        sleep(COOKIE_RESET_PERIOD);

        _cookieMap.clear();
    }
}

//
//
//
