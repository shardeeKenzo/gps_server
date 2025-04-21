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
    string password;
    string hash;
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
    ("password, p", po::value< string >(&config.password), "password")
    ("hash, h"    , po::value< string >(&config.hash)    , "hash")
    ;

    // command line first(preferred)
    po::store(
        po::command_line_parser(
              argc
            , argv
        ).options(desc).run()
        , map
    );

    po::notify(map);

    // making hash from password
    newHash    = crypt(config.password.c_str(), generateSalt().c_str());
    
    resultHash = crypt(config.hash.c_str(), newHash.c_str());

    cout << "crypted: :" << newHash << endl << flush;
    
    cout << "result hash :" << resultHash << endl << flush;

    return 0;
}

//
//
//
