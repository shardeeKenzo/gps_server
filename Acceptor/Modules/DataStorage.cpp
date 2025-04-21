#include "DataStorage.h"
#include "PSQLHandler.h"
#include "Logger.hpp"
#include "utils.hpp"

DataStorage::DataStorage(
    std::shared_ptr<pqxx::connection> readCon,
    std::shared_ptr<pqxx::connection> writeCon,
    const std::chrono::seconds flushInterval)
  : writeCon_{ std::move(writeCon) }
  , running_{ false }
  , flushInterval_{ flushInterval }
{
    if (!writeCon_ || !writeCon_->is_open()) {
        throw std::invalid_argument("DataStorage: invalid writeCon");
    }
}

DataStorage::~DataStorage() {
    stop();
}

void DataStorage::run() {
    std::unique_lock<std::mutex> lock(mutex_);
    while (running_) {
        // Wait until either timeout or notified of new data/stop
        cv_.wait_for(lock, flushInterval_, [this] {
            return !running_ || !points_.empty();
        });

        if (!points_.empty()) {
            // Swap buffer under lock
            std::vector<Point> toUpload;
            toUpload.swap(points_);
            lock.unlock();

            // Perform upload outside lock
            {
                work xact(*writeCon_);
                short res = PSQLHandler::uploadPoints(toUpload, xact);
                if (res == 0) {
                    xact.commit();
                    BOOST_LOG(logger) << getTimeString()
                                      << " Uploaded " << toUpload.size()
                                      << " points to DB.";
                } else {
                    BOOST_LOG(logger) << getTimeString()
                                      << " Error uploading batch of "
                                      << toUpload.size() << " points.";
                }
            }

            lock.lock();
        }
    }

    // Final flush before exit
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
    // Called only when holding lock, but no points on entry guard should be before call
    if (points_.empty()) return;

    // Swap and upload
    std::vector<Point> toUpload;
    toUpload.swap(points_);
    work xact(*writeCon_);
    short res = PSQLHandler::uploadPoints(toUpload, xact);
    if (res == 0) {
        xact.commit();
        BOOST_LOG(logger) << getTimeString()
                          << " Final upload of " << toUpload.size()
                          << " points.";
    } else {
        BOOST_LOG(logger) << getTimeString()
                          << " Error on final upload.";
    }
}

