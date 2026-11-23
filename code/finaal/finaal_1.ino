#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Draadloze communicatie: RX op pin 3, TX op pin 4
SoftwareSerial BT(3, 4);

// Motor pinnen (PWM-pinnen op Arduino Nano)
const int in1 = 6;
const int in2 = 5;
const int in3 = 9;
const int in4 = 10;

// Knop pin op D2 (hardware interrupt INT0)
const int buttonPin = 2;

// Sensor pinnen (A1..A5 analoog, D7 digitaal)
const int sensorPins[] = {A5, A4, A3, A2, A1, 7};
const int numSensors = 6;

// EEPROM configuratie
const uint16_t EEPROM_ADDR = 0;
const uint8_t EEPROM_MAGIC = 0xA5;
const uint8_t EEPROM_VERSION = 2;

struct RobotConfig {
  uint8_t magic;
  uint8_t version;
  float kp;
  float ki;
  float kd;
  int maxPower;
  float diffFactor;          // Bereik 0.0 .. 1.0
  unsigned long cycleTimeUs; // Cyclustijd in microseconden
  bool debugEnabled;
  int whiteValues[6];
  int blackValues[6];
  int thresholdValues[6];
};

// Knop variabelen (interrupt + debounce)
volatile bool buttonInterruptFlag = false;
unsigned long lastButtonPress = 0;

// PID & instelbare parameters
float Kp = 10.0;
float Ki = 0.0;
float Kd = 2.0;

int maxPower = 55;             // 0..255
float diffFactor = 0.4;        // 0..1
unsigned long cycleTimeUs = 10000; // 10000 µs = 10 ms
unsigned long lastCycleTimeUs = 0;

// Kalibratiewaarden
int whiteValues[numSensors] = {100, 100, 100, 100, 100, 0};
int blackValues[numSensors] = {800, 800, 800, 800, 800, 1};
int thresholdValues[numSensors] = {450, 450, 450, 450, 450, 1};

bool debugEnabled = false;
bool robotRunning = false;

float lastError = 0;
float integral = 0;

// Buffers voor draadloze communicatie (BT) en USB Serial
char btBuffer[64];
byte btIndex = 0;

char usbBuffer[64];
byte usbIndex = 0;

void buttonISR() {
  buttonInterruptFlag = true;
}

void setup() {
  Serial.begin(115200);
  BT.begin(38400);

  // Motor pinnen
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  // Knop pin + externe interrupt op D2
  pinMode(buttonPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonISR, FALLING);

  // Sensor pinnen
  for (int i = 0; i < numSensors; i++) {
    pinMode(sensorPins[i], INPUT);
  }

  // Parameters en kalibratie laden uit EEPROM
  loadConfigFromEEPROM();

  moveRobot(0, 0);
  sendMessage("READY");
  sendSettings();
}

void loop() {
  handleSerialPorts();
  handleButtonInterrupt();

  if (robotRunning) {
    unsigned long nowUs = micros();
    if (cycleTimeUs == 0 || (nowUs - lastCycleTimeUs >= cycleTimeUs)) {
      lastCycleTimeUs = nowUs;

      if (checkIntersection()) {
        handleIntersection();
      } else {
        int error = calculatePosition();

        // PID berekening met anti-windup
        integral = constrain(integral + error, -50, 50);
        float derivative = error - lastError;
        float correction = (Kp * error) + (Ki * integral) + (Kd * derivative);
        lastError = error;

        // Afremmen in bochten op basis van diff [0..1]
        float slowdownRatio = constrain((abs(error) / 5.0f) * diffFactor, 0.0f, 0.85f);
        int baseSpeed = maxPower * (1.0f - slowdownRatio);
        baseSpeed = constrain(baseSpeed, 20, maxPower);

        // Bereken wielsnelheden
        int leftSpeed = constrain(baseSpeed - correction, -baseSpeed, baseSpeed);
        int rightSpeed = constrain(baseSpeed + correction, -baseSpeed, baseSpeed);

        moveRobot(leftSpeed, rightSpeed);

        if (debugEnabled) {
          sendDebugInfo(error, correction, leftSpeed, rightSpeed);
        }
      }
    }
  } else {
    moveRobot(0, 0);
  }
}

