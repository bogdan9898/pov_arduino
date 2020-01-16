#define potentiometerPin A0
#define buttonPin 2
#define ena 6
#define in1 7
#define in2 8

#define gnd 12
#define vcc 11

#define maxSpeed 115//185//68//115

bool dir = true;
int prevButtonVal;
int prevSpeedVal = 0;

void setup() {
  pinMode(gnd, OUTPUT);
  pinMode(vcc, OUTPUT);
  digitalWrite(gnd, LOW);
  digitalWrite(vcc, HIGH);

  pinMode(10, OUTPUT);
  pinMode(9, OUTPUT);
  digitalWrite(10, LOW);
  digitalWrite(9, HIGH);
  
  pinMode(ena, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);

  setDir(dir);
  
  pinMode(potentiometerPin, INPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  prevButtonVal = digitalRead(buttonPin);
  
  // setup mode
  for(int i = 0; i < 3; i ++) {
    setSpeed(20);
    delay(500);
    setSpeed(0);   
    delay(500); 
  }

  for(int i = 50; i <= maxSpeed; i+=5) {
    setSpeed(i);
    delay(1000);
  }

  setSpeed(maxSpeed);
  // setup mode

//  Serial.begin(9600);
}

void loop() {
//  int buttonVal = digitalRead(2);
//  if(prevButtonVal != buttonVal && buttonVal == HIGH) {
//    dir = !dir;
//    setDir(dir);
//  }
//  prevButtonVal = buttonVal;
//  int speedVal = analogRead(potentiometerPin);
//  speedVal = map(speedVal, 0, 1024, 0, 255);
//  setSpeed(speedVal);
//  Serial.println(speedVal);
}

void setSpeed(int speedVal) {
//    if(speedVal <= 43)
//      speedVal = 0;
    analogWrite(ena, speedVal);
}

void setDir(bool clockwise) {
  if(clockwise){
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
  } else {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
  }
}
