#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <string>

#include "Parser.hpp"
#include "Reply.h"
#include "Authorization.h"
#include "DataStorage.h"

#define COOKIE_TIMEOUT         2 // 2 hours
#define COOKIE_RANDOM_PART_LEN 6

using boost::asio::ip::tcp;
using namespace std;

/// Represents a single Connection from a client.
class Connection : private boost::noncopyable
{
public:
    explicit Connection(
          boost::asio::io_service&       io_service
        , Authorization*                 auth
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

    string generateCookie(long userID);

    tcp::socket                      _socket;
    boost::array<char, 8192>         _buffer;
    pqxx::connection*                _readCon;
    pqxx::connection*                _writeCon;
    boost::mutex*                    _mutex;

    Parser_ptr                       _parser;
    Reply                            _httpReply;
    Authorization*                   _auth;
    DataStorage*                     _storage;
};

#endif // CONNECTION_H