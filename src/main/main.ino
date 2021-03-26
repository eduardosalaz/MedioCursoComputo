#include <CmdParser.hpp>

const float motorAngle = 1.8;
const float stepSize = 1;
unsigned long rpm = 100; //Default Speed - min = 100 max = 400

const float Cspu = 476;
const float Mspu = 555;
const float Yspu = 500;
const float Kspu = 530;

bool clean = true, pump = false;
unsigned long Red, Green, Blue, Volume;
unsigned long Cyan, Magenta, Yellow, Key;
float Csteps, Msteps, Ysteps, Ksteps;

#define enPin 8
#define cDirPin 5
#define cStepPin 2
#define mDirPin 6
#define mStepPin 3
#define yDirPin 7
#define yStepPin 4
#define kDirPin 13
#define kStepPin 12
#define buzPin A3

void setup() {
  pinMode(enPin, OUTPUT);
  pinMode(cDirPin, OUTPUT);
  pinMode(cStepPin, OUTPUT);
  pinMode(mDirPin, OUTPUT);
  pinMode(mStepPin, OUTPUT);
  pinMode(yDirPin, OUTPUT);
  pinMode(yStepPin, OUTPUT);
  pinMode(kDirPin, OUTPUT);
  pinMode(kStepPin, OUTPUT);
  
  pinMode(buzPin, OUTPUT);
  
  digitalWrite(enPin, HIGH);
  
  Serial.begin(38400);
  Serial.println("Send command to operate......");  // PUMP 10 20 30 100 // SPEED 500 // CLEAN
}

void loop() {
  cmdRead(Red, Green, Blue, Volume);
}

void cmdRead(unsigned long &R, unsigned long &G, unsigned long &B, unsigned long &vol) {
  CmdParser cmdParser;
  CmdBuffer<64> myBuffer;
  if (myBuffer.readFromSerial(&Serial)) {
    if (cmdParser.parseCmd(&myBuffer) != CMDPARSER_ERROR) {
      Serial.println(" ");
      Serial.print("Parsing Command : ");
      Serial.println(myBuffer.getStringFromBuffer());
      if (cmdParser.getParamCount() == 5) {
        if(cmdParser.equalCmdParam(0, "PUMP")){
          Serial.println(" ");Serial.print("Command : ");
          Serial.println(cmdParser.getCommand());
          R = atoi(cmdParser.getCmdParam(1));
          G = atoi(cmdParser.getCmdParam(2));
          B = atoi(cmdParser.getCmdParam(3));
          vol = atoi(cmdParser.getCmdParam(4));
          if(!clean && pump) {
            Serial.print("Base Volume : ");Serial.print(Volume);Serial.println("ml");
            Serial.println(" ");
            Serial.print("RGB : ");Serial.print(Red);Serial.print(" ");Serial.print(Green);Serial.print(" ");Serial.println(Blue);
            getCMYK(Cyan, Magenta, Yellow, Key);
            getSteps(Csteps, Msteps, Ysteps, Ksteps);
            makeColor();
            Serial.println(" ");
            Serial.println("Send command to operate......");
          }
          else {
            Serial.println("Load the pumps then try again...!");
            Serial.println(" ");
            Serial.println("Send command to operate......");
          }
         }
        else {
          Serial.println("Invalid Command...!");
          Serial.println(" ");
        }
      }
      
      else if (cmdParser.getParamCount() == 2) {
        if(cmdParser.equalCmdParam(0, "SPEED")) {
          rpm = atoi(cmdParser.getCmdParam(1));
          if (rpm > 400) {
            rpm = 400;
          }
          else if (rpm < 100) {
            rpm = 100;
          }
          Serial.print("Flow Rate set to ");Serial.print(rpm);Serial.println(" RPM");
          Serial.println(" ");
        }
        else {
          Serial.println("Invalid Command");
          Serial.println(" ");
        }
      }
      else if (cmdParser.getParamCount() == 1) {
        if(cmdParser.equalCmdParam(0, "LOAD")) {
          if(clean) {
            loadPump();
            pump = true;
            clean = false;
            Serial.println(" ");
          }
          else {
            Serial.println("Already Loaded...!");
            Serial.println(" ");
          }
        }
        else if(cmdParser.equalCmdParam(0, "LOAD_NOT_REQUIRED")) {
          Serial.println("Please ensure that fuild is loaded before continuing.....!");
          clean = false;
          pump = true;
        }
        else if(cmdParser.equalCmdParam(0, "CLEAN")) {
          if(!clean) {
            cleanPump();
            clean = true;
            pump = false;
            Serial.println(" ");
          }
          else {
            Serial.println("Pumps are clean...!");
            Serial.println(" ");
          }
        }
        else {
          Serial.println("Invalid Command");
          Serial.println(" ");
        }
      }
      else {
        Serial.println("Invalid Command");
        Serial.println(" ");
      }
    }
  }
}

