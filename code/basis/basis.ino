#include <SoftwareSerial.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <EEPROM.h>

//
SoftwareSerial BT(2, 3); 

const int in1 = 10; 
const int in2 = 5;  
const int in3 = 6;  
const int in4 = 9;  

//
const int buttonPin = 4; 

const int sensorPins[] = {A5, A4, A3, A2, A1, 7}; 
const int numSensors = 6;

int blackValues[6] = {800, 800, 800, 800, 800, 1};
int whiteValues[6] = {200, 200, 200, 200, 200, 0};
int thresholds[6]  = {500, 500, 500, 500, 500, 1};

volatile bool buttonPressedFlag = false;
volatile unsigned long lastInterruptTime = 0;

float Kp = 10.0;  
float Ki = 0.0;   
float Kd = 2.0;   

int power = 55;          
float diff = 1.0;        
bool debugEnabled = false;
volatile bool robotRunning = false;

unsigned long cycleTime = 10000; 
unsigned long previousMicros = 0;

float lastError = 0;
float integral = 0;

char rxBuffer[48];
byte rxIndex = 0;

struct RobotSettings {
  int magicNumber;
  float kp;
  float ki;
  float kd;
  int pwr;
  float diffVal;
  unsigned long cycle;
  bool debug;
  int thresh[6];
};

void setup() {
  Serial.begin(115200);
  BT.begin(38400); 

  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  pinMode(buttonPin, INPUT_PULLUP);
  
  //toegevoegd (Pin Change Interrupt activeren op pin 4 / PD4 voor ATmega328P Uno/Nano)
  PCICR |= (1 << PCIE2);    // Schakel Pin Change Interrupts in voor Port D (pinnen D0 t/m D7)
  PCMSK2 |= (1 << PCINT20); // Kies specifiek pin D4 (PCINT20) als interrupt-bron

  for (int i = 0; i < numSensors; i++) {
    pinMode(sensorPins[i], INPUT);
  }

  loadSettingsFromEEPROM();

  BT.println("READY");
  sendSettings();
}

void loop() {
  handleCommunication();

  if (buttonPressedFlag) {
    buttonPressedFlag = false;
    BT.print("OK,START=");
    BT.println(robotRunning ? 1 : 0);
  }

  unsigned long currentMicros = micros();
  if (currentMicros - previousMicros >= cycleTime) {
    previousMicros = currentMicros;

    if (robotRunning) {
      if (checkIntersection()) {
        handleIntersection();
      } else {
        int error = calculatePosition();

        // PID 
        integral += error;
        float derivative = error - lastError;
        float correction = ((Kp * error) + (Ki * integral) + (Kd * derivative)) * diff;
        lastError = error;

        int leftSpeed = power + correction;
        int rightSpeed = power - correction;

        leftSpeed = constrain(leftSpeed, 0, 255);
        rightSpeed = constrain(rightSpeed, 0, 255);

        moveRobot(leftSpeed, rightSpeed);

        if (debugEnabled) {
          BT.print("DBG,ERR=");
          BT.print(error);
          BT.print(",CORR=");
          BT.print(correction);
          BT.print(",L=");
          BT.print(leftSpeed);
          BT.print(",R=");
          BT.println(rightSpeed);
        }
      }
    } else {
      moveRobot(0, 0);
      integral = 0;
    }
  }
}

//toegevoegd (Interrupt Service Routine voor pin 4 via Pin Change Interrupt)
ISR(PCINT2_vect) {
  // Controleer of pin 4 (PD4) laag is (knop ingedrukt bij INPUT_PULLUP)
  if (!(PIND & (1 << PIND4))) {
    unsigned long interruptTime = millis();
    if (interruptTime - lastInterruptTime > 200) {
      robotRunning = !robotRunning;
      buttonPressedFlag = true;
      lastInterruptTime = interruptTime;
    }
  }
}

void handleCommunication() {
  while (BT.available()) {
    processIncomingChar(BT.read());
  }
  while (Serial.available()) {
    processIncomingChar(Serial.read());
  }
}

void processIncomingChar(char c) {
  if (c == '\r') return;

  if (c == '\n') {
    rxBuffer[rxIndex] = '\0';
    if (rxIndex > 0) {
      processCommand(rxBuffer);
    }
    rxIndex = 0;
  } else if (rxIndex < sizeof(rxBuffer) - 1) {
    rxBuffer[rxIndex++] = c;
  } else {
    rxIndex = 0;
    BT.println("ERR,LINE_TOO_LONG");
  }
}

