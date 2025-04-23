#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <memory>
#include <mutex>

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>

#include <pqxx/pqxx>

#include "IOServicePool.h"
#include "DataStorage.h"
#include "Connection.h"
#include "PSQLHandler.h"

struct ServerConfig {
    std::string host;
    std::string port;
    std::size_t poolSize;
    std::string psqlHost;
    std::string psqlLogin;
    std::string psqlDbName;
    std::string psqlDbPass;
};

class Server : private boost::noncopyable
{
public:
    /// Construct the server to listen on the specified TCP address and port, and
    /// serve up files from the given directory.
    explicit Server(const ServerConfig& aConfig);
    ~Server() = default;

    /// Run the server's io_service loop.
    void run();

private:
    void startAccept();
    void handleAccept(const boost::system::error_code& e);
    void handleStop();

    io_service_pool                  ioPool_;
    boost::asio::signal_set          signals_;
    boost::asio::ip::tcp::acceptor   acceptor_;
    boost::asio::io_service&         ios_;
    std::shared_ptr<Connection>      newConnection_;

    std::shared_ptr<PSQLHandler> readHandler_;
    std::shared_ptr<PSQLHandler> writeHandler_;

    std::mutex                       mutex_;
    std::unique_ptr<DataStorage>     storage_;
};

#endif // SERVER_H