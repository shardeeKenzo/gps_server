#ifndef AUTHORIZATION_H
#define AUTHORIZATION_H

#include <string>
#include <pqxx/pqxx>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

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

    bool          authorize(string anImei, string aPassword);

    bool          _authorized;
    int           _transportID;
    string        _imei;

private:
    pqxx::connection* _readCon;
    pqxx::connection* _writeCon;

    boost::mutex* _mutex;
};

typedef boost::shared_ptr<Authorization> Auth_ptr;

#endif // AUTHORIZATION_H

//
//
//
