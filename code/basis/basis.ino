#include <SoftwareSerial.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <EEPROM.h> //toegevoegd (EEPROM geheugen)

//toegevoegd (draadloze communicatie & interrupt pin vrijmaken: BT verplaatst van 2,3 naar 11,12)
SoftwareSerial BT(11, 12); 

const int in1 = 10; 
const int in2 = 5;  
const int in3 = 6;  
const int in4 = 9;  

//toegevoegd (externe interrupt: knop verplaatst naar pin 2 omdat alleen pin 2 en 3 interrupts ondersteunen op Uno/Nano)
const int buttonPin = 2; 

const int sensorPins[] = {A5, A4, A3, A2, A1, 7}; 
const int numSensors = 6;

//toegevoegd (externe interrupt variabelen voor debouncing binnen ISR)
volatile bool buttonPressedFlag = false;
volatile unsigned long lastInterruptTime = 0;

float Kp = 10.0;  
float Ki = 0.0;   
float Kd = 2.0;   

int maxSpeed = 55;    
bool debugEnabled = false;
volatile bool robotRunning = false; //toegevoegd (externe interrupt: volatile gemaakt omdat variabele in ISR wordt aangepast)
bool calibrationRequested = false;

//toegevoegd (cyclustijd in microseconden, standaard 10.000 us = 10 ms)
unsigned long cycleTime = 10000; 
unsigned long previousMicros = 0;

float lastError = 0;
float integral = 0;

char rxBuffer[48];
byte rxIndex = 0;

//toegevoegd (EEPROM structuur voor opslag van alle parameters)
struct RobotSettings {
  int magicNumber; // Controlegetal om te zien of EEPROM al geldige data bevat
  float kp;
  float ki;
  float kd;
  int maxSpd;
  unsigned long cycle;
  bool debug;
};

void setup() {
  Serial.begin(115200);
  BT.begin(38400); //toegevoegd (draadloze communicatie)

  // Motor 
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  // Knop met externe interrupt
  pinMode(buttonPin, INPUT_PULLUP);
  //toegevoegd (externe interrupt koppelen aan pin 2 bij neergaande flank / indrukken knop)
  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonISR, FALLING);

  // Sensor
  for (int i = 0; i < numSensors; i++) {
    pinMode(sensorPins[i], INPUT);
  }

  //toegevoegd (EEPROM geheugen inladen bij opstarten)
  loadSettingsFromEEPROM();

  BT.println("READY");
  sendSettings();
}

