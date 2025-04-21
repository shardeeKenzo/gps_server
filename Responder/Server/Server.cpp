#include <iostream>

#include "Server.h"
#include "Authorization.h"
#include <boost/bind.hpp>
#include <boost/thread.hpp>

using boost::asio::ip::tcp;
using namespace std;

Server::Server(
      const ServerConfig& aConfig
) :   _ioServicePool(aConfig.poolSize)
    , _signals(_ioServicePool.get_io_service())
    , _acceptor(_ioServicePool.get_io_service())
    , _newConnection()
{
    // Register to handle the signals that indicate when the server should exit.
    // It is safe to register for the same signal multiple times in a program,
    // provided all registration for the specified signal is made through Asio.
    _signals.add(SIGINT);
    _signals.add(SIGTERM);

#if defined(SIGQUIT)
    _signals.add(SIGQUIT);
#endif // defined(SIGQUIT)

    _signals.async_wait(boost::bind(&Server::handleStop, this));

    short res = PSQLHandler::connectToDB(
          aConfig.psqlHost
        , aConfig.psqlLogin
        , aConfig.psqlDbName
        , aConfig.psqlDbPass
        , &_readCon
    );

    if (res) {
        cerr << "ERROR:Server::Server: could not open a connection to db"
             << endl << flush;
        exit(1);
    }

    res = PSQLHandler::connectToDB(
          aConfig.psqlHost
        , aConfig.psqlLogin
        , aConfig.psqlDbName
        , aConfig.psqlDbPass
        , &_writeCon
    );

    if (res) {
        cerr << "Server::Server: could not open a connection to db"
             << endl << flush;
        exit(1);
    }

    // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
    tcp::resolver        resolver(_acceptor.get_io_service());
    tcp::resolver::query query(aConfig.host, aConfig.port);

    tcp::endpoint endpoint = *resolver.resolve(query);

    _acceptor.open(endpoint.protocol());
    _acceptor.set_option(tcp::acceptor::reuse_address(true));
    _acceptor.bind(endpoint);
    _acceptor.listen();

    _storage.reset(new DataStorage(_readCon, _writeCon, &_mutex));
    _auth.reset(new Authorization(_readCon, _writeCon, &_mutex));

    startAccept();

    // start periodically clearing data stored in RAM(cache)
    boost::thread storageLoop(
        boost::bind(
              &DataStorage::run
            , boost::ref(_storage)
        )
    );

    boost::thread cookieLoop(
        boost::bind(
              &Authorization::runCookieCleaner
            , boost::ref(_auth)
        )
    );
}

Server::~Server() {
    if (_readCon) {
        _readCon->disconnect();
        delete _readCon;
    }
    if (_writeCon) {
        _writeCon->disconnect();
        delete _writeCon;
    }

    _storage.reset();
}

void Server::run() {
    cout << "--GPS responder has started--" << endl << flush;

    _ioServicePool.run();
}

void Server::startAccept() {
    _newConnection = new Connection(
          _ioServicePool.get_io_service()
        , _auth.get()
        , _storage.get()
    );

    _acceptor.async_accept(
        _newConnection->socket(),
        boost::bind(
            &Server::handleAccept,
            this,
            boost::asio::placeholders::error
        )
    );
}

void Server::handleAccept(const boost::system::error_code& e) {
    if (!e) _newConnection->listen();

    startAccept();
}

void Server::handleStop() {
    _ioServicePool.stop();
}

//
//
//
