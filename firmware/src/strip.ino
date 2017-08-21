#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
// https example: https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/examples/HTTPSRequest/HTTPSRequest.ino
#include "secrets.h"
#include "JsonStreamingParser.h"
#include "JsonListener.h"
#include "UptimeRobotListener.h"

// adafruit library url = https://github.com/adafruit/HL1606-LED-Strip.git
#include <HL1606strip.h>

#define STRIP_D 14 // data
#define STRIP_C 13 // clock 
#define STRIP_L 12 // latch

#define NUM_LEDS 20

HL1606strip strip = HL1606strip(STRIP_D, STRIP_L, STRIP_C, NUM_LEDS);

// json streaming parser url = https://github.com/squix78/json-streaming-parser.git
JsonStreamingParser parser;
UptimeRobotListener listener;

// how often to poll uptime robot
#define FETCH_TIME_MS 10000

// state machine states
#define ST_START 1
#define ST_WAIT 2
#define ST_FETCH 3

int log_state = ST_START;

void setup()
{
    Serial.begin(115200);
    Serial.println();
    Serial.println();

    for (uint8_t i=0; i < strip.numLEDs(); i++) 
    {
        strip.setLEDcolor(i, RED);
    }
    strip.writeStrip();   

    parser.setListener(&listener);
}

void loop()
{
    static unsigned long start_time;

    //log state machine
    switch(log_state)
    {
        case ST_START:
        {
            // start wifi if necessary 
            if(WiFi.status() != WL_CONNECTED) 
                start_wifi();

            start_time = millis();
            log_state = ST_FETCH;
            break;
        }
        case ST_FETCH:
        {
            Serial.println("fetch");
            fetch_status();
            log_state = ST_WAIT;
            break;
        }
        case ST_WAIT:
        {
            if((millis() - start_time) > FETCH_TIME_MS)
                log_state = ST_START;
            break;
        }
    }
}

void fetch_status()
{
//    WiFiClient client;
    WiFiClientSecure client;
    const int httpPort = 443;

    Serial.print("connecting to host:");
    Serial.print(host);
    Serial.print(":");
    Serial.println(httpPort);

    if(!client.connect(host, httpPort)) 
    {
        Serial.println("failed to make connection");
        return;
    }

    Serial.println("connected");

    String url = "/v2/getMonitors";
    String content = "api_key=";
    content += api_key;
    content += "&format=json";

    Serial.print("posting to url:");
    Serial.println(url);
    Serial.print("post data:");
    Serial.println(content);

    // This will send the request to the server
    client.print(String("POST ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Content-Type: application/x-www-form-urlencoded\r\n" // this header necessary otherwise get crpytic missing api_key error
                 "Content-Length: " + content.length() + "\r\n\r\n" + 
                 content + "\r\n");
                
    int timeout = 0;
    // wait for data to start becoming available
    while(!client.available())
    {
        Serial.print(".");
        if(timeout ++ > 10)
            return;
        delay(100);
    }
    
    // reset parser and listener
    parser.reset();
    listener.reset();
    // keep reading until listener has ended (end of json doc)
    while(! listener.ended())
    {
        char c = client.read();
        parser.parse(c); 
//        Serial.print(c);
    }

    unsigned char num_monitors = listener.get_num_monitors();
    // for each monitor, set led to green or red
    for(int i = 0; i < num_monitors; i ++)
    {
        Serial.print(i);
        Serial.print(":");
        Serial.println(listener.get_status(i));

        // check that num_monitors <= strip.numleds
        if( i < strip.numLEDs() )
            strip.setLEDcolor(i, listener.get_status(i) == 2 ? BLUE : RED); // blue is green for some reason
    }
    // for unused LEDs
    for(int i = num_monitors; i < strip.numLEDs(); i ++)
        strip.setLEDcolor(i, BLACK);

    // update the strip
    strip.writeStrip();   
}

void start_wifi()
{
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    int count = 0;
    // this will block till wifi is available
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    
    if(WiFi.status() == WL_CONNECTED)
    {
        Serial.println("WiFi connected");  
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
    }

    delay(1000);
}
