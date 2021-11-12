#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include "Arduino.h"

// Display
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET 	-1 // This display does not have a reset pin accessible
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Motors
#define BASESPEED 1260
#define SINGLESPEED 1425
#define MOTORFREQ 1000
#define MDELAY 150
#define MSTOP 50
#define MOTOR_RF PA_0 // Right motor, forwards input
#define MOTOR_RB PA_1 // Right motor, backwards input
#define MOTOR_LF PA_2 // Right motor, forwards input
#define MOTOR_LB PA_3 // Right motor, backwards input
#define PADDLE PA_6 // Paddle motor
#define SERVO PA_8 // Servo motor

// Sensors
#define REFLECTANCE_L PA5
#define REFLECTANCE_R PA4

// Functions
void set_motor(int value, PinName forward, PinName backward);
double get_error(int left, int right);
int slope();

// Variables
#define KP 300
#define KI 0
#define KD 170
#define IMAX 1000
#define SERVO_INTERVAL 3000
#define PADDLE_INTERVAL 5000
#define NORMAL_SPEED_LOOPS 60
#define SLOW_SPEED_LOOPS 20
#define PADDLE_NORMAL_SPEED 3900
#define PADDLE_SLOW_SPEED 0
static double error, last_error = 0;
static int current_state = 0, last_state = 0;
static long start, mid, end; // m: time in previous error state, n: time in current error state
static bool last_left;
static int left_sensor, right_sensor;
static double p, i, d, g;
static int speed, single = 0;
static long previousServoTime = 0, currentServoTime = 0;
static bool zero_degrees = true;
static long previousPaddleTime = 0, currentPaddleTime = 0;
static bool normal_speed = true;
static int paddle_speed = PADDLE_NORMAL_SPEED;
static int loop_limit = NORMAL_SPEED_LOOPS, loop_counter = 0;

void setup() {
  Serial.begin(9600);

  // Motor setup
  pinMode(MOTOR_LF, OUTPUT); 
  pinMode(MOTOR_LB, OUTPUT);
  pinMode(MOTOR_RF, OUTPUT); 
  pinMode(MOTOR_RB, OUTPUT);
  pinMode(PADDLE, OUTPUT);
  pwm_start(PADDLE, MOTORFREQ, paddle_speed, RESOLUTION_12B_COMPARE_FORMAT);
  pinMode(SERVO, OUTPUT);
  pwm_start(SERVO,50,1500,TimerCompareFormat_t::MICROSEC_COMPARE_FORMAT);
  pinMode(SERVO, INPUT);

  // Sensor setup
  pinMode(REFLECTANCE_L, INPUT_PULLUP);
  pinMode(REFLECTANCE_R, INPUT_PULLUP);
  start = millis(), mid = millis(), end = millis();

  // Display setup
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(500);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.display();
}

