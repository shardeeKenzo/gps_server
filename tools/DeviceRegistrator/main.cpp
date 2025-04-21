#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <pqxx/pqxx>
#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include <crypt.h>

namespace po = boost::program_options;
using namespace std;

struct Config {
    string userName;
    string imei;
    string password;
    string dbHost;
    string dbUser;
    string dbName;
    string dbPass;
};

int main(int argc, char* argv[])
{
    Config                              config;
    pqxx::connection                   *con;
    string                              conStr;
    pqxx::result                        res;
    pqxx::result::const_iterator        row;
    pqxx::work                         *xact;

    string                              dbHash
                                      , resultHash;

    stringstream                        query;

    int                                 accID;

    po::variables_map                   map;
    po::options_description             desc("Allowed options");

    desc.add_options()
    ("userName,u", po::value< string >(&config.userName), "user name")
    ("password,p", po::value< string >(&config.password), "user password")
    ("imei,i"    , po::value< string >(&config.imei)    , "device imei")
    ("dbhost"    , po::value< string >(&config.dbHost)  , "psql host")
    ("dbUser"    , po::value< string >(&config.dbUser)  , "psql login")
    ("dbPass"    , po::value< string >(&config.dbPass)  , "psql password")
    ("dbName"    , po::value< string >(&config.dbName)  , "psql database name")
    ;

    std::ifstream file("config.ini");

    if (!file.is_open() && 15 != argc) {
        cerr << "not enough parameters given" << endl << flush;
        cout << desc << endl << flush;
        return 1;
        // NOTREACHED
    }

    // command line first(preferred)
    po::store(
        po::command_line_parser(
              argc
            , argv
        ).options(desc).run()
        , map
    );

    po::notify(map);

    // config file second
    po::store(po::parse_config_file(file, desc), map);
    file.close();
    po::notify(map);

    // check all parameters after parsing is done
    if (   config.dbName.empty() || config.dbPass.empty()
        || config.dbUser.empty() || config.dbHost.empty()
        || config.imei.empty()   || config.userName.empty()
        || config.password.empty())
    {
        cerr << "not enough parameters given" << endl << flush;
        cout << desc << endl << flush;
        return 1;
        // NOTREACHED
    }

    conStr.append(" dbname="   + config.dbName);
    conStr.append(" host="     + config.dbHost);
    conStr.append(" user="     + config.dbUser);
    conStr.append(" password=" + config.dbPass);

    try {
        con = new pqxx::connection(conStr);
    } catch (const pqxx::sql_error &e) {
        cerr << e.what() << endl;
        return 2;
        // NOTREACHED
    }

    xact = new pqxx::work(*con);

    query << "SELECT ID, password FROM accounts WHERE login = '"
          << config.userName << "'";
    try                              { res = xact->exec(query);  }
    catch (const pqxx::sql_error &e) { cerr << e.what() << endl; }

    if (!res.size()) {
        cerr << "there is no account with login " << config.userName << endl << flush;
        return 3;
        // NOTREACHED
    }

    row    = res.begin();
    dbHash = row["password"].as< string >();
    accID  = row["id"].as< int >();

    resultHash = crypt(config.password.c_str(), dbHash.c_str());

    if (dbHash.compare(resultHash)) {
        cerr << "wrong password" << endl << flush;
        return 4;
        // NOTREACHED
    }

    query.str("");

    query << "INSERT INTO SENSORS(imei, accid) VALUES ("
          << "'" << config.imei << "', " << accID
          << ");";

    try                              { res = xact->exec(query);  }
    catch (const pqxx::sql_error &e) { cerr << e.what() << endl; }

    xact->commit();

    con->disconnect();

    cout << "-- device with imei '" << config.imei << "'"
         << " has been successfully registered on '" << config.userName << "'"
         << " account!" << endl << flush;

    return 0;
}

//
//
//
