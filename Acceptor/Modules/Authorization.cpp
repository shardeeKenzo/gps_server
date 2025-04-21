#include "Authorization.h"
#include "PSQLHandler.h"
#include "utils.hpp"
#include "Logger.hpp"

#include <crypt.h>
#include <iostream>

Authorization::Authorization(
    pqxx::connection* readCon,
    pqxx::connection* writeCon,
    std::mutex*       mtx)
  : _readCon{ readCon }
  , _writeCon{ writeCon }
  , _mutex{ mtx }
{
    if (!_readCon || !_writeCon || !_mutex) {
        throw std::invalid_argument("Authorization: null pointer in ctor");
    }
}

Authorization::~Authorization() = default;

/*!
 * \brief Authorization::authorize
 * \param imei
 * \param password
 * \return
 */
bool Authorization::authorize(const std::string& imei,
                              const std::string& password)
{
    string dbHash, resultHash;

    std::lock_guard<std::mutex> lock(*_mutex);

    work xact(*_readCon);
    AuthData data = PSQLHandler::getCryptedPassword(imei, xact);
    
    // -- заплатка - добавление записи если нету ----------------------
    
    if(data.transportID == -1) {
        data = PSQLHandler::addSensor(imei, xact);
	work xact2(*_readCon);//xact was closed after comit
	data = PSQLHandler::getCryptedPassword(imei, xact2);
    }
    
//    dbHash  = data.passwordHash;
    
    _imei   = imei;
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

    const bool res = data.transportID != -1;
    
    if (!res) pushError(WRONG_PASSWORD, "Authorization::authorize");
    else      _authorized = true;
    
    return res;
}
