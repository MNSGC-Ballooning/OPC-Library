#include <OPCSensor.h>                                        //Include the library!

//#define PMS_SERIAL Serial1                                    //Macros to define the serials clearly
#define SPS_SERIAL Serial
//#define HPM_SERIAL Serial4

//Plantower PlanA(&PMS_SERIAL, 7000);                           //Construction of the OPC objects
SPS SpsA(&SPS_SERIAL);
//R1 r1A(15);
//HPM hpmA(&HPM_SERIAL);

unsigned long Timer[3] = {1,1500,7000};                       //Timers to regulate loops speed
unsigned long prevTime[3] = {0};

//float pullPlan[6];                                            //Data arrays to test the get data functions
//float pullSPS[10];
//float pullr1[16];
//float pullHPM[4];

void setup() {
//Serial.begin(115200);                                         //Begin the serial lines for the computer and the particle counters
//PMS_SERIAL.begin(9600);
SPS_SERIAL.begin(115200);
//HPM_SERIAL.begin(9600);

//PlanA.initOPC();                                              //Initialize OPC objects
SpsA.initOPC();
//r1A.initOPC();
//hpmA.initOPC();
Serial.println("System initialized!");
}

void loop() {
if (millis()-prevTime[0]>=Timer[0]){
  prevTime[0] = millis();

//  PlanA.readData();                                           //Read Plantower data (must happen very quickly)
}

if (millis()-prevTime[1]>=Timer[1]){
  prevTime[1] = millis();
  
}

if (millis()-prevTime[2]>=Timer[2]){
  prevTime[2] = millis();

  Serial.println();
  Serial.println("SPS: " + SpsA.logUpdate());                //Log the updates in CSV format
//  Serial.println("Plan: " + PlanA.logUpdate());
//  Serial.println("R1: " + r1A.logUpdate());
//  Serial.println("HPM: " + hpmA.logUpdate());
  Serial.println();

//  PlanA.getData(pullPlan,6);                                 //Pull data and put them into the arrays
//  SpsA.getData(pullSPS,10);
//  r1A.getData(pullr1,16);
//  hpmA.getData(pullHPM,4);
  
}

}
