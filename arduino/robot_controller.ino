#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// =====================================================
// BLUETOOTH
// =====================================================
SoftwareSerial BT(2, 3);

// =====================================================
// PCA9685
// =====================================================
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define SERVOMIN 110
#define SERVOMAX 690

// =====================================================
// SERVO CHANNELS
// =====================================================
#define GRIPPER_CH 0
#define BASE_CH    1
#define ELBOW_CH   2

// =====================================================
// SERVO ANGLES
// =====================================================
int gripperAngle = 0;
int elbowAngle   = 0;
int targetElbow  = 0;
int baseAngle    = 0;

unsigned long lastServoUpdate = 0;

// =====================================================
// MOTOR PINS
// =====================================================
#define IN1 7
#define IN2 8
#define IN3 9
#define IN4 10
#define ENA 5
#define ENB 6

int speedValue = 130;

// =====================================================
// LINE TRACKER
// =====================================================
#define LINE_L A0
#define LINE_M A1
#define LINE_R A2

// =====================================================
// ULTRASONIC — NON-BLOCKING
// =====================================================
#define TRIG 11
#define ECHO 12

int stopDistance   = 15;
int warnDistance   = 30;

enum UltraState { IDLE, LISTENING };
UltraState ultraState   = IDLE;
unsigned long ultraTimer    = 0;
unsigned long ultraInterval = 80;
int currentDistance         = 999;

// =====================================================
// MODES
// =====================================================
bool autoMode     = false;
bool goingForward = false;

unsigned long turnStartTime = 0;
int turnDuration = 300;
bool isTurning = false;

// =====================================================
// SEQUENCE STATE MACHINE
// =====================================================
enum SeqState {
  SEQ_NONE,
  // PICK sequence steps
  PICK_LOWER,   // lower elbow to pick position
  PICK_WAIT,    // wait for elbow to reach
  PICK_GRIP,    // close gripper
  PICK_RAISE,   // raise elbow home
  // PLACE sequence steps
  PLACE_LOWER,
  PLACE_WAIT,
  PLACE_OPEN,
  PLACE_RAISE,
  // U-TURN
  UTURN_SPIN,
  UTURN_DONE
};

SeqState seqState = SEQ_NONE;
unsigned long seqTimer = 0;

// =====================================================
// SERVO FUNCTION
// =====================================================
void moveServo(int ch, int angle) {
  angle = constrain(angle, 0, 180);
  int pulse = map(angle, 0, 180, SERVOMIN, SERVOMAX);
  pwm.setPWM(ch, 0, pulse);
}

// =====================================================
// NON-BLOCKING ELBOW
// =====================================================
void updateElbow() {
  if (millis() - lastServoUpdate > 20) {
    lastServoUpdate = millis();
    if (elbowAngle < targetElbow) elbowAngle++;
    else if (elbowAngle > targetElbow) elbowAngle--;
    moveServo(ELBOW_CH, elbowAngle);
  }
}

// =====================================================
// NON-BLOCKING ULTRASONIC
// =====================================================
void updateUltrasonic() {
  unsigned long now = millis();
  if (ultraState == IDLE && now - ultraTimer >= ultraInterval) {
    digitalWrite(TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG, LOW);
    ultraTimer = now;
    ultraState = LISTENING;
  }
  else if (ultraState == LISTENING) {
    long duration = pulseIn(ECHO, HIGH, 25000);
    currentDistance = (duration == 0) ? 999 : duration * 0.034 / 2;
    ultraTimer = millis();
    ultraState = IDLE;
  }
}

