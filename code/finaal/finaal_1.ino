#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Draadloze communicatie: RX op pin 3, TX op pin 4
SoftwareSerial BT(3, 4);

// Motor pinnen (moeten PWM-pinnen zijn: 3, 5, 6, 9, 10, 11 op Uno/Nano)
const int in1 = 6;
const int in2 = 5;
const int in3 = 9;
const int in4 = 10;

// Knop pin op D2 (hardware interrupt INT0)
const int buttonPin = 2;

// Sensor pinnen
const int sensorPins[] = {A5, A4, A3, A2, A1, 7};
const int numSensors = 6;

// EEPROM configuratie
const uint16_t EEPROM_ADDR = 0;
const uint8_t EEPROM_MAGIC = 0xA5;
const uint8_t EEPROM_VERSION = 1;

struct RobotConfig {
  uint8_t magic;
  uint8_t version;
  float kp;
  float ki;
  float kd;
  int maxSpeed;
  float turnSlowdown;
  unsigned long cycleTimeMs;
  bool debugEnabled;
};

// Knop variabelen (interrupt + debounce)
volatile bool buttonInterruptFlag = false;
unsigned long lastButtonPress = 0;

// PID & instelbare parameters
float Kp = 10.0;
float Ki = 0.0;
float Kd = 2.0;

int maxSpeed = 55;
// Hoeveel de robot afremt in bochten (hoger = meer afremmen in scherpe bochten)
float turnSlowdown = 4.0;

// Cyclus tijd in milliseconden (0 = zo snel mogelijk zonder vaste wachttijd)
unsigned long cycleTimeMs = 10;
unsigned long lastCycleTime = 0;

bool debugEnabled = false;
bool robotRunning = false;
bool calibrationRequested = false;

float lastError = 0;
float integral = 0;

// Buffers voor draadloze communicatie (BT) en USB Serial
char btBuffer[48];
byte btIndex = 0;

char usbBuffer[48];
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

  // Parameters laden uit EEPROM (indien geldig aanwezig)
  loadConfigFromEEPROM();

  moveRobot(0, 0);
  sendMessage("READY");
  sendSettings();
}

