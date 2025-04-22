#include "PSQLHandler.h"
#include "Logger.hpp"
#include <stdexcept>

PSQLHandler::PSQLHandler(std::shared_ptr<pqxx::connection> conn)
    : conn_(std::move(conn))
{
    if (!conn_ || !conn_->is_open()) {
        throw std::invalid_argument("PSQLHandler: Invalid or closed connection");
    }

    conn_->prepare("fetch_pw",
                   "SELECT globalid, password FROM transports "
                   "WHERE gd = $1 ORDER BY registration_time DESC LIMIT 1");
    conn_->prepare("add_sensor",
                   "INSERT INTO transports(compname, id, gd, name, last_time, lat, lon, registration_time) "
                   "VALUES ('logiston', 0, $1, '', 0, 0, 0, $2) RETURNING globalid");
    conn_->prepare("get_trans_id",
                   "SELECT globalid FROM transports WHERE gd = $1");
    conn_->prepare(
    "insert_point",
    "INSERT INTO points(transpid,lat,lon,alt,time,speed,direction,sc,signal) "
    "VALUES($1,$2,$3,$4,$5,$6,$7,$8,$9)"
);
}

AuthData PSQLHandler::fetchPassword(const std::string& imei, pqxx::work& tx) {
    AuthData result{ -1, "" };
    pqxx::result r = tx.exec_prepared("fetch_pw", imei);
    if (r.empty()) return result;
    result.transportID = r[0]["globalid"].as<int>();
    result.passwordHash = r[0]["password"].c_str();
    BOOST_LOG(logger) << "Fetched password for transportID " << result.transportID;
    return result;
}

AuthData PSQLHandler::addSensor(const std::string& imei, pqxx::work& tx) {
    long now = std::time(nullptr);
    pqxx::result r = tx.exec_prepared("add_sensor", imei, now);
    if (r.empty()) {
        throw std::runtime_error("Empty result on addSensor");
    }
    AuthData result;
    result.transportID = r[0][0].as<int>();
    result.passwordHash = "";
    BOOST_LOG(logger) << "Inserted new sensor, transportID " << result.transportID;
    return result;
}


int PSQLHandler::getTransportID(const std::string& imei, pqxx::work& tx) {
    pqxx::result r = tx.exec_prepared("get_trans_id", imei);
    if (r.empty()) {
        throw std::runtime_error("Transport ID not found for IMEI: " + imei);
    }
    return r[0][0].as<int>();
}

void PSQLHandler::uploadPoints(const std::vector<Point>& points, pqxx::work& tx) {
    if (points.empty()) return;

    for (const auto &p : points) {
        tx.exec_prepared(
            "insert_point",
            p.transportID,
            p.lat,
            p.lon,
            p.alt,
            p.time,
            p.speed,
            p.direction,
            p.satcnt,
            p.signal
        );
    }
    BOOST_LOG(logger) << "Bulk uploaded " << points.size() << " points.";
    BOOST_LOG(logger) << "Bulk uploaded " << points.size() << " points.";
}
