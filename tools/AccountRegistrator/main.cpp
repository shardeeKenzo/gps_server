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
    string adminName;
    string adminPassword;
    string userName;
    string userPassword;
    string dbHost;
    string dbUser;
    string dbName;
    string dbPass;
};

string
generateSalt() {
    // TODO: make more random salt!
    unsigned long seed[2];
    char salt[] = "$1$........";

    const char *const seedchars =
        "./0123456789ABCDEFGHIJKLMNOPQRST"
        "UVWXYZabcdefghijklmnopqrstuvwxyz";

    int i;

    seed[0] = time(NULL);
    seed[1] = getpid() ^ (seed[0] >> 14 & 0x30000);

    /* Turn it into printable characters from `seedchars'. */
    for (i = 0; i < 8; i++)
        salt[3+i] = seedchars[(seed[i/5] >> (i%5)*6) & 0x3f];

    string res(salt);

    return res;
}

int main(int argc, char* argv[])
{
    Config                              config;
    pqxx::connection                   *con;
    string                              conStr;
    pqxx::result                        res;
    pqxx::result::const_iterator        row;
    pqxx::work                         *xact;

    string                              dbHash
                                      , resultHash
                                      , newHash;

    stringstream                        query;

    po::variables_map                   map;
    po::options_description             desc("Allowed options");

    desc.add_options()
    ("adminName"    , po::value< string >(&config.adminName)    , "admin login")
    ("adminPassword", po::value< string >(&config.adminPassword), "admin password")
    ("userName"     , po::value< string >(&config.userName)     , "new user login")
    ("userPassword" , po::value< string >(&config.userPassword) , "new user password")
    ("dbhost"       , po::value< string >(&config.dbHost)       , "psql host")
    ("dbUser"       , po::value< string >(&config.dbUser)       , "psql login")
    ("dbPass"       , po::value< string >(&config.dbPass)       , "psql password")
    ("dbName"       , po::value< string >(&config.dbName)       , "psql database name")
    ;

    std::ifstream file("config.ini");

    if (!file.is_open() && 17 != argc) {
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
    if (   config.dbName.empty()    || config.dbPass.empty()
        || config.dbUser.empty()    || config.dbHost.empty()
        || config.userName.empty()  || config.userPassword.empty()
        || config.adminName.empty() || config.adminPassword.empty())
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

    query << "SELECT password FROM accounts WHERE login = '"
          << config.adminName << "';";
    try                              { res = xact->exec(query);  }
    catch (const pqxx::sql_error &e) { cerr << e.what() << endl; }

    if (!res.size()) {
        cerr << "there is no admin account with login " << config.adminName << endl << flush;
        return 3;
        // NOTREACHED
    }

    row    = res.begin();
    dbHash = row["password"].as< string >();

    resultHash = crypt(config.adminPassword.c_str(), dbHash.c_str());

    if (dbHash.compare(resultHash)) {
        cerr << "wrong admin password" << endl << flush;
        return 4;
        // NOTREACHED
    }

    query.str("");
	query << "SELECT password FROM accounts WHERE login = '" << config.userName << "';";
	try                              { res = xact->exec(query);  }
    catch (const pqxx::sql_error &e) { cerr << e.what() << endl; }
	
	
	query.str("");
	if (res.size()) 
		query << "DELETE FROM accounts WHERE login = '" << config.userName << "';" << endl;
	
    
    // making hash from password
    newHash = crypt(config.userPassword.c_str(), generateSalt().c_str());

    query << "INSERT INTO accounts(login, password) VALUES ("
          << "'" << config.userName << "', '" << newHash << "'"
          << ");";

    try                              { res = xact->exec(query);  }
    catch (const pqxx::sql_error &e) { cerr << e.what() << endl; }

    xact->commit();

    cout << "-- new account '" << config.userName << "'"
         << " has been successfully registered!" << endl << flush;

    con->disconnect();

    return 0;
}

//
//
//
