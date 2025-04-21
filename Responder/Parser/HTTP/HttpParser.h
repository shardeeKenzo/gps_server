#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include <boost/logic/tribool.hpp>
#include <string>

#include "Parser.hpp"
#include "DataTypes.h"

using namespace std;

class HttpParser : public Parser
{
public:
    HttpParser();

    void   initialize();

    Maps   parse(const string& aString);
    string answer(int aReqType, short aResult);

    static string parseSid(const string& aCookie);

private:
    /// Handle the next character of input.
    boost::tribool consume(TokenMap& aMap, char input);

    /// Check if a byte is an HTTP character.
    bool is_char(int c);

    /// Check if a byte is an HTTP control character.
    bool is_ctl(int c);

    /// Check if a byte is defined as an HTTP tspecial character.
    bool is_tspecial(int c);

    /// Check if a byte is a digit.
    bool is_digit(int c);

    /// The current state of the parser.
    enum state {
        method_start,
        method,
        uri,
        http_version_h,
        http_version_t_1,
        http_version_t_2,
        http_version_p,
        http_version_slash,
        http_version_major_start,
        http_version_major,
        http_version_minor_start,
        http_version_minor,
        expecting_newline_1,
        header_line_start,
        header_lws,
        header_name,
        space_before_header_value,
        header_value,
        expecting_newline_2,
        expecting_newline_3
    } _state;

    struct Header {
        string name;
        string value;
    };

    string         _method;
    string         _uri;
    int            _http_version_major;
    int            _http_version_minor;
    vector<Header> _headers;
};

#endif //HTTP_PARSER_HPP

//
//
//
