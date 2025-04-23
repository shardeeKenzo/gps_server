#include "Authorization.h"
#include "Logger.hpp"
#include "utils.hpp"

#include <stdexcept>
#include <crypt.h>

Authorization::Authorization(std::shared_ptr<PSQLHandler> handler,
                             std::mutex*                  mtx)
  : _psql{std::move(handler)}, _mutex{mtx}
{
    if (!_psql || !_mutex) {
        throw std::invalid_argument("Authorization: handler or mutex is null");
    }
}

/*!
 * \brief Authorization::authorize
 * \param imei
 * \param password
 * \return
 */
bool Authorization::authorize(const std::string& imei,
                              const std::string& password)
{
    std::lock_guard lock(*_mutex);

    const auto conn = _psql->connection();
    pqxx::work tx(*conn);

    // 1) Fetch or create transport
    AuthData data = _psql->fetchPassword(imei, tx);
    if (data.transportID < 0) {
        data = _psql->addSensor(imei, tx);
    }

    // (Placeholder) password verification logic
    // std::string dbHash = data.passwordHash;
    // std::string resultHash = crypt(password.c_str(), dbHash.c_str());
    // bool ok = (dbHash == resultHash);
    bool ok = (data.transportID >= 0);

    if (!ok) {
        pushError(WRONG_PASSWORD, "Authorization::authorize");
        tx.abort();
        return false;
    }

    tx.commit();

    _imei = imei;
    _transportID = data.transportID;
    _authorized = true;

    BOOST_LOG(logger) << "Authorized IMEI " << imei
                      << " as transport " << _transportID;
    return true;
}
