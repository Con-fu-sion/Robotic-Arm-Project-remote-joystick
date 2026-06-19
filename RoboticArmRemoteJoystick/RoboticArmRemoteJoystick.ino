#include <Servo.h>
#include <IRremote.hpp>

// ---------- PINS ----------
const int IR_PIN = 11;

const int BASE_PIN = 3;
const int RIGHT_PIN = 5;
const int LEFT_PIN = 9;
const int CLAW_PIN = 6;

// ---------- JOYSTICK PINS ----------
const int JOY_X_PIN = A0; // VRX
const int JOY_Y_PIN = A1; // VRY
const int JOY_SW_PIN = 2; // SW

// ---------- REMOTE BUTTON CODES ----------
const int BUTTON_1 = 12; // remote 1 = carry direction 1
const int BUTTON_2 = 24; // remote 2 = carry direction 2
const int BUTTON_3 = 94; // remote 3 = reset
const int BUTTON_9 = 74; // remote 9 = reset

// ---------- SERVOS ----------
Servo baseServo;
Servo rightServo;
Servo leftServo;
Servo clawServo;

// ---------- STARTING POSITION ----------
float BASE = 37;
float RIGHT = 80;
float LEFT = 30;
float CLAW = 90;

// ---------- CLAW SETTINGS ----------
const int CLAW_OPEN = 115;
const int CLAW_GRIP = 78;
const int CLAW_RESET = 90;

const int CLAW_MIN = 76;
const int CLAW_MAX = 118;

// ---------- JOYSTICK SMOOTH SETTINGS ----------
const int JOY_CENTER = 512;
const int JOY_DEADZONE = 120;

float smoothX = 512;
float smoothY = 512;

const float SMOOTH_AMOUNT = 0.10; // smaller = smoother
const int JOY_UPDATE_DELAY = 25;

const float BASE_MAX_SPEED = 0.9;
const float ARM_MAX_SPEED = 0.6;

bool clawIsOpen = false;

unsigned long lastJoyMove = 0;
unsigned long lastJoyButton = 0;

void setup()
{
  Serial.begin(9600);

  baseServo.attach(BASE_PIN);
  rightServo.attach(RIGHT_PIN);
  leftServo.attach(LEFT_PIN);
  clawServo.attach(CLAW_PIN);

  pinMode(JOY_SW_PIN, INPUT_PULLUP);

  IrReceiver.begin(IR_PIN, ENABLE_LED_FEEDBACK);

  resetArm();

  Serial.println("Ready");
  Serial.println("Remote 1 = carry direction 1");
  Serial.println("Remote 2 = carry direction 2");
  Serial.println("Remote 3 or 9 = reset");
  Serial.println("Joystick X = base");
  Serial.println("Joystick Y = arm up/down");
  Serial.println("Joystick press = claw open/close");
}

void loop()
{
  handleRemote();
  handleJoystick();
}

// ---------- REMOTE CONTROL ----------
void handleRemote()
{
  if (IrReceiver.decode())
  {

    if (!(IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT))
    {
      int button = IrReceiver.decodedIRData.command;

      Serial.print("Remote button code: ");
      Serial.println(button);

      if (button == BUTTON_1)
      {
        carryDirection1();
      }

      if (button == BUTTON_2)
      {
        carryDirection2();
      }

      if (button == BUTTON_3 || button == BUTTON_9)
      {
        resetArm();
      }
    }

    IrReceiver.resume();
  }
}

