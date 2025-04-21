#include <iostream>

#include "Server.h"

using boost::asio::ip::tcp;

Server::Server(const ServerConfig& aConfig)
  : ioPool_(aConfig.poolSize)
  , signals_(ioPool_.get_io_service())
  , acceptor_(ioPool_.get_io_service())
  , ios_(ioPool_.get_io_service())
{
    // Register to handle the signals that indicate when the server should exit.
    // It is safe to register for the same signal multiple times in a program,
    // provided all registration for the specified signal is made through Asio.
    signals_.add(SIGINT);
    signals_.add(SIGTERM);

#if defined(SIGQUIT)
    signals_.add(SIGQUIT);
#endif // defined(SIGQUIT)
    
    signals_.async_wait([this](auto&&){ handleStop(); });
    
    readCon_  = std::make_unique<pqxx::connection>(
                  "dbname="  + aConfig.psqlDbName +
                  " host="   + aConfig.psqlHost  +
                  " user="   + aConfig.psqlLogin +
                  " password="+ aConfig.psqlDbPass);
    if (!readCon_->is_open()) {
        throw std::runtime_error("Failed to open read DB connection");
    }

    writeCon_ = std::make_unique<pqxx::connection>(
                  "dbname="  + aConfig.psqlDbName +
                  " host="   + aConfig.psqlHost  +
                  " user="   + aConfig.psqlLogin +
                  " password="+ aConfig.psqlDbPass);
    if (!writeCon_->is_open()) {
        throw std::runtime_error("Failed to open write DB connection");
    }
    
    // Resolve endpoint and start acceptor
    tcp::resolver resolver(ios_);
    auto endpoints = resolver.resolve({aConfig.host, aConfig.port});
    
    acceptor_.open(endpoints->endpoint().protocol());
    acceptor_.set_option(tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoints->endpoint());
    acceptor_.listen();
    
    storage_ = std::make_unique<DataStorage>(
        readCon_.get(), writeCon_.get(), &mutex_);
    
    startAccept();

    // start periodically sending data stored in RAM to db
    std::thread([this] { storage_->run(); }).detach();
}

void Server::run() {
    ioPool_.run();
}

void Server::startAccept() {
    newConnection_ = std::make_shared<Connection>(
        ios_,
        std::make_shared<Authorization>(
            readCon_.get(), writeCon_.get(), &mutex_),
        storage_.get());

    acceptor_.async_accept(
        newConnection_->socket(),
        [this](auto&& ec) { handleAccept(ec); });
}

void Server::handleAccept(const boost::system::error_code& ec) {
    if (!ec) {
        newConnection_->listen();
    }
    startAccept();
}

void Server::handleStop() {
    ioPool_.stop();
}