

// RemoteXY select connection mode and include library 
#define REMOTEXY_MODE__ESP32CORE_BLE

#include <Arduino.h>
#include <BLEDevice.h>

// RemoteXY connection settings 
#define REMOTEXY_BLUETOOTH_NAME "RemoteXY"

#include <RemoteXY.h>
#include <ESP32Servo.h> 

// RemoteXY GUI configuration  
#pragma pack(push, 1)  
uint8_t RemoteXY_CONF[] =   // 115 bytes
  { 255,8,0,0,0,108,0,19,0,0,0,80,114,111,106,101,99,116,88,0,
  31,1,200,76,1,1,8,0,5,140,19,44,44,32,2,26,31,1,10,24,
  20,20,0,2,31,0,1,10,47,20,20,0,2,31,0,1,35,47,20,20,
  0,2,31,0,1,35,24,20,20,0,2,31,0,1,57,34,20,20,0,36,
  24,67,108,111,115,101,0,1,79,34,20,20,0,136,8,79,112,101,110,0,
  129,65,24,27,8,64,74,71,114,105,112,112,101,114,0 };
  
// this structure defines all the variables and events of your control interface 
struct {

    // input variables
  int8_t joystick_01_x; // from -100 to 100
  int8_t joystick_01_y; // from -100 to 100
  uint8_t button_01; // =1 if button pressed, else =0, from 0 to 1
  uint8_t button_02; // =1 if button pressed, else =0, from 0 to 1
  uint8_t button_03; // =1 if button pressed, else =0, from 0 to 1
  uint8_t button_04; // =1 if button pressed, else =0, from 0 to 1
  uint8_t button_05; // =1 if button pressed, else =0, from 0 to 1
  uint8_t button_06; // =1 if button pressed, else =0, from 0 to 1

    // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0

} RemoteXY;  
/////////////////////////////////////////////
//        END RemoteXY include             //
/////////////////////////////////////////////

// --- Motor Control Pins (for L293D) ---
// Left Motor
#define ENA_PIN 14   
#define LEFT_IN1 27  
#define LEFT_IN2 26  
// Right Motor
#define ENB_PIN 5   
#define RIGHT_IN3 17 
#define RIGHT_IN4 16 

// --- ESP32 PWM Configuration ---
#define PWM_FREQ 5000   
#define PWM_RESOLUTION 8
#define LEFT_PWM_CHANNEL 0
#define RIGHT_PWM_CHANNEL 1

// --- Servo Pin Definitions ---
#define SERVO_GRIPPER_PIN  13  // The servo for Open/Close
#define SERVO_ARM_PIN      15  // The servo for Arm Up/Down
#define SERVO_BASE_PIN 4 //The lowest servo


// --- Create Servo Objects ---
Servo servoGripper;
Servo servoArm;
Servo servoBase;

// Variables to hold the current target angle for each servo
int gripperAngle = 90; // Start gripper at 90
int armAngle = 90;
int baseAngle = 90;

// Variables to track button presses for incremental steps
uint8_t prev_arm_up_btn = 0;
uint8_t prev_arm_down_btn = 0;
uint8_t prev_base_left_btn = 0;
uint8_t prev_base_right_btn = 0;
uint8_t prev_gripper_open_btn = 0;  
uint8_t prev_gripper_close_btn = 0; 

// --- BUTTON MAPPING
#define arm_up_btn     RemoteXY.button_01
#define arm_down_btn   RemoteXY.button_02
#define base_left_btn  RemoteXY.button_03
#define base_right_btn RemoteXY.button_04
#define gripper_close_btn RemoteXY.button_05 // Red "Close"
#define gripper_open_btn  RemoteXY.button_06 // Green "Open"


void setup() 
{
  RemoteXY_Init (); 
  
  // --- Motor Pin Setup ---
  pinMode(ENA_PIN, OUTPUT);
  pinMode(LEFT_IN1, OUTPUT);
  pinMode(LEFT_IN2, OUTPUT);
  pinMode(ENB_PIN, OUTPUT);
  pinMode(RIGHT_IN3, OUTPUT);
  pinMode(RIGHT_IN4, OUTPUT);

  // --- ESP32 PWM Setup ---
  // Configure the PWM channels
  ledcAttachChannel(ENA_PIN, PWM_FREQ, PWM_RESOLUTION, LEFT_PWM_CHANNEL);
  ledcAttachChannel(ENB_PIN, PWM_FREQ, PWM_RESOLUTION, RIGHT_PWM_CHANNEL);

  // Stop motors at startup
  stopMotors();

  // --- Servo Setup ---
  servoGripper.attach(SERVO_GRIPPER_PIN);
  servoArm.attach(SERVO_ARM_PIN);
  servoBase.attach(SERVO_BASE_PIN);

  // Move all servos to the starting 90-degree position
  servoGripper.write(gripperAngle);
  servoArm.write(armAngle);
  servoBase.write(baseAngle);
  
  // Start Serial for debugging
  Serial.begin(115200);
  Serial.println("Robot Arm Car Ready. WARNING: Ensure external power for servos!");
}

