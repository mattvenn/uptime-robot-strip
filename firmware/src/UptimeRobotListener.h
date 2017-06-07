#pragma once

#include "JsonListener.h"
#define MAX_MONITORS 20

/*
0 - paused
1 - not checked yet
2 - up
8 - seems down
9 - down
*/

class UptimeRobotListener: public JsonListener {

    public:

    virtual void whitespace(char c);
    virtual void startDocument();
    virtual void key(String key);
    virtual void value(String value);
    virtual void endArray();
    virtual void endObject();
    virtual void endDocument();
    virtual void startArray();
    virtual void startObject();
    virtual unsigned char get_status(unsigned char);
    virtual unsigned char get_num_monitors();
    virtual boolean ended();
    virtual void reset();

    private:

    bool start_status = false;
    bool ended_status = false;
    unsigned char  monitor_status [MAX_MONITORS];
    unsigned char monitor_num = 0;
    
};
