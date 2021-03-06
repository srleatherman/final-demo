#include <Wire.h>
#include "Encoder.h" //this gives us access to the encoder.h files 
#define output2A 3 //this pin is output A for the encoder motor 2
#define output2B 6 //this pin is output B for the encoder motor 2
#define outputA 5 //this pin is output A for the encoder motor 1
#define outputB 2 // this pin is output B for the encoder motor 1
#define motorVoltageRight 9 //this is one of the two voltage pwm pins we could have picked
#define motorVoltageLeft 10//thi is for the second motor voltage and a pwm pin
#define button 4 // this pin is for setting the interupt
#define signOfVoltage 7 // this is what direction the wheel will turn motor 1
#define signOfVoltLeft 8
#define flagIndicator 12 // this pin is if there is a flag
#define SLAVE_ADDRESS 0x04

Encoder motorRight(outputA, outputB);
Encoder motorLeft(output2A, output2B);
bool stay = false;
int startTime=0;
float outcountRight = 0;
float outcountLeft = 0;
float countRight=0;
float countLeft =0;
float lastLeft=0;
float lastRight =0;
float pi = 3.14159;
int cpr = 3200;
float startAngleL =0;
float startAngleR = 0;
float fAngleL =0;
float fAngleR =0;
float angularVelocityR =0;
float angularVelocityL =0;
int time1 =0;
float Kp = 2;
float Ki = .85;
float Kd =0;
float Kp2 =5.9;
float Ki2 =0.3;
float Kd2 =0;
float I = 0;
float e_past = 0;
float Ts = 0;
float Tc = millis();
float r_desired=2;
float y =0;
float r = 0.076;
float d = 0.31;
float e =0;
float D =0;
float u =0;
float upwm =0;
float rvdelta;
float lvdelta;
int loopcount =0;
float desired_angle =pi/2 ;
float uout = 0;
float Dout = 0;
float eout = 0;
float e_past_out=0;
float phi = 0;
float yout =0;
float I2 =0;

int number = 0;
int angle = 0;
int angleWant = 0;
int state = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(19200);
  //set up all of my output pins 
  pinMode(button, OUTPUT);
  digitalWrite(button, HIGH);
  
  pinMode(signOfVoltage,OUTPUT);
  digitalWrite(signOfVoltage, HIGH);

  pinMode(signOfVoltLeft,OUTPUT);
  digitalWrite(signOfVoltLeft, HIGH);
  
  pinMode(motorVoltageRight, OUTPUT);
  analogWrite(motorVoltageRight, 0);
  
  pinMode(motorVoltageLeft, OUTPUT);
  analogWrite(motorVoltageLeft, 0);
  
  //set all of my input pins
  pinMode(flagIndicator, INPUT);

  // initialize i2c as slave
  Wire.begin(SLAVE_ADDRESS);
  // define callbacks for i2c communication
  Wire.onReceive(receiveData);

}

void loop() {
  if((state==0||state==1)&& !stay){
  loopcount = loopcount +1;
  //outerloop
  countRight = motorRight.read();
  countLeft = motorLeft.read();
  outcountRight = motorRight.read()- lastRight;
  outcountLeft = motorLeft.read()- lastLeft;

  lastRight = motorRight.read();
  lastLeft = motorLeft.read();
  phi = phi - (2*pi/3200)*r*(outcountRight - outcountLeft)/d;
  eout = desired_angle - phi ;
  if(Ts>0){
    Dout = (eout-e_past_out)/Ts;
    e_past_out= eout;  
  }
  else{
    Dout=0;
  }  

  I2 = I2 + (float)(eout*Ts)/(float)1000;
 
  uout = Kp2*eout + Ki2 *I2 + Kd2*Dout;
  r_desired=-1*uout;
    
  //r_desired =-1;
  //inner loop
  fAngleR = (float)countRight*2*pi/(float)cpr;
  if(Ts>0){
    angularVelocityR = -1*(float)1000*(fAngleR-startAngleR)/(float)(Ts);
  }
  startAngleR = fAngleR;
  
  fAngleL = (float)countLeft*2*pi/(float)cpr;
  if(Ts>0){
    angularVelocityL = -1*(float)1000*(fAngleL-startAngleL)/(float)(Ts);
  }
  startAngleL = fAngleL;
  

  y =(float)(-1*r*(angularVelocityR-angularVelocityL))/(float)d;
  e= r_desired-y;
  
  if(Ts>0){
    D = (e-e_past)/Ts;
    e_past= e;  
  }
  else{
    D=0;
  }  
  I = I  + (float)(Ts*e)/(float)1000;
 
  u = Kp*e + Ki*I + Kd*D;
  upwm = u*255/7.5;
  rvdelta =upwm;
  lvdelta = -upwm;
  
  if(rvdelta >255){
    rvdelta = 255;
  }
  if(rvdelta < -255){
    rvdelta = -255;
  }
  if(lvdelta >255){
    lvdelta = 255;
  }
  if(lvdelta < -255){
    lvdelta = -255;
  }
  if(lvdelta>0){
     digitalWrite(signOfVoltLeft, HIGH);
  }else{
     digitalWrite(signOfVoltLeft, LOW);
  }
  if(rvdelta>0){
     digitalWrite(signOfVoltage, LOW);
  }else{
     digitalWrite(signOfVoltage, HIGH);
  }
  //Serial.print(desired_angle);
  //Serial.print('\t');
  //Serial.print(phi);
  //Serial.print('\t');
  //Serial.print(eout);
  //Serial.print('\t');
  //Serial.print(motorRight.read());
  //Serial.print('\t');
  //Serial.print(lvdelta);
  //Serial.print('\t');
  //Serial.print(rvdelta);
  //Serial.print('\t');    
  //Serial.println(r_desired); 
  
  if(loopcount<1){
    analogWrite(motorVoltageRight,0); 
    analogWrite(motorVoltageLeft,0);
  }else if(eout == 0){
   analogWrite(motorVoltageRight,0); 
   analogWrite(motorVoltageLeft,0);
   motorRight.write(0);
   motorLeft.write(0);
   eout =0;
   phi = 0;
   lastcountLeft = 0;
   lastcountRight = 0;
   loopcount =0;
   I = 0;
   I2 = 0;
   startAngleR = 0;
   startAngleL = 0;
   
  }else{
    analogWrite(motorVoltageRight,abs(rvdelta)); 
    analogWrite(motorVoltageLeft,abs(lvdelta));
  }
  
  Ts =millis()- Tc;
  Tc = millis();
  delay(10);
  }
  if(state==2&& !stay){
    digitalWrite(signOfVoltLeft,LOW);
    digitalWrite(signOfVoltage,HIGH);
    analogWrite(motorVoltageRight,100); 
    analogWrite(motorVoltageLeft,100);
    }
  if(state==3 || stay){
    analogWrite(motorVoltageRight,0); 
    analogWrite(motorVoltageLeft,0);
    stay = true;
  }
  }
void receiveData(int byteCount) {
Serial.print("data received: ");
while (Wire.available()) {
    state = Wire.read();
    angle = Wire.read();

  if(state==1){
  if (angle>=40){
    desired_angle=(angle-256)*pi/180;
    }
  else{
    desired_angle=angle*pi/180;
  }}
  else{
    desired_angle=pi/4;
  }
Serial.print(state);
Serial.print('\n');
Serial.print(desired_angle);
Serial.print('\n');
}}
