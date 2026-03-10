#include <IBusBM.h>
#include <Servo.h>

// --- PIN DEFINITIONS (NodeMCU ESP8266) ---
// Right Motor
#define ENA D1  // GPIO 5
#define IN1 D2  // GPIO 4
#define IN2 D0  // GPIO 16 

// Left Motor
#define ENB D5  // GPIO 14
#define IN3 D6  // GPIO 12
#define IN4 D7  // GPIO 13

// Servos
#define SERVO1_PIN D4 // GPIO 2 (Joystick Arm)
#define SERVO2_PIN D8 // GPIO 15 (Switch Claw)

// --- OBJECTS ---
IBusBM ibus;    
Servo servo[2];

const int DEADZONE = 20;

void setup() {

  Serial.begin(115200); 
  
  ibus.begin(Serial, IBUSBM_NOTIMER); 
  
  // 2. Setup Motor Pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  
  // ESP8266 PWM Range (0-255)
  analogWriteRange(255); 

  // 3. Setup Servos
  servo[0].attach(SERVO1_PIN, 500, 2500);
  servo[1].attach(SERVO2_PIN, 500, 2500);

  stopAllMotors();
  servo[0].write(90);
  servo[1].write(90);
}

void stopAllMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0); 
  analogWrite(ENB, 0); 
}

void controlMotor(char motor, int speed) {
  int pwmPin = (motor == 'L') ? ENB : ENA;
  int inA    = (motor == 'L') ? IN3 : IN1;
  int inB    = (motor == 'L') ? IN4 : IN2;

  if (speed > 0) { // Forward
    digitalWrite(inA, HIGH);
    digitalWrite(inB, LOW);
    analogWrite(pwmPin, speed);
  } else if (speed < 0) { // Backward
    digitalWrite(inA, LOW);
    digitalWrite(inB, HIGH);
    analogWrite(pwmPin, abs(speed));
  } else { // Stop
    digitalWrite(inA, LOW);
    digitalWrite(inB, LOW);
    analogWrite(pwmPin, 0);
  }
}

void loop() {
  ibus.loop(); 

  int ch3_Throttle = ibus.readChannel(1); 
  int ch1_Steer    = ibus.readChannel(0); 

  // Safety Check: If radio off, stop.
  if (ch3_Throttle == 0) {
      stopAllMotors();
      return; 
  }

  // --- Speed Limiter ---
  int speedLimit = 255;
  // Apply deadzone
  if (abs(ch3_Throttle - 1500) < DEADZONE) ch3_Throttle = 1500;
  if (abs(ch1_Steer - 1500) < DEADZONE) ch1_Steer = 1500;

  // Map inputs
  long throttle = map(ch3_Throttle, 1000, 2000, -speedLimit, speedLimit);
  long steer    = map(ch1_Steer,    1000, 2000, -255, 255);
  
  long leftSpeed  = throttle + steer;
  long rightSpeed = throttle - steer;

  // Constrain to valid PWM range
  long maxVal = max(abs(leftSpeed), abs(rightSpeed));
  if (maxVal > 255) {
    leftSpeed  = (leftSpeed * 255) / maxVal;
    rightSpeed = (rightSpeed * 255) / maxVal;
  }

  controlMotor('L', (int)leftSpeed);
  controlMotor('R', (int)rightSpeed);


  delay(10);
}