void loop() 
{ 
  RemoteXY_Handler ();
  
  // --- Get Joystick Values ---
  int x = RemoteXY.joystick_01_x;
  int y = RemoteXY.joystick_01_y;

  // --- 1. Deadzone Logic ---
  int generalDeadzone = 15;
  int straightAssistDeadzone = 30;

  if (abs(x) < generalDeadzone) x = 0;
  if (abs(y) < generalDeadzone) y = 0;

  // --- 2. "Straight Assist" Logic ---
  if (abs(y) > 80 && abs(x) < straightAssistDeadzone) x = 0; 

  // --- 3. Mixing Logic ---
  int rawLeft = y + x;
  int rawRight = y - x;

  // --- 4. Map & Constrain Logic ---
  int motorLeftSpeed = map(rawLeft, -200, 200, -255, 255);
  int motorRightSpeed = map(rawRight, -200, 200, -255, 255);
  motorLeftSpeed = constrain(motorLeftSpeed, -255, 255);
  motorRightSpeed = constrain(motorRightSpeed, -255, 255);

  // --- 5. Control Motors ---
  setMotorSpeed(0, motorLeftSpeed);  // 0 = Left Motor
  setMotorSpeed(1, motorRightSpeed); // 1 = Right Motor

  // --- 6. SERVO CONTROL LOGIC ---
  
  // === Gripper Control (Incremental 5-Degree Steps) ===
  if (gripper_open_btn == 1 && prev_gripper_open_btn == 0) {
    gripperAngle = gripperAngle + 5; // Move 5 degrees
  }
  prev_gripper_open_btn = gripper_open_btn;

  if (gripper_close_btn == 1 && prev_gripper_close_btn == 0) {
    gripperAngle = gripperAngle - 5; // Move -5 degrees
  }
  prev_gripper_close_btn = gripper_close_btn;


  // === Arm Control (Incremental 1-Degree Steps) ===
  if (arm_up_btn == 1 && prev_arm_up_btn == 0) {
    armAngle = armAngle + 1; // Move 1 degree
  }
  prev_arm_up_btn = arm_up_btn;

  if (arm_down_btn == 1 && prev_arm_down_btn == 0) {
    armAngle = armAngle - 1; // Move -1 degree
  }
  prev_arm_down_btn = arm_down_btn;

  // === Base Control (Incremental 1-Degree Steps) ===
  if (base_left_btn == 1 && prev_base_left_btn == 0) {
    baseAngle = baseAngle + 1; // Move 1 degree
  }
  prev_base_left_btn = base_left_btn;

  if (base_right_btn == 1 && prev_base_right_btn == 0) {
    baseAngle = baseAngle - 1; // Move -1 degree
  }
  prev_base_right_btn = base_right_btn;

  // === 7. Safety Limits & Write to Servos ===
  gripperAngle = constrain(gripperAngle, 0, 180);
  Serial.print("Gripper angle: "); Serial.println(gripperAngle);

  armAngle = constrain(armAngle, 0, 180);
  Serial.print("arm1 angle: "); Serial.println(armAngle);

  baseAngle = constrain(baseAngle, 0, 180);
  Serial.print("base angle: "); Serial.println(baseAngle);
  

  servoGripper.write(gripperAngle);
  servoArm.write(armAngle);
  servoBase.write(baseAngle);

  // --- 8. Debug Print ---
  Serial.print("X:"); Serial.print(x);
  Serial.print(" Y:"); Serial.print(y);
  Serial.print(" | Left:"); Serial.print(motorLeftSpeed);
  Serial.print(" Right:"); Serial.print(motorRightSpeed);
  Serial.print(" | Base:"); Serial.print(baseAngle);
  Serial.print(" Arm:"); Serial.print(armAngle);
  Serial.print(" Grip:"); Serial.println(gripperAngle);

  RemoteXY_delay(20); // Delay for stability
}



void setMotorSpeed(int motor, int speed) {
  int in1, in2, enChannel;
  
  if (motor == 0) { // Left Motor
    in1 = LEFT_IN1;
    in2 = LEFT_IN2;
    enChannel = LEFT_PWM_CHANNEL;
  } else { // Right Motor
    in1 = RIGHT_IN3;
    in2 = RIGHT_IN4;
    enChannel = RIGHT_PWM_CHANNEL;
  }

  int pwmSpeed = abs(speed);
  
  if (speed > 0) { // Forward
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    ledcWrite(enChannel, pwmSpeed);
  } 
  else if (speed < 0) { // Backward
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    ledcWrite(enChannel, pwmSpeed);
  } 
  else { // Stop
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    ledcWrite(enChannel, 0);
  }
}

void stopMotors() {
  setMotorSpeed(0, 0);
  setMotorSpeed(1, 0);
}