void getCMYK(unsigned long &Cyan, unsigned long &Magenta, unsigned long &Yellow, unsigned long &Key) {
  Serial.println("Converting RGB to CMYK (0-255 => 0-100)");
  float R = (float)Red/255.0;
  float G = (float)Green/255.0;
  float B = (float)Blue/255.0;
  float K = (1 - max(max(R, G),B));
  float C = (1 - R - K)/(1 - K);
  float M = (1 - G - K)/(1 - K);
  float Y = (1 - B - K)/(1 - K);
  Cyan = (float)C*100;
  Magenta = (float)M*100;
  Yellow = (float)Y*100;
  Key = (float)K*100;
  Serial.print("CMYK : ");Serial.print(Cyan);Serial.print(" ");Serial.print(Magenta);Serial.print(" ");Serial.print(Yellow);Serial.print(" ");Serial.println(Key);
}

void getSteps(float &Csteps, float &Msteps, float &Ysteps, float &Ksteps) {
  float Cvol = ((float)(Cyan*Volume))/100;
  float Mvol = ((float)(Magenta*Volume))/100;
  float Yvol = ((float)(Yellow*Volume))/100;
  float Kvol = ((float)(Key*Volume))/100;

  Csteps = Cvol * Cspu;
  Msteps = Mvol * Mspu;
  Ysteps = Yvol * Yspu;
  Ksteps = Kvol * Kspu;

  Serial.println(" ");
  Serial.print("Color Volume: ");Serial.print(Cvol);Serial.print(" ");Serial.print(Mvol);Serial.print(" ");Serial.print(Yvol);Serial.print(" ");Serial.println(Kvol);
  Serial.println(" ");
}

void loadPump() {
  digitalWrite(enPin, LOW);
  Serial.print("Loading Fluid into Pumps...");
  cStepperRotate(3000, rpm);
  Serial.print("...");
  mStepperRotate(3000, rpm);
  Serial.print("...");
  yStepperRotate(3000, rpm);
  Serial.print("...");
  kStepperRotate(3000, rpm);
  Serial.println("...Done!");
}

void cleanPump() {
  digitalWrite(enPin, LOW);
  Serial.print("Cleaning Pumps...");
  cStepperRotate(-3000, rpm);
  Serial.print("...");
  mStepperRotate(-3000, rpm);
  Serial.print("...");
  yStepperRotate(-3000, rpm);
  Serial.print("...");
  kStepperRotate(-3000, rpm);
  Serial.println("...Done!");
  digitalWrite(enPin, HIGH);
}

void makeColor() {
  digitalWrite(enPin, LOW);
  if (Csteps != 0) {
    Serial.print("Pumping Cyan with ");
    Serial.print(Csteps);
    Serial.print(" Steps");
    cStepperRotate(Csteps, rpm);
    Serial.println("    .....Done!");
    setBuzz();
  }
  if (Msteps != 0) {
    Serial.print("Pumping Magenta with ");
    Serial.print(Msteps);
    Serial.print(" Steps");
    mStepperRotate(Msteps, rpm);
    Serial.println("    .....Done!");
    setBuzz();
  }
  if (Ysteps != 0) {
    Serial.print("Pumping Yellow with ");
    Serial.print(Ysteps);
    Serial.print(" Steps");
    yStepperRotate(Ysteps, rpm);
    Serial.println("    .....Done!");
    setBuzz();
  }
  if (Ksteps != 0) {
    Serial.print("Pumping Key with ");
    Serial.print(Ksteps);
    Serial.print(" Steps");
    kStepperRotate(Ksteps, rpm);
    Serial.println("    .....Done!");
    setBuzz();
  }
    
  Serial.println(" ");
  Serial.println("All colors pumped!");
  Serial.println(" ");
}

void setBuzz() {
  analogWrite(buzPin, 255);
  delay(100);
  analogWrite(buzPin, 0);
  delay(50);
}

