#include "DataStorage.h"
#include <vector>
#include <sstream>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/thread/mutex.hpp>
#include <unistd.h>

#include <sstream>
#include <iostream>

#include "rapidjson/document.h"     // rapidjson's DOM-style API
#include "rapidjson/prettywriter.h" // for stringify JSON
#include "rapidjson/filestream.h"
#include "rapidjson/stringwriter.h"

#include "PSQLHandler.h"

DataStorage::DataStorage(
      pqxx::connection *aRreadCon
    , pqxx::connection *aWriteCon
    , boost::mutex     *aMutex
)
{
    if (!aWriteCon || !aWriteCon) {
        // TODO: remake with error code
        cerr << "    !DataStorage::DataStorage: pointer to connection or mutex is null"
             << endl << flush;
        exit(1);
        // NOTREACHED
    }

    _writeCon   = aWriteCon;
    _readCon    = aRreadCon;
    _readMutex  = aMutex;

    _purgingInProgress = false;
    _purgingAllowed    = true;
}

DataStorage::~DataStorage() {
    // EMPTY
}

/*!
 * \brief DataStorage::run
 */
void
DataStorage::run() {
    for (;;) {
        sleep(PURGE_PERIOD);

        while (!_purgingAllowed) usleep(1);

        if (_tracks.size()) {
            _purgingInProgress = true;
            _tracks.clear();
            _purgingInProgress = false;

            cout << " - clearing up the cache" << endl << flush;
        }
    }
}

/*!
 * \brief DataStorage::getTrack
 * \param anID
 * \param aPeriodStart
 * \param aPeriodEnd
 * \return
 */
Track
DataStorage::getTrack(
      long anID
    , long aPeriodStart
    , long aPeriodEnd
)
{
    Track  track;
    string key;

    key = to_string(anID) + to_string(aPeriodStart) + to_string(aPeriodEnd);

    if (!_tracks.count(key) || _purgingInProgress) {
        boost::mutex::scoped_lock lock(*_readMutex);

        cout << " - giving track from DB" << endl;

        work xact(*_readCon);
        track = PSQLHandler::getTrack(anID, aPeriodStart, aPeriodEnd, xact);

        _purgingAllowed = false;
        _tracks[key] = track;
        _purgingAllowed = true;
    } else {
        _purgingAllowed = false;
        track = _tracks[key];
        _purgingAllowed = true;

        cout << " - giving track from RAM" << endl;
    }

    return track;
}

void  
DataStorage::getAllTracks(const vector<pair<long,long>> &idsAndStartTimes, vector<std::tuple<long,long,Track>> &tracks)
{
    boost::mutex::scoped_lock lock(*_readMutex);
    cout << " - giving all tracks from DB" << endl;
    work xact(*_readCon);
    PSQLHandler::getAllTracks(idsAndStartTimes, tracks, xact);
}

/*!
 * \brief DataStorage::trackToJson
 * \param aTrack
 * \return formatted track in json
 */
string
DataStorage::trackToJson(const Track& aTrack, string anID) {
    using namespace rapidjson;

    Document d;
    int      i
           , len = aTrack.size();
    Point    p;

    d.SetArray();

    for (i = 0; i < len; i++) {
        Value obj;
        p = aTrack[i];

        obj.SetObject();

        // TODO: make it arch dependable
        obj.AddMember("t" , (int)p.time     , d.GetAllocator());
        obj.AddMember("s" , p.speed         , d.GetAllocator());
        obj.AddMember("x" , p.lat / 1000000., d.GetAllocator());
        obj.AddMember("y" , p.lon / 1000000., d.GetAllocator());
        obj.AddMember("z" , p.alt           , d.GetAllocator());

        obj.AddMember("c" , p.direction     , d.GetAllocator());
        obj.AddMember("sc", p.satcnt        , d.GetAllocator());
        obj.AddMember("l" , ""              , d.GetAllocator());

        d.PushBack(obj, d.GetAllocator());
    }

    Value obj;
    obj.SetObject();
    obj.AddMember("id", anID.c_str(), d.GetAllocator());
    d.PushBack(obj, d.GetAllocator());

    // Stringify json
    ostringstream outStream;
    StreamWriter<ostringstream> writer(outStream);
    d.Accept(writer);

    // Extract the message from the stream and format it.
    return outStream.str();
}

string 
DataStorage::tracksToJson(const vector<std::tuple<long,long,Track>>& tracks) {
    using namespace rapidjson;
    Document d;
    d.SetArray();
    for(const auto &track : tracks)
    {
        Value obj_track;
        obj_track.SetObject();
        obj_track.AddMember("id", get<0>(track), d.GetAllocator());
        obj_track.AddMember("lastupl", get<1>(track), d.GetAllocator());
        Value arr_points;
        arr_points.SetArray();
        for(const auto &p : get<2>(track))
        {
            Value obj_point;
            obj_point.SetObject();

            // TODO: make it arch dependable
            obj_point.AddMember("t" , (int)p.time     , d.GetAllocator());
            obj_point.AddMember("s" , p.speed         , d.GetAllocator());
            obj_point.AddMember("x" , p.lat / 1000000., d.GetAllocator());
            obj_point.AddMember("y" , p.lon / 1000000., d.GetAllocator());
            obj_point.AddMember("z" , p.alt           , d.GetAllocator());

            obj_point.AddMember("c" , p.direction     , d.GetAllocator());
            obj_point.AddMember("sc", p.satcnt        , d.GetAllocator());
            obj_point.AddMember("l" , ""              , d.GetAllocator());

            arr_points.PushBack(obj_point, d.GetAllocator());
        }
        obj_track.AddMember("points", arr_points, d.GetAllocator());
        d.PushBack(obj_track, d.GetAllocator());
    }
    
    // Stringify json
    ostringstream outStream;
    StreamWriter<ostringstream> writer(outStream);
    d.Accept(writer);

    // Extract the message from the stream and format it.
    return outStream.str();
}

//
//
//