// =====================================================
// SEQUENCE STATE MACHINE
// Runs pick/place/uturn non-blocking
// =====================================================
void updateSequence() {
  unsigned long now = millis();

  switch (seqState) {

    // ── PICK ──────────────────────────────────────
    case PICK_LOWER:
      targetElbow = 50;              // lower elbow to pick
      seqTimer = now;
      seqState = PICK_WAIT;
      break;

    case PICK_WAIT:
      // wait ~1000ms for elbow to reach position
      if (now - seqTimer >= 1000) {
        gripperAngle = 80;           // close gripper
        moveServo(GRIPPER_CH, gripperAngle);
        seqTimer = now;
        seqState = PICK_GRIP;
      }
      break;

    case PICK_GRIP:
      // wait 400ms for gripper to close
      if (now - seqTimer >= 400) {
        targetElbow = 0;             // raise elbow home
        seqState = PICK_RAISE;
      }
      break;

    case PICK_RAISE:
      // sequence complete when elbow reaches home
      if (elbowAngle <= 2) {
        seqState = SEQ_NONE;
        Serial.println("PICK DONE");
      }
      break;

    // ── PLACE ─────────────────────────────────────
    case PLACE_LOWER:
      targetElbow = 50;
      seqTimer = now;
      seqState = PLACE_WAIT;
      break;

    case PLACE_WAIT:
      if (now - seqTimer >= 1000) {
        gripperAngle = 0;            // open gripper
        moveServo(GRIPPER_CH, gripperAngle);
        seqTimer = now;
        seqState = PLACE_OPEN;
      }
      break;

    case PLACE_OPEN:
      if (now - seqTimer >= 400) {
        targetElbow = 0;             // raise elbow home
        seqState = PLACE_RAISE;
      }
      break;

    case PLACE_RAISE:
      if (elbowAngle <= 2) {
        seqState = SEQ_NONE;
        Serial.println("PLACE DONE");
      }
      break;

    // ── U-TURN ────────────────────────────────────
    case UTURN_SPIN:
      if (now - seqTimer >= 120) {  // 120ms = safe 180° at drive speed
        stopMotors();
        goingForward = false;
        seqState = SEQ_NONE;
        Serial.println("UTURN DONE");
      }
      break;

    case SEQ_NONE:
    default:
      break;
  }
}

