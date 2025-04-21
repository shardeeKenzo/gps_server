#ifndef SERVER_H
#define SERVER_H

#include <string>

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread/mutex.hpp>

#include <pqxx/connection>

#include "IOServicePool.h"
#include "DataStorage.h"
#include "Connection.h"
#include "PSQLHandler.h"

struct ServerConfig {
    string host;
    string port;
    size_t poolSize;
    string psqlHost;
    string psqlLogin;
    string psqlDbName;
    string psqlDbPass;
};

class Server : private boost::noncopyable
{
public:
    /// Construct the server to listen on the specified TCP address and port, and
    /// serve up files from the given directory.
    explicit Server(
          const ServerConfig& aConfig
    );
    ~Server();

    /// Run the server's io_service loop.
    void run();

private:
    void startAccept();
    void handleAccept(const boost::system::error_code& e);
    void handleStop();

    io_service_pool                _ioServicePool;
    boost::asio::signal_set        _signals;
    boost::asio::ip::tcp::acceptor _acceptor;
    connection_ptr                 _newConnection;

    pqxx::connection* _readCon;
    pqxx::connection* _writeCon;

    boost::mutex _mutex;

    boost::scoped_ptr< DataStorage > _storage;
};

#endif // SERVER_H