/*
  ESP32 TANK-DRIVE ROBOT & GRIPPER CONTROLLER (Version 4)
  - Uses the 'FlyskyIBUS' library by derdoktor667
  - Reads 6-Channel Flysky IBUS remote
  - Controls 1x L298N drive
  - Controls 2x Servos for a gripper
  - Includes a "deadzone" for main sticks
*/

// --- 1. INCLUDE LIBRARIES ---
#include <FlyskyIBUS.h>  // Use the new library
#include <ESP32Servo.h>  // For servo control on ESP32


// IBUS Receiver Pin
// This library defaults to Serial2 (GPIO 16)

// L298N Motor Driver Pins

#define ENA 13 // Speed control (PWM)
#define IN1 12
#define IN2 14

#define ENB 27 // Speed control (PWM)
#define IN3 26
#define IN4 25


// Gripper Servo Pins
#define SERVO1_PIN 33 //Arm Up/Down
#define SERVO2_PIN 32 //Claw Open/Close


// --- 3. GLOBAL OBJECTS & SETTINGS ---
FlyskyIBUS ibus; // Create the IBUS object
Servo servo[2];


// Control Settings
const int DEADZONE = 20; //To prevent small change in the joystick




// ESP32 PWM (ledc) Setup for motors
const int PWM_FREQ = 5000;
const int PWM_RESOLUTION = 8;
const int PWM_CHANNEL_A = 0;
const int PWM_CHANNEL_B = 1;

// --- 4. SETUP FUNCTION ---
void setup() {


  Serial.begin(115200);
  Serial.println("ESP32 Robot Controller - Booting...");
  Serial.println("Using FlyskyIBUS.h library");


  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  


  // 1. Begin IBUS communication
  // This library defaults to Serial2 (GPIO 16)
  ibus.begin();
  Serial.println("IBUS Receiver interface started on GPIO 16.");

  // 2. Setup Motor Driver Pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Configure ESP32's LEDC (PWM) controller
  // ledcSetup(PWM_CHANNEL_A, PWM_FREQ, PWM_RESOLUTION);
  // ledcSetup(PWM_CHANNEL_B, PWM_FREQ, PWM_RESOLUTION);

  // // Attach PWM channels to ENA/ENB pins
  // ledcAttachPin(ENA, PWM_CHANNEL_A);
  // ledcAttachPin(ENB, PWM_CHANNEL_B);
  
  Serial.println("L298N motor driver pins configured.");

  // 3. Attach Servo Pins
  servo[0].setPeriodHertz(50);
  servo[1].setPeriodHertz(50);
  servo[0].attach(SERVO1_PIN,500,2500);
  servo[1].attach(SERVO2_PIN,500,2500);
  
  Serial.println("Servo pins attached.");
  
  Serial.println("Setup complete. Waiting for transmitter signal...");
  Serial.println("-------------------------------------------------");

  //Stop alll motor and set servo to 90
  stopAllMotors();
  servo[0].write(90);
  servo[1].write(90);
  
}

// --- 5. HELPER FUNCTION: STOP ALL MOTORS ---
void stopAllMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0); // Right speed 0
  analogWrite(ENB, 0); // Left speed 0
}

// --- 6. HELPER FUNCTION: CONTROL MOTORS ---
void controlMotor(char motor, int speed) {
  int pwmChannel = (motor == 'L') ? ENB : ENA;
  int inA = (motor == 'L') ? IN3 : IN1;
  int inB = (motor == 'L') ? IN4 : IN2;

  if (speed > 0) { // Forward
    digitalWrite(inA, HIGH);
    digitalWrite(inB, LOW);
    // ledcWrite(pwmChannel, speed);
    analogWrite(pwmChannel, speed);
  } else if (speed < 0) { // Backward
    digitalWrite(inA, LOW);
    digitalWrite(inB, HIGH);
    analogWrite(pwmChannel, abs(speed));
  } else { // Stop
    digitalWrite(inA, LOW);
    digitalWrite(inB, LOW);
    analogWrite(pwmChannel, 0);
  }
}


