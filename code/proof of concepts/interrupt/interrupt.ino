const int buttonPin = 8;           
const int statusLed = LED_BUILTIN; 

bool isRunning = false;            
int lastButtonState = HIGH;        
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;   

void setup() {
  pinMode(buttonPin, INPUT_PULLUP); 
  pinMode(statusLed, OUTPUT);
  
  Serial.begin(9600);
  Serial.println("Systeem gereed. Druk op de knop op D8 om te starten.");
}

void loop() {
  int reading = digitalRead(buttonPin);


  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }


  if ((millis() - lastDebounceTime) > debounceDelay) {
 
    if (reading == LOW && lastButtonState == HIGH) {
      isRunning = !isRunning; 
      
      if (isRunning) {
        Serial.println("--> Programma GESTART");
      } else {
        Serial.println("--> Programma GESTOPT");
      }
    }
  }

  lastButtonState = reading; 
  if (isRunning) {
    digitalWrite(statusLed, HIGH);
    


  } else {
    digitalWrite(statusLed, LOW);  
  }
}