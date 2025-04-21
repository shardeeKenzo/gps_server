#include <vector>
#include <sstream>
#include <iostream>

#include <pqxx/pqxx>

#include <boost/logic/tribool.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/bind.hpp>
#include <boost/lambda/construct.hpp>
#include "boost/date_time/gregorian/gregorian.hpp"

#include "Connection.h"

#include "HttpParser.h"
#include "UriParser.h"
#include "utils.hpp"

using namespace std;
using boost::asio::ip::tcp;

Connection::Connection(
      boost::asio::io_service& ioService
    , Authorization*           anAuth
    , DataStorage*             aStorage
) :   _socket(ioService)
{
    _storage  = aStorage;
    _auth     = anAuth;
}

Connection::~Connection() {
    hangUp();
}

tcp::socket& Connection::socket() {
    return _socket;
}

/*!
 * \brief Wait for a message to come
 */
void Connection::listen() {
    _socket.async_read_some(
        boost::asio::buffer(_buffer),
        boost::bind(
            &Connection::handleRead,
            this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred
        )
    );
}

/*!
 * \brief Shut down socket connection
 */
void
Connection::hangUp() {
    // close the socket
    boost::system::error_code ignored_ec;
    _socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
}

/*!
 * \brief Connection::handleRead
 * \param e
 * \param bytesTransferred
 */
