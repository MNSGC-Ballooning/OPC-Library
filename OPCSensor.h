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

The HPM runs the read data function with the log update function, and can
record new data every 1 seconds.
  
*/


#ifndef OPCSensor_h
#define OPCSensor_h

#include <arduino.h>
#include <SPI.h>
#include <i2c_t3.h>
#include <Stream.h>
#define R1_SPEED 300000
#define N3_SPEED 300000

class OPC																//Parent OPC class
{
	protected:
	bool goodLog;														//Describe a state of successful or unsuccessful data intakes
	int badLog;															//Number of bad hits in a row
	int nTot;															//Number of good hits, culminative
	Stream *s;															//Declares data IO stream
	unsigned long goodLogAge;											//Age of the last good set of data
	unsigned long resetTime;											//Age the last good log must reach to trigger a reset
	uint16_t bytes2int(byte LSB, byte MSB);								//Convert given bytes to integers
	
	public:
	OPC();
	OPC(Stream* ser);													//Parent Constructor
	int getTot();														//Parent quality checks
	bool getLogQuality();												//get the quality of the log
	void initOPC();														//Initialization
	String CSVHeader();													//Placeholders
	String logUpdate();
	String logReadout(String name);													
	bool readData();
	void powerOn();
	void powerOff();
	void setReset(unsigned long resetTimer);							//Manually set the bad log reset timer
};



class Plantower: public OPC
{                              
	private:
	unsigned int logRate;												//System log rate
	void command(uint8_t CMD, uint8_t MODE);							//Command base
	
	public:
	struct PMS5003data {												//Struct that holds Plantower data
		uint16_t framelen;
		uint16_t pm10_standard, pm25_standard, pm100_standard;
		uint16_t pm10_env, pm25_env, pm100_env;
		uint16_t particles_03um, particles_05um, particles_10um, particles_25um, particles_50um, particles_100um;
		uint16_t unused;
		uint16_t checksum;
	} PMSdata;
	
	Plantower(Stream* ser, unsigned int logRate);						//Plantower constructor
	void powerOn();
	void powerOff();
	void passiveMode();
	void activeMode();
	void initOPC();
	String CSVHeader();													//Overrides of OPC data functions
	String logUpdate();
	String logReadout(String name);
	bool readData();
};



class SPS: public OPC
{
	private:
	bool altCleaned = false;											//The boolean for altitude based fan clean operation
	bool iicSystem = false;												//Indication of i2c or serial system 
	i2c_t3 *SPSWire;													//Local wire bus
	i2c_pins SPSpins;													//Local wire pins
	uint8_t	CalcCrc(uint8_t data[2]);									//SPS wire checksum calculation
	bool dataReady();													//data indicator
	
	
	public:
	struct SPS30data {													//struct for SPS30 data
		float mas[4];
		float nums[5];
		float aver;		
	}SPSdata;

	SPS(i2c_t3 wireBus, i2c_pins pins);									//I2C Constructor
	SPS(Stream* ser);													//Serial Constructor
	void powerOn();														//System commands for SPS
	void powerOff();
	void clean();
	void initOPC();														//Overrides of OPC data functions and initialization
	String CSVHeader();													//Returns a CSV header for log update
	String logUpdate();													//Returns the CSV string of SPS data
	String logReadout(String name);										//Log update, but with a nice serial print
	bool readData();													//data reader- generally controlled internally
};




class R1: public OPC {													//The R1 runs on SPI Communication
	private:
	uint8_t CS;															//Slave Select pin for specification. The code will only run on the default SPI pins.
	uint16_t data[25];													//Data arrays
	unsigned int CalcCRC(unsigned char data[], unsigned char nbrOfBytes);//Checksum calculator
	
	struct R1data{														//R1 data struct
		uint16_t bins[16];
		uint8_t bin1time, bin2time, bin3time, bin4time;
		float sampleFlowRate; 
		uint16_t temp, humid;
		float samplePeriod;
		uint8_t rejectCountGlitch, rejectCountLong;
		float pm1, pm2_5, pm10;
		unsigned int checksum;
	} localData;
	
	public:
	R1(uint8_t slave);													//Alphasense constructor
	void powerOn();														//Power on will activate the fan, laser, and data communication
	void powerOff();													//Power off will deactivate these same things
	void initOPC();														//Initializes the OPC
	String CSVHeader();													//Overrrides the OPC data functions
	String logUpdate();
	String logReadout(String name);
	bool readData();												
};



class HPM: public OPC{
	private:
	bool autoSend;														//Auto send data state
	bool command(byte cmd, byte chk);									//Command base
	
	public:	
	struct HPMdata{
		uint16_t PM1_0, PM2_5, PM4_0, PM10_0, checksum, checksumR;		//Data structure
	}localData;
	
	HPM(Stream* ser);												
	void powerOn();														//Power on will start the measurement system
	void powerOff();													//Power off will stop measurements
	void autoSendOn();													//Will automatically send data
	void autoSendOff();													//Will wait for data requests (recommended)
	void initOPC();														//Initialize the system
	String CSVHeader();													//Header in CSV format
	String logUpdate();													//Update data in CSV string
	bool readData();													//Read incoming data
};

class N3: public OPC {													//The R1 runs on SPI Communication
	private:
	uint8_t CS;															//Slave Select pin for specification. The code will only run on the default SPI pins.
	bool initCommand(byte command);
	unsigned int CalcCRC(unsigned char data[], unsigned char nbrOfBytes);//Checksum calculator
	
	public:
	struct N3data{														//N3 Public data struct
		uint16_t bins[24];
		uint8_t bin1time, bin2time, bin3time, bin4time;
		uint16_t samplePeriod, sampleFlowRate, temp, humid;
		float pm1, pm2_5, pm10;
		uint16_t rejectCountGlitch, rejectCountLong, rejectCountRatio, rejectCountRange, fanRevCount, laserStatus, checkSum;
	} localData;
	
	N3(uint8_t slave);													//Alphasense constructor
	void laserOn();														//Laser on command
	void fanOn();														//Fan on command
	void laserOff();													//Laser off command
	void fanOff();														//fan off command
	void powerOn();														//Power on will activate the fan, laser, and data communication
	void powerOnPump();													//Power on for use with an external pump
	void powerOff();													//Power off will deactivate these same things
	void initOPC(char t);												//Initializes the OPC
	String CSVHeader();													//Overrrides the OPC data functions
	String logUpdate();	
	String logReadout(String name);												
	bool readData();												
};

#endif
