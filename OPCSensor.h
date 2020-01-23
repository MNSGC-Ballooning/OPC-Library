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
#include <Stream.h>
#define R1_SPEED 500000
#define N3_SPEED 500000

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
	bool readData();
	void getData(float dataPtr[], unsigned int arrayFill);
	void getData(float dataPtr[], unsigned int arrayFill, unsigned int arrayStart);
	void powerOn();
	void powerOff();
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
	unsigned int logRate;												//System log rate
	
	void command(uint8_t CMD, uint8_t MODE);							//Command base
	
	public:
	Plantower(Stream* ser, unsigned int logRate);						//Plantower constructor
	void powerOn();
	void powerOff();
	void passiveMode();
	void activeMode();
	void initOPC();
	String CSVHeader();													//Overrides of OPC data functions
	String logUpdate();
	bool readData();
	void getData(float dataPtr[], unsigned int arrayFill);				//Get data will pass the data into an array via a pointer
	void getData(float dataPtr[], unsigned int arrayFill, unsigned int arrayStart);
};



class SPS: public OPC
{
	private:
	byte buffers[40] = {0}, systemInfo [5] = {0}, MassC[16] = {0};      //Byte variables for collection and organization of data from the SPS30.
	byte NumC[20] = {0}, AvgS[4] = {0}, data = 0, checksum = 0, SPSChecksum = 0;
	bool altCleaned = false;
	
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
	void getData(float dataPtr[], unsigned int arrayFill);				//Get data will pass the data into an array via a pointer
	void getData(float dataPtr[], unsigned int arrayFill, unsigned int arrayStart);
	bool altClean(float altitude, float cleanAlt);						//Set a function to flush the sensor at some height to clear the particles
};



class R1: public OPC {													//The R1 runs on SPI Communication
	private:
	uint8_t CS;															//Slave Select pin for specification. The code will only run on the default SPI pins.
	uint16_t data[25];													//Data arrays
	unsigned int CalcCRC(unsigned char data[], unsigned char nbrOfBytes);//Checksum calculator
	
	union byteToFloat                                                   //Defines the union for sample flow rate, sampling period, and PM bins
	{
		byte byteIn[4];
		float floatOut;
	}sfr,sp,a,b,c;
	
	public:
	R1(uint8_t slave);													//Alphasense constructor
	void powerOn();														//Power on will activate the fan, laser, and data communication
//	void powerOnPump();													//Power on for use with an external pump
	void powerOff();													//Power off will deactivate these same things
	void initOPC();														//Initializes the OPC
	String CSVHeader();													//Overrrides the OPC data functions
	String logUpdate();
	bool readData();
//	void getData(float dataPtr[], unsigned int arrayFill);				//Get data will pass the data into an array via a pointer
//	void getData(float dataPtr[], unsigned int arrayFill, unsigned int arrayStart);													
};



class HPM: public OPC{
	private:
	struct HPMdata{
		uint16_t PM1_0, PM2_5, PM4_0, PM10_0, checksum, checksumR;		//Data structure
	}localData;
	bool autoSend;														//Auto send data state
	
	bool command(byte cmd, byte chk);									//Command base
	
	public:	
	HPM(Stream* ser);												
	void powerOn();														//Power on will start the measurement system
	void powerOff();													//Power off will stop measurements
	void autoSendOn();													//Will automatically send data
	void autoSendOff();													//Will wait for data requests (recommended)
	void initOPC();														//Initialize the system
	String CSVHeader();													//Header in CSV format
	String logUpdate();													//Update data in CSV string
	bool readData();													//Read incoming data
	void getData(float dataPtr[], unsigned int arrayFill);				//Get data will pass the data into an array via a pointer
	void getData(float dataPtr[], unsigned int arrayFill, unsigned int arrayStart);	
};

class N3: public OPC {													//The R1 runs on SPI Communication
	private:
	uint8_t CS;															//Slave Select pin for specification. The code will only run on the default SPI pins.
	bool initCommand(byte command);
	unsigned int CalcCRC(unsigned char data[], unsigned char nbrOfBytes);//Checksum calculator
	
	struct N3data{
		uint16_t bins[24];
		uint8_t bin1time, bin2time, bin3time, bin4time;
		uint16_t samplePeriod, sampleFlowRate, temp, humid;
		float pm1, pm2_5, pm10;
		uint16_t rejectCountGlitch, rejectCountLong, rejectCountRatio, rejectCountRange, fanRevCount, laserStatus, checkSum;
	} localData;
	
	public:
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
	bool readData();
//	void getData(float dataPtr[], unsigned int arrayFill);				//Get data will pass the data into an array via a pointer
//	void getData(float dataPtr[], unsigned int arrayFill, unsigned int arrayStart);													
};

#endif
