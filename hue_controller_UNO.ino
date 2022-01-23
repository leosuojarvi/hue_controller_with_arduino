/*************************************************************
  Arduino Uno part of the hue controller
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

#define JOYX A0
#define JOYY A1
#define SWITCHPIN 2
#define TRIGGER_PIN 12
#define ECHO_PIN 11
#define MAX_DISTANCE 60

#define STEADYLIMIT 4
#define MINHEIGHT 9
#define ARRAYSIZE 40

#include <NewPing.h>
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

double defYPos = 0.0;
double defXPos = 0.0;
unsigned long lastSwitch = 0;
unsigned long lastLoop = 0;
  
int message;
int lastMessage = -1;
int measures[ARRAYSIZE]; // Values initialized to 0
int mPtr = 0; // Pointer to latest added value

int minVal = 0;
int maxVal = 0;
int oldMax = 0;

bool groupState = false;

void setup() {
  Serial.begin(115200);
  defYPos = analogRead(JOYY);
  defXPos = analogRead(JOYX);
  pinMode(SWITCHPIN, INPUT_PULLUP);

}

void loop() {
  // HC-SR04 can read with minimum of 33 ms intervals. This also keeps the controlling speed sensible.
  if(millis() - lastLoop > 50) {  
    if (joystickSwitch()) sendMessage(NOMODIFIER); // SWITCHVAL, IDLEFT or IDRIGHT
    else if (joystickAdjust()) sendMessage(JOYMODIFIER); // Value between [-10 11] + 100 to distinguish from other adjustments
    
    if (sonarAdjust()) sendMessage(SONARMODIFIER); //Value > 0 + 150
    if (group()) sendMessage(NOMODIFIER); // GROUPON or GROUPOFF
    lastLoop = millis();
  }
}

void sendMessage(int modifier) {
  int newMessage = message + modifier;
  // Other than sonar can send same value repeatedly
  if(modifier != SONARMODIFIER || newMessage != lastMessage) {
    Serial.write(newMessage);
    //Serial.println(newMessage);
    lastMessage = newMessage;
  }
}


bool joystickSwitch() {
  bool ret = false;
  // Prevents accidental switches
  if (millis() - lastSwitch > 600) {
    int xValue = analogRead(JOYX);

    // Toggles light on/off
    if (digitalRead(SWITCHPIN) == 0) {
      message = SWITCHVAL;
      lastSwitch = millis();
      ret = true;
    }
    // Swicth id to left
    else if (xValue < 23) {
      message = IDLEFT;
      lastSwitch = millis();
      ret = true;
    }
    // Switch id to right
    else if (xValue > 1000) {
      message = IDRIGHT;
      lastSwitch = millis();
      ret = true;
    }
  }
  return ret;
}

bool joystickAdjust() {
  bool ret = false;
  int adjust = round( (defYPos - analogRead(JOYY)) / 50);
  if (adjust != 0) {
    message = adjust;
    ret = true;
  }
  return ret;
}

bool group() {
  bool ret = false;

  // Is hand moved away from the sensor?
  if(oldMax > 0 && maxVal == 0) {
    message = GROUPOFF;
    groupState = false;
    ret = true;
  }
  // Is hand steady within STEADYLIMIT range?
  else if (!groupState && (minVal != 0 && maxVal - minVal <= STEADYLIMIT)) {
    message = GROUPON;
    groupState = true;
    ret = true;
  }
  oldMax = maxVal;
  
  return ret;
}

bool sonarAdjust() {
  bool ret = false;
  int d = sonar.ping_cm();

  // Updates pointer and adds new measure
  int old = measures[mPtr];
  measures[mPtr] = d;
  if (mPtr == ARRAYSIZE - 1) mPtr = 0;
  else mPtr ++;

  // Updates min and max if necessary. For d < and > are enough, for old <= and >= are necessary
  if(d != old && ( (d < minVal || old <= minVal) || (d > maxVal || old >= maxVal) )) {
    updateMinMax();
  }

  if (d != 0) {
    message = max(d - MINHEIGHT, 0); // Less than MINHEIGHT means 0 brightness
    ret = true;
  }
  return ret;
}

void updateMinMax() {
  minVal = measures[0];
  maxVal = measures[0];

  for (int i = 0; i < ARRAYSIZE; i++) {
    minVal = min(measures[i], minVal);
    maxVal = max(measures[i], maxVal);
  }
}
