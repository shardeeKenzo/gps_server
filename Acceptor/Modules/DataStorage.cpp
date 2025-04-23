#include "DataStorage.h"
#include "Logger.hpp"
#include "utils.hpp"

DataStorage::DataStorage(
    std::shared_ptr<PSQLHandler> writeHandler_,
    const std::chrono::seconds flushInterval_)
  : psql_{std::move(writeHandler_)}
, running_{false}
, flushInterval_{flushInterval_}
{
    if (!psql_ || !psql_->connection() || !psql_->connection()->is_open()) {
        throw std::invalid_argument("DataStorage: invalid write handler");
    }
}

DataStorage::~DataStorage() {
    stop();
}

void DataStorage::run() {
    std::unique_lock lock(mutex_);
    while (running_) {
        // Wake on timeout or new data/stop
        cv_.wait_for(lock, flushInterval_, [this] {
            return !running_ || !points_.empty();
        });

        if (!points_.empty()) {
            // Move data out under lock
            std::vector<Point> toUpload;
            toUpload.swap(points_);
            lock.unlock();

            // Perform upload
            try {
                pqxx::work xact(*psql_->connection());
                psql_->uploadPoints(toUpload, xact);
                xact.commit();
                BOOST_LOG(logger) << getTimeString()
                                  << " Uploaded " << toUpload.size()
                                  << " points to DB.";
            }
            catch (const std::exception& e) {
                BOOST_LOG(logger) << getTimeString()
                                  << " Error uploading batch of "
                                  << toUpload.size()
                                  << " points: " << e.what();
            }

            lock.lock();
        }
    }

    // Final flush on shutdown
    if (!points_.empty()) {
        lock.unlock();
        uploadPoints();
        lock.lock();
    }
}

/*!
 * \brief Store point data in RAM for further uploading to database
 * \param transportID   - sensor id in database
 * \param tokenMap - map of tokens got from data package
 */
bool DataStorage::store(const int transportID, const TokenMap& tokenMap) {
    Point p;
    p.transportID = transportID;

    try {
        p.lat       = std::stoi(tokenMap.at("lat"));
        p.lon       = std::stoi(tokenMap.at("lon"));
        p.alt       = std::stoi(tokenMap.at("alt"));
        p.speed     = std::stoi(tokenMap.at("speed"));
        p.direction = std::stoi(tokenMap.at("direction"));
        p.satcnt    = std::stoi(tokenMap.at("satcnt"));
        p.time      = std::stol(tokenMap.at("time"));
    } catch (const std::exception&) {
        return false;
    }
    p.signal = false;  // placeholder

    {
        std::lock_guard<std::mutex> lock(mutex_);
        points_.emplace_back(p);
    }
    cv_.notify_one();
    return true;
}

void DataStorage::start() {
    running_ = true;
    thread_ = std::thread(&DataStorage::run, this);
}

void DataStorage::stop() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        running_ = false;
    }
    cv_.notify_one();
    if (thread_.joinable())
        thread_.join();
}

void DataStorage::uploadPoints() {
    if (points_.empty()) return;

    // Swap and upload remaining points
    std::vector<Point> toUpload;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        toUpload.swap(points_);
    }

    try {
        pqxx::work xact(*psql_->connection());
        psql_->uploadPoints(toUpload, xact);
        xact.commit();
        BOOST_LOG(logger) << getTimeString()
                          << " Final upload of " << toUpload.size()
                          << " points.";
    }
    catch (const std::exception& e) {
        BOOST_LOG(logger) << getTimeString()
                          << " Error on final upload: " << e.what();
    }
}

