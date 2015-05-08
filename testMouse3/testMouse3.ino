
#include <hidboot.h>
#include <usbhub.h>
#include <SPI.h>
#include <SM.h>
#include <Servo.h> 

class MouseRptParser : public MouseReportParser
{

public:
        long curPos;
        // unsigned long timeStamp;
        
protected:
	void OnMouseMove	(MOUSEINFO *mi);
};

void MouseRptParser::OnMouseMove(MOUSEINFO *mi)
{
      //timeStamp=millis();
      curPos=curPos+(mi->dX);
};

// ---------------------- Params
//**** Mouse Crap
USB     Usb;
USBHub  Hub(&Usb);
HIDBoot<HID_PROTOCOL_MOUSE>    HidMouse(&Usb);
MouseRptParser  Prs;
int lastPos;
int mouseDelta;

//**** Servo Stuff
Servo myservo;
int rewardPos=19;
int restPos=26;

//**** Other Vars
long timeOffset;
unsigned long tS;
unsigned long beginTime;
int lastKnownState=49;
int sB;

//**** Trial Stuff
long lFreq[]={700,2000};
int lRand=0;
int rRand=1;
int clickTime=1000;  // in microseconds
long clickPosL;
long clickPosR;
int clickDeltaL;
int clickDeltaR;
int clickLBool;
int clickRBool;

long tRange=90000;
long tRangeE;
long targPos=10000;
const float pi = 3.14;
int tCount=1;
long lowPos=10000;
long highPos=60000;
int rewardTime=2000;  // in ms
int solenoidTime=50;  // in ms


# define rPin 6
# define gPin 5
# define bPin 3
# define clickPinL 7
# define clickPinR 8
# define servoPin 9
# define solenoidPin 12

SM Simple(S1_H, S1_B); // Trial State Machine


//------------- Program Block

void setup()
{
    Serial.begin(115200);
    if (Usb.Init() == -1)
      Serial.println("OSC did not start.");
    Serial.setTimeout(100);
    delay(200);
    
    beginTime=millis();
    Serial.println("Start"); 
    HidMouse.SetReportParser(0,(HIDReportParser*)&Prs);
    
    pinMode(rPin, OUTPUT);
    pinMode(gPin, OUTPUT);
    pinMode(bPin, OUTPUT);
    pinMode(clickPinL,OUTPUT);
    pinMode(clickPinR,OUTPUT);
    pinMode(solenoidPin,OUTPUT);
    
    digitalWrite(rPin, HIGH);
    digitalWrite(gPin, HIGH);
    digitalWrite(bPin, HIGH);
    digitalWrite(clickPinL, LOW);
    digitalWrite(clickPinR, LOW);
    digitalWrite(solenoidPin,LOW);
    
    myservo.attach(servoPin);
    myservo.write(restPos); 
    
    randomSeed(analogRead(0));
    sB=49;
}

void loop()
{
  Usb.Task();    // HID Parsing
  EXEC(Simple);  // State Machine
}


//------------ State Definitions

State S1_H(){
  digitalWrite(rPin, LOW);
  digitalWrite(gPin, HIGH);
  digitalWrite(bPin, LOW);
  lastPos=Prs.curPos;
  Prs.curPos=0;
  lastKnownState=49;
}


State S1_B(){ 
  mouseDelta=Prs.curPos-lastPos;
  lastPos=Prs.curPos;
  tS=Simple.Statetime();
  Serial.println(1);
  Serial.println(Prs.curPos);
  Serial.println(mouseDelta);
  Serial.println(tS);
  sB=lookForSerial();
  Serial.println(millis()-beginTime);
  Serial.println(tCount);
  Serial.println(targPos);
  Serial.println(tRangeE);
  Serial.println(clickLBool);
  Serial.println(clickRBool);
  Serial.println(lFreq[lRand]);
  Serial.println(lFreq[rRand]);
  if(Simple.Timeout(3000)) Simple.Set(S2_H,S2_B);
  if(sB==50) Simple.Set(S2_H,S2_B);
}

State S2_H(){
  digitalWrite(rPin, HIGH);
  digitalWrite(gPin, LOW);
  digitalWrite(bPin, HIGH);
  lastPos=Prs.curPos;
  Prs.curPos=0;
  lastKnownState=50;
  clickDeltaL=0;
  clickDeltaR=0;
}

State S2_B(){
    mouseDelta=Prs.curPos-lastPos;
    lastPos=Prs.curPos;
    clickDeltaL=clickDeltaL+abs(mouseDelta);
    clickDeltaR=clickDeltaR+abs(mouseDelta);
   
   if (clickDeltaL >= clickPosL){
    digitalWrite(clickPinL,HIGH);
    delayMicroseconds(clickTime);
    digitalWrite(clickPinL,LOW);
    clickPosL=getNextClickTarget(Prs.curPos, targPos, tRange, lFreq[lRand], lFreq[rRand]);
    clickDeltaL=0;
    clickLBool=1;
   }
   else if (clickDeltaL < clickPosL) {
     clickLBool=0;
   }
   
   if (clickDeltaR >= clickPosR){
     digitalWrite(clickPinR,HIGH);
     delayMicroseconds(clickTime);
     digitalWrite(clickPinR,LOW);
     clickPosR=getNextClickTarget(Prs.curPos, targPos, tRange, lFreq[rRand], lFreq[lRand]);
     clickDeltaR=0;
     clickRBool=1;
   }
   else if (clickDeltaR < clickPosR) {
     clickRBool=0;
   }
  
  tS=Simple.Statetime();
  Serial.println(2);
  Serial.println(Prs.curPos);
  Serial.println(mouseDelta);
  Serial.println(tS);
  sB=lookForSerial();
  Serial.println(millis()-beginTime);
  Serial.println(tCount);
  Serial.println(targPos);
  Serial.println(tRangeE);
  Serial.println(clickLBool);
  Serial.println(clickRBool);
  Serial.println(lFreq[lRand]);
  Serial.println(lFreq[rRand]);
  if(Simple.Timeout(60000)) Simple.Set(S3_H,S3_B);
  // if(sB==49) Simple.Set(S1_H,S1_B);
  if(sB==51) Simple.Set(S3_H,S3_B);
  if(sB==52) Simple.Set(S4_H,S4_B);
}