void handleButtonInterrupt() {
  if (buttonInterruptFlag) {
    noInterrupts();
    buttonInterruptFlag = false;
    interrupts();

    unsigned long now = millis();
    if (now - lastButtonPress > 250 && digitalRead(buttonPin) == LOW) {
      lastButtonPress = now;
      toggleRunState();
    }
  }
}

void setRunState(bool run) {
  robotRunning = run;
  resetPID();
  lastCycleTimeUs = micros();

  if (!robotRunning) {
    moveRobot(0, 0);
  }

  sendMessage(robotRunning ? "OK,START=1" : "OK,START=0");
}

void toggleRunState() {
  setRunState(!robotRunning);
}

void resetPID() {
  lastError = 0;
  integral = 0;
}

// SENSOR UITLEZEN (ONDERSTEUNT ZOWEL ANALOGE PINNEN A1..A5 ALS DIGITALE PIN D7)
int readRawSensor(int index) {
  if (sensorPins[index] >= A0) {
    return analogRead(sensorPins[index]);
  } else {
    return digitalRead(sensorPins[index]);
  }
}

bool isSensorBlack(int index) {
  int val = readRawSensor(index);
  if (sensorPins[index] >= A0) {
    if (blackValues[index] >= whiteValues[index]) {
      return val >= thresholdValues[index];
    } else {
      return val <= thresholdValues[index];
    }
  } else {
    return (val == HIGH);
  }
}

// KALIBRATIE FUNCTIES
void calibrateWhite() {
  for (int i = 0; i < numSensors; i++) {
    long sum = 0;
    for (int s = 0; s < 10; s++) {
      sum += readRawSensor(i);
      delay(5);
    }
    whiteValues[i] = sum / 10;
    thresholdValues[i] = (whiteValues[i] + blackValues[i]) / 2;
  }
  saveConfigToEEPROM();
  sendMessage("OK,CALIBRATE_WHITE");
}

void calibrateBlack() {
  for (int i = 0; i < numSensors; i++) {
    long sum = 0;
    for (int s = 0; s < 10; s++) {
      sum += readRawSensor(i);
      delay(5);
    }
    blackValues[i] = sum / 10;
    thresholdValues[i] = (whiteValues[i] + blackValues[i]) / 2;
  }
  saveConfigToEEPROM();
  sendMessage("OK,CALIBRATE_BLACK");
}

// EEPROM OPSLAG EN LADEN
void saveConfigToEEPROM() {
  RobotConfig cfg;
  cfg.magic = EEPROM_MAGIC;
  cfg.version = EEPROM_VERSION;
  cfg.kp = Kp;
  cfg.ki = Ki;
  cfg.kd = Kd;
  cfg.maxPower = maxPower;
  cfg.diffFactor = diffFactor;
  cfg.cycleTimeUs = cycleTimeUs;
  cfg.debugEnabled = debugEnabled;
  for (int i = 0; i < numSensors; i++) {
    cfg.whiteValues[i] = whiteValues[i];
    cfg.blackValues[i] = blackValues[i];
    cfg.thresholdValues[i] = thresholdValues[i];
  }

  EEPROM.put(EEPROM_ADDR, cfg);
}

