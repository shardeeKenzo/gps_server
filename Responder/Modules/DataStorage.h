#ifndef DATA_STORAGE_H
#define DATA_STORAGE_H

#define PURGE_PERIOD 15 // seconds

#include <pqxx/pqxx>
#include <boost/thread/mutex.hpp>

#include "DataTypes.h"

class DataStorage
{
public:
    DataStorage(
          pqxx::connection *aRreadCon
        , pqxx::connection *aWriteCon
        , boost::mutex     *aMutex
    );
    virtual ~DataStorage();

    void run();

    Track getTrack(
          long anID
        , long aPeriodStart
        , long aPeriodEnd
    );
    void getAllTracks(const vector<pair<long,long>> &idsAndStartTimes, vector<std::tuple<long,long,Track>> &tracks);

    static
    string trackToJson(const Track& track, string anID);
    string tracksToJson(const vector<std::tuple<long,long,Track>>& tracks);

private:
    pqxx::connection* _writeCon;
    pqxx::connection* _readCon;
    boost::mutex*     _readMutex;
    boost::mutex      _writeMutex;
    TracksHash        _tracks;

    bool              _purgingInProgress;
    bool              _purgingAllowed;
};

#endif // DATA_PROCESSOR_H

//
//
//
