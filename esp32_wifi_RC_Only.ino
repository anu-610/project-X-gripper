#define REMOTEXY_MODE__WIFI_POINT

#include <WiFi.h>

// RemoteXY connection settings 
#define REMOTEXY_WIFI_SSID "projectx"
#define REMOTEXY_WIFI_PASSWORD "projectx@mandi"
#define REMOTEXY_SERVER_PORT 6377


#include <RemoteXY.h>

// RemoteXY GUI configuration  
#pragma pack(push, 1)  
uint8_t RemoteXY_CONF[] =   // 37 bytes
  { 255,2,0,0,0,30,0,19,0,0,0,80,114,111,106,101,99,116,88,0,
  31,1,200,76,1,1,1,0,5,140,19,44,44,32,2,26,31 };
  
// this structure defines all the variables and events of your control interface 
struct {

    // input variables
  int8_t joystick_01_x; // from -100 to 100
  int8_t joystick_01_y; // from -100 to 100

    // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0

} RemoteXY;   
#pragma pack(pop)




// --- Motor Control Pins
// Left Motor
#define ENA_PIN 14   // (Pin 1 on L293D) - Must be PWM-capable
#define LEFT_IN1 27  // (Pin 2 on L293D)
#define LEFT_IN2 26  // (Pin 7 on L293D)
// Right Motor
#define ENB_PIN 5   // (Pin 9 on L293D) - Must be PWM-capable
#define RIGHT_IN3 16 // (Pin 10 on L293D)
#define RIGHT_IN4 17 // (Pin 15 on L293D)

// --- ESP32 PWM Configuration ---
// ESP32 uses ledc channels for PWM
#define PWM_FREQ 5000   // 5kHz frequency for motors
#define PWM_RESOLUTION 8 // 8-bit resolution (0-255)
#define LEFT_PWM_CHANNEL 0
#define RIGHT_PWM_CHANNEL 1


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
  
  Serial.begin(115200);
  Serial.println("RC Car Ready to Connect...");
}

void loop() 
{ 
  RemoteXY_Handler ();
  
  // --- Get Joystick Values ---
  // We copy them to new variables so we can modify them
  int x = RemoteXY.joystick_01_x;
  int y = RemoteXY.joystick_01_y;

  // General deadzone to prevent "creep" when joystick is centered
  int generalDeadzone = 15;

  int straightAssistDeadzone = 30;

  if (abs(x) < generalDeadzone) {
    x = 0;
  }
  if (abs(y) < generalDeadzone) {
    y = 0;
  }

  // If Y is pushed hard (e.g., >80) and X is only slightly moved, force X to 0
  if (abs(y) > 80 && abs(x) < straightAssistDeadzone) {
    x = 0; 
  }


  int rawLeft = y + x;
  int rawRight = y - x;

  // Map the -200 to +200 range to our motor speed range (-255 to +255)
  int motorLeftSpeed = map(rawLeft, -200, 200, -255, 255);
  int motorRightSpeed = map(rawRight, -200, 200, -255, 255);

  // code to prevent it want to beyond range
  motorLeftSpeed = constrain(motorLeftSpeed, -255, 255);
  motorRightSpeed = constrain(motorRightSpeed, -255, 255);

  // --- 5. Control Motors ---
  setMotorSpeed(0, motorLeftSpeed);  // 0 = Left Motor
  setMotorSpeed(1, motorRightSpeed); // 1 = Right Motor

  // Print values for debugging
  Serial.print("X:"); Serial.print(x);
  Serial.print(" Y:"); Serial.print(y);
  Serial.print("  |  Left:"); Serial.print(motorLeftSpeed);
  Serial.print("  Right:"); Serial.println(motorRightSpeed);
  Serial.println("\n\n\n");
  RemoteXY_delay(20); 
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

  // Get absolute speed for PWM
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
