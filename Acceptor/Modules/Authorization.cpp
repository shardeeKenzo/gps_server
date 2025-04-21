#include <crypt.h>
#include <time.h>
#include <iostream>

#include <boost/thread/mutex.hpp>

#include "Authorization.h"
#include "PSQLHandler.h"

#include "utils.hpp"

Authorization::Authorization(
    pqxx::connection* aReadCon,
    pqxx::connection* aWriteCon,
    boost::mutex* aMutex
)
{
    if (!aReadCon || !aWriteCon) {
        cerr << "Connection::Connection: An attempt to make a connection "
             << "with null pointer to pqxx::connection was made" << endl << flush;
        exit(1);
        // TERMINATION
    }

    _readCon    = aReadCon;
    _writeCon   = aWriteCon;
    _mutex      = aMutex;

    _authorized = false;

    _transportID = -1;
    _imei       = "";

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
bool Authorization::authorize(string anImei, string aPassword) {
    AuthData data;
    string dbHash, resultHash;

    boost::mutex::scoped_lock lock(*_mutex);

    work xact(*_readCon);
    data = PSQLHandler::getCryptedPassword(anImei, xact);
    
    // -- заплатка - добавление записи если нету ----------------------
    
    if(data.transportID == -1) {
        data = PSQLHandler::addSensor(anImei, xact);
	work xact2(*_readCon);//xact was closed after comit
	data = PSQLHandler::getCryptedPassword(anImei, xact2);
    }
    
//    dbHash  = data.passwordHash;
    
    _imei   = anImei;
    _transportID = data.transportID;

    
    // -- убираем авторизацию ------------------------------------------
    
//     if (!dbHash.length() || -1 == _userID) return false;
//     // NOTREACHED
//     
//     resultHash = crypt(aPassword.c_str(), dbHash.c_str());
//     
//     bool res = !dbHash.compare(resultHash);
//     
//     if (!res) pushError(WRONG_PASSWORD, "Authorization::authorize");
//     else      _authorized = true;
//     
//     // DEBUG
//     if (res) cout << "device " << anImei
//                   << " registered successfully"     << endl;
//     else     cout << "device " << anImei
//                   << " provided wrong account data" << endl;
    
    bool res = data.transportID != -1;
    
    if (!res) pushError(WRONG_PASSWORD, "Authorization::authorize");
    else      _authorized = true;
    
    return res;
}
