#include <A4988.h>

// pin 
#define DIR 8
#define STEP 9
#define MS1 10
#define MS2 11
#define MS3 12

// nema17 
#define MOTOR_STEPS 200
#define RPM 500
#define MICROSTEPS 16

A4988 stepper(MOTOR_STEPS,DIR,STEP,MS1,MS2,MS3);

void setup(){
  Serial.begin(115200);
  pinMode(M1,OUTPUT);
  pinMode(M2,OUTPUT);
  pinMode(M3,OUTPUT);
  digitalWrite(DIR,HIGH);
  digitalWrite(M1,HIGH);
  digitalWrite(M2,HIGH);
  digitalWrite(M3,HIGH);
  stepper.begin(RPM,MICROSTEPS);
}


void loop(){
  Serial.println("Start");
  stepper.startMove(100 * MOTOR_STEPS * MICROSTEPS);   
  while(1); // stop
  
}
