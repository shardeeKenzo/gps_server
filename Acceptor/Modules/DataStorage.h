#ifndef DATA_STORAGE_H
#define DATA_STORAGE_H


#include <chrono>
#include <condition_variable>
#include <mutex>
#include <PSQLHandler.h>
#include <thread>
#include <vector>

#include <pqxx/pqxx>

#include "DataTypes.h"

using namespace std;

/// Buffers incoming GPS points in memory and periodically flushes them to PostgreDB.
class DataStorage
{
public:
    /// @param readCon   Shared connection for reads.
    /// @param writeCon  Shared connection for writes.
    /// @param flushInterval How often to flush the buffer to the database.
    DataStorage(std::shared_ptr<pqxx::connection> readCon,
                std::shared_ptr<pqxx::connection> writeCon,
                std::chrono::seconds flushInterval = std::chrono::seconds(15));

    ~DataStorage();
    
    /// Cache a new Point parsed from a token map.
    /// @returns true on success; false on parse error.
    bool store(int transportID, const TokenMap& tokenMap);

    /// Start the background flushing thread.
    void start();

    /// Stop the background thread and flush any remaining points.
    void stop();
    
private:

    /// The background loop: wait for either flushInterval or a stop signal.
    void run();

    /// Flush the current buffer to the database (called under lock).
    void uploadPoints();

    std::shared_ptr<pqxx::connection> writeCon_;
    std::shared_ptr<pqxx::connection> readCon_;
    std::mutex                        mutex_;
    std::condition_variable           cv_;
    std::thread                       thread_;
    bool                              running_;

    PSQLHandler                       psql_;


    std::chrono::seconds              flushInterval_;
    std::vector<Point>                points_;
//    vector<Transport> _transports;
//    vector<Transport> _transports_copy;
};

#endif // DATA_PROCESSOR_H

