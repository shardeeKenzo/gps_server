#ifndef UTILS_H
#define UTILS_H

#include <sstream>
#include <fstream>
#include <time.h>
#include <stdlib.h>

#include <time.h>

#include <boost/program_options.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/insert_linebreaks.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/ostream_iterator.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include "Globals.hpp"

using namespace std;
namespace po = boost::program_options;

inline
void
readConfig(
      po::options_description& aDesc
    , po::variables_map& aMap
)
{
    ifstream file("config.ini");

    if (!file.is_open()) return;
    // NOTREACHED

    // Clear the map.
    //aMap = po::variables_map();

    po::store(po::parse_config_file(file, aDesc, true), aMap);
    file.close();
    po::notify(aMap);
}

/*!
 * Convert up to len bytes of binary data in src to base64 and store it in dest
 *
 * \param dest Destination buffer to hold the base64 data.
 * \param src Source binary data.
 * \param len The number of bytes of src to convert.
 *
 * \return The number of characters written to dest.
 * \remarks Does not store a terminating null in dest.
 */
inline
uint
base64_encode(char* dest, const char* src, uint len) {
    using namespace boost::archive::iterators;

    char tail[3] = {0,0,0};
    typedef base64_from_binary<transform_width<const char *, 6, 8> > base64_enc;

    uint one_third_len = len/3;
    uint len_rounded_down = one_third_len*3;
    uint j = len_rounded_down + one_third_len;

    copy(base64_enc(src), base64_enc(src + len_rounded_down), dest);

    if (len_rounded_down != len) {
        uint i=0;
        for(; i < len - len_rounded_down; ++i) {
            tail[i] = src[len_rounded_down+i];
        }

        copy(base64_enc(tail), base64_enc(tail + 3), dest + j);

        for(i=len + one_third_len + 1; i < j+4; ++i) {
            dest[i] = '=';
        }

        return i;
        // NOTREACHED
    }

    return j;
}

/*!
 * Convert null-terminated string src from base64 to binary and store it in dest.
 *
 * \brief base64_decode
 * \param dest Destination buffer
 * \param src Source base64 string
 * \param len Pointer to unsigned int representing size of dest buffer. After function returns this is set to the number of character written to dest.
 * \return Pointer to first character in source that could not be converted (the terminating null on success)
 */
inline
const char*
base64_decode(char* dest, const char* src, uint* len) {
    using namespace boost::archive::iterators;

    uint output_len = *len;

    typedef transform_width<binary_from_base64<const char*>, 8, 6> base64_dec;

    uint i=0;
    try {
        base64_dec src_it(src);
        for(; i < output_len; ++i) {
            *dest++ = *src_it;
            ++src_it;
        }
    }
    catch(dataflow_exception&) {}

    *len = i;
    return src + (i+2)/3*4; // bytes in = bytes out / 3 rounded up * 4
}

inline
string
generateRandomStr(int len) {
    stringstream res;
    string chars(
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "1234567890"
        "!@#$%^&*()"
        "`~-_=+[{]{\\|;:'\",<.>/? ");
    //boost::random::random_device rng;
    //boost::random::uniform_int_distribution<> index_dist(0, chars.size() - 1);
    for(int i = 0; i < len; ++i) {
        //res << chars[index_dist(rng)];
        res << chars[ rand() % chars.length()];
    }

    return res.str();
}

inline
string
getTimeString() {
    char strtime[30];

    time_t rawtime; time( &rawtime );
    struct tm * timeinfo = localtime ( &rawtime );
    strftime (strtime, 30, "[%d.%m.%y %H:%M:%S] ", timeinfo);

    return string(strtime);
}

inline
string
getTimeString(time_t rawtime) {
    char strtime[30];

    struct tm * timeinfo = localtime ( &rawtime );
    strftime (strtime, 30, "[%d.%m.%y %H:%M:%S] ", timeinfo);

    return string(strtime);
}

#endif // UTILS_H

//
//
//
