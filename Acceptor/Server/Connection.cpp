#include <vector>
#include <sstream>
#include <iostream>

#include <pqxx/pqxx>
#include <boost/bind.hpp>
#include <boost/lambda/construct.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "Logger.hpp"
#include "utils.hpp"

#include "Connection.h"
#include "PSQLHandler.h"

#include "WialonIPS.h"

#include "utils.hpp"

using namespace std;
using boost::asio::ip::tcp;

Connection::Connection(boost::asio::io_service & ioService, Auth_ptr anAuth, DataStorage * aStorage) : _socket(ioService) {
    _storage = aStorage;
    
    _auth = anAuth;
}

Connection::~Connection() {
    using namespace boost::posix_time;
    
    hangUp();
    
    // DEBUG
    ptime now = second_clock::local_time();
    //cout << endl << "--" << endl << "--"
    //     << now.time_of_day() << ": Connection destructor for " << _auth->_imei
    //     << " called" << endl;
}

tcp::socket& Connection::socket() {
    return _socket;
}

void Connection::listen() {
    _socket.async_read_some(
        boost::asio::buffer(_buffer),
        boost::bind(
            &Connection::handleRead,
            shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred
        )
    );
}

void Connection::hangUp() {
    // close the socket
    boost::system::error_code ignored_ec;
    _socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
}

void Connection::handleRead(const boost::system::error_code& e, size_t bytesTransferred) {
    if (e) { 
	BOOST_LOG(logger) << getTimeString() << _auth->imei() << " Error: " << e.message() << endl << endl;
        return;
    }
    // NOTREACHED
    
    string request = string(_buffer.data()).substr(0, bytesTransferred);
    
    BOOST_LOG(logger) << getTimeString() << _auth->imei() << " Request from " << _socket.remote_endpoint().address().to_string() << ": " << request;
    //cout << endl << "--" << endl << "--" << endl
    //     << getTimeString() << "tcp request: " << request << endl;
    
    std::size_t found_end = request.find("\r\n");
    if (found_end != std::string::npos) {
        request = _request + request;
        _request = "";
    } else {
	BOOST_LOG(logger) << getTimeString() << _auth->imei() << " Wrong end of request!" << endl << request;
        _request += request;
        listen();
        return;
    }
    
    processingRequest(request);
}

void Connection::processingRequest(string request) {
  
    string   answer;
    Maps     maps;
    TokenMap tokenMap;
    short    res           = false
           , endConnection = false;
    int      i
           , mapsCnt
           , successfulTransfers = 0;
    
    WialonIPS::REQUEST reqType = WialonIPS::UNDEFINED_REQ;
    // PLACEHOLDER
    // TODO: parser should be set after protocol recognition
    _parser.reset(new WialonIPS());

    maps    = _parser->parse(request);
    mapsCnt = maps.size();

    // taking first map out of vector
    if (mapsCnt) {
        tokenMap = maps[0];
    }
    
    // hang up connection if an error during parsing has occurred
    if (!mapsCnt || !tokenMap.count("requestType") || !tokenMap.size()) {
        //cout << endl << "    !an error during parsing has occurred!" << endl;
        
	BOOST_LOG(logger) << getTimeString() << _auth->imei() << " Error: Wrong request!" << endl;

        return;
    }
    
    //string imei = tokenMap["imei"];

    //BOOST_LOG(logger) << getTimeString() << _auth->_imei << ": "<< "Packet parsed, got " << mapsCnt << " maps.";
    
    if (1 == maps.size()) {
        reqType = WialonIPS::reqStringToType(
            tokenMap["requestType"]
        );
    } else {
        reqType = WialonIPS::B_BLACK_BOX;
    }
    
    //BOOST_LOG(logger) << getTimeString() << imei << ": "<< "Packet type: " << tokenMap["requestType"];
    
    // do not process the signal if there was no previous authorization
    if (!_auth->isAuthorized() && WialonIPS::L_LOGIN != reqType) {
        reqType = WialonIPS::UNDEFINED_REQ;
    }
    
    switch(reqType) {
        case WialonIPS::L_LOGIN:
            //cout << " - WialonIPS::L_LOGIN" << endl;
	    BOOST_LOG(logger) << getTimeString() << tokenMap["imei"] << ": " << "Request type: Login.";

            res = _auth->authorize(
                  tokenMap["imei"]
                , tokenMap["password"]
            );
            answer        = _parser->answer(WialonIPS::L_LOGIN, res);
            endConnection = (res) ? false : true;

            break;
            
        case WialonIPS::P_PING:
            //cout << " - WialonIPS::P_PING" << endl;
	    BOOST_LOG(logger) << getTimeString() << _auth->imei() << ": " << "Request type: Ping.";
            
            answer = _parser->answer(WialonIPS::P_PING, res);
            break;
            
        case WialonIPS::D_DATA:
            //cout << " - WialonIPS::D_DATA" << endl;
	    BOOST_LOG(logger) << getTimeString() << _auth->imei() << ": " << "Request type: Data.";

            res = _storage->store(_auth->transportID(), tokenMap);
            answer = _parser->answer(WialonIPS::D_DATA, res);
            break;
            
        case WialonIPS::SD_SHORT_DATA:
            //cout << " - WialonIPS::SD_SHORT_DATA" << endl;
	    BOOST_LOG(logger) << getTimeString() << _auth->imei() << ": " << "Request type: Short data.";
            
            res = _storage->store(_auth->transportID(), tokenMap);
            answer = _parser->answer(WialonIPS::SD_SHORT_DATA, res);
            break;
            
        case WialonIPS::B_BLACK_BOX:
            //cout << " - WialonIPS::B_BLACK_BOX" << endl;
	    BOOST_LOG(logger) << getTimeString() << _auth->imei() << ": " << "Request type: Black box.";
            
            // iterate through all tokens(maps) in vector
            for (i = 0; i < mapsCnt; i++) {
                tokenMap = maps[i];
                if (!_storage->store(_auth->transportID(), tokenMap)) continue;
                // LOOP VIOLATION
                
                successfulTransfers++;
            }
            
            answer = _parser->answer(
                  WialonIPS::B_BLACK_BOX
                , successfulTransfers
            );
            break;
            
        default:
            endConnection = true;
            break;
    }
    
    // send the answer
    boost::asio::async_write(
          _socket
        , boost::asio::buffer(answer)
        , boost::bind(
            &Connection::handleWrite,
            this,
            boost::asio::placeholders::error
        )
    );
    
    //cout << endl << getTimeString() << " answer sent: " << answer << endl;
    BOOST_LOG(logger) << getTimeString() << _auth->imei() << ": " << "Sent answer: " << answer;

    // continue session
    if(!endConnection) {
	//BOOST_LOG(logger) << getTimeString() << _auth->_imei << ": "  << endl << endl;
        listen();
    }
    else {
	BOOST_LOG(logger) << getTimeString() << _auth->imei() << ": " << "Close connection." << endl;
    }
    
}

void Connection::handleWrite(const boost::system::error_code& e) {}