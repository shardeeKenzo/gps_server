#ifndef GPSACCEPTOR_TELTONICA_H
#define GPSACCEPTOR_TELTONICA_H

#include "Parser.hpp"
#include "DataTypes.h"

//#define TEST_TELTONICA

enum class Teltonica_SIG{LOGIN, DATA};

class Teltonica: public Parser{
#ifdef TEST_TELTONICA
public:
#endif
    Teltonica_SIG SIG;
    TokenMap parse_one_frame(string& message, string& tmp, int pos);
    TokenMap parse_login(string& message, string& tmp);
    long str_to_int(string& s);
    int n;
    long data_lenght;
    int data_number;
    int frame_size;
public:
    Maps     parse (string aMessage) override ;
    string   answer(int    aReqType, short aResult){}
};

#endif //GPSACCEPTOR_TELTONICA_H
