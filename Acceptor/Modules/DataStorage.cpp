#include <vector>
#include <iostream>
#include <sstream>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/thread/mutex.hpp>
#include <unistd.h>

#include "DataStorage.h"
#include "PSQLHandler.h"
#include "Logger.hpp"
#include "utils.hpp"

DataStorage::DataStorage(
      pqxx::connection *aRreadCon
    , pqxx::connection *aWriteCon
    , boost::mutex     *aMutex
)
{
    if (!aWriteCon || !aMutex) {
        // TODO: remake with error code
        cerr << "DataStorage::DataStorage: pointer to connection or mutex is null"
             << endl << flush;
        exit(1);
        // NOTREACHED
    }

    _writeCon   = aWriteCon;
    _readCon    = aRreadCon;
    _readMutex  = aMutex;

    _purgingInProgress = false;
}

DataStorage::~DataStorage() {
    if (!_points.empty()) {
        _pointsCopy = _points;
        uploadPoints();
    }
}

[[noreturn]] void
DataStorage::run() {
    for (;;) {
        usleep(SLEEP_DELAY);
        if (!_points.empty()) {
            _pointsCopy = _points;

            _purgingInProgress = true;
            _points.clear();
            _purgingInProgress = false;

             uploadPoints();
             BOOST_LOG(logger) << getTimeString() << "Uploaded " << _pointsCopy.size() << " points to db."  << endl;

            _purgingInProgress = true;
            _pointsCopy.clear();
            _purgingInProgress = false;
        }
    }
}

/*!
 * \brief Store point data in RAM for further uploading to database
 * \param aTransportID   - sensor id in database
 * \param aTokenMap - map of tokens got from data package
 * \return 0 - error
 *         1 - success
 */
short
DataStorage::store(int aTransportID, TokenMap aTokenMap) {
    Point p;

    p.transportID = aTransportID;

    try {
        p.lat       = stoi(aTokenMap["lat"]);
        p.lon       = stoi(aTokenMap["lon"]);
        p.alt       = stoi(aTokenMap["alt"]);
        p.speed     = stoi(aTokenMap["speed"]);
        p.direction = stoi(aTokenMap["direction"]);
        p.satcnt    = stoi(aTokenMap["satcnt"]);
        p.time      = stol(aTokenMap["time"]);
    } catch(const std::invalid_argument e) {
        return 0;
        // NOTREACHED
    } catch(const std::out_of_range e) {
        return 0;
        // NOTREACHED
    }

    // not sure what to be held here
    // PLACEHOLDER
    p.signal = false;
//    BOOST_LOG(logger) << getTimeString() << "DataStorage::store mutex locked";
    boost::mutex::scoped_lock lock(_writeMutex);
//    BOOST_LOG(logger) << getTimeString() << "DataStorage::store mutex unlocked";

    while (_purgingInProgress) usleep(1);

    _points.push_back(p);
//    BOOST_LOG(logger) << getTimeString() << "Point cached in RAM storage";

    return 1;
}

void
DataStorage::uploadPoints() {
//    BOOST_LOG(logger) << getTimeString() << "DataStorage::uploadPoints mutex locked";
    boost::mutex::scoped_lock lock(_writeMutex);
//    BOOST_LOG(logger) << getTimeString() << "DataStorage::uploadPoints mutex unlocked";

    work xact(*_writeCon);
    short res = PSQLHandler::uploadPoints(_pointsCopy, xact);
    if (!res) {
        xact.commit();
        BOOST_LOG(logger) << getTimeString() << "Pack of points was successfully uploaded to db." << endl << endl << endl;
    }
    else {
        // TODO: remake with error codes
        BOOST_LOG(logger) <<  getTimeString() << "Error has occured during uploading to db." << endl << endl << endl;
    }
}

//
//
//
