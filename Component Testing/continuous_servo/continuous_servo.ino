#include <Servo.h>

Servo dispenser;

int servoPin = 9;
int servoPos = 0;

void setup() {
  dispenser.attach(servoPin);
}

void loop() {
  servoPos += 1;
  dispenser.write(servoPos);
  delay(15);
}
