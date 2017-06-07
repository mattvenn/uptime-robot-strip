#include <ESP8266WiFi.h>
#include "secrets.h"
#include "JsonStreamingParser.h"
#include "JsonListener.h"
#include "UptimeRobotListener.h"

// adafruit library url = https://github.com/adafruit/HL1606-LED-Strip.git
#include <HL1606strip.h>

// use -any- 3 pins!
#define STRIP_D 14 // data
#define STRIP_C 13 // clock 
#define STRIP_L 12 // latch

// Pin S is not really used in this demo since it doesnt use the built in PWM fade
// The last argument is the number of LEDs in the strip. Each chip has 2 LEDs, and the number
// of chips/LEDs per meter varies so make sure to count them! if you have the wrong number
// the strip will act a little strangely, with the end pixels not showing up the way you like
HL1606strip strip = HL1606strip(STRIP_D, STRIP_L, STRIP_C, 20);

// json streaming parser url = https://github.com/squix78/json-streaming-parser.git
JsonStreamingParser parser;
UptimeRobotListener listener;

// how often to poll uptime robot
#define POST_TIME_MS 10000

// state machine states
#define LOG_START 1
#define LOG_WAIT 2
#define LOG_POSTING 3

int log_state = LOG_START;

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
        case LOG_START:
        {
            // start wifi if necessary 
            if(WiFi.status() != WL_CONNECTED) 
                start_wifi();

            start_time = millis();
            log_state = LOG_WAIT;
            break;
        }
        case LOG_WAIT:
        {
            if((millis() - start_time) > POST_TIME_MS)
                log_state = LOG_POSTING;
            break;
        }
        case LOG_POSTING:
        {
            Serial.println("posting");
            fetch_status();
            log_state = LOG_START;
            break;
        }
    }
}

void fetch_status()
{
    WiFiClient client;
    const int httpPort = 80;

    Serial.print("connecting to :");

    if(!client.connect(host, httpPort)) 
    {
        Serial.println("failed to make connection");
        return;
    }

    String url = "/v2/getMonitors";
    Serial.print("with url : ");
    Serial.println(url);

    delay(100);

    String content = "api_key=";
    content += api_key;
    content += "&format=json";
    Serial.print("data : ");
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
