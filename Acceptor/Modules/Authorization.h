#ifndef AUTHORIZATION_H
#define AUTHORIZATION_H

#include <string>
#include <memory>
#include <mutex>
#include "DataTypes.h"
#include "PSQLHandler.h"

/// Manages device authorization and auto‑registration using PSQLHandler.
class Authorization
{
public:
    /**
     * @param handler Shared PSQLHandler for database operations.
     * @param mtx     Pointer to a std::mutex for serializing DB access.
     * @throws std::invalid_argument if handler or mtx is null.
     */
    Authorization(std::shared_ptr<PSQLHandler> handler,
                  std::mutex*                  mtx);

    ~Authorization() = default;

    /**
     * @brief Authorize a device by IMEI and password.
     *        If IMEI not known, auto-registers a new transport.
     * @param imei     Device IMEI string.
     * @param password Plaintext password.
     * @return true if authorization (or auto-registration) succeeded.
     * @throws std::runtime_error on DB errors.
     */
    bool authorize(const std::string& imei,
                   const std::string& password);

    bool isAuthorized() const { return _authorized; }
    int  transportID()  const { return _transportID; }
    const std::string& imei() const { return _imei; }

private:
    std::shared_ptr<PSQLHandler> _psql;  ///< Handler for DB calls
    std::mutex*                  _mutex; ///< Mutex for DB access

    bool         _authorized{false};     ///< Authorization flag
    int          _transportID{-1};       ///< Transport ID from DB
    std::string  _imei;                  ///< Last IMEI authorized;
};

using Auth_ptr = std::shared_ptr<Authorization>;

#endif // AUTHORIZATION_H

