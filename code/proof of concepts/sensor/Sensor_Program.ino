
const int powerPin = 8;     


const int sensorPins[] = {7, A1, A2, A3, A4, A5}; 
const String pinNames[] = {"D7", "A1", "A2", "A3", "A4", "A5"};
const int numSensors = 6;

void setup() {
 
  Serial.begin(9600);

  pinMode(powerPin, OUTPUT);
  digitalWrite(powerPin, HIGH);
  

  for (int i = 0; i < numSensors; i++) {
    pinMode(sensorPins[i], INPUT);
  }
  
  Serial.println("Sensortest gestart (D4 = HIGH, sensoren op D7 en A1 t/m A5)...");
}

void loop() {
  Serial.print("Sensoren: ");
  

  for (int i = 0; i < numSensors; i++) {
    int waarde = digitalRead(sensorPins[i]);
    
    Serial.print(pinNames[i]);
    Serial.print(":");
    Serial.print(waarde);
    Serial.print("   ");
  }
  
  Serial.println(); 
  
 
  delay(5000); 
}
