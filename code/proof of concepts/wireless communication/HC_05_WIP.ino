#include <SoftwareSerial.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// HC-05: TXD -> D2, RXD <- D3 via spanningsdeler
SoftwareSerial BT(2, 3);

float Kp = 1.0;
float Ki = 0.0;
float Kd = 10.0;

int maxSpeed = 150;
bool debugEnabled = false;
bool robotRunning = false;
bool calibrationRequested = false;

char rxBuffer[48];
byte rxIndex = 0;

void setup() {
  Serial.begin(115200);
  BT.begin(38400);

  BT.println("READY");
  sendSettings();
}

void loop() {
  handleBluetooth();
}

void handleBluetooth() {
  while (BT.available()) {
    char c = BT.read();

    if (c == '\r') continue;

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
}

void processCommand(char* cmd) {
  // Commando's hoofdletter-onafhankelijk maken
  for (char* p = cmd; *p; p++) {
    *p = toupper((unsigned char)*p);
  }

  if (strcmp(cmd, "GET") == 0) {
    sendSettings();
    return;
  }

  if (strcmp(cmd, "START") == 0) {
    robotRunning = true;
    BT.println("OK,START=1");
    return;
  }

  if (strcmp(cmd, "STOP") == 0) {
    robotRunning = false;
    BT.println("OK,START=0");
    return;
  }

  if (strcmp(cmd, "CAL") == 0) {
    calibrationRequested = true;
    BT.println("CAL_REQUESTED");
    return;
  }

  // Andere commando's hebben het formaat NAAM=WAARDE
  char* eq = strchr(cmd, '=');

  if (eq == NULL) {
    BT.println("ERR,UNKNOWN_COMMAND");
    return;
  }

  *eq = '\0';
  char* valueText = eq + 1;
  char* endPtr;

  float value = strtod(valueText, &endPtr);

  if (endPtr == valueText ||
      *endPtr != '\0' ||
      !(value >= 0.0 && value <= 1000.0)) {
    BT.println("ERR,INVALID_VALUE");
    return;
  }

  if (strcmp(cmd, "KP") == 0) {
    Kp = value;
  } else if (strcmp(cmd, "KI") == 0) {
    Ki = value;
  } else if (strcmp(cmd, "KD") == 0) {
    Kd = value;
  } else if (strcmp(cmd, "MAX") == 0) {
    if (value > 255) {
      BT.println("ERR,MAX_RANGE_0_255");
      return;
    }
    maxSpeed = (int)value;
  } else if (strcmp(cmd, "DEBUG") == 0) {
    if (value != 0 && value != 1) {
      BT.println("ERR,DEBUG_USE_0_OR_1");
      return;
    }
    debugEnabled = (value == 1);
  } else {
    BT.println("ERR,UNKNOWN_PARAMETER");
    return;
  }

  BT.println("OK");
  sendSettings();
}

void sendSettings() {
  BT.print("KP=");
  BT.println(Kp, 3);

  BT.print("KI=");
  BT.println(Ki, 3);

  BT.print("KD=");
  BT.println(Kd, 3);

  BT.print("MAX=");
  BT.println(maxSpeed);

  BT.print("DEBUG=");
  BT.println(debugEnabled ? 1 : 0);

  BT.print("RUN=");
  BT.println(robotRunning ? 1 : 0);

  BT.println("END");
}