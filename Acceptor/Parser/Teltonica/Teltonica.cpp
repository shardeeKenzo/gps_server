#include "Teltonica.h"
#include <iostream>

Maps Teltonica::parse(string aMessage) {
    Maps maps;
    if (aMessage.empty()) return maps;
    string tmp=aMessage.substr(0,2);
    if (str_to_int(tmp)==0x000f){
        SIG=Teltonica_SIG::LOGIN;
        maps.push_back(parse_login(aMessage, tmp));
        return maps;
    }
    if (aMessage.size()<45){
        std::cerr<<"error with frames in parsing Teltonica input data";
    }
    int pos=0;
    tmp=aMessage.substr(0,4);
    if (str_to_int(tmp)==0x0000'0000){
        SIG=Teltonica_SIG::DATA;
        pos+=4;
        tmp=aMessage.substr(pos, 4);
        data_lenght=str_to_int(tmp);
        pos+=4;
        //process codec
        if (aMessage[pos++]!='\b'){
            cerr<<"error Codec id don't == 8";
        }
        //process codec
        //process number_of_data1 and 2
        data_number=int(aMessage[pos]);
        if (aMessage[pos+data_lenght-2]!=aMessage[pos]){
            cerr<<"Number of data don't match";
        }
        pos++;
        //process number_of_data1 and 2
        //calculate frame_size
        frame_size=(data_lenght-3)/data_number;
        if ((data_lenght-3)%data_number!=0){
            cerr<<"UPS";
        }
        for (int i=0; i<data_number;i++){
            maps.push_back(parse_one_frame(aMessage, tmp, pos));
            pos+=frame_size;
        }
        return maps;
        //TODO check integrity of data (CRC-16)
    }


}


TokenMap Teltonica::parse_one_frame(string &message, string& tmp, int pos) {
    TokenMap tmap;
    //process time
    tmp=message.substr(pos, 8);
    tmap["time"]=to_string(str_to_int(tmp));
    pos+=8;
    //process time

    //ignore priority
    pos++;
    //ignore priority

    //process lan
    tmp=message.substr(pos, 4);
    tmap["lan"]=to_string(str_to_int(tmp));
    pos+=4;
    //process lan

    //process lat
    tmp=message.substr(pos, 4);
    tmap["lat"]=to_string(str_to_int(tmp));
    pos+=4;
    //process lat

    //process alt
    tmp=message.substr(pos, 2);
    tmap["alt"]=to_string(str_to_int(tmp));
    pos+=2;
    //process alt

    //process direction
    tmp=message.substr(pos, 2);
    tmap["direction"]=to_string(str_to_int(tmp));
    pos+=2;
    //process direction

    //process satcnt
    tmp=message.substr(pos, 1);
    tmap["satcnt"]=to_string(str_to_int(tmp));
    pos+=1;
    //process satcnt

    //process speed
    tmp=message.substr(pos, 2);
    tmap["speed"]=to_string(str_to_int(tmp));
    pos+=2;
    //process speed

    return tmap;
}

long Teltonica::str_to_int(string &s) {
    long res=0;
    long p=1;
    for (int i=s.size()-1;i>=0;i--){
        if (long(s[i])<0){
            res+=(256+long(s[i]))*p;
        }
        else res+=long(s[i])*p;
        p*=256;
    }
    return res;
}

TokenMap Teltonica::parse_login(string &message, string& tmp) {
    tmp=message.substr(2);
    TokenMap tmap;
    tmap["imei"]=tmp;
    return tmap;
}
