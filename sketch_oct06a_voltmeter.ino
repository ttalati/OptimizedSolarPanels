/*
  Volt Meter: for 4 input from Solar panel or battery
  Stepper motor: drive stepper motor based on input from power source
*/
#include "pitches.h"
// include the library code:
#include <LiquidCrystal.h>
///#include <Stepper.h>
#include <Servo.h>

#define NOTE_BLANK 0

const int nservo_set = 3;
const int dbg = 1;
const int isLCD = 0;
const int noiseLVL = 20;
const int melody[][2] =
{
   {NOTE_C4, 4},
   {NOTE_G3, 8},
   {NOTE_G3, 8},
   {NOTE_A3, 4},
   {NOTE_G3, 4},
   {NOTE_BLANK, 4},
   {NOTE_B3, 4},
   {NOTE_C4, 4}
};


// initialize the library with the numbers of the interface pins
// lcd pins
int lcdRSpin = 13; //12
int lcdEpin  = 12; //11
int lcd4pin  = 5;
int lcd5pin  = 4;
int lcd6pin  = 3;
int lcd7pin  = 2;

// Servo pins
int set1SMx = 2;
int set1SMy = 3;
int set2SMx = 4;
int set2SMy = 5;
int set3SMx = 12;
int set3SMy = 13;

//  speaker pins
int spkrPin = 7;
int echoPin = 8;
int trigPin = 9;

LiquidCrystal lcd(lcdRSpin, lcdEpin, lcd4pin, lcd5pin, lcd6pin, lcd7pin);

//// stepper motor pins: current not used
//int stepIN1pin = 8;
//int stepIN2pin = 9;
//int stepIN3pin = 10;
//int stepIN4pin = 11;
//const int stepsPerRevolution = 100;//200;//2048;
///Stepper myStepper(stepsPerRevolution, stepIN1pin, stepIN2pin, stepIN3pin, stepIN4pin);

Servo servoX[nservo_set]; // create servo object to control a servo
Servo servoY[nservo_set];

int pos = 0;
int posH0 = 0, posH1 = 0, posV0 = 0, posV1 = 0;
int xPos = 0;
int yPos = 0;

int is_xPosChg = 0;
int is_yPosChg = 0;

int   a0, a1, a2, a3, a_scan_delay = 0, sys_delay = 0; // variable to store the value coming from the sensor
float v0, v1, v2, v3; // variable to store the voltage coming from the sensor

int   motorSpeed    = 100; // max speed is 100 or 200
int   prv_sensorVal = 0; 
int   dif_sensorVal = 0;
int   step_count    = 0;

float prv_dist = 0;

void mymelody() {
  int numNotes = sizeof(melody) / sizeof(melody[0]);

  for (int thisNote=0; thisNote < numNotes; thisNote++) {
    int thisNoteTone = melody[thisNote][0];
    int thisNoteDuration = melody[thisNote][1];
    int noteDurationMS = 1000 / thisNoteDuration;

    tone(spkrPin, thisNoteTone, noteDurationMS);
    delay(noteDurationMS * 1.30);
  }
}

int system_delay(int scan_delay) {

  if (scan_delay < 128) sys_delay = 0;          // 0 sec
  else if (scan_delay < 256) sys_delay = 10;    // 10 sec
  else if (scan_delay < 384) sys_delay = 30;    // 30 sec
  else if (scan_delay < 512) sys_delay = 60;    // 1min    ___
  else if (scan_delay < 640) sys_delay = 120;   // 2min
  else if (scan_delay < 768) sys_delay = 180;   // 3min
  else if (scan_delay < 896) sys_delay = 300;   // 5min
  else                       sys_delay = 600;   //10min

  sys_delay *= 1000;
  if (dbg) { Serial.print("sys_delay:"); Serial.println(sys_delay); }
  return sys_delay;
}
//----------------------------------------------------------- setup
void setup() {
  if (dbg)  Serial.begin(9600);  // start serial for output

  mymelody();
  
/*
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  //lcd.print("Motor Speed:");
///  // set the RPM
///  myStepper.setSpeed(motorSpeed);
*/
  servoX[0].attach(set1SMx); // attaches the servo on pin 6 to the servo object
  servoY[0].attach(set1SMy);
  servoX[1].attach(set2SMx); // attaches the servo on pin 6 to the servo object
  servoY[1].attach(set2SMy);
  servoX[2].attach(set3SMx); // attaches the servo on pin 6 to the servo object
  servoY[2].attach(set3SMy);

  // initialize to east facing setup
  for (int idx = 0; idx < nservo_set; idx++) {
    servoX[idx].write(179);
    servoY[idx].write(179);
    if (dbg) { Serial.print("Initializing servoSet: "); Serial.println(idx); }
  }
  
  delay(5000);
  if (dbg) { Serial.println("Setup procedure is over"); }

  pinMode(echoPin, INPUT);
  pinMode(trigPin, OUTPUT);

  
}