void loop() {
  //toegevoegd (draadloze communicatie & SerialCommands uitlezen via BT en USB Serial)
  handleCommunication();

  //toegevoegd (externe interrupt afhandeling in main loop voor veilige BT-melding)
  if (buttonPressedFlag) {
    buttonPressedFlag = false;
    BT.print("OK,START=");
    BT.println(robotRunning ? 1 : 0);
    Serial.print("OK,START=");
    Serial.println(robotRunning ? 1 : 0);
  }

  //toegevoegd (cyclustijd bewaking: voer PID en motorsturing uit op vaste interval)
  unsigned long currentMicros = micros();
  if (currentMicros - previousMicros >= cycleTime) {
    previousMicros = currentMicros;

    //WERKING
    if (robotRunning) {
      if (checkIntersection()) {
        handleIntersection();
      } else {
        int error = calculatePosition();

        // PID 
        integral += error;
        float derivative = error - lastError;
        float correction = (Kp * error) + (Ki * integral) + (Kd * derivative);
        lastError = error;

        int leftSpeed = maxSpeed + correction;
        int rightSpeed = maxSpeed - correction;

        leftSpeed = constrain(leftSpeed, 0, 255);
        rightSpeed = constrain(rightSpeed, 0, 255);

        moveRobot(leftSpeed, rightSpeed);

        //toegevoegd (debug on/off: stuur live sensordata en PID-waarden indien actief)
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
      integral = 0; // Reset integraal bij stilstand
    }
  }
}

//toegevoegd (externe interrupt Service Routine voor start/stop knop)
void buttonISR() {
  unsigned long interruptTime = millis();
  // Debounce van 200ms binnen de interrupt
  if (interruptTime - lastInterruptTime > 200) {
    robotRunning = !robotRunning;
    buttonPressedFlag = true;
    lastInterruptTime = interruptTime;
  }
}

//toegevoegd (draadloze communicatie & SerialCommands: ondersteunt zowel Bluetooth als USB Serial)
void handleCommunication() {
  // Lees van Bluetooth
  while (BT.available()) {
    processIncomingChar(BT.read());
  }
  // Lees van USB Serial Monitor
  while (Serial.available()) {
    processIncomingChar(Serial.read());
  }
}

//toegevoegd (SerialCommands buffer verwerking)
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

//SerialCommands verwerken
void processCommand(char* cmd) {
  for (char* p = cmd; *p; p++) {
    *p = toupper((unsigned char)*p);
  }

  if (strcmp(cmd, "GET") == 0) {
    sendSettings();
    return;
  }

  //toegevoegd (SerialCommands: start/stop)
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
      !(value >= 0.0 && value <= 1000000.0)) { // Limiet verhoogd voor cyclustijd in microseconden
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
  //toegevoegd (SerialCommands: cyclustijd instellen in microseconden, bijv. CYCLE=10000)
  } else if (strcmp(cmd, "CYCLE") == 0) {
    if (value < 1000) { // Minimaal 1ms (1000us) om vastlopen van serial/BT te voorkomen
      BT.println("ERR,CYCLE_MIN_1000US");
      return;
    }
    cycleTime = (unsigned long)value;
  //toegevoegd (SerialCommands: debug on/off instellen via DEBUG=1 of DEBUG=0)
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

  //toegevoegd (EEPROM geheugen: sla gewijzigde instellingen direct op)
  saveSettingsToEEPROM();

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

  //toegevoegd (cyclustijd meesturen in overzicht)
  BT.print("CYCLE=");
  BT.println(cycleTime);

  BT.print("DEBUG=");
  BT.println(debugEnabled ? 1 : 0);

  BT.print("RUN=");
  BT.println(robotRunning ? 1 : 0);

  BT.println("END");
}

//toegevoegd (EEPROM geheugen: functie om instellingen op te slaan)
void saveSettingsToEEPROM() {
  RobotSettings data;
  data.magicNumber = 12345; // Herkenningsgetal
  data.kp = Kp;
  data.ki = Ki;
  data.kd = Kd;
  data.maxSpd = maxSpeed;
  data.cycle = cycleTime;
  data.debug = debugEnabled;

  EEPROM.put(0, data); // EEPROM.put schrijft alleen als de waarde echt veranderd is (spaart geheugen)
}

//toegevoegd (EEPROM geheugen: functie om instellingen in te laden bij opstarten)
void loadSettingsFromEEPROM() {
  RobotSettings data;
  EEPROM.get(0, data);

  // Alleen inladen als het geheugen eerder door dit programma is beschreven
  if (data.magicNumber == 12345) {
    Kp = data.kp;
    Ki = data.ki;
    Kd = data.kd;
    maxSpeed = data.maxSpd;
    cycleTime = data.cycle;
    debugEnabled = data.debug;
  } else {
    // Eerste keer opstarten: sla de standaardwaarden op
    saveSettingsToEEPROM();
  }
}

//KRUISPUNT DETECTEREN
bool checkIntersection() {
  int blackCount = 0;
  for (int i = 0; i < numSensors; i++) {
    int sensorVal = digitalRead(sensorPins[i]);
    if (sensorVal == HIGH) {
      blackCount++;
    }
  }
  if (blackCount == numSensors) {
    return true;
  }
  return false;
}

void handleIntersection() {
  if (debugEnabled) { //toegevoegd (debug on/off controle bij kruispuntmelding)
    BT.println("INFO,INTERSECTION_DETECTED");
  }
  moveRobot(maxSpeed, maxSpeed);
  delay(200); 
}

//BEREKENEN POSITIE 
int calculatePosition() {
  int weights[6] = {-5, -3, -1, 1, 3, 5};
  long weightedSum = 0;
  int activeSensors = 0;

  for (int i = 0; i < numSensors; i++) {
    int sensorVal = digitalRead(sensorPins[i]);
    
    if (sensorVal == HIGH) { 
      weightedSum += weights[i];
      activeSensors++;
    }
  }

  if (activeSensors == 0) {
    return lastError > 0 ? 5 : -5;
  }

  return weightedSum / activeSensors;
}

//MOTORSTURING 
void moveRobot(int leftSpeed, int rightSpeed) {
  digitalWrite(in1, LOW);
  analogWrite(in2, leftSpeed);

  analogWrite(in3, rightSpeed);
  digitalWrite(in4, LOW);
}