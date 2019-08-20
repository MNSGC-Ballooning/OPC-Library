#include <OPCSensor.h>                                        //Include the library!

#define PMS_SERIAL Serial2
//#define SPS_SERIAL Serial5
#define HPM_SERIAL Serial1

#define logRate 4000

Plantower PlanA(&PMS_SERIAL, logRate);
//SPS SpsA(&SPS_SERIAL);
//R1 r1A(15);
HPM hpmA(&HPM_SERIAL);

unsigned long Timer[3] = {1,1500,4000};
unsigned long prevTime[3] = {0};
//float pullPlan[6];
//float pullSPS[10];
//float pullr1[16];
float pullHpm[4];

void setup() {
Serial.begin(115200);
while (!Serial) ;
delay (100);
Serial.println("Serial active!");
PMS_SERIAL.begin(9600);
HPM_SERIAL.begin(9600);
while (!HPM_SERIAL) Serial.println("Waiting...");
Serial.println("HPM connected!");
//SPS_SERIAL.begin(115200);
delay(1000);
PlanA.initOPC();
//SpsA.initOPC();
//r1A.initOPC();
  hpmA.initOPC();
delay(1000);
Serial.println("System initialized!");
}

void loop() {

if (millis()-prevTime[0]>=Timer[0]){
  prevTime[0] = millis();

  PlanA.readData();
  }



if (millis()-prevTime[1]>=Timer[1]){
  prevTime[1] = millis();
  
}

if (millis()-prevTime[2]>=Timer[2]){
  prevTime[2] = millis();

  Serial.println();
//  Serial.println("SPS: " + SpsA.logUpdate());                //Log the updates in CSV format
  Serial.println("Plan: " + PlanA.logUpdate());
//  Serial.println("R1: " + r1A.logUpdate());
    Serial.println("HPM: " + hpmA.logUpdate());
  Serial.println();

//  PlanA.getData(pullPlan,6);                                 //Pull data and put them into the arrays
//  SpsA.getData(pullSPS,10);
//  r1A.getData(pullr1,16);
  hpmA.getData(pullHpm,4);
  Serial.println(String(pullHpm[2]));
  Serial.println();
  
}

}