void servoControl(int name, int dir)
{
  servo[name].write(dir);
  Serial.print("\n");Serial.print(name);Serial.print(" :");Serial.println(dir);
}
void servoControlMicroseconds(int name, int dir)
{
  servo[name].writeMicroseconds(dir);
  Serial.print("\n");Serial.print(name);Serial.print(" :");Serial.println(dir);
}
// --- 7. MAIN LOOP ---
void loop() {

  // --- A. READ RAW CHANNEL VALUES (1000 to 2000) ---
  // This library is 1-indexed (CH1 is 1, CH2 is 2, etc.)
  // int ch4_Steer = ibus.getChannel(4);
 
  // int ch3_Throttle = ibus.getChannel(3);
  // int ch1_Servo2 = ibus.getChannel(1);
  // int ch5_Servo1 = ibus.getChannel(5);
  // int ch6_speedBoost = ibus.getChannel(6);

  
  // int ch6_speedBoost = ibus.getChannel(6);
  
  // --- B. CHECK FOR FAILSAFE ---
  // When signal lost with remote, all motor will stop.
   int ch4_Steer = ibus.getChannel(1);
  int ch3_Throttle = ibus.getChannel(2);
  int ch1_Servo2 = ibus.getChannel(4);
  int ch5_Servo1 = ibus.getChannel(5);
  if (ibus.hasFailsafe()) {
    
    Serial.println("!!! NO TRANSMITTER SIGNAL (FAILSAFE) !!!");
    stopAllMotors();
    
    //center servos on failsafe
    servo[0].write(90);
    servo[1].write(90);
    

  } else {
    // --- C. SIGNAL IS GOOD, PROCEED WITH CONTROL ---
   
    // Apply Deadzone
    if (abs(ch4_Steer - 1500)< DEADZONE) {
      ch4_Steer = 1500;
    }
    if (abs(ch3_Throttle - 1500) < DEADZONE) {
      ch3_Throttle = 1500;
    }
   
    // --- D. CALCULATE FINAL VALUES ---
    int throttle;
    int steer;
    if(ch4_Steer!=1500){
      
      throttle = map(ch3_Throttle, 1000, 2000, -255, 255);
      steer = map(ch4_Steer, 1000, 2000, -128, 128);

      Serial.print("Throttle: "); Serial.print(throttle);Serial.print("\tSteer: "); Serial.print(steer);
    }
    else{
      throttle = map(ch3_Throttle, 1000, 2000, -255, 255);
      steer = map(ch4_Steer, 1000, 2000, -255, 255);
      Serial.print("Throttle: "); Serial.print(throttle);Serial.print("\tSteer: "); Serial.print(steer);
    }
        
    
    // int boost = map(ch6_speedBoost,1000,2000,0,200);
    
    // //--Boost feature--//
    // bool boostEnabled = (ch6_speedBoost > 1500); // Assuming >1800 is the 'high' position
    
    // if (boostEnabled) {
    //   leftSpeed *=(float)boost/100.00;
    //   rightSpeed *=(float)boost/100.00 ;
    // }
    int leftSpeed = throttle + steer;
    int rightSpeed = throttle - steer;
    leftSpeed = constrain(leftSpeed, -255, 255);
    rightSpeed = constrain(rightSpeed, -255, 255);

    // int servo2Angle;
    // int servo1Angle = map(ch1_Servo2, 1000, 2000, 0, 180);
    if ((abs(ch5_Servo1-1500))<50){
      ch5_Servo1=1500;
    }
    if ((abs(ch1_Servo2-1500))<50){
      ch1_Servo2=1500;
    }
    if(ch5_Servo1>1800){
      servoControl(0, 96);
      // Serial.print("Servo2: "); Serial.println(100);

    }
    else if(ch5_Servo1<1200){
      servoControl(0, 85);
      // Serial.print("Servo2: "); Serial.println(80);

    }
    else{
      servoControl(0, 90);
    }

    if(ch1_Servo2>1500 + DEADZONE){
      servoControlMicroseconds(1, 1570);
      // Serial.print("Servo2: "); Serial.println(95);
    }
    else if(ch1_Servo2<1500 - DEADZONE){
      servoControl(1, 85);
      // Serial.print("Servo2: "); Serial.println(85);

    }
    else{
      servoControl(1, 90);
    }
    // --- E. PRINT ALL VALUES TO SERIAL MONITOR ---
    Serial.print("RAW CHANNELS: [");
    Serial.print(ch3_Throttle); Serial.print(", ");
    Serial.print(ch4_Steer); Serial.print(", ");
    Serial.print(ch5_Servo1); Serial.print(", ");
    Serial.print(ch1_Servo2); Serial.print(", ");
    

    Serial.print("CALCULATED: ");
    Serial.print("LeftMotor: "); Serial.print(leftSpeed);
    Serial.print(" | RightMotor: "); Serial.println(rightSpeed);
    // Serial.print(" | S1: "); Serial.print(servo1Angle);
    // Serial.print(" | S2: "); Serial.println(servo2Angle);

    // --- F. WRITE TO HARDWARE ---
    controlMotor('L', leftSpeed);
    controlMotor('R', rightSpeed);
    // servo[0].write(servo1Angle);
    // servo[].write(servo2Angle);
  }
  
  delay(200); // Small delay to stabilize the loop
}