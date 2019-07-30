//Optical Particle Counter Library

//University of Minnesota - Candler MURI
//Written July 2019

/*This is the header file for the OPC library.
This will run any optical particle counters used for MURI.
All particle counters will need to be run in loops of different speeds.
Serial begin must be called separately.

The PMS 5003 runs the read data function as fast as possible, and can
record new data every 2.3 seconds.
 
The SPS 30 runs the read data function with the record data function, and
can record new data every 1 seconds. */


#ifndef OPCSensor_h
#define OPCSensor_h

#include <arduino.h>
#include <Stream.h>

class OPC																//Parent OPC class
{
	protected:
	bool goodLog;														//Values for log tracking
	int badLog;
	int nTot;
	Stream *s;
	
	public:
	OPC(Stream* ser);													//Parent Constructor
	int getTot();														//Parent quality checks
	bool getLogQuality();
	void initOPC();														//Initializations for three key functions
	String logUpdate();											
	bool readData();
};

class Plantower: public OPC
{                              
	private:
	struct PMS5003data {												//Struct that holds Plantower data
		uint16_t framelen;
		uint16_t pm10_standard, pm25_standard, pm100_standard;
		uint16_t pm10_env, pm25_env, pm100_env;
		uint16_t particles_03um, particles_05um, particles_10um, particles_25um, particles_50um, particles_100um;
		uint16_t unused;
		uint16_t checksum;
	} PMSdata;
	unsigned long goodLogAge;
	unsigned int logRate;
	
	public:
	Plantower(Stream* ser, unsigned int planLog);						//Plantower constructor
	String logUpdate();													//Overrides of OPC data functions
	bool readData();
};

class SPS: public OPC
{
	private:
	byte buffers[40] = {0}, systemInfo [5] = {0}, MassC[16] = {0};      //Byte variables for collection and organization of data from the SPS30.
	byte NumC[20] = {0}, AvgS[4] = {0}, data = 0, checksum = 0, SPSChecksum = 0;
	
	union mass                                                          //Defines the union for mass concentration
	{
		byte MCA[16];
		float MCF[4];
	}m;

	union num                                                           //Defines the union for number concentration
	{
		byte NCA[20];
		float NCF[5];
	}n;

	union avg                                                           //Defines the union for average sizes
	{
		byte ASA[4];
		float ASF;
	}a;

	public:
	SPS(Stream* ser);													//Parent Constructor
	void powerOn();														//Special commands for SPS
	void powerOff();
	void clean();
	void initOPC();														//Overrides of OPC data functions and initialization
	String logUpdate();
	bool readData();
};

#endif