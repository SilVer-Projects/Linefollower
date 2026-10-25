// Pin D4 levert de 5V voeding voor de sensor
const int powerPin = 4;     

// De 6 sensorpiningangen (sensor D2 zit nu op D7, de rest op A1 t/m A5)
const int sensorPins[] = {7, A1, A2, A3, A4, A5}; 
const String pinNames[] = {"D7", "A1", "A2", "A3", "A4", "A5"};
const int numSensors = 6;

void setup() {
  // Start seriële communicatie
  Serial.begin(9600);
  
  // Zet D4 hoog om de sensor van stroom te voorzien
  pinMode(powerPin, OUTPUT);
  digitalWrite(powerPin, HIGH);
  
  // Stel alle sensorpinnen in als digitale ingang
  for (int i = 0; i < numSensors; i++) {
    pinMode(sensorPins[i], INPUT);
  }
  
  Serial.println("Sensortest gestart (D4 = HIGH, sensoren op D7 en A1 t/m A5)...");
}

void loop() {
  Serial.print("Sensoren: ");
  
  // Lees alle sensoren uit en print de naam + waarde
  for (int i = 0; i < numSensors; i++) {
    int waarde = digitalRead(sensorPins[i]);
    
    Serial.print(pinNames[i]);
    Serial.print(":");
    Serial.print(waarde);
    Serial.print("   ");
  }
  
  Serial.println(); // Nieuwe regel
  
  // Wacht 5 seconden voor de volgende meting
  delay(5000); 
}