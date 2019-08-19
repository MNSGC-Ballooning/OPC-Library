#include <OPCSensor.h>

#define PMS_SERIAL Serial1
//#define SPS_SERIAL Serial5

#define logRate 4000

Plantower PlanA(&PMS_SERIAL, logRate);
//SPS SpsA(&SPS_SERIAL);
//R1 r1A(15);

unsigned long Timer[3] = {1,1500,4000};
unsigned long prevTime[3] = {0};
//float pullPlan[6];
//float pullSPS[10];
//float pullr1[16];

void setup() {
Serial.begin(115200);
PMS_SERIAL.begin(9600);
//SPS_SERIAL.begin(115200);
//while (!Serial5) Serial.println("Waiting...");
PlanA.initOPC();
//SpsA.initOPC();
//r1A.initOPC();
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

//  PlanA.getData(pullPlan,6);
//  SpsA.getData(pullSPS,10);
//  r1A.getData(pullr1,16);
  
  Serial.println("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
//  Serial.println("SPS: " + SpsA.logUpdate());
    Serial.println("Plan: " + PlanA.logUpdate());
//  Serial.println("R1: " + r1A.logUpdate());
  Serial.println("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
}

}
