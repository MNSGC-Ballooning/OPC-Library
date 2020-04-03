//Optical Particle Counter Library

//University of Minnesota - Candler MURI
//Written July 2019

/*This is the definitions file for the OPC library.
This will run any optical particle counters used for MURI.
All particle counters will need to be run in loops of different speeds.
Serial begin must be called separately.

The PMS 5003 runs the read data function as fast as possible, and can
record new data every 2.3 seconds.
 
The SPS 30 runs the read data function with the log update function, and
can record new data every 1 seconds.

The Alphasense R1 runs the read data function with the log update function,
and can record new data every 1 seconds. The R1 runs on SPI.

The HPM runs the read data function with the log update function, and can
record new data every 1 seconds.*/

#include "OPCSensor.h"



//////////OPC//////////



OPC::OPC(){}															//Non-serial constructor

OPC::OPC(Stream* ser){													//Establishes data IO stream
	s = ser;
}

int OPC::getTot(){ return nTot; }										//get the total number of data points

bool OPC::getLogQuality(){ return goodLog; }							//get the log quality

void OPC::initOPC(){													//Initialize the serial and OPC variables
	goodLog = false;													//Describe a state of successful or unsuccessful data intakes
	goodLogAge = 0;														//Age of the last good set of data
	badLog = 0;															//Number of bad hits in a row
	nTot = 1;															//Number of good hits, culminative
	resetTime = 1200000;												//autotrigger forced reset timer
}

String OPC::CSVHeader(){ return ("~"); }								//Placeholders: will always be redefined

String OPC::logUpdate(){				
	String localDataLog = "OPC not specified!";
	return localDataLog;
}

String OPC::logReadout(String name){return "";}

bool OPC::readData(){ return false; }

void OPC::powerOn(){}

void OPC::powerOff(){}

void OPC::setReset(unsigned long resetTimer){ resetTime = resetTimer; } //Manually set the length of the forced reset

uint16_t OPC::bytes2int(byte LSB, byte MSB){							//Two byte conversion to integers
	uint16_t val = ((MSB << 8) | LSB);
	return val;
}



//////////PLANTOWER//////////



void Plantower::command(byte CMD, byte Mode){							//Command system, that allows for base commands to be easily sent
	uint16_t verify = 0x42 + 0x4d + CMD + 0x00 + Mode;					//Checksum calculation
	uint8_t LRCH, LRCL;
	
	LRCL = (verify & 0xff);												//Split checksum into a most significant and least significant byte
	LRCH = (verify >> 8);
	
	s->write(0x42);														//Send data
	s->write(0x4d);
	s->write(CMD);
	s->write((byte)0x00);
	s->write(Mode);
	s->write((byte)LRCH);
	s->write((byte)LRCL);	
}

Plantower::Plantower(Stream* ser, unsigned int planLog) : OPC(ser){ 	//Plantower constructor- contains the log rate and the plantower stream
	logRate = planLog;
}
	
	
void Plantower::powerOn(){												//Power on
	command(0xe4,0x01);
	
	delay(20);
	for (unsigned short i=0; i<8; i++) s->read();						//Clear buffer
}

void Plantower::powerOff(){												//Power off
	command(0xe4,0x00);
	
	delay(20);
	for (unsigned short i=0; i<8; i++) s->read();
}

void Plantower::passiveMode(){											//Passive mode
	command(0xe1,0x00);

	delay(20);
	for (unsigned short i=0; i<8; i++) s->read();
}

void Plantower::activeMode(){											//Active mode
	command(0xe1, 0x01);
	
	delay(20);
	for (unsigned short i=0; i<8; i++) s->read();
}

void Plantower::initOPC(){												//System initalization
	OPC::initOPC();
	
	powerOn();
	delay(100);
	activeMode();
}
	
String Plantower::CSVHeader(){											//Returns a data header in CSV formate
	String header = "hits,lastLog,MC1um,MC2.5um,MC10um,AMC1um,AMC2.5um,AMC10um,";
	header += "NC03um,NC05um,NC10um,NC25um,NC50um,NC100um";
	return header;
}

String Plantower::logUpdate(){
	unsigned int lastLog = millis() - goodLogAge;

	String dataLogLocal = "";											//Log sample number, in flight time																							
    dataLogLocal += String(nTot) + "," + String(lastLog) + ",";  
    
    if (goodLog){                  			    						//If data is in the buffer, log it
		dataLogLocal += String(PMSdata.pm10_standard);
		dataLogLocal += "," + String(PMSdata.pm25_standard);
		dataLogLocal += "," + String(PMSdata.pm100_standard);
		dataLogLocal += "," + String(PMSdata.pm10_env);
		dataLogLocal += "," + String(PMSdata.pm25_env);
		dataLogLocal += "," + String(PMSdata.pm100_env);
		dataLogLocal += "," + String(PMSdata.particles_03um);
		dataLogLocal += "," + String(PMSdata.particles_05um);
		dataLogLocal += "," + String(PMSdata.particles_10um);
		dataLogLocal += "," + String(PMSdata.particles_25um);
		dataLogLocal += "," + String(PMSdata.particles_50um);
		dataLogLocal += "," + String(PMSdata.particles_100um);
		nTot ++;                                                   		//Total samples
		
	} else {
		dataLogLocal += "-,-,-,-,-,-,-,-,-,-,-,-";
		badLog++;                                                       //If there are five consecutive bad logs, the data string will print a warning
		if (badLog >= 5){
			goodLog = false;
		}

		if ((millis()-goodLogAge)>=resetTime){							//System reset if the reset time is tripped
		powerOff();
		delay(20000);
		powerOn();
		goodLogAge = millis();
		}
	}
  return dataLogLocal;
}