//----------------------------------------------------------- AnalogLCDprint
void analogLCDprint(int a0, int a1, int a2, int a3, int a_scan_dly) {
  if (isLCD) lcd.setCursor(0,0);  
  if (isLCD) lcd.print("A0:     A1:     ");
  if (isLCD) lcd.setCursor(3, 0);  lcd.print(a0);
  if (isLCD) lcd.setCursor(11, 0); lcd.print(a1);
  if (dbg) { Serial.print("A0: "); Serial.println(a0); }
  if (dbg) { Serial.print("A1: "); Serial.println(a1); }
  if (isLCD) lcd.setCursor(0,1);
  if (isLCD) lcd.print("A2:     A3:     ");
  if (isLCD) lcd.setCursor(3, 1);  lcd.print(a2);
  if (isLCD) lcd.setCursor(11, 1); lcd.print(a3);
  if (dbg) { Serial.print("A2: "); Serial.println(a2); }
  if (dbg) { Serial.print("A3: "); Serial.println(a3); }
  if (dbg) { Serial.print("scan delay: "); Serial.println(a_scan_dly); }
  
}
//----------------------------------------------------------- voltageLCDprint
void voltageLCDprint(float v0, float v1, float v2, float v3) {
  if (isLCD) lcd.setCursor(0,0);  
  if (isLCD) lcd.print("V0:     V1:     ");
  if (isLCD) lcd.setCursor(3, 0);  lcd.print(v0);
  if (isLCD) lcd.setCursor(11, 0); lcd.print(v1);
  if (dbg) { Serial.print("V0: "); Serial.println(v0); }
  if (dbg) { Serial.print("V1: "); Serial.println(v1); } 
  if (isLCD) lcd.setCursor(0,1);
  if (isLCD) lcd.print("V2:     V4:     ");
  if (isLCD) lcd.setCursor(3, 1);  lcd.print(v2);
  if (isLCD) lcd.setCursor(11, 1); lcd.print(v3);
  if (dbg) { Serial.print("V2: "); Serial.println(v2); }
  if (dbg) { Serial.print("V3: "); Serial.println(v3); }
}
//----------------------------------------------------------- ServoPositionprint
void servoPositionprint(int xPos, int yPos) {
  if (isLCD) lcd.setCursor(0,0);  
  if (isLCD) lcd.print("xPos:           ");
  if (isLCD) lcd.setCursor(6, 0);  lcd.print(xPos);
  if (dbg) { Serial.print("xPos: "); Serial.println(xPos); }
  if (isLCD) lcd.setCursor(0,1);
  if (isLCD) lcd.print("yPos:           ");
  if (isLCD) lcd.setCursor(6, 1);  lcd.print(yPos);
  if (dbg) { Serial.print("yPos: "); Serial.println(yPos); }
}
//----------------------------------------------------------- ultrasonic_dist_print
void  ultrasonic_dist_print(int dist_cm) {
  if (isLCD) lcd.setCursor(0,0);
  if (isLCD) lcd.print("Object at (cm):");
  if (isLCD) lcd.setCursor(0,1);
  if (isLCD) lcd.print("                ");
  if (isLCD) lcd.setCursor(0,1);
  if (isLCD) lcd.print(dist_cm);
  if (dbg) { Serial.print("Object at (cm)"); Serial.println(dist_cm);}
}

//----------------------------------------------------------- loop
/*                      **SUN LIGHT**
 *            V0
 *        --------+
 *             .  +
 *           V3   +  V1
 *          .     +
 *        --------+
 *            V2
 */