void loadConfigFromEEPROM() {
  RobotConfig cfg;
  EEPROM.get(EEPROM_ADDR, cfg);

  if (cfg.magic != EEPROM_MAGIC || cfg.version != EEPROM_VERSION) {
    saveConfigToEEPROM();
    return;
  }

  if (!isnan(cfg.kp) && cfg.kp >= 0.0f) Kp = cfg.kp;
  if (!isnan(cfg.ki) && cfg.ki >= 0.0f) Ki = cfg.ki;
  if (!isnan(cfg.kd) && cfg.kd >= 0.0f) Kd = cfg.kd;
  if (cfg.maxPower >= 0 && cfg.maxPower <= 255) maxPower = cfg.maxPower;
  if (!isnan(cfg.diffFactor) && cfg.diffFactor >= 0.0f && cfg.diffFactor <= 1.0f) {
    diffFactor = cfg.diffFactor;
  }
  if (cfg.cycleTimeUs <= 1000000UL) cycleTimeUs = cfg.cycleTimeUs;
  debugEnabled = cfg.debugEnabled;

  for (int i = 0; i < numSensors; i++) {
    whiteValues[i] = cfg.whiteValues[i];
    blackValues[i] = cfg.blackValues[i];
    thresholdValues[i] = cfg.thresholdValues[i];
  }
}

void resetDefaults() {
  Kp = 10.0;
  Ki = 0.0;
  Kd = 2.0;
  maxPower = 55;
  diffFactor = 0.4;
  cycleTimeUs = 10000;
  debugEnabled = false;
  resetPID();
  saveConfigToEEPROM();
}

// DRAADLOZE EN SERIELE COMMUNICATIE
void sendMessage(const char* msg) {
  BT.println(msg);
  Serial.println(msg);
}

void handleSerialPorts() {
  while (BT.available()) {
    char c = BT.read();
    if (c == '\r') continue;

    if (c == '\n') {
      btBuffer[btIndex] = '\0';
      if (btIndex > 0) {
        processCommand(btBuffer);
      }
      btIndex = 0;
    } else if (btIndex < sizeof(btBuffer) - 1) {
      btBuffer[btIndex++] = c;
    } else {
      btIndex = 0;
      sendMessage("ERR,LINE_TOO_LONG");
    }
  }

  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\r') continue;

    if (c == '\n') {
      usbBuffer[usbIndex] = '\0';
      if (usbIndex > 0) {
        processCommand(usbBuffer);
      }
      usbIndex = 0;
    } else if (usbIndex < sizeof(usbBuffer) - 1) {
      usbBuffer[usbIndex++] = c;
    } else {
      usbIndex = 0;
      sendMessage("ERR,LINE_TOO_LONG");
    }
  }
}

void processCommand(char* cmd) {
  while (*cmd && isspace((unsigned char)*cmd)) cmd++;
  size_t len = strlen(cmd);
  while (len > 0 && isspace((unsigned char)cmd[len - 1])) cmd[--len] = '\0';

  for (char* p = cmd; *p; p++) {
    *p = tolower((unsigned char)*p);
  }

  if (strcmp(cmd, "start") == 0) {
    setRunState(true);
    return;
  }

  if (strcmp(cmd, "stop") == 0) {
    setRunState(false);
    return;
  }

  if (strcmp(cmd, "get") == 0) {
    sendSettings();
    return;
  }

  if (strcmp(cmd, "save") == 0) {
    saveConfigToEEPROM();
    sendMessage("OK,SAVED");
    return;
  }

  if (strcmp(cmd, "reset") == 0) {
    resetDefaults();
    sendMessage("OK,DEFAULTS_RESTORED");
    sendSettings();
    return;
  }

  if (strcmp(cmd, "debug on") == 0) {
    debugEnabled = true;
    saveConfigToEEPROM();
    sendMessage("OK,DEBUG=ON");
    return;
  }

  if (strcmp(cmd, "debug off") == 0) {
    debugEnabled = false;
    saveConfigToEEPROM();
    sendMessage("OK,DEBUG=OFF");
    return;
  }

  if (strcmp(cmd, "calibrate black") == 0) {
    calibrateBlack();
    return;
  }

  if (strcmp(cmd, "calibrate white") == 0) {
    calibrateWhite();
    return;
  }

  // Verwerk 'set <parameter> <waarde>' commando's
  if (strncmp(cmd, "set ", 4) == 0) {
    char* param = cmd + 4;
    while (*param && isspace((unsigned char)*param)) param++;

    char* space = strchr(param, ' ');
    if (space == NULL) {
      sendMessage("ERR,MISSING_VALUE");
      return;
    }

    *space = '\0';
    char* valueText = space + 1;
    while (*valueText && isspace((unsigned char)*valueText)) valueText++;

    char* endPtr;
    float value = strtod(valueText, &endPtr);

    if (endPtr == valueText || *endPtr != '\0' || isnan(value)) {
      sendMessage("ERR,INVALID_VALUE");
      return;
    }

    if (strcmp(param, "cycle") == 0) {
      if (value < 0) {
        sendMessage("ERR,CYCLE_MIN_0");
        return;
      }
      cycleTimeUs = (unsigned long)value;
    } else if (strcmp(param, "power") == 0) {
      if (value < 0 || value > 255) {
        sendMessage("ERR,POWER_RANGE_0_255");
        return;
      }
      maxPower = (int)value;
    } else if (strcmp(param, "diff") == 0) {
      if (value < 0.0f || value > 1.0f) {
        sendMessage("ERR,DIFF_RANGE_0_1");
        return;
      }
      diffFactor = value;
    } else if (strcmp(param, "kp") == 0) {
      if (value < 0.0f) {
        sendMessage("ERR,KP_MIN_0");
        return;
      }
      Kp = value;
    } else if (strcmp(param, "ki") == 0) {
      if (value < 0.0f) {
        sendMessage("ERR,KI_MIN_0");
        return;
      }
      Ki = value;
      resetPID();
    } else if (strcmp(param, "kd") == 0) {
      if (value < 0.0f) {
        sendMessage("ERR,KD_MIN_0");
        return;
      }
      Kd = value;
    } else {
      sendMessage("ERR,UNKNOWN_PARAMETER");
      return;
    }

    saveConfigToEEPROM();
    sendMessage("OK");
    sendSettings();
    return;
  }

  sendMessage("ERR,UNKNOWN_COMMAND");
}

