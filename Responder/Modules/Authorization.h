#ifndef AUTHORIZATION_H
#define AUTHORIZATION_H

#include <string>
#include <pqxx/pqxx>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

#include "DataTypes.h"

#define COOKIE_RESET_PERIOD 7200 // 2 hours

using namespace std;

class Authorization
{
public:

    Authorization(
        pqxx::connection* readCon,
        pqxx::connection* writeCon,
        boost::mutex* mutex
    );
    ~Authorization();


    static string generateSalt();
           bool   authorize(const string& aLogin, const string& aPassword);
           string generateGeodinAnswer(long anAccID, string aLogin);
           long   getUserID(const string& aLogin);

    void runCookieCleaner();

    CookieMap         _cookieMap;

private:
    pqxx::connection* _readCon;
    pqxx::connection* _writeCon;

    boost::mutex*     _mutex;
};

typedef boost::shared_ptr<Authorization> Auth_ptr;

#endif // AUTHORIZATION_H

//
//
//