void loop() {
  // Servo code
  currentServoTime = millis();
  if (zero_degrees == true) {
    if (currentServoTime - previousServoTime >= SERVO_INTERVAL) {
      previousServoTime = currentServoTime;
      pinMode(SERVO, OUTPUT);
      zero_degrees = false;
      pwm_start(SERVO,50,500,TimerCompareFormat_t::MICROSEC_COMPARE_FORMAT);
      pinMode(SERVO, INPUT);
    }
  } else {
    if (currentServoTime - previousServoTime >= 300) {
      pinMode(SERVO, OUTPUT);
      zero_degrees = true;
      pwm_start(SERVO,50,1500,TimerCompareFormat_t::MICROSEC_COMPARE_FORMAT);
      pinMode(SERVO, INPUT);
    } 
  }

  // Paddle code
  if (loop_counter % loop_limit == 0) {
    loop_counter = 0;
    if (normal_speed == true) {
      normal_speed = false;
      paddle_speed = PADDLE_SLOW_SPEED;
      loop_limit = SLOW_SPEED_LOOPS;
    } else {
      normal_speed = true;
      paddle_speed = PADDLE_NORMAL_SPEED;
      loop_limit = NORMAL_SPEED_LOOPS;
    }
  }
  loop_counter++;
  pwm_start(PADDLE, MOTORFREQ, paddle_speed, RESOLUTION_12B_COMPARE_FORMAT);

  left_sensor = digitalRead(REFLECTANCE_L);
  right_sensor = digitalRead(REFLECTANCE_R);
  error = get_error(left_sensor, right_sensor);
  p = KP * error;
  d = KD * slope();
  i = KI * error + i;
  if (i > IMAX) {
    i = IMAX;
  } else if (i < -IMAX) {
    i = -IMAX;
  }
  g = p + i + d;
  if (g < 0) {
    speed = -(int) g;
    if (error > 1 || error < -1) {
      set_motor(0, MOTOR_RF, MOTOR_RB);
      set_motor(-BASESPEED - speed, MOTOR_LF, MOTOR_LB);
      delay(MDELAY);
      set_motor(0, MOTOR_RF, MOTOR_RB);
      set_motor(0, MOTOR_LF, MOTOR_LB);
      delay(MSTOP);
    } else {
      set_motor(SINGLESPEED + speed, MOTOR_RF, MOTOR_RB);
      set_motor(0, MOTOR_LF, MOTOR_LB);
    }
  } else if (g > 0) {
    speed = (int) g;
    if (error > 1 || error < -1) {
      set_motor(-BASESPEED - speed, MOTOR_RF, MOTOR_RB);
      set_motor(0, MOTOR_LF, MOTOR_LB);
      delay(MDELAY);
      set_motor(0, MOTOR_RF, MOTOR_RB);
      set_motor(0, MOTOR_LF, MOTOR_LB);
      delay(MSTOP);
    } else {
      set_motor(0, MOTOR_RF, MOTOR_RB);
      set_motor(SINGLESPEED + speed, MOTOR_LF, MOTOR_LB);
    }
  } else {
    set_motor(BASESPEED, MOTOR_RF, MOTOR_RB);
    set_motor(BASESPEED, MOTOR_LF, MOTOR_LB);
  }
  
  
  
  last_error = error;
  
  display.print("left_sensor: "); display.print(left_sensor); display.print("\n");
  display.print("right_sensor: "); display.print(right_sensor); display.print("\n");
  display.print("P: "); display.print(p); display.print("\n");
  display.print("I: "); display.print(i); display.print("\n");
  display.print("D: "); display.print(d); display.print("\n");
  display.print("G: "); display.print(g); display.print("\n");
  display.print("error: "); display.print(error); display.print("\n");
  display.print("last_left: "); display.print(last_left); display.print("\n");
  display.display();
  display.setCursor(0,0);
  display.clearDisplay();
}

void set_motor(int value, PinName forward, PinName backward) {
  int offset = 1250;
  if (value > 0) { // Motor turns forwards, backwards value set to 0
    pwm_start(forward, MOTORFREQ, value + offset, RESOLUTION_12B_COMPARE_FORMAT);
    pwm_start(backward, MOTORFREQ, 0, RESOLUTION_12B_COMPARE_FORMAT);
  } else if (value < 0) {  // Motor turns backwards, forwards value set to 0
    pwm_start(forward, MOTORFREQ, 0, RESOLUTION_12B_COMPARE_FORMAT);
    pwm_start(backward, MOTORFREQ, -value + offset, RESOLUTION_12B_COMPARE_FORMAT);
  } else { // Motor stops
    pwm_start(forward, MOTORFREQ, 0, RESOLUTION_12B_COMPARE_FORMAT);
    pwm_start(backward, MOTORFREQ, 0, RESOLUTION_12B_COMPARE_FORMAT);
  }
}

double get_error(int left, int right) {
  if (left == 1 && right == 1) { // Both sensors on tape
    return 0;
  } else if (left == 1 && right == 0) { // Left sensor on tape, right sensor off
    last_left = true;
    return 1;
  } else if (left == 0 && right == 1) { // Right sensor on tape, left sensor off
    last_left = false;
    return -1;
  } else if (left == 0 && right == 0) {
    if (last_left == true) {
      return 1.2;
    } else {
      return -1.2;
    }
  }
}

int slope() {
  if (error == last_error) {
    end = millis();
    return 0;
  } else {
    start = mid;
    mid = millis();
  }
  double time = (double) (end - start) / 1000;
  return (double) ((error - last_error) / time);
}
