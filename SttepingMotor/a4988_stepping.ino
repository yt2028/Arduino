#include <A4988.h>

const int MOTOR_STEPS = 200;
const int DIR  =  8;
const int STEP =  9;

float rpm      = 10;
int microsteps = 30;

A4988 stepper(MOTOR_STEPS, DIR, STEP);

void setup() {
  stepper.begin(rpm, microsteps);
}

void loop() {
  stepper.rotate(360);
  delay(1000);

  stepper.rotate(-360);
  delay(1000);
}