State S3_H(){
  digitalWrite(rPin, LOW);
  digitalWrite(gPin, HIGH);
  digitalWrite(bPin, HIGH);
  lastPos=Prs.curPos;
  Prs.curPos=0;
  lastKnownState=51;
  myservo.write(restPos);
  tCount=tCount+1;
  targPos=random(lowPos, highPos);
  lRand=int(random(0,2));
  rRand=1-lRand;
  lastPos=Prs.curPos;  
}

State S3_B(){
  mouseDelta=Prs.curPos-lastPos;
  lastPos=Prs.curPos;
  tS=Simple.Statetime();
  Serial.println(3);
  Serial.println(Prs.curPos);
  Serial.println(mouseDelta);
  Serial.println(tS);
  sB=lookForSerial();
  Serial.println(millis()-beginTime);
  Serial.println(tCount);
  Serial.println(targPos);
  Serial.println(tRangeE);
  Serial.println(clickLBool);
  Serial.println(clickRBool);
  Serial.println(lFreq[lRand]);
  Serial.println(lFreq[rRand]);
  // if(Simple.Timeout(10000)) Simple.Set(S2_H,S2_B);
  if(sB==49) Simple.Set(S1_H,S1_B);
  if(sB==50) Simple.Set(S2_H,S2_B);
}

State S4_H(){
  digitalWrite(rPin, HIGH);
  digitalWrite(gPin, HIGH);
  digitalWrite(bPin, LOW);
  lastPos=Prs.curPos;
  Prs.curPos=0;
  lastKnownState=52; 
  myservo.write(rewardPos);
  digitalWrite(solenoidPin, HIGH);
}

State S4_B(){
  mouseDelta=Prs.curPos-lastPos;
  lastPos=Prs.curPos;
  tS=Simple.Statetime();
  Serial.println(4);
  Serial.println(Prs.curPos);
  Serial.println(mouseDelta);
  Serial.println(tS);
  sB=lookForSerial();
  Serial.println(millis()-beginTime);
  Serial.println(tCount);
  Serial.println(targPos);
  Serial.println(tRangeE);
  Serial.println(clickLBool);
  Serial.println(clickRBool);
  Serial.println(lFreq[lRand]);
  Serial.println(lFreq[rRand]);
  if (tS>solenoidTime){
    digitalWrite(solenoidPin,LOW);
  }
  if(Simple.Timeout(rewardTime))  Simple.Set(S3_H,S3_B);
//  if(sB==49) Simple.Set(S1_H,S1_B);
//  if(sB==50)  myservo.write(restPos); Simple.Set(S2_H,S2_B);
//  if(sB==51) myservo.write(restPos); Simple.Set(S3_H,S3_B);
}

State S5_H(){
  digitalWrite(rPin, HIGH);
  digitalWrite(gPin, LOW);
  digitalWrite(bPin, LOW);
  lastPos=Prs.curPos;
  Prs.curPos=0;
  lastKnownState=53;
  Serial.flush(); 
}

State S5_B(){
  tS=Simple.Statetime();
  sB=lookForSerial();
  if(Simple.Timeout(1500))  Simple.Set(S3_H,S3_B);
  if(sB==49) Simple.Set(S1_H,S1_B);
  if(sB==50) Simple.Set(S2_H,S2_B);
  if(sB==51) Simple.Set(S3_H,S3_B);
}


// ---------- Helper Functions

//void sin_texture(int pos, int freq)
//{
//  if (sin(2*pi*pos*freq)>0){
//    digitalWrite(texturePin, HIGH);
//    //digitalWrite(texturePinG, HIGH);
//  } 
//  else if (sin(2*pi*pos*freq)<=0){
//    digitalWrite(texturePin, LOW);
//    //digitalWrite(texturePinG, LOW);
//  }          
//}

//void burriedSin_texture(int pos, int targetPos, int targetRange, int lowFreq, int highFreq)
//{
//  if (invertRun==0){
//    if (pos < targetPos | pos > targetPos+targetRange){ 
//      sin_texture(pos, lowFreq);
//    }
//    else if (pos >= targetPos | pos <= targetPos+targetRange){
//      sin_texture(pos, highFreq);
//    }
//  }
//  else
//    if (pos > targetPos | pos < (targetPos-targetRange)){ 
//      sin_texture(pos, lowFreq);
//    }
//    else if (pos <= targetPos | pos >= (targetPos-targetRange)){
//      sin_texture(pos, highFreq);
//    }  
//}

int lookForSerial(){
  int saBit;
  if(Serial.available()>0){
      saBit=Serial.read();
      lastKnownState=saBit;
  }
  else if(Serial.available()<=0){
    saBit=lastKnownState;
  }
  return saBit;
}

long getNextClickTarget(long pos, long targetPos, long targetRange, long mean1, long mean2){
  long nextClickPos;
    if (pos < targetPos | pos > targetPos+targetRange){ 
      nextClickPos=long(-log(random(1,101)*0.01)*mean1);
      //nextClickPos=100000;
    }
    else if (pos >= targetPos | pos <= targetPos+targetRange){
      nextClickPos=int(-log(random(1,101)*0.01)*mean2);
    }
      return nextClickPos;
}

    
    



