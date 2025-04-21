#include "HttpParser.h"
#include <boost/algorithm/string.hpp>

HttpParser::HttpParser() {
    initialize();
}

void
HttpParser::initialize() {
    _state              = method_start;
    _method             = "";
    _uri                = "";
    _http_version_major = 0;
    _http_version_minor = 0;
    _headers.clear();
}

string
HttpParser::answer(int /*aReqType*/, short /*aResult*/) {
    // PLACEHOLDER
    return string("");
}

Maps
HttpParser::parse(const string& aString) {
    initialize();

    TokenMap map;
    Maps     maps;
    int      i
           , len;
    string   key;

    string::const_iterator begin = aString.begin();
    string::const_iterator end   = aString.end();

    while (begin != end) {
        boost::tribool result = consume(map, *begin++);
        if (!result) {
            return maps;
            // NOTREACHED
        }
    }

    map["uri"]                = _uri;
    map["method"]             = _method;
    map["http_version_major"] = _http_version_major;
    map["http_version_minor"] = _http_version_minor;

    len = _headers.size();
    for (i = 0; i < len; i++) {
        key      = _headers[i].name;
        map[key] = _headers[i].value;
    }

    maps.push_back(map);

    return maps;
}

boost::tribool HttpParser::consume(TokenMap& aMap, char input) {
    switch (_state) {
    case method_start:
        if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
            return false;
            // NOTREACHED
        }
        else {
            _state = method;
            _method.push_back(input);
            return boost::indeterminate;
            // NOTREACHED
        }
    case method:
        if (input == ' ') {
            _state = uri;
            return boost::indeterminate;
            // NOTREACHED
        }
        else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
            return false;
            // NOTREACHED
        }
        else {
            _method.push_back(input);
            return boost::indeterminate;
            // NOTREACHED
        }
    case uri:
        if (input == ' ') {
            _state = http_version_h;
            return boost::indeterminate;
            // NOTREACHED
        }
        else if (is_ctl(input)) {
            return false;
            // NOTREACHED
        }
        else {
            _uri.push_back(input);
            return boost::indeterminate;
            // NOTREACHED
        }
    case http_version_h:
        if (input == 'H') {
            _state = http_version_t_1;
            return boost::indeterminate;
            // NOTREACHED
        }
        else {
            return false;
            // NOTREACHED
        }
    case http_version_t_1:
        if (input == 'T') {
            _state = http_version_t_2;
            return boost::indeterminate;
            // NOTREACHED
        }
        else {
            return false;
            // NOTREACHED
        }
    case http_version_t_2:
        if (input == 'T') {
            _state = http_version_p;
            return boost::indeterminate;
            // NOTREACHED
        }
        else {
            return false;
            // NOTREACHED
        }
    case http_version_p:
        if (input == 'P') {
            _state = http_version_slash;
            return boost::indeterminate;
            // NOTREACHED
        }
        else {
            return false;
            // NOTREACHED
        }
    case http_version_slash:
        if (input == '/') {
            _http_version_major = 0;
            _http_version_minor = 0;
            _state = http_version_major_start;
            return boost::indeterminate;
            // NOTREACHED
        }
        else {
            return false;
            // NOTREACHED
        }
    case http_version_major_start:
        if (is_digit(input)) {
            _http_version_major = _http_version_major * 10 + input - '0';
            _state = http_version_major;
            return boost::indeterminate;
            // NOTREACHED
        }
        else {
            return false;
            // NOTREACHED
        }
    case http_version_major:
        if (input == '.') {
            _state = http_version_minor_start;
            return boost::indeterminate;
            // NOTREACHED
        }
        else if (is_digit(input)) {
            _http_version_major = _http_version_major * 10 + input - '0';
            return boost::indeterminate;
            // NOTREACHED
        }
        else {
            return false;
            // NOTREACHED
        }
    case http_version_minor_start:
        if (is_digit(input)) {
            _http_version_minor = _http_version_minor * 10 + input - '0';
            _state = http_version_minor;
            return boost::indeterminate;
            // NOTREACHED
        }
        else {
          return false;
          // NOTREACHED
        }
    case http_version_minor:
        if (input == '\r') {
            _state = expecting_newline_1;
            return boost::indeterminate;
            // NOTREACHED
        }
        else if (is_digit(input)) {
            _http_version_minor = _http_version_minor * 10 + input - '0';
            return boost::indeterminate;
            // NOTREACHED
        }
        else {
          return false;
          // NOTREACHED
        }
    case expecting_newline_1:
        if (input == '\n') {
            _state = header_line_start;
            return boost::indeterminate;
            // NOTREACHED
        }
        else {
            return false;
            // NOTREACHED
        }
    case header_line_start:
        if (input == '\r') {
            _state = expecting_newline_3;
            return boost::indeterminate;
        }
        else if (!_headers.empty() && (input == ' ' || input == '\t')) {
            _state = header_lws;
            return boost::indeterminate;
            // NOTREACHED
        }
        else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
            return false;
            // NOTREACHED
        }
        else {
            _headers.push_back(Header());
            _headers.back().name.push_back(input);
            _state = header_name;
            return boost::indeterminate;
            // NOTREACHED
        }
    case header_lws:
        if (input == '\r') {
            _state = expecting_newline_2;
            return boost::indeterminate;
            // NOTREACHED
        }
        else if (input == ' ' || input == '\t') {
            return boost::indeterminate;
            // NOTREACHED
        }
        else if (is_ctl(input)) {
            return false;
            // NOTREACHED
        }
        else {
            _state = header_value;
            _headers.back().value.push_back(input);
            return boost::indeterminate;
            // NOTREACHED
        }
    case header_name:
        if (input == ':') {
            _state = space_before_header_value;
            return boost::indeterminate;
            // NOTREACHED
        }
        else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
            return false;
            // NOTREACHED
        }
        else {
            _headers.back().name.push_back(input);
            return boost::indeterminate;
            // NOTREACHED
        }
    case space_before_header_value:
        if (input == ' ') {
            _state = header_value;
            return boost::indeterminate;
            // NOTREACHED
        }
        else {
            return false;
            // NOTREACHED
        }
    case header_value:
        if (input == '\r') {
            _state = expecting_newline_2;
            return boost::indeterminate;
            // NOTREACHED
        }
        else if (is_ctl(input)) {
            return false;
            // NOTREACHED
        }
        else {
            _headers.back().value.push_back(input);
            return boost::indeterminate;
            // NOTREACHED
        }
    case expecting_newline_2:
        if (input == '\n') {
            _state = header_line_start;
            return boost::indeterminate;
            // NOTREACHED
        }
        else {
            return false;
            // NOTREACHED
        }
    case expecting_newline_3:
        return (input == '\n');
        // NOTREACHED
    default:
        return false;
        // NOTREACHED
    }
}

bool HttpParser::is_char(int c) {
    return c >= 0 && c <= 127;
}

bool HttpParser::is_ctl(int c) {
    return (c >= 0 && c <= 31) || (c == 127);
}

bool HttpParser::is_tspecial(int c) {
    switch (c) {
        case '(': case ')': case '<': case '>': case '@':
        case ',': case ';': case ':': case '\\': case '"':
        case '/': case '[': case ']': case '?': case '=':
        case '{': case '}': case ' ': case '\t':
        return true;
    default:
        return false;
    }
}

bool HttpParser::is_digit(int c) {
    return c >= '0' && c <= '9';
}

string
HttpParser::parseSid(const string &aCookie) {
    if (string::npos == aCookie.find("sid=")) return string("");
    // NOTREACHED

    string res = aCookie.substr(4, aCookie.length() - 4);

    return res;
}

//
//
//
