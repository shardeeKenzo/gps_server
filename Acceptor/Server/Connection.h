#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <string>

#include "Parser.hpp"
#include "Authorization.h"
#include "DataStorage.h"

#define CONNECTION_TIMEOUT 120 // 2 minutes

using boost::asio::ip::tcp;
using namespace std;

/// Represents a single Connection from a client.
class Connection : public boost::enable_shared_from_this<Connection>, private boost::noncopyable
{
public:
    explicit Connection(
          boost::asio::io_service&       io_service
        , Auth_ptr                       auth
        , DataStorage*                   storage
    );
    ~Connection();

    tcp::socket& socket();
    void listen();
    void hangUp();

private:
    void handleRead(const boost::system::error_code& e, size_t);
    void handleWrite(const boost::system::error_code& e);
    
    void launchTimer();
    void onTimeout();
    
    void processingRequest(string request);
    
    tcp::socket                      _socket;
    boost::array<char, 8192>         _buffer;
    pqxx::connection*                _readCon;
    pqxx::connection*                _writeCon;
    boost::mutex*                    _mutex;
    
    Parser_ptr                       _parser;
    Auth_ptr                         _auth;
    DataStorage*                     _storage;
    
    std::string                      _request;
};

typedef boost::shared_ptr<Connection> connection_ptr;

#endif // CONNECTION_H