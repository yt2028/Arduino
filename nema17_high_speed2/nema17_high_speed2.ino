#include <A4988.h>

const int MOTOR_STEPS = 200;
const int DIR  =  8;
const int STEP =  9;
const int M1 = 10;
const int M2 = 11;
const int M3 = 12;
float rpm      = 100;
int microsteps = 16;

A4988 stepper(MOTOR_STEPS, DIR, STEP);

void setup() {
  pinMode(M1,OUTPUT);
  pinMode(M2,OUTPUT);
  pinMode(M3,OUTPUT);
  digitalWrite(DIR,HIGH);
  digitalWrite(M1,HIGH);
  digitalWrite(M2,HIGH);
  digitalWrite(M3,HIGH);
  stepper.begin(rpm, microsteps);
}

void loop() {
  stepper.rotate(360);
 

}
