#include <Stepper.h>
#include <Servo.h>

#define SERVO_PIN 3

#define COMBO_1 22
#define COMBO_2 24
#define COMBO_3 34

const int stepsPerRevolution = 200;
const int comboSteps = 40;
const float gearRatio = 1;

Stepper myStepper = Stepper(stepsPerRevolution, 8, 9, 10, 11);
Servo servo;

int currentCombo = 0;
boolean ccw = true;

void setup() {
  servo.attach(SERVO_PIN);
  servo.write(50);

  myStepper.setSpeed(120);
  Serial.begin(9600);
}

void loop() {
  if (Serial.available()) {
    String command = Serial.readStringUntil(' ');
    int data = Serial.parseInt();

    command.trim();

    if (command == "REVS") {
      Serial.println("Stepping " + String(data) + " revolutions");
      stepRevs(data);
    } else if (command == "CCW") {
      Serial.println("Turning to " + String(data) + " counterclockwise");
      dialCombo(data, true);
    } else if (command == "CW") {
      Serial.println("Turning to " + String(data) + " clockwise");
      dialCombo(data, false);
    } else if (command == "SERVO") {
      runServo();
    } else if (command == "COMBO") {
      dialFullCombination();
    } else if(command == "FULL") {
      dialFullCombination();
      delay(200);
      runServo();
    }
  }
}

void runServo() {
  servo.write(90);
  delay(1000);
  servo.write(50);
}

void dialFullCombination() {
  currentCombo = 0;
  stepRevs(3);
  dialCombo(COMBO_1, false);
  stepRevs(-1);
  dialCombo(COMBO_2, true);
  dialCombo(COMBO_3, false);
}

void dialCombo(int comboPos, bool ccw) {
  dialComboRelative(comboPos, currentCombo, ccw);
  currentCombo = comboPos;
}

void dialComboRelative(int finalPos, int startPos, bool ccw) {
  int deltaCombo = ((finalPos - startPos) + comboSteps) % comboSteps;
  if (!ccw) deltaCombo = comboSteps - deltaCombo;
  float revs = deltaCombo  / (float) comboSteps;
  stepRevs((revs + 0.015) * (ccw ? -1 : 1)); // add small tolerance value
}

void stepRevs(float revs) {
  myStepper.step((int) (revs * stepsPerRevolution * gearRatio));
}
