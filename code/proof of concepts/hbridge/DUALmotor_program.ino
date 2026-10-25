// Pin-definities voor de DRV8833
const int in1 = 3;
const int in2 = 5;
const int in3 = 6;
const int in4 = 9;

void setup() {

  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
}

void loop() {

  analogWrite(in1, 100);
  digitalWrite(in2, LOW);


  analogWrite(in3, 100);
  digitalWrite(in4, LOW);
  

}