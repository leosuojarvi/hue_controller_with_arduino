/*************************************************************
  ESP8266 part of the hue controller
  Contains information to be filled with users own
*************************************************************/

// Communication codes
#define SWITCHVAL 0
#define IDLEFT 1
#define IDRIGHT 2
#define GROUPON 10
#define GROUPOFF 11

// Communication modifiers
#define NOMODIFIER 0
#define JOYMODIFIER 100
#define SONARMODIFIER 150

// How many light units (0-255) is one cm in distance
#define MULTIPLIER 15

#define STARTBRI 100 //Optional
#define MAXBRI 254

#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

// IP of Hue gateway
String ip = ""; //Fill


// Hue gateway user name
String user_name = ""; //Fill

// Wifi network SSID
const char* ssid = ""; //Fill

// Wifi network password
const char* password = ""; //Fill

// Changes putRequest
bool group = false;

// IDs of the lamps in use
const int lamps[] = {2, 3, 5, 9};
int lampIndex = 0;
int lastIndex = sizeof(lamps)/sizeof(lamps[0])-1;

unsigned int lastSend = 0;

WiFiClient wclient;
HTTPClient http;

int bri = 100; //Some starting value
int lastBri = 100;
int r = 0;

void setup() {

  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected to the WiFi network");

}

// Checks if there is commands in buffer
void loop() {
  if (Serial.available() > 0) {
    r = Serial.read();

    switch (r) {
      // Turn light full/off
      case SWITCHVAL:
        if(bri == 0){
          bri = MAXBRI;
        }
        else {
          bri = 0;
        }
        break;

      // Id left
      case IDLEFT:
        //breath(false);
        if(lampIndex <= 0) lampIndex = lastIndex;
        else lampIndex -= 1;
        //bri = STARTBRI; // Optional
        lastBri = -1;
        //breath(true);
        break;

      // Id right
      case IDRIGHT:
        //breath(false);
        if (lampIndex >= lastIndex) lampIndex = 0;
        else lampIndex += 1;
        //bri = STARTBRI; // Uuden lampun aloitusarvo, jos haluaa
        lastBri = -1;
        //breath(true);
        break;

      case GROUPON:
        group = true;
        setBri();
        break;

      case GROUPOFF:
        group = false;
        setBri();
        break;
        
      // Adjust brightness
      default:
        if(r >= 150){
          bri = (r - 150)*MULTIPLIER;
        }
        else bri += (r - 100);
        
        if(bri > MAXBRI) bri = MAXBRI;
        else if(bri < 0) bri = 0;
        break;
    }
  }
  
  // Checks if there is need for value change
  if(lastBri != bri) {
    setBri();
  }
}

// Sends a put request once or for each lamp according to group-value
int putRequest (String put_string) {
  int limit = (group)? lastIndex : 0;
  String req_string;
  int httpResponseCode = 0;
  for(int i = 0; i <= limit; i++) {
    req_string = "http://";
    req_string += ip;
    req_string += "/api/";
    req_string += user_name;
    req_string += "/lights/";
    req_string += (group)? lamps[i]: lamps[lampIndex];
    req_string += "/state";
  
    http.begin(wclient, req_string);
    http.addHeader("Content-Type", "text/plain");
  
    httpResponseCode = http.PUT(put_string);
    Serial.println(put_string);
    http.end();
  }
  
  return httpResponseCode;
}

// Selected lamp breathes
void breath(bool b) {

  String put_string;
  put_string = "{\"alert\": ";
  put_string += (b) ? "\"select\"" : "\"none\"";
  put_string += "}";

  if (putRequest(put_string) <= 0) {
    Serial.println("breath send failed");
  }
  Serial.println("Breath");
}

// Turns light on and sets brightness
void setBri() {
  if(millis()-lastSend > 50 && lastBri != bri) {  
    String put_string;
    put_string = "{\"on\":";
    put_string += (bri>0) ? "true" : "false";
    put_string += ",";
 
    put_string += " \"bri\":";
    put_string += bri;
    put_string += ",";

    put_string += "\"transitiontime\":";
    put_string += 3; // Value x 100 ms, 3 => 200 ms
    put_string += "}";
  
    if (putRequest(put_string) <= 0) {
        Serial.println("setBri send failed");
    }
    lastSend = millis();
    lastBri = bri;
  }
}
