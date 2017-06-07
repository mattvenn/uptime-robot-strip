#include "UptimeRobotListener.h"
#include "JsonListener.h"

/* unused */
void UptimeRobotListener::whitespace(char c) { }
void UptimeRobotListener::startDocument() { }
void UptimeRobotListener::endArray() { }
void UptimeRobotListener::endObject() { }
void UptimeRobotListener::startArray() { }
void UptimeRobotListener::startObject() { }

unsigned char UptimeRobotListener::get_status(unsigned char num)
{
    return monitor_status[num];
}

unsigned char UptimeRobotListener::get_num_monitors()
{
    return monitor_num;
}

boolean UptimeRobotListener::ended()
{
    return ended_status;
}

void UptimeRobotListener::reset()
{
    ended_status = false;
    monitor_num = 0;
}

void UptimeRobotListener::key(String key) 
{
    if(key == "status")
    {
        start_status = true;
    }
}

void UptimeRobotListener::value(String value) 
{
    if( start_status )
    {
        monitor_status[monitor_num++] = value.toInt();
        start_status = false;
    }
}

void UptimeRobotListener::endDocument() 
{
    Serial.println("end document. ");
    ended_status = true;
}