void
Connection::handleRead(
      const boost::system::error_code& e
    , size_t bytesTransferred
)
{
    if (e) return;
    // NOTREACHED

    string   request = string(_buffer.data()).substr(0, bytesTransferred)
           , answer
           , sid;   // session id

    Maps     httpMaps;
    Maps     maps;
    TokenMap tokenMap;
    int      mapsCnt;
    long     periodStart
           , periodEnd;

    cout << endl << "--" << endl << "--" << endl
         << getTimeString() << "http request: " << request << endl;

    _parser.reset(new HttpParser());

    httpMaps = _parser->parse(request);
    mapsCnt  = httpMaps.size();

    // taking first map out of vector
    if (mapsCnt) {
        tokenMap = httpMaps[0];
    }

    cout << " - request parsed, got " << tokenMap.size() << " tokens" << endl;

    // parsing sid from cookie
    if (tokenMap.size()) {
        uint len;
        char buff[50] = "";

        sid = HttpParser::parseSid(tokenMap["Cookie"]);
        base64_decode(buff, sid.c_str(), &len);

        // this will give us user id
        //sid = sid.substr(0, sid.length() - COOKIE_RANDOM_PART_LEN);
        //sid = sid.substr(COOKIE_RANDOM_PART_LEN, sid.length());
    }
    
    // hang up connection if an error during parsing has occurred
    if (!mapsCnt || !tokenMap.count("uri") || !tokenMap.size()) {
        
        cout << endl
             << getTimeString()
             << "    !hanging up connection, an error during parsing has occurred!"
             << endl;
        
        delete this;
        return;
        // NOTREACHED
    }
    
    cout << " - parsing uri: " << tokenMap["uri"] << endl;
    
    _parser.reset(new UriParser());
    
    maps = _parser->parse(tokenMap["uri"]);
    mapsCnt = maps.size();
    
    // taking first map out of vector
    if (!mapsCnt) {
        cout << " - uri parsing failed!" << endl;
        
        answer = "bad request";
        _httpReply.status = Reply::status_type::bad_request;
        
        boost::asio::async_write(
              _socket
            , _httpReply.to_buffers()
            , boost::bind(
                &Connection::handleWrite,
                this,
                boost::asio::placeholders::error
            )
        );
        return;
        // NOTREAHCED
    } else {
        tokenMap = maps[0];
    }
    
    // default answer
    answer = "bad request";
    _httpReply.status = Reply::status_type::bad_request;

    // .../GetTrack?id=...&startTime=...&finishTime=...
    if (!tokenMap["requestType"].compare("GetTrack")
        // && _auth->_cookieMap.count(sid)
    ) {
        cout << " - GetTrack requested" << endl;

        try {
            periodStart = stol(tokenMap["startTime"]);
            periodEnd   = stol(tokenMap["finishTime"]);

            Track track;

            track = _storage->getTrack(
                  stol(tokenMap["id"])
                , periodStart
                , periodEnd
            );

            cout << " - converting obtained track to json" << endl;

            answer = _storage->trackToJson(track, tokenMap["id"]);
            _httpReply.status = Reply::status_type::ok;
        }
        catch(const std::invalid_argument e) { }
        catch(const std::out_of_range e)     { }

        _httpReply.content = answer;
    }
    // .../GetDevices?login=...&password=...
    else
    if (!tokenMap["requestType"].compare("GetDevices")) {
        cout << " - GetDevices requested" << endl;
        
        bool res    = _auth->authorize(tokenMap["login"], tokenMap["password"]);
        long userID = _auth->getUserID(tokenMap["login"]);
        
        // generate ok answer in any case
        if (res) {
            answer = _auth->generateGeodinAnswer(userID, tokenMap["login"]);
            _httpReply.status = Reply::status_type::ok;
        }
        
        // if client provided cookie that is not in RAM
        if (res && !_auth->_cookieMap.count(sid)) {
            cout << " - client provided cookie that is not in RAM" << endl;
            
            string idEnc = generateCookie(userID);
            
            header cookie;
            cookie.name  = "Set-Cookie";
            cookie.value = "sid=" + idEnc;
            // store it in RAM
            _auth->_cookieMap[idEnc] = userID;
            
            _httpReply.headers.push_back(cookie);
        }
        // if client provided cookie that is still in RAM
        else
        if (res && _auth->_cookieMap.count(sid)) {
            cout << " - client provided cookie that is still in RAM" << endl;
            
            header cookie;
            cookie.name  = "Set-Cookie";
            cookie.value = "sid=" + sid;
            _httpReply.headers.push_back(cookie);
        }
        // authorization failed
        else
        if (!res) {
            cout << " - authorization failed" << endl;

            answer = "Forbidden";
            _httpReply.status = Reply::status_type::forbidden;
        }
    }
    else
	if (!tokenMap["requestType"].compare("AccReg")) {
		
	}
	else
	if (!tokenMap["requestType"].compare("DevReg")) {
		
	}
    else
    // .../GetAllTracks?startTime=...
    if (!tokenMap["requestType"].compare("GetAllTracks")
        // && _auth->_cookieMap.count(sid)
    ) {
        cout << " - GetAllTracks requested" << endl;

        try {
            vector<pair<long,long>> idsAndStartTimes;
            string s = tokenMap["idsAndStartTimes"];

            size_t pos = 0, pos2;
            std::string token;
            while ((pos = s.find(";")) != std::string::npos) {
                token = s.substr(0, pos);
                pos2 = token.find(",");
                idsAndStartTimes.emplace_back(stol(token.substr(0, pos2)),stol(token.substr(pos2 + 1)));
                s.erase(0, pos + 1);
            }
            pos2 = s.find(",");
            idsAndStartTimes.emplace_back(stol(s.substr(0, pos2)),stol(s.substr(pos2 + 1)));

            vector<std::tuple<long,long,Track>> tracks;

            _storage->getAllTracks(idsAndStartTimes, tracks);

            cout << " - converting obtained tracks to json" << endl;

            answer = _storage->tracksToJson(tracks);
            cout << answer <<endl;
            _httpReply.status = Reply::status_type::ok;
        }
        catch(const std::invalid_argument e) { }
        catch(const std::out_of_range e)     { }

        _httpReply.content = answer;
    }
	
    // send the answer
    _httpReply.content = answer;
    boost::asio::async_write(
          _socket
        , _httpReply.to_buffers()
        , boost::bind(
            &Connection::handleWrite,
            this,
            boost::asio::placeholders::error
        )
    );

    cout << endl << "--" << endl << "--" << endl
         << getTimeString() << "connection ended" << endl;
}

/*!
 * \brief Connection::handleWrite
 * \param e
 */
void
Connection::handleWrite(const boost::system::error_code& e) {
    delete this;
}

string
Connection::generateCookie(long userID) {
    string idEnc, idDec;
    char buff[50] = "";
    //
    idDec = generateRandomStr(COOKIE_RANDOM_PART_LEN)
          + to_string(userID)
          + generateRandomStr(COOKIE_RANDOM_PART_LEN);
    //

    base64_encode(buff, idDec.c_str(), idDec.length());

    return string(buff);
}