#ifndef AUTHORIZATION_H
#define AUTHORIZATION_H

#include <string>
#include <pqxx/pqxx>
#include <mutex>

class Authorization
{
public:

    /// @param readCon   Shared connection for lookups.
    /// @param writeCon  Shared connection for inserts/updates.
    /// @param mtx       Pointer to a std::mutex guarding DB access.
    Authorization(pqxx::connection* readCon,
                  pqxx::connection* writeCon,
                  std::mutex*       mtx);

    ~Authorization();

    /// Returns true if device with IMEI+password is authorized.
    bool authorize(const std::string& imei,
                   const std::string& password);

    bool _authorized { false };
    int  _transportID { -1 };
    std::string _imei;

private:
    pqxx::connection* _readCon;
    pqxx::connection* _writeCon;

    std::mutex* _mutex;;
};

using Auth_ptr = std::shared_ptr<Authorization>;

#endif // AUTHORIZATION_H