void processCommand(char* cmd) {
  for (char* p = cmd; *p; p++) {
    *p = tolower((unsigned char)*p);
  }
  int len = strlen(cmd);
  while (len > 0 && isspace((unsigned char)cmd[len - 1])) {
    cmd[--len] = '\0';
  }

  if (strcmp(cmd, "get") == 0) {
    sendSettings();
    return;
  }
  if (strcmp(cmd, "start") == 0) {
    robotRunning = true;
    BT.println("OK,START=1");
    return;
  }
  if (strcmp(cmd, "stop") == 0) {
    robotRunning = false;
    BT.println("OK,START=0");
    return;
  }
  if (strcmp(cmd, "debug on") == 0) {
    debugEnabled = true;
    saveSettingsToEEPROM();
    BT.println("OK,DEBUG=ON");
    return;
  }
  if (strcmp(cmd, "debug off") == 0) {
    debugEnabled = false;
    saveSettingsToEEPROM();
    BT.println("OK,DEBUG=OFF");
    return;
  }
  if (strcmp(cmd, "calibrate black") == 0) {
    calibrateSensors(true);
    return;
  }
  if (strcmp(cmd, "calibrate white") == 0) {
    calibrateSensors(false);
    return;
  }

  if (strncmp(cmd, "set ", 4) == 0) {
    char* param = cmd + 4;
    char* spacePos = strchr(param, ' ');

    if (spacePos == NULL) {
      BT.println("ERR,MISSING_VALUE");
      return;
    }

    *spacePos = '\0';
    char* valueText = spacePos + 1;
    char* endPtr;
    float value = strtod(valueText, &endPtr);

    if (endPtr == valueText || *endPtr != '\0') {
      BT.println("ERR,INVALID_NUMBER");
      return;
    }

    if (strcmp(param, "cycle") == 0) {
      if (value < 1000) {
        BT.println("ERR,CYCLE_MIN_1000");
        return;
      }
      cycleTime = (unsigned long)value;
    } else if (strcmp(param, "power") == 0) {
      if (value < 0 || value > 255) {
        BT.println("ERR,POWER_0_255");
        return;
      }
      power = (int)value;
    } else if (strcmp(param, "diff") == 0) {
      if (value < 0.0 || value > 1.0) {
        BT.println("ERR,DIFF_0_TO_1");
        return;
      }
      diff = value;
    } else if (strcmp(param, "kp") == 0) {
      if (value < 0.0) { BT.println("ERR,NEGATIVE_VALUE"); return; }
      Kp = value;
    } else if (strcmp(param, "ki") == 0) {
      if (value < 0.0) { BT.println("ERR,NEGATIVE_VALUE"); return; }
      Ki = value;
    } else if (strcmp(param, "kd") == 0) {
      if (value < 0.0) { BT.println("ERR,NEGATIVE_VALUE"); return; }
      Kd = value;
    } else {
      BT.println("ERR,UNKNOWN_SET_PARAM");
      return;
    }

    saveSettingsToEEPROM();
    BT.println("OK");
    sendSettings();
    return;
  }

  BT.println("ERR,UNKNOWN_COMMAND");
}

void calibrateSensors(bool isBlack) {
  BT.print(isBlack ? "CALIBRATING_BLACK: " : "CALIBRATING_WHITE: ");
  
  for (int i = 0; i < numSensors; i++) {
    int val;
    if (i < 5) {
      val = analogRead(sensorPins[i]);
    } else {
      val = digitalRead(sensorPins[i]);
    }

    if (isBlack) {
      blackValues[i] = val;
    } else {
      whiteValues[i] = val;
    }

    if (i < 5) {
      thresholds[i] = (blackValues[i] + whiteValues[i]) / 2;
    } else {
      thresholds[i] = 1;
    }

    BT.print(val);
    BT.print(" ");
  }
  BT.println();

  saveSettingsToEEPROM();
  BT.println("OK,CALIBRATION_SAVED");
}

bool isSensorBlack(int index) {
  if (index < 5) {
    int val = analogRead(sensorPins[index]);
    if (blackValues[index] >= whiteValues[index]) {
      return val > thresholds[index];
    } else {
      return val < thresholds[index];
    }
  } else {
    return digitalRead(sensorPins[index]) == HIGH;
  }
}

void sendSettings() {
  BT.print("KP="); BT.println(Kp, 3);
  BT.print("KI="); BT.println(Ki, 3);
  BT.print("KD="); BT.println(Kd, 3);
  BT.print("POWER="); BT.println(power);
  BT.print("DIFF="); BT.println(diff, 3);
  BT.print("CYCLE="); BT.println(cycleTime);
  BT.print("DEBUG="); BT.println(debugEnabled ? "ON" : "OFF");
  BT.print("RUN="); BT.println(robotRunning ? 1 : 0);
  BT.println("END");
}

void saveSettingsToEEPROM() {
  RobotSettings data;
  data.magicNumber = 54321;
  data.kp = Kp;
  data.ki = Ki;
  data.kd = Kd;
  data.pwr = power;
  data.diffVal = diff;
  data.cycle = cycleTime;
  data.debug = debugEnabled;
  for (int i = 0; i < 6; i++) {
    data.thresh[i] = thresholds[i];
  }

  EEPROM.put(0, data);
}

void loadSettingsFromEEPROM() {
  RobotSettings data;
  EEPROM.get(0, data);

  if (data.magicNumber == 54321) {
    Kp = data.kp;
    Ki = data.ki;
    Kd = data.kd;
    power = data.pwr;
    diff = data.diffVal;
    cycleTime = data.cycle;
    debugEnabled = data.debug;
    for (int i = 0; i < 6; i++) {
      thresholds[i] = data.thresh[i];
    }
  } else {
    saveSettingsToEEPROM();
  }
}

bool checkIntersection() {
  int blackCount = 0;
  for (int i = 0; i < numSensors; i++) {
    if (isSensorBlack(i)) {
      blackCount++;
    }
  }
  return (blackCount == numSensors);
}

void handleIntersection() {
  if (debugEnabled) {
    BT.println("INFO,INTERSECTION_DETECTED");
  }
  moveRobot(power, power);
  delay(200); 
}

int calculatePosition() {
  int weights[6] = {-5, -3, -1, 1, 3, 5};
  long weightedSum = 0;
  int activeSensors = 0;

  for (int i = 0; i < numSensors; i++) {
    if (isSensorBlack(i)) { 
      weightedSum += weights[i];
      activeSensors++;
    }
  }

  if (activeSensors == 0) {
    return lastError > 0 ? 5 : -5;
  }

  return weightedSum / activeSensors;
}

void moveRobot(int leftSpeed, int rightSpeed) {
  digitalWrite(in1, LOW);
  analogWrite(in2, leftSpeed);

  analogWrite(in3, rightSpeed);
  digitalWrite(in4, LOW);
}
