#include <OPCSensor.h>                                        //Include the library!
#include <Wire.h>

#define PMS_SERIAL Serial4                                     //Define the serial ports!
#define SPS_SERIAL Serial3
//#define HPM_SERIAL Serial5

#define logRate 4000                                          //Establish a log rate
#define slavePin 15                                           //Establish a slave pin

Plantower PlanA(&PMS_SERIAL, logRate);                        //Construct the objects
SPS SpsA(&SPS_SERIAL);
R1 r1A(slavePin);
//HPM hpmA(&HPM_SERIAL);
//N3 n3A(slavePin);

unsigned long Timer[3] = {1,1500,6000};                       //These are the timers for the loops- not critical to operation
unsigned long prevTime[3] = {0};                              
//float pullPlan[6];                                            //These arrays pull data from the sensors directly- not critical to CSV operation
//float pullSPS[10];
//float pullr1[27];
//float pullHpm[4];

void setup() {
  Serial.begin(115200);                                       //Wait for the serial monitor to be connected before turning the system on- not critical to operation
  while (!Serial) ;
  delay (100);
  Serial.println("Serial active!");
  PMS_SERIAL.begin(9600);                                     //Begin the serial connections!
  SPS_SERIAL.begin(115200);
//  HPM_SERIAL.begin(9600);
//  Wire.begin();
  delay(1000);                                                //Delay to ensure connection, can be much shorter

  PlanA.initOPC();                                            //Initialize objects
  Serial.println("Plantower active!");
  SpsA.initOPC();
  Serial.println("SPS active!");
  r1A.initOPC();
  Serial.println("R1 active!");
//  hpmA.initOPC();
//  Serial.println("Honeywell active!");
//    n3A.initOPC('d');
//    Serial.println("N3 active!");
  delay(2000);                                                //The particle counters will not collect accurate data until 30 seconds after being turned on
  Serial.println("System initialized!");
}

void loop() {

if (millis()-prevTime[0]>=Timer[0]){                          //1ms loop
  prevTime[0] = millis();

  PlanA.readData();                                           //Check for plantower data very quickly!
  }
//
//if (millis()-prevTime[1]>=Timer[1]){                          //1500ms loop
//  prevTime[1] = millis();
//  
//}

if (millis()-prevTime[2]>=Timer[2]){                          //4000ms loop
  prevTime[2] = millis();

  Serial.println("Plan: " + PlanA.logUpdate());
  Serial.println("SPS: " + SpsA.logUpdate());                 //Print the updates in CSV format
  Serial.println("R1: " + r1A.logUpdate());
//  Serial.println("HPM: " + hpmA.logUpdate());
//  Serial.println("N3: " + n3A.logUpdate());

//  PlanA.getData(pullPlan,6);                                  //Pull data and put them into the arrays- not critical to operation
//  SpsA.getData(pullSPS,10);
//  r1A.getData(pullr1,16);
//  hpmA.getData(pullHpm,4);
//  Serial.println();  
}
}
