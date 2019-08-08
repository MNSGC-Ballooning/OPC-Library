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
can record new data every 1 seconds.

The Alphasense R1 runs the read data function with the log update function,
and can record new data every 1 seconds. The R1 runs on SPI.
 */


#ifndef OPCSensor_h
#define OPCSensor_h

#include <arduino.h>
#include <SPI.h>
#include <Stream.h>

class OPC																//Parent OPC class
{
	protected:
	bool goodLog;														//Describe a state of successful or unsuccessful data intakes
	int badLog;															//Number of bad hits in a row
	int nTot;															//Number of good hits, culminative
	Stream *s;															//Declares data IO stream
	unsigned long goodLogAge;											//Age of the last good set of data
	unsigned long resetTime;											//Age the last good log must reach to trigger a reset
	
	public:
	OPC();
	OPC(Stream* ser);													//Parent Constructor
	int getTot();														//Parent quality checks
	bool getLogQuality();												//get the quality of the log
	void initOPC();														//Initialization
	String CSVHeader();													//Placeholders
	String logUpdate();													
	bool readData();
	void setReset(unsigned long resetTimer);							//Manually set the bad log reset timer
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
	unsigned int logRate;
	
	public:
	Plantower(Stream* ser, unsigned int planLog);						//Plantower constructor
	String CSVHeader();													//Overrides of OPC data functions
	String logUpdate();
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
	String CSVHeader();
	String logUpdate();
	bool readData();
};

class R1: public OPC {													//The R1 runs on SPI Communication
	private:
	uint8_t SSpin;														//Slave Select pin for specification. The code will only run on the default SPI pins.
	byte test[2];														//Data arrays
	byte raw[64];
	uint16_t com[16];
	
	public:
	R1(uint8_t slave);													//Alphasense constructor
	void initOPC();														//Initializes the OPC
	void powerOn();														//Power on will activate the fan, laser, and data communication
	void powerOff();													//Power off will deactivate these same things
	uint16_t bytes2int(byte LSB, byte MSB);								//Convert given bytes to integers
	String CSVHeader();													//Overrrides the OPC data functions
	bool readData();													
	String logUpdate();
};

#endif
