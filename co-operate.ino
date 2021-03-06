#include <SerialCommand.h>
#define onLED 7
#define engineLED 8
#define shieldLED 3

#define testSwitch 6
#define keySwitch 5
#define startButton 4
#define boostSwitch 2

#define slider A0
#define rotary A1

#define testSwitchIndex 0
#define keySwitchIndex 1
#define startButtonIndex 2
#define boostSwitchIndex 3

#define sliderIndex 0
#define rotaryIndex 1

#define START_LED_ON "STRT:1"
#define START_LED_OFF "STRT:0"
#define ENGINE_LED_ON "ENG:1"
#define ENGINE_LED_OFF "ENG:0"
#define SHIELD_LED_ON "SHLD:1"
#define SHIELD_LED_OFF "SHLD:0"

SerialCommand sCmd;

int switchState[4];
int lastSwitchState[4];
unsigned long lastSwitchDebounceTime[4];

int potValue[2];
int lastPotValue[2];

const long debounceDelay = 10;
const int maxQueuedStateChanges = 20;

int queuedStateChangesCount = 0;
String queuedStateChanges[maxQueuedStateChanges];

void setup() {
  pinMode(onLED, OUTPUT);
  pinMode(engineLED, OUTPUT);
  pinMode(shieldLED, OUTPUT);
  pinMode(testSwitch, INPUT_PULLUP);
  pinMode(keySwitch, INPUT_PULLUP);
  pinMode(startButton, INPUT_PULLUP);
  pinMode(boostSwitch, INPUT_PULLUP);
  digitalWrite(onLED, LOW);
  
  Serial.begin(9600);
 
  sCmd.addCommand("STRT", startHandler);
  sCmd.addCommand("A", actionHandler);
  sCmd.addCommand("R", requestHandler);
  sCmd.setDefaultHandler(unrecognized);
}

void loop() {
  sCmd.readSerial();

  handleSwitch(testSwitch, testSwitchIndex, "TEST_SWITCH");
  handleSwitch(keySwitch, keySwitchIndex, "KEY_SWITCH");
  handleSwitch(startButton, startButtonIndex, "START_BTN");
  handleSwitch(boostSwitch, boostSwitchIndex, "BOOST_SWITCH");
}

void startHandler() {
  digitalWrite(onLED, HIGH);
}

void actionHandler() {
  char *arg;
  arg = sCmd.next();
  while (arg != NULL) {
    if (String(arg) == START_LED_ON) {
      digitalWrite(onLED, HIGH);
    } else if (String(arg) == START_LED_OFF) {
      digitalWrite(onLED, LOW);
    } else if (String(arg) == ENGINE_LED_ON) {
      digitalWrite(engineLED, HIGH);
    } else if (String(arg) == ENGINE_LED_OFF) {
      digitalWrite(engineLED, LOW);
    } else if (String(arg) == SHIELD_LED_ON) {
      digitalWrite(shieldLED, HIGH);
    } else if (String(arg) == SHIELD_LED_OFF) {
      digitalWrite(shieldLED, LOW);
    } else if (String(arg) == "STOP") {
      digitalWrite(onLED, LOW);
    }
    arg = sCmd.next();
  }
}

void requestHandler() {
  handlePot(slider, sliderIndex, "SLIDER");
  handlePot(rotary, rotaryIndex, "ROTARY");
  
  if (queuedStateChangesCount == 0) return;

  String response;
  // Build response
  for (int i = 0; i < queuedStateChangesCount; i++)
  {
    response = response + "," + queuedStateChanges[i];
  }
  response.remove(0, 1);
  
  Serial.println(response);
  queuedStateChangesCount = 0;
}

void handleSwitch(int switchPin, int switchIndex, String switchName) {
  switchState[switchIndex] = digitalRead(switchPin);

  // Return if nothing changed
  if (switchState[switchIndex] == lastSwitchState[switchIndex]) return; 

  // If it was less then delay ago, we ignore it.
  if ((millis() - lastSwitchDebounceTime[switchIndex]) < debounceDelay) return;
  
  // Now we're good to go
  lastSwitchDebounceTime[switchIndex] = millis();

  if (switchState[switchIndex] == HIGH) {
    storeNewState(switchName + ":0");
  } else {
    storeNewState(switchName + ":1");
  }

  lastSwitchState[switchIndex] = switchState[switchIndex];
}

void handlePot(int potPin, int potIndex, String potName) {
  potValue[potIndex] = analogRead(potPin);

  if (potValue[potIndex] <= lastPotValue[potIndex] + 1 && potValue[potIndex] >= lastPotValue[potIndex] - 1) return;

  storeNewState(potName + ":" + (potValue[potIndex]/1023.0));

  lastPotValue[potIndex] = potValue[potIndex];
}

void unrecognized(const char *command) {
  // For more complex commands?
}

void storeNewState(String state) {
    queuedStateChanges[queuedStateChangesCount] = state;
    queuedStateChangesCount++;
}