// =====================================================
// MOTOR FUNCTIONS
// =====================================================
void moveForward() {
  analogWrite(ENA, speedValue);
  analogWrite(ENB, speedValue);
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void moveForwardSlow() {
  int s = max(50, speedValue / 2);
  analogWrite(ENA, s);
  analogWrite(ENB, s);
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void moveBackward() {
  analogWrite(ENA, speedValue);
  analogWrite(ENB, speedValue);
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
}

void turnLeft() {
  analogWrite(ENA, speedValue);
  analogWrite(ENB, speedValue);
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void turnRight() {
  analogWrite(ENA, speedValue);
  analogWrite(ENB, speedValue);
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
}

void curveLeft() {
  analogWrite(ENA, max(40, speedValue / 3));
  analogWrite(ENB, speedValue);
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void curveRight() {
  analogWrite(ENA, speedValue);
  analogWrite(ENB, max(40, speedValue / 3));
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void stopMotors() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}

void smartForward() {
  if (currentDistance <= stopDistance) {
    stopMotors();
  } else if (currentDistance <= warnDistance) {
    moveForwardSlow();
  } else {
    moveForward();
  }
}

// =====================================================
// SETUP
// =====================================================
void setup() {
  Serial.begin(9600);
  BT.begin(9600);

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT);

  pinMode(LINE_L, INPUT);
  pinMode(LINE_M, INPUT);
  pinMode(LINE_R, INPUT);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  pwm.begin();
  pwm.setPWMFreq(50);

  moveServo(GRIPPER_CH, gripperAngle);
  moveServo(BASE_CH, baseAngle);
  moveServo(ELBOW_CH, elbowAngle);

  stopMotors();
  Serial.println("ROBOT READY");
}

// =====================================================
// LOOP
// =====================================================
void loop() {
  updateElbow();
  updateUltrasonic();
  updateSequence();   // runs pick/place/uturn state machine

  if (!autoMode && goingForward) {
    smartForward();
  }

  if (isTurning && millis() - turnStartTime >= turnDuration) {
    stopMotors();
    isTurning = false;
  }

  // AUTO MODE
  if (autoMode) {
    if (currentDistance < stopDistance) {
      stopMotors();
    } else {
      int L = digitalRead(LINE_L);
      int M = digitalRead(LINE_M);
      int R = digitalRead(LINE_R);
      if      (M == LOW) moveForward();
      else if (L == LOW) turnLeft();
      else if (R == LOW) turnRight();
      else               moveForward();
    }
  }

  // BLUETOOTH
  if (BT.available()) {
    char cmd = BT.read();
    Serial.print("CMD: ");
    Serial.println(cmd);

    // MODE
    if      (cmd == 'A') { autoMode = true;  goingForward = false; Serial.println("AUTO MODE"); }
    else if (cmd == 'M') { autoMode = false; goingForward = false; stopMotors(); Serial.println("MANUAL MODE"); }

    // SPEED
    else if (cmd == '1') speedValue = 25;
    else if (cmd == '2') speedValue = 50;
    else if (cmd == '3') speedValue = 75;
    else if (cmd == '4') speedValue = 100;
    else if (cmd == '5') speedValue = 125;
    else if (cmd == '6') speedValue = 150;
    else if (cmd == '7') speedValue = 175;
    else if (cmd == '8') speedValue = 200;
    else if (cmd == '9') speedValue = 225;
    else if (cmd == '0') speedValue = 255;

    // SEQUENCES — only trigger if no sequence running
    else if (cmd == 'P' && seqState == SEQ_NONE) {
      seqState = PICK_LOWER;
      Serial.println("PICK START");
    }
    else if (cmd == 'X' && seqState == SEQ_NONE) {
      seqState = PLACE_LOWER;
      Serial.println("PLACE START");
    }
    else if (cmd == 'T' && seqState == SEQ_NONE) {
      turnLeft();
      seqTimer = millis();
      seqState = UTURN_SPIN;
      Serial.println("UTURN START");
    }

    // CANCEL sequence
    else if (cmd == 'K') {
      seqState = SEQ_NONE;
      stopMotors();
      Serial.println("SEQUENCE CANCELLED");
    }

    // MANUAL DRIVING
    if (!autoMode) {
      if      (cmd == 'F') { goingForward = true;  smartForward(); }
      else if (cmd == 'B') { goingForward = false; moveBackward(); }
      else if (cmd == 'L') { goingForward = false; turnLeft(); }
      else if (cmd == 'R') { goingForward = false; turnRight(); }
      else if (cmd == 'S') { goingForward = false; stopMotors(); }
      else if (cmd == 'l') { goingForward = false; curveLeft(); }
      else if (cmd == 'r') { goingForward = false; curveRight(); }
      else if (cmd == '<') {
        goingForward = false;
        turnLeft();
        turnStartTime = millis();
        isTurning = true;
      }
      else if (cmd == '>') {
        goingForward = false;
        turnRight();
        turnStartTime = millis();
        isTurning = true;
      }
    }

    // GRIPPER
    if (cmd == '+') {
      gripperAngle = constrain(gripperAngle + 2, 0, 80);
      moveServo(GRIPPER_CH, gripperAngle);
    }
    else if (cmd == '-') {
      gripperAngle = constrain(gripperAngle - 2, 0, 80);
      moveServo(GRIPPER_CH, gripperAngle);
    }
    else if (cmd == 'O') { gripperAngle = 0;  moveServo(GRIPPER_CH, gripperAngle); }
    else if (cmd == 'C') { gripperAngle = 80; moveServo(GRIPPER_CH, gripperAngle); }

    // ELBOW
    else if (cmd == 'U') targetElbow = 0;
    else if (cmd == 'D') targetElbow = 50;
    else if (cmd == 'u') targetElbow = constrain(targetElbow - 2, 0, 50);
    else if (cmd == 'd') targetElbow = constrain(targetElbow + 2, 0, 50);

    // BASE
    else if (cmd == 'Q') { baseAngle = 0;  moveServo(BASE_CH, baseAngle); }
    else if (cmd == 'E') { baseAngle = 45; moveServo(BASE_CH, baseAngle); }
    else if (cmd == 'q') {
      baseAngle = constrain(baseAngle - 2, 0, 45);
      moveServo(BASE_CH, baseAngle);
    }
    else if (cmd == 'e') {
      baseAngle = constrain(baseAngle + 2, 0, 45);
      moveServo(BASE_CH, baseAngle);
    }
  }
}