String Plantower::logReadout(String name){
	unsigned int lastLog = millis() - goodLogAge;
	
	String dataLogLocal = "";											//Log sample number, in flight time																							
    dataLogLocal += String(nTot) + "," + String(lastLog) + ",";  
    
    if (goodLog){                  			    						//If data is in the buffer, log it
		dataLogLocal += String(PMSdata.pm10_standard);
		dataLogLocal += "," + String(PMSdata.pm25_standard);
		dataLogLocal += "," + String(PMSdata.pm100_standard);
		dataLogLocal += "," + String(PMSdata.pm10_env);
		dataLogLocal += "," + String(PMSdata.pm25_env);
		dataLogLocal += "," + String(PMSdata.pm100_env);
		dataLogLocal += "," + String(PMSdata.particles_03um);
		dataLogLocal += "," + String(PMSdata.particles_05um);
		dataLogLocal += "," + String(PMSdata.particles_10um);
		dataLogLocal += "," + String(PMSdata.particles_25um);
		dataLogLocal += "," + String(PMSdata.particles_50um);
		dataLogLocal += "," + String(PMSdata.particles_100um);
		nTot ++;                                                   		//Total samples
		
		Serial.println();
		Serial.println("=======================");
		Serial.println(("Plantower: " + name));
		Serial.println();
		Serial.print("Successful Data Hits: ");
		Serial.println(String(nTot));
		Serial.print("Last log time: ");
		Serial.println(String(lastLog));
		Serial.println();
		Serial.print(".3 microns and greater: ");
		Serial.println(String(PMSdata.particles_03um));
		Serial.print(".5 microns and greater: ");
		Serial.println(String(PMSdata.particles_05um));
		Serial.print("1 microns and greater: ");
		Serial.println(String(PMSdata.particles_10um));
		Serial.print("2.5 microns and greater: ");
		Serial.println(String(PMSdata.particles_25um));
		Serial.print("5 microns and greater: ");
		Serial.println(String(PMSdata.particles_50um));
		Serial.print("10 microns and greater: ");
		Serial.println(String(PMSdata.particles_100um));
		Serial.println("=======================");
			
	} else {
		dataLogLocal += "-,-,-,-,-,-,-,-,-,-,-,-";
		badLog++;                                                       //If there are five consecutive bad logs, the data string will print a warning
		if (badLog >= 5){
			goodLog = false;
		}
		
		Serial.println();
		Serial.println("=======================");
		Serial.println(("Plantower: " + name));
		Serial.println();
		Serial.print("Successful Data Hits: ");
		Serial.println(String(nTot));
		Serial.print("Last log time: ");
		Serial.println(String(lastLog));
		Serial.println();
		Serial.println("Bad log");
		Serial.println("=======================");

		if ((millis()-goodLogAge)>=resetTime){							//System reset if the reset time is tripped
		powerOff();
		delay(20000);
		powerOn();
		goodLogAge = millis();
		}
	}
	
  return dataLogLocal;
}

bool Plantower::readData(){												//Command that calls bytes from the plantower
  if (! s->available()){
    return false;
  }
  
  if (s->peek() != 0x42){ 												//Read a byte at a time until we get to the special '0x42' start-byte
    s->read();
    return false;
  }
 
  if (s->available() < 32){  											//Now read all 32 bytes
    return false;
  }
    
  uint8_t buffer[32];    
  uint16_t sum = 0;
  s->readBytes(buffer, 32);
 
  for (uint8_t i=0; i<30; i++){  										//Get checksum ready
    sum += buffer[i];
  }

  uint16_t buffer_u16[15];												//Making bins exclusive for each particulate size
  for (uint8_t i=0; i<15; i++){										
    buffer_u16[i] = buffer[2 + i*2 + 1];
    buffer_u16[i] += (buffer[2 + i*2] << 8);
  }
 
  memcpy((void *)&PMSdata, (void *)buffer_u16, 30);						//Put it into a nice struct :)
 
  if (sum != PMSdata.checksum){									    	//if the checksum fails, return false
    goodLog = false;
    return false;
  }

	goodLog = true;														//goodLog is set to true of every good log
	goodLogAge = millis();
	badLog = 0;															//The badLog counter and the goodLogAge are both reset.
  return true;
}



//////////SPS//////////




SPS::SPS(i2c_t3 wireBus, i2c_pins pins) : OPC()							//I2C constructor for SPS object
{
	SPSWire = &wireBus;
	SPSpins = pins;
	iicSystem = true;
}

SPS::SPS(Stream* ser) : OPC(ser) {}										//Initialize stream using base OPC constructor

void SPS::powerOn()                                			            //SPS Power on command. This sends and recieves the power on frame
{
	if (!iicSystem){													//If the system is running serial...
		s->write(0x7E);                                                 //Send startup frame
		s->write((byte)0x00);
		s->write((byte)0x00);                                           //This is the actual command
		s->write(0x02);
		s->write(0x01);
		s->write(0x03);
		s->write(0xF9);
		s->write(0x7E);

		delay (100);
		for (unsigned int q = 0; q<7; q++) s->read();                   //Read the response bytes
		
	} else {															//If the system is running I2C...
		byte data[2] = {0x03,0x00};										//Data to write to set proper mode
		SPSWire->beginTransmission(SPS_ADDRESS);
		SPSWire->write(0x0010);											//Set Pointer
		SPSWire->write(data[0]);										//Write power on Data
		SPSWire->write(data[1]);
		SPSWire->write(CalcCrc(data));									//Every two bytes requires a checksum
		SPSWire->endTransmission();
	}
}

void SPS::powerOff()                              		                //SPS Power off command. This sends and recieves the power off frame
{
	if(!iicSystem){														//If the system is running serial...	
		s->write(0x7E);                                                 //Send shutdown frame
		s->write((byte)0x00);
		s->write(0x01);                                                 //This is the actual command
		s->write((byte)0x00);
		s->write(0xFE);
		s->write(0x7E);

		delay(100);
		for (unsigned int q = 0; q<7; q++) s->read();                   //Read the response bytes
	} else {															//If the system is running I2C...
		SPSWire->beginTransmission(SPS_ADDRESS);
		SPSWire->write(0x0104);											//Set Pointer
		SPSWire->endTransmission();
	}
}

void SPS::clean()                                		                //SPS Power off command. This sends and recieves the power off frame
{
	if(!iicSystem){														//If the system is running serial...
		s->write(0x7E);                                                 //Send clean frame
		s->write((byte)0x00);
		s->write(0x56);                                                 //This is the actual command
		s->write((byte)0x00);
		s->write(0xA9);
		s->write(0x7E);

		delay(100); 
		for (unsigned int q = 0; q<7; q++) s->read();                   //Read the response bytes
	} else {															//If the system is running I2C...
		SPSWire->beginTransmission(SPS_ADDRESS);
		SPSWire->write(0x5607);											//Set Pointer
		SPSWire->endTransmission();
	}
}