// ---------- SMOOTH JOYSTICK CONTROL ----------
void handleJoystick()
{
  int rawX = analogRead(JOY_X_PIN);
  int rawY = analogRead(JOY_Y_PIN);

  smoothX = smoothX + (rawX - smoothX) * SMOOTH_AMOUNT;
  smoothY = smoothY + (rawY - smoothY) * SMOOTH_AMOUNT;

  int xDiff = smoothX - JOY_CENTER;
  int yDiff = smoothY - JOY_CENTER;

  if (millis() - lastJoyMove > JOY_UPDATE_DELAY)
  {
    bool moved = false;

    // X axis = base left/right
    if (abs(xDiff) > JOY_DEADZONE)
    {
      float strength = (float)(abs(xDiff) - JOY_DEADZONE) / (512 - JOY_DEADZONE);
      strength = constrain(strength, 0, 1);

      float baseStep = strength * BASE_MAX_SPEED;

      if (xDiff > 0)
      {
        BASE += baseStep;
      }
      else
      {
        BASE -= baseStep;
      }

      moved = true;
    }

    // Y axis = arm up/down
    if (abs(yDiff) > JOY_DEADZONE)
    {
      float strength = (float)(abs(yDiff) - JOY_DEADZONE) / (512 - JOY_DEADZONE);
      strength = constrain(strength, 0, 1);

      float armStep = strength * ARM_MAX_SPEED;

      if (yDiff > 0)
      {
        // Lower arm
        RIGHT -= armStep * 0.8;
        LEFT += armStep * 2.5;
      }
      else
      {
        // Lift arm
        RIGHT += armStep * 0.8;
        LEFT -= armStep * 2.5;
      }

      moved = true;
    }

    if (moved)
    {
      moveArm();
    }

    lastJoyMove = millis();
  }

  // Joystick button = open/close claw
  if (digitalRead(JOY_SW_PIN) == LOW && millis() - lastJoyButton > 400)
  {
    if (clawIsOpen == false)
    {
      openClaw();
    }
    else
    {
      gripClawHard();
    }

    lastJoyButton = millis();
  }
}

// ---------- MOVE ALL SERVOS ----------
void moveArm()
{
  BASE = constrain(BASE, 0, 180);
  RIGHT = constrain(RIGHT, 0, 180);
  LEFT = constrain(LEFT, 0, 180);
  CLAW = constrain(CLAW, CLAW_MIN, CLAW_MAX);

  baseServo.write(round(BASE));
  rightServo.write(round(RIGHT));
  leftServo.write(round(LEFT));
  clawServo.write(round(CLAW));
}

// ---------- CLAW MOVEMENT ----------
void moveClawTo(int target)
{
  target = constrain(target, CLAW_MIN, CLAW_MAX);

  while (abs(CLAW - target) > 1)
  {
    if (CLAW < target)
    {
      CLAW += 2;
      if (CLAW > target)
        CLAW = target;
    }
    else
    {
      CLAW -= 2;
      if (CLAW < target)
        CLAW = target;
    }

    moveArm();
    delay(20);
  }

  CLAW = target;
  moveArm();
}

void openClaw()
{
  Serial.println("Claw open");
  moveClawTo(CLAW_OPEN);
  clawIsOpen = true;
}

void gripClawHard()
{
  Serial.println("Claw close hard");
  moveClawTo(CLAW_GRIP);
  clawIsOpen = false;
}

// ---------- ARM MOVEMENT ----------
void lowerArm()
{
  for (int index = 0; index < 15; index++)
  {
    RIGHT += -1;
    LEFT += 3.8;
    moveArm();
    delay(70);
  }
}

void liftArm()
{
  for (int index = 0; index < 15; index++)
  {
    RIGHT += 1;
    LEFT += -3.8;
    moveArm();
    delay(50);
  }
}

// ---------- RESET ----------
void resetArm()
{
  BASE = 37;
  RIGHT = 80;
  LEFT = 30;
  CLAW = CLAW_RESET;

  moveArm();
  clawIsOpen = false;

  Serial.println("Reset to origin");
}

// ---------- REMOTE BUTTON 1 ----------
void carryDirection1()
{
  for (int index = 0; index < 15; index++)
  {
    BASE += -3;
    moveArm();
    delay(50);
  }

  delay(300);

  openClaw();
  lowerArm();
  gripClawHard();
  liftArm();

  delay(300);

  for (int index = 0; index < 15; index++)
  {
    BASE += 6;
    moveArm();
    delay(50);
  }

  delay(300);

  lowerArm();
  openClaw();
  liftArm();

  delay(300);

  for (int index = 0; index < 15; index++)
  {
    BASE += -3;
    moveArm();
    delay(50);
  }
}

// ---------- REMOTE BUTTON 2 ----------
void carryDirection2()
{
  for (int index = 0; index < 15; index++)
  {
    BASE += 3;
    moveArm();
    delay(50);
  }

  delay(300);

  openClaw();
  lowerArm();
  gripClawHard();
  liftArm();

  delay(300);

  for (int index = 0; index < 15; index++)
  {
    BASE += -6;
    moveArm();
    delay(50);
  }

  delay(300);

  lowerArm();
  openClaw();
  liftArm();

  delay(300);

  for (int index = 0; index < 15; index++)
  {
    BASE += 3;
    moveArm();
    delay(50);
  }
}
