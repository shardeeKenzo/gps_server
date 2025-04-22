#ifndef PSQLHANDLER_H
#define PSQLHANDLER_H

#include <memory>
#include <string>
#include <vector>
#include <pqxx/pqxx>
#include "DataTypes.h"

#define PACKSIZE 1000

struct AuthData {
    int   transportID;
    string passwordHash;
};

/// Handles PostgreSQL operations for transports and points.
class PSQLHandler
{
public:
    /**
    * @brief Construct with an existing pqxx::connection.
    * @param conn Shared pointer to an open pqxx::connection.
    * @throws std::invalid_argument if conn is null or not open.
    */
    explicit PSQLHandler(std::shared_ptr<pqxx::connection> conn);

    ~PSQLHandler() = default;

    /**
     * @brief Retrieves transport ID and password hash for the given IMEI.
     * @param imei Device IMEI string.
     * @param tx   Active transaction.
     * @return AuthData with transportID = -1 if not found.
     * @throws pqxx::sql_error on query failure.
     */
    AuthData fetchPassword(const std::string& imei, pqxx::work& tx);

    /**
     * @brief Inserts a new transport record with the given IMEI.
     * @param imei Device IMEI string.
     * @param tx   Active transaction.
     * @return AuthData containing the new transportID.
     * @throws pqxx::sql_error on insert failure.
     */
    AuthData addSensor(const std::string& imei, pqxx::work& tx);

    /**
     * @brief Looks up the transport ID for a given IMEI.
     * @param imei Device IMEI string.
     * @param tx   Active transaction.
     * @return transportID.
     * @throws std::runtime_error if not found, pqxx::sql_error on query failure.
     */
    int getTransportID(const std::string& imei, pqxx::work& tx);

    /**
     * @brief Bulk uploads a batch of points to the database using COPY.
     * @param points Vector of Point structs to upload.
     * @param tx     Active transaction.
     * @throws pqxx::sql_error on failure.
     */
    void uploadPoints(const std::vector<Point>& points, pqxx::work& tx);

    std::shared_ptr<pqxx::connection> connection() const { return conn_; }

private:
    std::shared_ptr<pqxx::connection> conn_;
};

#endif // PSQLHANDLER_H