void SPS::initOPC()                            			  		        //SPS initialization code. Requires input of SPS serial stream.
{
	OPC::initOPC();														//Calls original init
	
	if(iicSystem) SPSWire->begin(I2C_MASTER,0x69,SPSpins,I2C_PULLUP_EXT,I2C_RATE_100); //Begin the wire if I2C with required specifications
	
	powerOn();                                       	            	//Sends SPS active measurement command
	delay(100);
	clean();															//clean to start. This does nothing if attached to the pump
	
	
}

String SPS::CSVHeader(){												//Returns the .logUpdate() data header in CSV format
	String header = "hits,lastLog,MC-1um,MC-2.5um,MC-4.0um,MC-10um,NC-0.5um,NC-1um,NC-2.5um,NC-4.0um,NC-10um,Avg. PM";
	return header;
}

String SPS::logUpdate(){                          				        //This function will parse the data and form loggable strings.
	unsigned int lastLog;
    String dataLogLocal; 
    if (readData()){                                                    //Read the data and determine the read success.
       goodLog = true;                                                  //This will establish the good log inidicators.
       goodLogAge = millis();
       badLog = 0;
       lastLog = millis() - goodLogAge;
	   dataLogLocal = (String(nTot++) + "," + String(lastLog) + ",");

   for(unsigned short k = 0; k<4; k++){                                 //This loop will populate the data string with mass concentrations.                                                       //below it.
         dataLogLocal += String(SPSdata.mas[k],6) + ',';            		    
   }

   for(unsigned short k = 0; k<5; k++){                                 //This loop will populate the data string with number concentrations.
        dataLogLocal += String(SPSdata.nums[k],6) + ',';
   }
   
   dataLogLocal += String(SPSdata.aver,6);                              //This adds the average particle size to the end of the bin.
    
  } else {
	 badLog ++;
	 if (badLog >= 5) goodLog = false;	
	 lastLog = millis() - goodLogAge;									//Good log situation the same as in the Plantower code
	 dataLogLocal = String(nTot) + "," + String(lastLog);
	 dataLogLocal += ",-,-,-,-,-,-,-,-,-,-";							//If there is bad data, the string is populated with failure symbols.              
	 if ((millis()-goodLogAge)>=resetTime) {							//If the age of the last good log exceeds the automatic reset trigger,
		 powerOff();													//the system will cycle and clean the dust bin.
		 delay (2000);
		 powerOn();
		 delay (100);
		 clean();
		 delay(2000);
		 goodLogAge = millis();
	 }
	}
	return dataLogLocal;
  }
  
String SPS::logReadout(String name){     
	unsigned int lastLog;
    String dataLogLocal; 
    if (readData()){                                                    //Read the data and determine the read success.
       goodLog = true;                                                  //This will establish the good log inidicators.
       goodLogAge = millis();
       badLog = 0;
       lastLog = millis() - goodLogAge;
	   dataLogLocal = (String(nTot++) + "," + String(lastLog) + ",");

   for(unsigned short k = 0; k<4; k++){                                 //This loop will populate the data string with mass concentrations.                                                       //below it.
         dataLogLocal += String(SPSdata.mas[k],6) + ',';            		    
   }

   for(unsigned short k = 0; k<5; k++){                                 //This loop will populate the data string with number concentrations.
        dataLogLocal += String(SPSdata.nums[k],6) + ',';
   }
   
   dataLogLocal += String(SPSdata.aver,6);                              //This adds the average particle size to the end of the bin.
    
    Serial.println();													//Clean serial monitor print
	Serial.println("=======================");
	Serial.println(("SPS: " + name));
	Serial.println();
	Serial.print("Successful Data Hits: ");
	Serial.println(String(nTot));
	Serial.print("Last log time: ");
	Serial.println(String(lastLog));
	Serial.println();
	Serial.print(".3 to .5 microns per cubic cm: ");
	Serial.println(String(SPSdata.nums[0],6));
	Serial.print(".3 to 1 microns per cubic cm: ");
	Serial.println(String(SPSdata.nums[1],6));
	Serial.print(".3 to 2.5 microns per cubic cm: ");
	Serial.println(String(SPSdata.nums[2],6));
	Serial.print(".3 to 4 microns per cubic cm: ");
	Serial.println(String(SPSdata.nums[3],6));
	Serial.print(".3 to 10 microns per cubic cm: ");
	Serial.println(String(SPSdata.nums[4],6));
	Serial.println("======================="); 
    
  } else {
	 badLog ++;
	 if (badLog >= 5) goodLog = false;	
	 lastLog = millis() - goodLogAge;									//Good log situation the same as in the Plantower code
	 dataLogLocal = "," + String(lastLog);
	 dataLogLocal += ",-,-,-,-,-,-,-,-,-,-";							//If there is bad data, the string is populated with failure symbols.              
	 
	 Serial.println();													//clean serial print
	 Serial.println("=======================");
	 Serial.println(("SPS: " + name));
	 Serial.println();
	 Serial.print("Successful Data Hits: ");
	 Serial.println(String(nTot));
	 Serial.print("Last log time: ");
	 Serial.println(String(lastLog));
	 Serial.println();
	 Serial.println("Bad log");
	 Serial.println("=======================");	 
	 
	 if ((millis()-goodLogAge)>=resetTime) {							//If the age of the last good log exceeds the automatic reset trigger,
		 powerOff();													//the system will cycle and clean the dust bin.
		 delay (2000);
		 powerOn();
		 delay (100);
		 clean();
		 delay(2000);
		 goodLogAge = millis();
	 }
	}
	return dataLogLocal;	
}

