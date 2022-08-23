
//Servo
#include <Servo.h>
Servo myservo;
const int openAngle = 130;
const int closedAngle = 23;
const int waitTime = 500;

//Sonar
#include <NewPing.h>   // include the NewPing library for this program
 
#define VCC_PIN 13
 
#define TRIGGER_PIN 12 // sonar trigger pin will be attached to Arduino pin 12
 
#define ECHO_PIN 11 // sonar echo pint will be attached to Arduino pin 11
 
#define GROUND_PIN 10
 
#define MAX_DISTANCE 200 // fmaximum distance set to 200 cm
 
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // initialize NewPing\\

const int emptyDistance = 15;



//RTC
#include "RTClib.h"
#include <Wire.h>

RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//Pushbutton - for diagnostic and opening
#define ON HIGH
#define OFF LOW
//#define pushbutton 4

//LED
const int LIGHT = 6;

//constants

const int serveBrfH = 9;
const int serveM = 4;
const int serveDinH = 21;
const int serveSec = 0;
const int checkDuration = 10000; //3 min
const int checkFrequency = 1000; //30 sec
const int arrSize = 10;
const int thrshld = 8;
//array
bool checkRes [arrSize];


void setup() {
  
 //Serial monitor
 Serial. begin(9600);  // set data transmission rate to communicate with computer

 //Sonar
 pinMode(ECHO_PIN, INPUT) ;
 pinMode(TRIGGER_PIN, OUTPUT) ;
 pinMode(GROUND_PIN, OUTPUT);  // tell pin 10 it is going to be an output
 pinMode(VCC_PIN, OUTPUT);  // tell pin 13 it is going to be an output
 digitalWrite(GROUND_PIN,LOW); // tell pin 10 to output LOW (OV, or ground)
 digitalWrite(VCC_PIN, HIGH) ; // tell pin 13 to output HIGH (+5V)
 
 //Servo
 myservo.attach(9); //attaches the servo on pin 9 to the servo object 
 myservo.write(closedAngle); //130 is the open angle
                    // 23 is the close angle
 //RTC-DS3231
  delay(3000); // wait for console opening

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if(rtc.lostPower()) {
     rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  //pushbutton
 // pinMode(pushbutton, INPUT);

  //LED
  pinMode(LIGHT, OUTPUT);
 
 
}

void wait(int milliseconds) {
  unsigned long prevTime = millis();
  unsigned long currentTime = millis();

  bool wait = true;
  while(wait) {
    if((currentTime - prevTime) >= milliseconds) {
      wait = false;
    }
    currentTime = millis();
  }
}

void openServo() {
  myservo.write(openAngle);

  wait(waitTime);

  myservo.write(closedAngle);
}

bool checkTime(bool &serve, int &h, int &m, int &sec ) {
  if((h == serveBrfH) && (m == serveM) && (sec == 0)) {
   serve = true;
  }
  else if((h==serveDinH) && (m == serveM) && (sec == 0)) {
    serve = true;
  }

  return serve;
}

void acctDistance(const int &distance, int pos) {
  bool res = true;
  if(distance < emptyDistance) {
    res = false;
  }
  checkRes[pos] = res;
}

bool cmptServng() {
  int countT = 0;
  int countF = 0;
  for(int i = 0; i < arrSize; i++) {
    if(checkRes[i] == true) {
      countT++;
    }
    else if(checkRes[i] == false) {
      countF++;
    }
  }
  if(countT >= thrshld) {
    return true;
  }
  //it doesn't matter what we are comparing next actually. An else statement will work but to make it clear what we are doing, we are comparing
  else if(countF >=thrshld) {
    return false;
  }
}

bool checkFood() {

   unsigned long prevTime = millis();
   unsigned long currentTime = millis();

   bool dispensing = true;

   int timer = checkDuration;
   int pos = 0;

    digitalWrite(LIGHT, ON);
   while (dispensing == true) {
        
        if((currentTime - prevTime) >= checkFrequency) {
          //timer is reduced until it reaches zero
          timer = timer - checkFrequency;

          //the distance is taken into account and whether meal is served is computed later
          int distance = sonar.ping_cm();
          acctDistance(distance, pos);

          //serial monitor output
          Serial.print("Distance: ");
          Serial.print(distance);
          Serial.print(" timer: ");
          Serial.println(timer);
          
          //updating time as well as the next segment of data to be allocated. 
          prevTime = currentTime;
          pos++;
        }

        if(timer <= 0) {
          dispensing = false;
        }
         
        currentTime = millis();
   }

       //if conditions are not met, return false if met, then return true;
       bool res = cmptServng();

      digitalWrite(LIGHT, OFF);
      return res;
}


void loop() {
  
  DateTime now = rtc.now();
  int h = now.hour();
  int m = now.minute();
  int sec = now.second();

  //prints time on serial monitor
  Serial.print("Time:");
  
  Serial.print(h);
  Serial.print("/");
  Serial.print(m);
  Serial.print("/");
  Serial.println(sec);

  bool serve = false;

  //in case of jamming or whatever a pushbutton is necessary
  /*if(digitalRead(pushbutton) == ON) {
    Serial.println("PUSHBUTTON ON");
    openServo();
  }*/
  
  //else {
       checkTime(serve, h, m, sec);

      if(serve) {
        bool noFood = checkFood();
        if(noFood) {
          Serial.println("Dispensing food");
          openServo();
        }
     }
   //}
}
