/* 
 * Robotics B1 Project code
 */

#include <SoftwareSerial.h>
SoftwareSerial bluetooth(11, 12); // RX, TX; connecc TX of Bluetooth to RX, and RX of Bluetooth to TX via voltage divider (2k to ground, 1k to Arduino)

#define MOTL1 3
#define MOTL2 5
#define MOTR1 6
#define MOTR2 10
#define MOT_DELAY 50

#define TRIG_PIN 2
#define ECHO_PIN 4
#define AVG_NUM  5

#define LIM1 40
#define LIM2 25
#define LIM3 15
#define TONE1 700
#define TONE2 1400
#define TONE3 2500
#define HORN  400

#define BUZZER_PIN 7

#define BACKLED_R 8
#define BACKLED_G 9
#define BACKLED_B 13

#define FRONTLED 1

#define MAX_DIFFERENCE 20                           // adjust sensitivity

double distance, last_distance;

char bt_chr = 'S';        // initialised as "stop"
int motor_speed = 255;    // initialised at full speed

bool clear_ahead = true;
bool no_obstacle_near = true;
//bool horn_on = false;


void motorWrite(int motL1, int motL2, int motR1, int motR2) {
  analogWrite(MOTL1, motL1);
  analogWrite(MOTL2, motL2);
  analogWrite(MOTR1, motR1);
  analogWrite(MOTR2, motR2);
}

void ledWrite(bool led_r, bool led_g, bool led_b) {
  digitalWrite(BACKLED_R, led_r);
  digitalWrite(BACKLED_G, led_g);
  digitalWrite(BACKLED_B, led_b);
}

double getDistance() {
  long duration;
  distance = 0;
  
  for (int i = 0; i < AVG_NUM; i++) {        
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
  
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    
    duration = pulseIn(ECHO_PIN, HIGH);
    distance += duration * 0.034/2;
  }

  return distance/AVG_NUM;
}



void setup() {
  pinMode(MOTL1, OUTPUT);
  pinMode(MOTL2, OUTPUT);
  pinMode(MOTR1, OUTPUT);
  pinMode(MOTR2, OUTPUT);
  motorWrite(0,0,0,0);      // initialise all pins with 0

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  last_distance = getDistance();

  pinMode(BUZZER_PIN,OUTPUT);
  pinMode(BACKLED_R,OUTPUT);
  pinMode(BACKLED_G,OUTPUT);
  pinMode(BACKLED_B,OUTPUT);
  pinMode(FRONTLED,OUTPUT);

  ledWrite(true, false, false);

  bluetooth.begin(9600);
  //Serial.begin(9600);     // can't use, as pins 0 and 1 will otherwise be unusable                  DISABLEEEEE
}

void loop() {
  distance = getDistance();
  if ( abs(distance - last_distance) > MAX_DIFFERENCE) {      // to smooth out spikes in distance measuring
    distance = getDistance();                                 // can use a while loop instead, but it might lock into an endless loop
  }
  last_distance = distance;

  if (distance != 0) {
    if (distance <= LIM3) {
      clear_ahead = false;
      no_obstacle_near = false;

      if (bt_chr == 'F' or bt_chr == 'G' or bt_chr == 'I') {
        motorWrite(0,0,0,0);            // stop the motor if it's moving forward
      }
      
      tone(BUZZER_PIN, TONE3);
      ledWrite(true, false, false);
    } else {
      clear_ahead = true;

      if (distance <= LIM2) {
        no_obstacle_near = false;
        tone(BUZZER_PIN, TONE2);
      } else if (distance <= LIM1) {
        no_obstacle_near = false;
        tone(BUZZER_PIN, TONE1);
      } else {
        no_obstacle_near = true;
        noTone(BUZZER_PIN);
      }
    }
  } else {
    clear_ahead = true;
    no_obstacle_near = true;
    noTone(BUZZER_PIN);
  }
  /*Serial.print(no_obstacle_near);                                         // DISABLEEEEEEEEEE
  Serial.print(clear_ahead);
  Serial.print(" ");*/

  // NOTE: clear_ahead =/= obstacle_near. obstacle_near triggers when there's an object within LIM1, and clear_ahead when within LIM3.


  
  if (bluetooth.available() > 0) {
    bt_chr = bluetooth.read();
    //Serial.println(bt_chr);

    // only when a change in the controls is detected will this switch block execute
    switch (bt_chr) {
      case 'S':     // stop
        motorWrite(0,0,0,0);
        ledWrite(true, false, false);
        break;
        
      case 'F':     // forward
        if (clear_ahead) {
          delay(MOT_DELAY);
          motorWrite(motor_speed,0,0,motor_speed);
          ledWrite(false, true, false);
        }
        break;
      case 'B':     // backward
        delay(MOT_DELAY);
        motorWrite(0,motor_speed,motor_speed,0);
        ledWrite(true, true, true);
        break;
      case 'L':     // left
        delay(MOT_DELAY);
        motorWrite(0,motor_speed,0,motor_speed);
        ledWrite(false, false, true);
        break;
      case 'R':     // right
        delay(MOT_DELAY);
        motorWrite(motor_speed,0,motor_speed,0);
        ledWrite(false, false, true);
        break;
        
      case 'G':     // forward-left
        if (clear_ahead) {
          delay(MOT_DELAY);
          motorWrite(0,0,0,motor_speed);
          ledWrite(false, true, true);
        }
        break;
      case 'I':     // forward-right
        if (clear_ahead) {
          delay(MOT_DELAY);
          motorWrite(motor_speed,0,0,0);
          ledWrite(false, true, true);
        }
        break;
      case 'H':     // backward-left
        delay(MOT_DELAY);
        motorWrite(0,motor_speed,0,0);
        ledWrite(true, true, true);
        break;
      case 'J':     // backward-right
        delay(MOT_DELAY);
        motorWrite(0,0,motor_speed,0);
        ledWrite(true, true, true);
        break;

      // controlling the motor speed for all motorWrite()'s after this loop
      case 'q':
        motor_speed = 255;
        break;
      case '9':
        motor_speed = 230;
        break;
      case '8':
        motor_speed = 204;
        break;
      case '7':
        motor_speed = 179;
        break;
      case '6':
        motor_speed = 153;
        break;
      case '5':
        motor_speed = 128;
        break;
      case '4':
        motor_speed = 102;
        break;
      case '3':
        motor_speed = 77;
        break;
      case '2':
        motor_speed = 51;
        break;
      case '1':
        motor_speed = 26;
        break;
      case '0':
        motor_speed = 0;
        break;

      case 'W':
        digitalWrite(FRONTLED, HIGH);
        break;
      case 'w':
        digitalWrite(FRONTLED, LOW);
        break;

      case 'V':
        //horn_on = true;
        if (no_obstacle_near) {
          tone(BUZZER_PIN, HORN);
        }
        break;
      case 'v':
        //horn_on = false;
        if (no_obstacle_near) {
          noTone(BUZZER_PIN);
        }
        break;

      case 'D':     // stop all? not sure
        motorWrite(0,0,0,0);
        noTone(BUZZER_PIN);
        ledWrite(false, false, false);
        digitalWrite(FRONTLED, LOW);
        break;
    }
  }
  
  delay(30);
}