bool SPS::readData(){
	byte buffers[40] = {0};												//Reading buffer

	if(!iicSystem){														//If the SPS is configured in serial mode
		byte systemInfo[5] = {0};
		byte data = 0;
		byte checksum = 0;
		byte SPSChecksum = 0;


		s->write(0x7E);                                                 //The read data function will return true if the data request is successful.
		s->write((byte)0x00);
		s->write(0x03);                                                 //This is the actual command
		s->write((byte)0x00);
		s->write(0xFC);
		s->write(0x7E);

		if (! s->available()) return false;                             //If the given serial connection is not available, the data request will fail.

		if (s->peek() != 0x7E){                                         //If the sent start byte is not as expected, the data request will fail.
		 for (unsigned short j = 0; j<60; j++) data = s->read();        //The data buffer will be wiped to ensure the next data pull isn't corrupt.
		 return false;
		}

		if (s->available() < 47){                                       //If there are not enough data bytes available, the data request will fail. This
		return false;                                                   //will not clear the data buffer, because the system is still trying to fill it.
		}

		for(unsigned short j = 0; j<5; j++){                            //This will populate the system information array with the data returned by the                  
			systemInfo[j] = s->read();                                  //by the system about the request. This is not the actual data, but will provide
			if (j != 0) checksum += systemInfo[j];                      //information about the data. The information is also added to the checksum.
	

		if (systemInfo[3] != (byte)0x00){                               //If the system indicates a malfunction of any kind, the data request will fail.
		 for (unsigned short j = 0; j<60; j++) data = s->read();        //Any data that populates the main array will be thrown out to prevent future corruption.
		 return false;
		}

		byte stuffByte = 0;
		for(unsigned short j = 3; j < 40; j+=4){      					//This nested loop will read the bytes and convert to MSB
			for(unsigned short i = 0; i < 4; i++){
				unsigned short x = j - i;
				buffers[x] = s->read();
				
				if (buffers[x] == 0x7D) {                               //This hex indicates that byte stuffing has occurred. The
					stuffByte = s->read();                              //series of if statements will determine the original value
					if (stuffByte == 0x5E) buffers[x] = 0x7E;			//based on the following hex and replace the data.
					if (stuffByte == 0x5D) buffers[x] = 0x7D;
					if (stuffByte == 0x31) buffers[x] = 0x11;
					if (stuffByte == 0x33) buffers[x] = 0x13;
				}
				checksum += buffers[x];                                 //The data is added to the checksum.
			}
		}

		SPSChecksum = s->read();                                        //The provided checksum byte is read.
		data = s->read();                                               //The end byte of the data is read.

		if (data != 0x7E){                                              //If the end byte is bad, the data request will fail.
		   for (unsigned short j = 0; j<60; j++) data = s->read();      //At this point, there likely isn't data to throw out. However,
		   data = 0;                                                    //The removal is completed as a redundant measure to prevent corruption.
		   return false;
		}

		checksum = checksum & 0xFF;                                     //The local checksum is calculated here. The LSB is taken by the first line.
		checksum = ~checksum;                                           //The bit is inverted by the second line.

		if (checksum != SPSChecksum){                                   //If the checksums are not equal, the data request will fail.  
		  for (unsigned short j = 0; j<60; j++) data = s->read();       //Just to be certain, any remaining data is thrown out to prevent corruption.
		  return false;
		}
  
	} else {															//If the SPS is configured in I2C mode
		if(dataReady()){												//Check if data is available to pull
			SPSWire->beginTransmission(SPS_ADDRESS);
			SPSWire->write(0x0202);										//Set Pointer
			SPSWire->endTransmission(I2C_NOSTOP);						//request read
			SPSWire->sendRequest(SPS_ADDRESS,60,I2C_STOP);				//Fill the buffer
			SPSWire->finish();											//Wait for the buffer to fill
			
			if(SPSWire->available() != 60) return false;				//If the buffer does not fill, the data read failed.
			
			unsigned short i = 0;
			
			while(SPSWire->available()){								//Clear the buffer
				uint8_t data[3];
				
				for (uint8_t j = 0; j < 3; j++){						//read three bytes
					data[j] = SPSWire->readByte();
				}
				
				if (CalcCrc(data) != data[2]) return false;				//if the bytes fail the checksum, the data read failed.
				buffers[i++] = data[0];									//Otherwise, add the data to the buffer
				buffers[i++] = data[1];
			}		
		}else return false;												//If the data is not available to pull, the data read failed.
	}  
	
	memcpy((void *)&SPSdata, (void *)buffers, 40);						//Copy the data to the struct
	return true;                   
}

bool SPS::dataReady(){													//Check if the SPS is ready to send measurement data
	SPSWire->beginTransmission(SPS_ADDRESS);
	SPSWire->write(0x0202);												//Set Pointer
	SPSWire->endTransmission(I2C_NOSTOP);								//request read
	SPSWire->sendRequest(SPS_ADDRESS,3,I2C_STOP);
	SPSWire->finish();													//Wait to finish
	
	uint8_t data[3];
	uint8_t i = 0;
	
	while (SPSWire->available()){										//read data
		if (i < 3) data[i++] = SPSWire->readByte();
		else return false;
	}
	
	if((CalcCrc(data) == data[2])&&(data[0] == 0)&&(data[1] == 1)) return true;	//if the data is correctly transmitted and indicates data is ready, return true
	
	return false;
}

uint8_t SPS::CalcCrc(uint8_t data[2]) {									//Calculate the two byte checksum for I2C
	uint8_t crc = 0xFF;
	for(int i = 0; i < 2; i++) {
		crc ^= data[i];
		for(uint8_t bit = 8; bit > 0; --bit) {
			if(crc & 0x80) {
				crc = (crc << 1) ^ 0x31u;
			} else {
				crc = (crc << 1);
			}
		}
	}
	return crc;
}
	


//////////R1//////////



R1::R1(uint8_t slave) : OPC() { 										//Constructor
	CS = slave; 														//Set up SPI slave pin
	pinMode(CS,OUTPUT);
	}						

void R1::powerOn(){														//system activation
  byte inData[3] = {0}; 
  unsigned short loopy = 0; 
  unsigned short bail = 0;
  
  
  digitalWrite(CS,LOW);													//Open data translation
  SPI.beginTransaction(SPISettings(R1_SPEED, MSBFIRST, SPI_MODE1));
 
  do{																	//Cycle to attempt power on
	  inData[0] = SPI.transfer(0x03);									//Power signal byte
	  delay(10);
	  loopy++;
	  
	  if (loopy > 20){													//If 20 attempts to communicate fail, then turn off and back on
		digitalWrite(CS, HIGH);                                         //The power on and off for this system takes extra time, due to the sensitivity of SPI. 
		SPI.endTransaction();											//With these commands, it is critical to connect. Later, when reading data, missing a hit
		delay(2000);													//can be recovered later.
		loopy = 0;
		SPI.beginTransaction(SPISettings(R1_SPEED, MSBFIRST, SPI_MODE1));
		digitalWrite(CS,LOW);
		bail++;
	  }
  } while ((inData[0] != 0xF3)&&(bail <= 5));							//If the system successfully activates, or fails to power on after 120 attempts, give up

  inData[1] = SPI.transfer(0x03);										//Control bytes

  digitalWrite(CS, HIGH);                                          
  SPI.endTransaction();
  
  if (bail >= 5) {														//give up :(
	  return;
  }
}

void R1::powerOff(){													//This is the power down sequence
  byte inData[3] = {0}; 
  unsigned short loopy = 0; 
  unsigned short bail = 0;
  
  
  digitalWrite(CS,LOW);													//Open communication
  SPI.beginTransaction(SPISettings(R1_SPEED, MSBFIRST, SPI_MODE1));
 
  do{																	//Power down cycle attempts, same system as power on
	  inData[0] = SPI.transfer(0x03);
	  delay(10);
	  loopy++;
	  
	  if (loopy > 20){													//Same loop set as above
		digitalWrite(CS, HIGH);                                          
		SPI.endTransaction();
		delay(2000);
		loopy = 0;
		SPI.beginTransaction(SPISettings(R1_SPEED, MSBFIRST, SPI_MODE1));
		digitalWrite(CS,LOW);
		bail++;
	  }
  } while ((inData[0] != 0xF3)||(bail <= 5));

  inData[1] = SPI.transfer(0x00);										//Control bytes

  digitalWrite(CS, HIGH);                                          
  SPI.endTransaction();
  
  if (bail >= 5) {
	  return;
  }
}

void R1::initOPC(){
	OPC::initOPC();														//Calls original init

	SPI.begin();        											 	//Intialize SPI in Arduino
	digitalWrite(CS,HIGH);												//Pull the pin up so there is no data leakage
	delay(1000);
	powerOn();														
}

String R1::CSVHeader(){													//Returns a data header in CSV formate
	String header = "hits,lastLog,Bin0,Bin1,Bin2,Bin3,Bin4,Bin5,Bin6,Bin7,Bin8,Bin9,";
	header += "Bin10,Bin11,Bin12,Bin13,Bin14,Bin15,Bin1 Time,Bin3 Time,";
	header += "Bin5 Time,Bin7 Time,Flow Rate,Temp,Humidity,Sample Period,";
	header += "PMA,PMB,PMC";
	return header;
}

String R1::logUpdate(){													//If the log is successful, each bin will be logged.
	unsigned int lastLog;
	String dataLogLocal = String(nTot);
	if (readData()){													//If the data is read, create the data string
	   goodLog = true;                                                  
       goodLogAge = millis();
       badLog = 0;
       nTot++;
       lastLog = millis() - goodLogAge;
       dataLogLocal += "," + String(lastLog);
	
		for (unsigned short i = 0; i < 16; i++){
			dataLogLocal += "," + String(localData.bins[i]);
		}
		
		dataLogLocal += "," + String(localData.bin1time);
		dataLogLocal += "," + String(localData.bin2time);
		dataLogLocal += "," + String(localData.bin3time);
		dataLogLocal += "," + String(localData.bin4time);
		
		dataLogLocal += "," + String(localData.sampleFlowRate); 
		dataLogLocal += "," + String(localData.temp); 
		dataLogLocal += "," + String(localData.humid);
		dataLogLocal += "," + String(localData.samplePeriod); 
		dataLogLocal += "," + String(localData.pm1);
		dataLogLocal += "," + String(localData.pm2_5);   
		dataLogLocal += "," + String(localData.pm10);   
		
	} else {															//if the data cannot be read, bad log situation
		badLog ++;
		if (badLog >= 5) goodLog = false;								//Good log situation the same as in the Plantower code
		lastLog = millis() - goodLogAge;
		dataLogLocal += "," + String(lastLog);
		dataLogLocal += ",-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-";	                   
																		//If there is bad data, the string is populated with failure symbols.
		if ((millis()-goodLogAge)>=resetTime) {							//If the age of the last good log exceeds the automatic reset trigger,
			powerOff();													//the system will cycle and clean the dust bin.
			delay (2000);												//The system now has a function checksum
			powerOn();
			delay (100);
			goodLogAge = millis();
		}
	}
	 return dataLogLocal;
 }
 
String R1::logReadout(String name){										//Same as log update, but with a clean readout
	unsigned int lastLog;
	String dataLogLocal = String(nTot);
	
	if (readData()){
	   goodLog = true;                                                  
       goodLogAge = millis();
       badLog = 0;
       nTot++;
       lastLog = millis() - goodLogAge;
       dataLogLocal += "," + String(lastLog);
	
		for (unsigned short i = 0; i < 16; i++){
			dataLogLocal += "," + String(localData.bins[i]);
		}
		
		dataLogLocal += "," + String(localData.bin1time);
		dataLogLocal += "," + String(localData.bin2time);
		dataLogLocal += "," + String(localData.bin3time);
		dataLogLocal += "," + String(localData.bin4time);
		
		dataLogLocal += "," + String(localData.sampleFlowRate); 
		dataLogLocal += "," + String(localData.temp); 
		dataLogLocal += "," + String(localData.humid);
		dataLogLocal += "," + String(localData.samplePeriod); 
		dataLogLocal += "," + String(localData.pm1);
		dataLogLocal += "," + String(localData.pm2_5);   
		dataLogLocal += "," + String(localData.pm10);   
		
		Serial.println();
		Serial.println("=======================");
		Serial.println(("R1: " + name));
		Serial.println();
		Serial.print("Successful Data Hits: ");
		Serial.println(String(nTot));
		Serial.print("Last log time: ");
		Serial.println(String(lastLog));
		Serial.println();
		for (unsigned short i = 0; i < 16; i++){
			Serial.print(("Bin " + String(i) + ": "));
			Serial.println("," + String(localData.bins[i]));
		}
		Serial.println();
		Serial.println("=======================");
		
	} else {
		badLog ++;
		if (badLog >= 5) goodLog = false;								//Good log situation the same as in the Plantower code
		lastLog = millis() - goodLogAge;
		dataLogLocal += "," + String(lastLog);
		dataLogLocal += ",-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-";	                   
		
		Serial.println();
		Serial.println("=======================");
		Serial.println(("R1: " + name));
		Serial.println();
		Serial.print("Successful Data Hits: ");
		Serial.println(String(nTot));
		Serial.print("Last log time: ");
		Serial.println(String(lastLog));
		Serial.println();
		Serial.println("Bad log");
		Serial.println("=======================");
																		//If there is bad data, the string is populated with failure symbols.
		if ((millis()-goodLogAge)>=resetTime) {							//If the age of the last good log exceeds the automatic reset trigger,
			powerOff();													//the system will cycle and clean the dust bin.
			delay (2000);												//The system now has a function checksum
			powerOn();
			delay (100);
			goodLogAge = millis();
		}
	}
	 return dataLogLocal;
}

bool R1::readData(){													//Data reading system
	byte transmitData[64] = {0};
	
	byte byte1 = 0x00;
	byte byte2 = 0x00; 
	unsigned short bail = 0;
	bool success = false;
																		//Open data translation
	SPI.beginTransaction(SPISettings(N3_SPEED, MSBFIRST, SPI_MODE1));
	
	digitalWrite(CS,LOW);

		while (!success && (bail < 25)){ 								//Make 25 attempts to make contact as fast as possible
			byte1 = byte2;
			delay(10);
			byte2 = SPI.transfer(0x30);
			bail++;
			success = ((byte1 == 0x31)&&(byte2 == 0xF3));				//The busy and then active byte indicates success
		}
  
		if (!success) {													//If connection fails, return a read failure.
			digitalWrite(CS, HIGH);
			SPI.endTransaction(); 
			return false;
		}

			for (int i = 0; i<64; i++){									//Pull the data from the system
				delayMicroseconds(10);
				transmitData[i] = SPI.transfer(0x00);
			}

			digitalWrite(CS, HIGH); 	 
			SPI.endTransaction();

			memcpy(&localData, &transmitData, 50);						//Memcpy didn't like the last chunk of bytes for some reason
	
			localData.humid = (localData.humid/(pow(2,16)-1.0))*100;	//Update the humidity and temperature data with the calculated values
			localData.temp = -45 + 175*(localData.temp/(pow(2,16)-1.0));
	
			union pmData{												//The last bytes would not copy, so this cludge makes the system work.
				byte inputs[4];
				float outputs;
			}pmInfo[3];
			
			 for (unsigned short i = 0; i < 3; i++){
				 for (unsigned short j = 0; j < 4; j++){
					 pmInfo[i].inputs[j] = transmitData[50 + i*4 + j];
				 }
			 }
			 localData.pm1 = pmInfo[0].outputs;
			 localData.pm2_5 = pmInfo[1].outputs;
			 localData.pm10 = pmInfo[2].outputs;
			 
			 localData.checksum = bytes2int(transmitData[62],transmitData[63]);
		 	 return (localData.checksum == CalcCRC(transmitData, 62));	//Return the checksum result
}

unsigned int R1::CalcCRC(unsigned char data[], unsigned char nbrOfBytes) {
    #define POLYNOMIAL 0xA001 											//Generator polynomial for CRC
    #define InitCRCval 0xFFFF 											//Initial CRC value
    unsigned char _bit; 												//Bit mask
    unsigned int crc = InitCRCval; 										//Initialise calculated checksum 
    unsigned char byteCtr; 												//Byte counter
																		//Calculates 16-Bit checksum with given polynomial  
    for(byteCtr = 0; byteCtr < nbrOfBytes; byteCtr++) {
      crc ^= (unsigned int)data[byteCtr]; 
      for(_bit = 0; _bit < 8; _bit++) {
        if (crc & 1) 													//If bit0 of crc is 1
        {
            crc >>= 1;
            crc ^= POLYNOMIAL; 
        } else crc >>= 1;
      }
    }
    return crc; 
}



//////////HPM//////////													//The HPM is no longer supported.



HPM::HPM(Stream* ser) : OPC(ser) {}										//Constructor	

bool HPM::command(byte cmd, byte chk){									//Command system, will return true if command successful
  byte checkIt[2] = {0};
  unsigned short attempt = 0;
  
  do{																	//Will send the  command until the maximum
  attempt++;															//number of attempts are reached or the proper bytes are
  s->write(0x68);														//returned
  s->write(0x01);
  s->write(cmd);
  s->write(chk);

  delay(50);
  checkIt[0] = s->read();
  checkIt[1] = s->read();

  } while ((attempt < 50)&&(!((checkIt[0] == 0xA5)&&(checkIt[1] == 0xA5))));
  if ((checkIt[0] == 0xA5)&&(checkIt[1] == 0xA5)){
	  return true;
  } else {
	  return false;
  }
}									

void HPM::powerOn(){													//Power on
  command(0x01,0x96);
}

void HPM::powerOff(){													//Power off
  command(0x02,0x95);
}

void HPM::autoSendOn(){													//Auto send on
  if(command(0x40,0x57)){												//When this is active, the HPM will automatically send data.
	  autoSend = true;													//This is off by default.
  }
}

void HPM::autoSendOff(){												//Auto send off
  if(command(0x20,0x77)){												//This setting is recommended, and has been successfully tested.
	  autoSend = false;
  }
}

void HPM::initOPC(){													//System initialization
	OPC::initOPC();
	autoSend = true;
		
	powerOn();															//Will turn on particle detector
	delay(100);
	autoSendOff();														//Will turn off auto sending of data.
}	

String HPM::CSVHeader(){												//Data header in CSV format
	String header = "hits,1um,2.5um,4.0um,10um";
	return header;
}

String HPM::logUpdate(){												//This will update the data log in CSV format
  unsigned int lastLog = millis() - goodLogAge;
  String localDataLog = String(nTot) + ",";								//This system only works when data is not being automatically sent.
  
  if (readData()){														//If the data is successfully read, it will be logged
    nTot++;
    goodLog = true;
    badLog = 0;
    goodLogAge = millis();
    localDataLog = String(lastLog) + ",";

    localDataLog += String(localData.PM1_0) + "," + String(localData.PM2_5) + "," + String(localData.PM4_0) + "," + String(localData.PM10_0);
    
  } else {																//Otherwise, the data string will be populated with error symbols and
    localDataLog = String(lastLog) + ",";
    localDataLog += "-,-,-,-";											//the system will indicate that the log is bad.
    badLog++;
    if (badLog >= 5) goodLog = false;
    if ((millis()-goodLogAge)>=resetTime){								//If it has been a certain amount of time since the system has had a
		powerOff();														//good log, it will reset.
		delay(20000);
		powerOn();
		goodLogAge = millis();
	}
  }
  return localDataLog;
}

bool HPM::readData(){													//This function will read the data
  if (autoSend){														//If the system is in autosend mode, the first part of the code will try to read
    byte inputArray[32] = {0};											//it. This should be run as fast as possible to get the data.
	    
    if (!s->available()){												//If the serial is not available, the data will not be read.
      return false;
    }
  
    if (s->peek() != 0x42){												//If the start byte is not found, the byte is discarded, and the data will not be read.
      s->read();
      return false;
    }
  
    if (s->available() < 32){											//If there are not enough data bytes, the data will not be read.
      return false;
    }
    
    localData.checksum = 0;
    for (int i = 0; i<32; i++){											//Data is read into the input array
      inputArray[i] = s->read();
      if (i<30) localData.checksum += inputArray[i];					//Checksum is calculated
    }
  
    localData.checksumR = bytes2int(inputArray[31],inputArray[30]);		//Sent checksum is read
   if (localData.checksum != localData.checksumR){						//If the checksums do not match, the data will not be saved.
     return false;
   }

   localData.PM1_0 = bytes2int(inputArray[5],inputArray[4]);			//Data is saved
   localData.PM2_5 = bytes2int(inputArray[7],inputArray[6]);
   localData.PM4_0 = bytes2int(inputArray[9],inputArray[8]);
   localData.PM10_0 = bytes2int(inputArray[11],inputArray[10]);

   return true;
   
  } else {																//If the system is not in auto send mode, then this code will be used.
   byte head;
   byte len;
   byte cmd;

   for (unsigned short i = 0; i<32; i++) s->read();						//Clear the buffer (redundant, but helpful)

   s->write(0x68);														//Data is requested
   s->write(0x01);
   s->write(0x04);
   s->write(0x93);

   delay(50);
   
   if (!s->available()){ 												//If the serial port is not available, the data is not read.
     return false;
   }

   if (s->peek() == 0x96){												//If the failure bytes are sent, the data is not read.
      s->read();
      s->read();
      return false;
   }

    head = s->read();
    len = s->read();
    cmd = s->read();

   if (head != 0x40){													//If the start byte is not correct, the data is not read.
     return false;
   }  

    if (s->available()<(len)){											//If there are not enough bytes, the data is not read.
     return false;
   }

   if (cmd != 0x04){													//If the command is incorrect, the data is not read.
     return false;
   }

   uint16_t *inputArray = new uint16_t[len];							//Array for data is created
   unsigned short i = 0;

   while ((i<len)||(s->available())){									//Data is read into an array
     inputArray[i] = s->read();
     i++;
   }

   localData.checksum = 65536 - (head + len + cmd);						//Checksum is calculated
   for (unsigned short i = 0; i<len-1; i++) localData.checksum -= inputArray[i];
   localData.checksum = localData.checksum % 256;
   localData.checksumR = inputArray[(len-1)];
  
   if (localData.checksum != localData.checksumR){						//If the checksums do not match, the data will not be saved.
     return false;
   }

   localData.PM1_0 = inputArray[0]*256 + inputArray[1];					//Data is saved
   localData.PM2_5 = inputArray[2]*256 + inputArray[3];
   localData.PM4_0 = inputArray[4]*256 + inputArray[5];
   localData.PM10_0 = inputArray[6]*256 + inputArray[7];
  
   delete [] inputArray;
   return true;
  }	
}	



//////////N3///////////



N3::N3(uint8_t slave) : OPC() { 										//Constructor
	CS = slave; 														//Set up SPI slave pin
	pinMode(CS,OUTPUT);
}	

bool N3::initCommand(byte command){										//starting command system. This is the internal guts as a condensed version of the 
  byte byte1 = 0;														//R1 protocol. This system takes a long period of time, under the theory that once
  byte byte2 = 0; 														//the command connection is established, the system will be able to successfully connect
  unsigned short bail = 0;												//regularly.
  unsigned short superBail = 0;
  bool success = false;
  
  digitalWrite(CS,LOW);													//Open data translation
  SPI.beginTransaction(SPISettings(N3_SPEED, MSBFIRST, SPI_MODE1));
  delay(10);
  
  while (!success && (superBail < 20)){									//Check for success or bail circumstances
	  delay(1);
	  byte1 = byte2;
	  byte2 = SPI.transfer(0x03);
	  Serial.println(byte2,HEX);
	  delay(10);
	  bail++;
	  success = ((byte1 == 0x31)&&(byte2 == 0xF3));
	  if ((byte1 != 0x31)&&(byte2 !=0x31)&&(bail > 10)){
		bail = 0;
		superBail++;
		digitalWrite(CS, HIGH); 		
		SPI.endTransaction(); 
		delay(3000);
		digitalWrite(CS,LOW);											//Open data translation
		SPI.beginTransaction(SPISettings(N3_SPEED, MSBFIRST, SPI_MODE1));
		delay(10);
	}
  }
  
  if (success) {														//If the system is able to connect, indicate success
	  	byte2 = SPI.transfer(command);
		SPI.endTransaction(); 
		digitalWrite(CS, HIGH); 
		return true;
  } else {
	  SPI.endTransaction(); 
	  digitalWrite(CS, HIGH); 
	  return false;
  }
}


void N3::laserOn(){														//laser on
	if(initCommand(0x07)) return;										//The system has a built in redundant attempt for each general command.
		delay(20000);
		initCommand(0x07);
}

void N3::laserOff(){													//laser off
	if(initCommand(0x06)) return;
		delay(20000);
		initCommand(0x06);
}

void N3::fanOn(){														//fan on
	if(initCommand(0x03)) return;
		delay(20000);		
		initCommand(0x03);
}

void N3::fanOff(){														//fan off
	if(initCommand(0x02)) return;
	delay(20000);
	(initCommand(0x02));
}

void N3::powerOn(){														//This pulls fan and laser commands together to mirror other systems
	delay(1000);
	fanOn();
	delay(500);
	laserOn();
	delay(1000);
}

void N3::powerOnPump(){													//This system only turns on the laser for pump use
	delay(1000);
	fanOff();
	delay(50);
	laserOn();
	delay(1000);
}

void N3::powerOff(){
	fanOff();
	delay(50);
	laserOff();
	delay(50);
}

void N3::initOPC(char t){
	OPC::initOPC();														//Calls original init

	SPI.begin();        											 	//Intialize SPI in Arduino
	digitalWrite(CS,HIGH);												//Pull the pin up so there is no data leakage
	delay(2500);
	if (t == 'p') powerOnPump();										//if the correct trigger is passed, the system will init in pump mode
	else powerOn();												
}

String N3::CSVHeader(){													//Header for log update								
	String header = "hits,lastLog,Bin0,Bin1,Bin2,Bin3,Bin4,Bin5,Bin6,Bin7,Bin8,Bin9,";
	header += "Bin10,Bin11,Bin12,Bin13,Bin14,Bin15,Bin16,Bin17,Bin18,Bin19,";
	header += "Bin20,Bin21,Bin22,Bin23,Bin1 Time,Bin3 Time,Bin5 Time,Bin7 Time,";
	header += "Sampling Period,Flow Rate,Temp,Humidity,PM1,PM2_5,PM10";
	return header;
}

String N3::logUpdate(){													//CSV creator and system updator
	unsigned int lastLog;
	String dataLogLocal = String(nTot);
	if (readData()){													//If the data can be read, shift the data from the struct into the CSV.
	   goodLog = true;                                                  
       goodLogAge = millis();
       badLog = 0;
       nTot++;
       lastLog = millis() - goodLogAge;
       dataLogLocal += "," + String(lastLog);
	
	   for (unsigned short i = 0; i < 24; i++) dataLogLocal += "," + String(localData.bins[i]);
	   dataLogLocal += "," + String(localData.bin1time);
	   dataLogLocal += "," + String(localData.bin2time);
	   dataLogLocal += "," + String(localData.bin3time);
	   dataLogLocal += "," + String(localData.bin4time);
	   dataLogLocal += "," + String(localData.samplePeriod);
	   dataLogLocal += "," + String(localData.sampleFlowRate);   
	   dataLogLocal += "," + String(localData.temp);
	   dataLogLocal += "," + String(localData.humid); 
	   dataLogLocal += "," + String(localData.pm1);  
	   dataLogLocal += "," + String(localData.pm2_5);
	   dataLogLocal += "," + String(localData.pm10);	   		   		   	    	     	      	   	      
	} else {
		lastLog = millis() - goodLogAge;
		badLog ++;
		if (badLog >= 5) goodLog = false;								//Good log situation the same as in the Plantower code
		dataLogLocal += "," + String(lastLog);
		dataLogLocal += ",-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-";	//35                   
																		//If there is bad data, the string is populated with failure symbols.
		if ((millis()-goodLogAge)>=resetTime) {							//If the age of the last good log exceeds the automatic reset trigger,
			powerOff();													//the system will cycle and clean the dust bin.
			delay (2000);												//The system now has a function checksum
			powerOn();
			delay (100);
			goodLogAge = millis();
		}
	}
	 return dataLogLocal;
}

String N3::logReadout(String name){logUpdate();}						//Log Readout is not implemented yet!

bool N3::readData(){ 													//Internal data reading function
	byte transmitData[86] = {0};
	
	byte byte1 = 0x00;
	byte byte2 = 0x00; 
	unsigned short bail = 0;
	bool success = false;
																		//Open data translation
	SPI.beginTransaction(SPISettings(N3_SPEED, MSBFIRST, SPI_MODE1));
	
	digitalWrite(CS,LOW);

		while (!success && (bail < 25)){ 								//Attempt to make a connection
			byte1 = byte2;
			delay(10);
			byte2 = SPI.transfer(0x30);
			bail++;
			success = ((byte1 == 0x31)&&(byte2 == 0xF3));				//Success if the busy and active bytes are recieved
		}
  
		if (!success) {													//If the system does not succeed, return a failure
			digitalWrite(CS, HIGH);
			SPI.endTransaction(); 
			return false;
		}
			Serial.println();
			for (int i = 0; i<86; i++){									//Read all of the bytes from the system
				delayMicroseconds(10);
				transmitData[i] = SPI.transfer(0x00);
				Serial.print(transmitData[i],HEX);
				Serial.print(" ");
			}
			Serial.println();
			digitalWrite(CS, HIGH); 	 
			SPI.endTransaction();
	
			memcpy(&localData, &transmitData, 86);						//Copy the data to the struct
	
			localData.humid = (localData.humid/(pow(2,16)-1.0))*100;	//Update the humidity and temperature data with the calculated data
			localData.temp = -45 + 175*(localData.temp/(pow(2,16)-1.0));
	
			return (localData.checkSum == CalcCRC(transmitData, 84));	//return the checksum results
}
	
unsigned int N3::CalcCRC(unsigned char data[], unsigned char nbrOfBytes){
    #define POLYNOMIAL 0xA001 											//Generator polynomial for CRC
    #define InitCRCval 0xFFFF 											//Initial CRC value
    unsigned char _bit; 												//Bit mask
    unsigned int crc = InitCRCval; 										//Initialise calculated checksum 
    unsigned char byteCtr; 												//Byte counter
																		//Calculates 16-Bit checksum with given polynomial  
    for(byteCtr = 0; byteCtr < nbrOfBytes; byteCtr++) {
      crc ^= (unsigned int)data[byteCtr]; 
      for(_bit = 0; _bit < 8; _bit++) {
        if (crc & 1) 													//If bit0 of crc is 1
        {
            crc >>= 1;
            crc ^= POLYNOMIAL; 
        } else crc >>= 1;
      }
    }
    return crc; 
}
