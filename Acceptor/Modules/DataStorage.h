#ifndef DATA_STORAGE_H
#define DATA_STORAGE_H


#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>
#include <memory>

#include "PSQLHandler.h"
#include "DataTypes.h"


/// Buffers incoming GPS points in memory and periodically flushes them to PostgreDB.
class DataStorage
{
public:
    explicit DataStorage(std::shared_ptr<PSQLHandler> writeHandler_,
                std::chrono::seconds flushInterval_ = std::chrono::seconds(15));

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

    std::shared_ptr<PSQLHandler>      psql_;
    std::mutex                        mutex_;
    std::condition_variable           cv_;
    std::thread                       thread_;
    bool                              running_;

    std::chrono::seconds              flushInterval_;
    std::vector<Point>                points_;
//    vector<Transport> _transports;
//    vector<Transport> _transports_copy;
};

#endif // DATA_PROCESSOR_H