void sendSettings() {
  BT.print("cycle="); BT.println(cycleTimeUs);
  Serial.print("cycle="); Serial.println(cycleTimeUs);

  BT.print("power="); BT.println(maxPower);
  Serial.print("power="); Serial.println(maxPower);

  BT.print("diff="); BT.println(diffFactor, 3);
  Serial.print("diff="); Serial.println(diffFactor, 3);

  BT.print("kp="); BT.println(Kp, 3);
  Serial.print("kp="); Serial.println(Kp, 3);

  BT.print("ki="); BT.println(Ki, 3);
  Serial.print("ki="); Serial.println(Ki, 3);

  BT.print("kd="); BT.println(Kd, 3);
  Serial.print("kd="); Serial.println(Kd, 3);

  BT.print("debug="); BT.println(debugEnabled ? "on" : "off");
  Serial.print("debug="); Serial.println(debugEnabled ? "on" : "off");

  BT.print("run="); BT.println(robotRunning ? 1 : 0);
  Serial.print("run="); Serial.println(robotRunning ? 1 : 0);

  sendMessage("END");
}

void sendDebugInfo(int error, float correction, int leftSpeed, int rightSpeed) {
  BT.print("DBG,ERR=");
  BT.print(error);
  BT.print(",CORR=");
  BT.print(correction, 1);
  BT.print(",L=");
  BT.print(leftSpeed);
  BT.print(",R=");
  BT.println(rightSpeed);
}

// KRUISPUNT DETECTEREN
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
  sendMessage("INFO,INTERSECTION_DETECTED");
  moveRobot(maxPower, maxPower);
  delay(200);
}

// BEREKENEN POSITIE
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

// MOTORSTURING
void moveRobot(int leftSpeed, int rightSpeed) {
  if (leftSpeed >= 0) {
    analogWrite(in1, leftSpeed);
    digitalWrite(in2, LOW);
  } else {
    digitalWrite(in1, LOW);
    analogWrite(in2, -leftSpeed);
  }

  if (rightSpeed >= 0) {
    analogWrite(in3, rightSpeed);
    digitalWrite(in4, LOW);
  } else {
    digitalWrite(in3, LOW);
    analogWrite(in4, -rightSpeed);
  }
}