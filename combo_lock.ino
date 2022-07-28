#include <Stepper.h>
#include <Servo.h>
#include <EEPROM.h>

#define SERVO_PIN 3
#define ECHO_PIN 4
#define TRIG_PIN 5
#define STATUS_PIN 13

#define COMBO_1 22
#define COMBO_2 24
#define COMBO_3 34

#define DISTANCE_ADDR 0x0

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

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(STATUS_PIN, OUTPUT);

  Serial.begin(9600);
  Serial.println("Welcome. Speed is set at 120 rpm");

  digitalWrite(STATUS_PIN, LOW);
}

void loop() {
  if (Serial.available()) {
    String command = Serial.readStringUntil(' ');
    int data = Serial.parseInt();

    command.trim();

    if (command == "REVS") {
      Serial.print("Stepping " + String(data) + " revolutions.");
      stepRevs(data);
    } else if (command == "CCW") {
      Serial.print("Turning to " + String(data) + " counterclockwise.");
      dialCombo(data, true);
    } else if (command == "CW") {
      Serial.print("Turning to " + String(data) + " clockwise.");
      dialCombo(data, false);
    } else if (command == "SERVO") {
      Serial.print("Running servo.");
      runServo();
    } else if (command == "COMBO") {
      Serial.print("Dialing combination.");
      dialFullCombination();
    } else if(command == "FULL") {
      Serial.print("Dialing combination and running servo!");
      dialFullCombination();
      runServo();
    } else if(command == "SPEED") {
      Serial.print("Setting speed to " + String(data) + " rpm.");
      myStepper.setSpeed(data);
    } else if(command == "WRITE") {
      Serial.print("Writing distance threshold as " + String(data) + " cm.");
      writeDistanceEEPROM(data);
    } else if(command == "DIST") {
      Serial.println("Reading distance.");
      while(!Serial.available()) {
        int distance = getDistance();
        Serial.println(String(distance) + " cm.");
      }
    } else if(command == "SENSE") {
      digitalWrite(STATUS_PIN, HIGH);
      const int distanceThreshold = readDistanceEEPROM();

      Serial.print("Waiting for distance. Distance threshold: " + String(distanceThreshold) + " cm.");
      
      while(!Serial.available()) {
        int distance = getDistance();
        if(distance <= distanceThreshold) {
          Serial.print("\nDialing combination and running servo!");
//          delay(120000);
          dialFullCombination();
          runServo();
          break;
        }
      }
      
      digitalWrite(STATUS_PIN, LOW);
    } else {
      Serial.print("Invalid command.");
    }

    Serial.println(" Done.");
  }
}

int readDistanceEEPROM() {
  return EEPROM.read(DISTANCE_ADDR);
}

void writeDistanceEEPROM(int distance) {
  EEPROM.write(DISTANCE_ADDR, distance);
}

int getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 10000);
  int distance = duration * 0.034 / 2;
  return distance;
}

void runServo() {
  servo.write(110);
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
  stepRevs((revs + 0) * (ccw ? -1 : 1)); // add small tolerance value
}

void stepRevs(float revs) {
  myStepper.step((int) (revs * stepsPerRevolution * gearRatio));
}