void loop() {
  float dist_cm;
  int   pulseLen_us; // micro_seconds;


  //bit-bang a small squae wave on the trig pin
  // to start the range finder
  digitalWrite(trigPin, LOW);
  delayMicroseconds(20);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(100);
  digitalWrite(trigPin, LOW);

  //measure the pulse length from the echo pin
  pulseLen_us = pulseIn(echoPin, HIGH);
  //calculate the distance using the speed of sound
  dist_cm = pulseLen_us / 29.387 / 2;

  if (dist_cm < prv_dist) {
    mymelody();
  } else {
      noTone(spkrPin);
  }
  prv_dist = dist_cm;

  ultrasonic_dist_print(dist_cm);
  delay(sys_delay);  

  
  // Read the value from the analog input pin
  // A value of 1023 = 5V, a value of 0 = 0V
  a0 = analogRead(A0); // read the analog input pin0   
  a1 = analogRead(A1);
  a2 = analogRead(A3);
  a3 = analogRead(A4);
  a_scan_delay = analogRead(A5); // value 0..1023 is divided in to 8 equal parts

  sys_delay = 0;system_delay(a_scan_delay);
  // ---------to avoid noise when servo motor is connected
  if (a0 < noiseLVL) a0=0;
  if (a1 < noiseLVL) a1=0;
  if (a2 < noiseLVL) a2=0;
  if (a3 < noiseLVL) a3=0;
  analogLCDprint(a0,a1,a2,a3,a_scan_delay);
  
  // Convert sensor value to voltage
  v0 = a0 *(5.0/1023.0);
  v1 = a1 *(5.0/1023.0);
  v2 = a2 *(5.0/1023.0);
  v3 = a3 *(5.0/1023.0);
  ///voltageLCDprint(v0, v1, v2, v3);

  // stay where sunflower points
  if ((a0 == 0) && (a1 == 0) && (a2 == 0) && (a3 == 0)) {
    is_xPosChg = 0;
    is_yPosChg = 0;
    if (dbg) { Serial.println("No horizontal & Vertical change"); } 
    ;
  } else {
    // no x-axis movement
    if ((a0 == 0) && (a1 == 0) && (a2 == 0)) {
      is_xPosChg = 0;
      if (dbg) { Serial.println("No horizontal change"); } 
      ;
    } else {
      is_xPosChg = 1;
      posH0 = posH1 = 0;
      //-----------------setting X position
      if ((a0+a1) > 0) {
        posH0 = (a1 * 90) / (a0+a1);
      }
      if ((a1+a2) > 0) {  
        posH1 = (a2 * 90) / (a1+a2);
      }
      if (dbg) { Serial.print("posH0:"); Serial.println(posH0); }
      if (dbg) { Serial.print("posH1:"); Serial.println(posH1); }
   
      if ((a0 >= a2) && (a1 >= a2)) {
        xPos = posH0;
      }
      else if ((a1 >= a0) && (a2 >= a0)) {
        xPos = posH1 + 90;                
      }
    }
    // no Yaxis movement
    if ((a0 == 0) && (a2 == 0) && (a3 == 0)) {
      is_yPosChg = 0;
      if (dbg) { Serial.println("No vertical change"); } 
      ;
    } else {
      is_yPosChg = 1;
      posV0 = posV1 = 0;
      //-----------------setting Y position
      if ((a0+a3) > 0) {
        if (a0 == 0) posV0 = 90;
        else posV0 = (a3 * 90) / (a0+a3);
      }
      if ((a2+a3) > 0) {
        if (a3 == 0) posV1 = 90;
        else posV1 = (a2 * 90) / (a2+a3);
      }
      if (dbg) { 
        Serial.print("posV0:"); Serial.println(posV0); 
      }
      if (dbg) { Serial.print("posV1:"); Serial.println(posV1); }

      if ((a0 >= a2) && (a3 >= a2)) {
        yPos = posV0;
      }
      else if ((a2 >= a0) && (a3 >= a0)) {
        yPos = posV1 + 90;                
      }

      yPos = 180 - yPos;
    }
  }

  //------------------------------------ Setting servo position ####
  servoPositionprint(xPos, yPos);
   
  //if (is_xPosChg == 1) {
  //  for (int idx=0; idx < nservo_set; idx++) {
  //    servoX[idx].write(xPos);
  //  }
  //} else {
  //  if (dbg) Serial.println("No change for xpos");
  //}
  delay(100);

  if (xPos > 90) yPos = 180 - yPos;

  //if (is_yPosChg == 1) {
  //  for (int idx=0; idx < nservo_set; idx++) {
  //    servoY[idx].write(yPos);
  //  }
  //} else {
  //  if (dbg) Serial.println("No change in ypos");
  //}
  delay(100);

  is_xPosChg = 0;
  is_yPosChg = 0;
  
  //--xx--sunpos = map(v0, 0, 1023, 0, 180);
  //--xx--myservo.write(val);
  ///---motorSpeed = map(sensorValue, 0, 1023, 0, 100);
//  Serial.print("motorSpeed: ");
//  Serial.println(motorSpeed);
/*
  for (pos=0; pos <=180; pos += 1) {
    //myservoX.write(pos);
    myservoY.write(pos);
    delay(15);
  }
  */
  /*
  for (pos=180; pos >=0; pos -= 1) {
    myservoX.write(pos);
    myservoY.write(pos);
    delay(15);
  }
*/
}
