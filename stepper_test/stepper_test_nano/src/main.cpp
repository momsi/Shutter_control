#include <Arduino.h>

const int stepPin = A1;
const int dirPin = A0;
const int steps_per_rev = 40;
const int step_delay = 1;

void setup() {
  pinMode(13,OUTPUT);

  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  digitalWrite(dirPin,HIGH);
}
void loop() {
delay(2000);
for (int i = 0; i < 5 * steps_per_rev; i++) {
    // These four lines result in 1 step:
    digitalWrite(stepPin, HIGH);
    delay(step_delay);
    digitalWrite(stepPin, LOW);
    delay(step_delay);
}
}