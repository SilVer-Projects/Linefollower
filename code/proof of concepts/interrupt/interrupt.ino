const int buttonPin = 2;
volatile bool buttonInterruptFlag = false;
unsigned long lastButtonPress = 0;

void buttonISR() {
  buttonInterruptFlag = true;
}

void setupInterrupt() {
  pinMode(buttonPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonISR, FALLING);
}

void handleButtonInterrupt() {
  if (buttonInterruptFlag) {
    noInterrupts();
    buttonInterruptFlag = false;
    interrupts();

    unsigned long now = millis();
    if (now - lastButtonPress > 250 && digitalRead(buttonPin) == LOW) {
      lastButtonPress = now;
      robotRunning = !robotRunning;
      resetPID();
      if (!robotRunning) {
        moveRobot(0, 0);
      }
      BT.print("OK,START=");
      BT.println(robotRunning ? 1 : 0);
    }
  }
}