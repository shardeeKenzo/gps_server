#ifndef DATA_STORAGE_H
#define DATA_STORAGE_H

#define SLEEP_DELAY 15000000

#include <pqxx/pqxx>
#include <boost/thread/mutex.hpp>
#include <sstream>

#include "DataTypes.h"

using namespace std;

class DataStorage
{
public:
    DataStorage(
          pqxx::connection *aRreadCon
        , pqxx::connection *aWriteCon
        , boost::mutex     *aMutex
    );
    virtual ~DataStorage();
    
    short store(int aTransportID, TokenMap aTokenMap);
    void uploadPoints();
    void run();
    
private:
    pqxx::connection* _writeCon;
    pqxx::connection* _readCon;
    boost::mutex*     _readMutex;
    boost::mutex      _writeMutex;
    vector<Point>     _points;
    vector<Point>     _pointsCopy;
//    vector<Transport> _transports;
//    vector<Transport> _transports_copy;

    bool              _purgingInProgress;
};

#endif // DATA_PROCESSOR_H

//
//
//