void cStepperRotate(float steps, float rpm) {
  float stepsPerRotation = (360.00 / motorAngle) / stepSize;
  unsigned long stepPeriodmicroSec = ((60.0000 / (rpm * stepsPerRotation)) * 1E6 / 2.0000) - 5;
  
  if (steps > 0) {
    digitalWrite(cDirPin, HIGH);
  }
  else {
    digitalWrite(cDirPin, LOW);
  }
  
  if (steps > 0) {
    float totalSteps = steps;
    for (unsigned long i = 0; i < totalSteps; i++) {
      digitalWrite(cStepPin, HIGH);
      delayMicroseconds(stepPeriodmicroSec);
      digitalWrite(cStepPin, LOW);
      delayMicroseconds(stepPeriodmicroSec);
    }
  }
  else {
    float totalSteps = steps * -1;
    for (unsigned long i = 0; i < totalSteps; i++) {
      digitalWrite(cStepPin, HIGH);
      delayMicroseconds(stepPeriodmicroSec);
      digitalWrite(cStepPin, LOW);
      delayMicroseconds(stepPeriodmicroSec);
    }
  }
}

void mStepperRotate(float steps, float rpm) {
  float stepsPerRotation = (360.00 / motorAngle) / stepSize;
  unsigned long stepPeriodmicroSec = ((60.0000 / (rpm * stepsPerRotation)) * 1E6 / 2.0000) - 5;
  
  if (steps > 0) {
    digitalWrite(mDirPin, HIGH);
  }
  else {
    digitalWrite(mDirPin, LOW);
  }
  
  if (steps > 0) {
    float totalSteps = steps;
    for (unsigned long i = 0; i < totalSteps; i++) {
      digitalWrite(mStepPin, HIGH);
      delayMicroseconds(stepPeriodmicroSec);
      digitalWrite(mStepPin, LOW);
      delayMicroseconds(stepPeriodmicroSec);
    }
  }
  else {
    float totalSteps = steps * -1;
    for (unsigned long i = 0; i < totalSteps; i++) {
      digitalWrite(mStepPin, HIGH);
      delayMicroseconds(stepPeriodmicroSec);
      digitalWrite(mStepPin, LOW);
      delayMicroseconds(stepPeriodmicroSec);
    }
  }
}

void yStepperRotate(float steps, float rpm) {
  float stepsPerRotation = (360.00 / motorAngle) / stepSize;
  unsigned long stepPeriodmicroSec = ((60.0000 / (rpm * stepsPerRotation)) * 1E6 / 2.0000) - 5;
  
  if (steps > 0) {
    digitalWrite(yDirPin, HIGH);
  }
  else {
    digitalWrite(yDirPin, LOW);
  }
  
  if (steps > 0) {
    float totalSteps = steps;
    for (unsigned long i = 0; i < totalSteps; i++) {
      digitalWrite(yStepPin, HIGH);
      delayMicroseconds(stepPeriodmicroSec);
      digitalWrite(yStepPin, LOW);
      delayMicroseconds(stepPeriodmicroSec);
    }
  }
  else {
    float totalSteps = steps * -1;
    for (unsigned long i = 0; i < totalSteps; i++) {
      digitalWrite(yStepPin, HIGH);
      delayMicroseconds(stepPeriodmicroSec);
      digitalWrite(yStepPin, LOW);
      delayMicroseconds(stepPeriodmicroSec);
    }
  }
}

void kStepperRotate(float steps, float rpm) {
  float stepsPerRotation = (360.00 / motorAngle) / stepSize;
  unsigned long stepPeriodmicroSec = ((60.0000 / (rpm * stepsPerRotation)) * 1E6 / 2.0000) - 5;
  
  if (steps > 0) {
    digitalWrite(kDirPin, HIGH);
  }
  else {
    digitalWrite(kDirPin, LOW);
  }
  
  if (steps > 0) {
    float totalSteps = steps;
    for (unsigned long i = 0; i < totalSteps; i++) {
      digitalWrite(kStepPin, HIGH);
      delayMicroseconds(stepPeriodmicroSec);
      digitalWrite(kStepPin, LOW);
      delayMicroseconds(stepPeriodmicroSec);
    }
  }
  else {
    float totalSteps = steps * -1;
    for (unsigned long i = 0; i < totalSteps; i++) {
      digitalWrite(kStepPin, HIGH);
      delayMicroseconds(stepPeriodmicroSec);
      digitalWrite(kStepPin, LOW);
      delayMicroseconds(stepPeriodmicroSec);
    }
  }
}