void loop() {
  handleSerialPorts();
  handleButtonInterrupt();

  // WERKING VAN DE ROBOT
  if (robotRunning) {
    unsigned long now = millis();
    if (cycleTimeMs == 0 || (now - lastCycleTime >= cycleTimeMs)) {
      lastCycleTime = now;

      if (checkIntersection()) {
        handleIntersection();
      } else {
        int error = calculatePosition();

        // PID berekening met anti-windup
        integral = constrain(integral + error, -50, 50);
        float derivative = error - lastError;
        float correction = (Kp * error) + (Ki * integral) + (Kd * derivative);
        lastError = error;

        // Rem automatisch af naarmate de fout (bocht) groter wordt
        int baseSpeed = maxSpeed - (abs(error) * turnSlowdown);
        baseSpeed = constrain(baseSpeed, 20, maxSpeed);

        // Bereken wielsnelheden (begrens buitenste wiel op baseSpeed tegen versnellen in bochten)
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
  lastCycleTime = millis();

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

// EEPROM OPSLAG EN LADEN
void saveConfigToEEPROM() {
  RobotConfig cfg;
  cfg.magic = EEPROM_MAGIC;
  cfg.version = EEPROM_VERSION;
  cfg.kp = Kp;
  cfg.ki = Ki;
  cfg.kd = Kd;
  cfg.maxSpeed = maxSpeed;
  cfg.turnSlowdown = turnSlowdown;
  cfg.cycleTimeMs = cycleTimeMs;
  cfg.debugEnabled = debugEnabled;

  EEPROM.put(EEPROM_ADDR, cfg);
}

void loadConfigFromEEPROM() {
  RobotConfig cfg;
  EEPROM.get(EEPROM_ADDR, cfg);

  if (cfg.magic != EEPROM_MAGIC || cfg.version != EEPROM_VERSION) {
    saveConfigToEEPROM();
    return;
  }

  if (!isnan(cfg.kp) && cfg.kp >= 0.0f && cfg.kp <= 1000.0f) Kp = cfg.kp;
  if (!isnan(cfg.ki) && cfg.ki >= 0.0f && cfg.ki <= 1000.0f) Ki = cfg.ki;
  if (!isnan(cfg.kd) && cfg.kd >= 0.0f && cfg.kd <= 1000.0f) Kd = cfg.kd;
  if (cfg.maxSpeed >= 0 && cfg.maxSpeed <= 255) maxSpeed = cfg.maxSpeed;
  if (!isnan(cfg.turnSlowdown) && cfg.turnSlowdown >= 0.0f && cfg.turnSlowdown <= 100.0f) {
    turnSlowdown = cfg.turnSlowdown;
  }
  if (cfg.cycleTimeMs <= 1000) cycleTimeMs = cfg.cycleTimeMs;
  debugEnabled = cfg.debugEnabled;
}

void resetDefaults() {
  Kp = 10.0;
  Ki = 0.0;
  Kd = 2.0;
  maxSpeed = 55;
  turnSlowdown = 4.0;
  cycleTimeMs = 10;
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
  while (*cmd && isspace((unsigned char)*cmd)) {
    cmd++;
  }
  size_t len = strlen(cmd);
  while (len > 0 && isspace((unsigned char)cmd[len - 1])) {
    cmd[--len] = '\0';
  }

  for (char* p = cmd; *p; p++) {
    *p = toupper((unsigned char)*p);
  }

  if (strcmp(cmd, "GET") == 0) {
    sendSettings();
    return;
  }

  if (strcmp(cmd, "START") == 0) {
    setRunState(true);
    return;
  }

  if (strcmp(cmd, "STOP") == 0) {
    setRunState(false);
    return;
  }

  if (strcmp(cmd, "SAVE") == 0) {
    saveConfigToEEPROM();
    sendMessage("OK,SAVED");
    return;
  }

  if (strcmp(cmd, "RESET") == 0) {
    resetDefaults();
    sendMessage("OK,DEFAULTS_RESTORED");
    sendSettings();
    return;
  }

  if (strcmp(cmd, "CAL") == 0) {
    calibrationRequested = true;
    sendMessage("CAL_REQUESTED");
    return;
  }

  char* eq = strchr(cmd, '=');
  if (eq == NULL) {
    sendMessage("ERR,UNKNOWN_COMMAND");
    return;
  }

  *eq = '\0';
  char* valueText = eq + 1;
  char* endPtr;

  float value = strtod(valueText, &endPtr);

  if (endPtr == valueText ||
      *endPtr != '\0' ||
      !(value >= 0.0 && value <= 1000.0)) {
    sendMessage("ERR,INVALID_VALUE");
    return;
  }

  if (strcmp(cmd, "KP") == 0) {
    Kp = value;
  } else if (strcmp(cmd, "KI") == 0) {
    Ki = value;
    resetPID();
  } else if (strcmp(cmd, "KD") == 0) {
    Kd = value;
  } else if (strcmp(cmd, "MAX") == 0) {
    if (value > 255) {
      sendMessage("ERR,MAX_RANGE_0_255");
      return;
    }
    maxSpeed = (int)value;
  } else if (strcmp(cmd, "CYCLE") == 0) {
    cycleTimeMs = (unsigned long)value;
  } else if (strcmp(cmd, "DEBUG") == 0) {
    if (value != 0 && value != 1) {
      sendMessage("ERR,DEBUG_USE_0_OR_1");
      return;
    }
    debugEnabled = (value == 1);
  } else {
    sendMessage("ERR,UNKNOWN_PARAMETER");
    return;
  }

  saveConfigToEEPROM();
  sendMessage("OK");
  sendSettings();
}

void sendSettings() {
  BT.print("KP=");
  BT.println(Kp, 3);
  Serial.print("KP=");
  Serial.println(Kp, 3);

  BT.print("KI=");
  BT.println(Ki, 3);
  Serial.print("KI=");
  Serial.println(Ki, 3);

  BT.print("KD=");
  BT.println(Kd, 3);
  Serial.print("KD=");
  Serial.println(Kd, 3);

  BT.print("MAX=");
  BT.println(maxSpeed);
  Serial.print("MAX=");
  Serial.println(maxSpeed);

  BT.print("CYCLE=");
  BT.println(cycleTimeMs);
  Serial.print("CYCLE=");
  Serial.println(cycleTimeMs);

  BT.print("DEBUG=");
  BT.println(debugEnabled ? 1 : 0);
  Serial.print("DEBUG=");
  Serial.println(debugEnabled ? 1 : 0);

  BT.print("RUN=");
  BT.println(robotRunning ? 1 : 0);
  Serial.print("RUN=");
  Serial.println(robotRunning ? 1 : 0);

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
    if (digitalRead(sensorPins[i]) == HIGH) {
      blackCount++;
    }
  }
  return (blackCount == numSensors);
}

void handleIntersection() {
  sendMessage("INFO,INTERSECTION_DETECTED");
  moveRobot(maxSpeed, maxSpeed);
  delay(200);
}

// BEREKENEN POSITIE
int calculatePosition() {
  int weights[6] = {-5, -3, -1, 1, 3, 5};
  long weightedSum = 0;
  int activeSensors = 0;

  for (int i = 0; i < numSensors; i++) {
    if (digitalRead(sensorPins[i]) == HIGH) {
      weightedSum += weights[i];
      activeSensors++;
    }
  }

  if (activeSensors == 0) {
    return lastError > 0 ? 5 : -5;
  }

  return weightedSum / activeSensors;
}

// MOTORSTURING (ondersteunt vooruit en achteruit draaien)
void moveRobot(int leftSpeed, int rightSpeed) {
  // Linkermotor
  if (leftSpeed >= 0) {
    analogWrite(in1, leftSpeed);
    digitalWrite(in2, LOW);
  } else {
    digitalWrite(in1, LOW);
    analogWrite(in2, -leftSpeed);
  }

  // Rechtermotor
  if (rightSpeed >= 0) {
    analogWrite(in3, rightSpeed);
    digitalWrite(in4, LOW);
  } else {
    digitalWrite(in3, LOW);
    analogWrite(in4, -rightSpeed);
  }
}