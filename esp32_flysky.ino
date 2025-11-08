#include <ESP32Servo.h>

// ---------------------- FlySky Receiver Input Pins ----------------------
#define CH1_PIN 35  // Forward/Backward
#define CH2_PIN 34  // Left/Right
#define CH3_PIN 36  // Servo 1
#define CH4_PIN 25  // Servo 2
#define CH5_PIN 39  // Speed Mode toggle Switch
#define CH6_PIN 13  // Servo 3

// ---------------------- L298N Motor Driver Pins ----------------------
// Right Motor
#define R_IN1 27  // IN1
#define R_IN2 26  // IN2
#define R_EN 14   // PWM (EnA)

// Left Motor
#define L_IN1 25  // IN3
#define L_IN2 33  // IN4
#define L_EN 32   // PWM (EnB)

// ---------------------- PWM Channels ----------------------
#define L_EN_CH 0
#define R_EN_CH 1

// ---------------------- Servo Pins ----------------------
#define SERVO1_PIN 12
#define SERVO2_PIN 23
#define SERVO3_PIN 22

// ---------------------- Global Variables ----------------------
float speedMultiplier = 1.0;  // Initially Full Speed
Servo servo1, servo2, servo3;

// ---------------------- Functions ----------------------
int applyDeadband(int value, int threshold = 10) {
  return (abs(value) < threshold) ? 0 : value;
}

void setup() {
  Serial.begin(115200);

  // Receiver inputs
  pinMode(CH1_PIN, INPUT);
  pinMode(CH2_PIN, INPUT);
  pinMode(CH3_PIN, INPUT);
  pinMode(CH4_PIN, INPUT);
  pinMode(CH5_PIN, INPUT);
  pinMode(CH6_PIN, INPUT);

  // Motor control pins
  pinMode(L_IN1, OUTPUT);
  pinMode(L_IN2, OUTPUT);
  pinMode(R_IN1, OUTPUT);
  pinMode(R_IN2, OUTPUT);

  // Setup PWM channels for motor enable pins
  ledcSetup(L_EN_CH, 1000, 8);
  ledcAttachPin(L_EN, L_EN_CH);
  ledcSetup(R_EN_CH, 1000, 8);
  ledcAttachPin(R_EN, R_EN_CH);

  // Attach servos
  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);
  servo3.attach(SERVO3_PIN);

  stopMotors();
}

void loop() {
  // Read FlySky PWM signals
  int ch1 = pulseIn(CH1_PIN, HIGH, 25000);
  int ch2 = pulseIn(CH2_PIN, HIGH, 25000);
  int ch3 = pulseIn(CH3_PIN, HIGH, 25000);
  int ch4 = pulseIn(CH4_PIN, HIGH, 25000);
  int ch5 = pulseIn(CH5_PIN, HIGH, 25000);
  int ch6 = pulseIn(CH6_PIN, HIGH, 25000);

  // Fail-safe: stop motors if signals are invalid
  if (ch1 < 500 || ch1 > 2200 || ch2 < 500 || ch2 > 2200) {
    stopMotors();
    return;
  }

  // Speed mode toggle (CH5 switch)
  speedMultiplier = (ch5 < 1500) ? 1.0 : 0.5;

  // Map FlySky joystick to movement
  int x = map(ch1, 1000, 2000, -255, 255) * speedMultiplier;
  int y = map(ch2, 1000, 2000, -255, 255) * speedMultiplier;

  x = applyDeadband(x);
  y = applyDeadband(y);

  // Motor mixing for left and right wheels
  int leftSpeed, rightSpeed;
  if (y <= -150) {  // backward mixing
    leftSpeed = y - x;
    rightSpeed = y + x;
  } else {  // forward mixing
    leftSpeed = y + x;
    rightSpeed = y - x;
  }

  leftSpeed = constrain(leftSpeed, -255, 255);
  rightSpeed = constrain(rightSpeed, -255, 255);

  // Apply motor control
  setMotor(L_IN1, L_IN2, L_EN_CH, leftSpeed);
  setMotor(R_IN1, R_IN2, R_EN_CH, rightSpeed);

  // ---------------------- Servo Control ----------------------
  int servo1_angle = map(ch3, 1000, 2000, 0, 180);
  int servo2_angle = map(ch4, 1000, 2000, 0, 180);
  int servo3_angle = map(ch6, 1000, 2000, 0, 180);

  servo1.write(servo1_angle);
  servo2.write(servo2_angle);
  servo3.write(servo3_angle);
}

// ---------------------- Motor Functions ----------------------
void setMotor(uint8_t IN1, uint8_t IN2, uint8_t EN_CH, int speed) {
  if (speed > 0) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    ledcWrite(EN_CH, speed);
  } else if (speed < 0) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    ledcWrite(EN_CH, -speed);
  } else {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    ledcWrite(EN_CH, 0);
  }
}

void stopMotors() {
  setMotor(L_IN1, L_IN2, L_EN_CH, 0);
  setMotor(R_IN1, R_IN2, R_EN_CH, 0);
}
