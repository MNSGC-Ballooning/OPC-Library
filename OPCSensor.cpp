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

bool OPC::readData(){ return false; }

void OPC::getData(float dataPtr[], unsigned int arrayFill){}

void OPC::getData(float dataPtr[], unsigned int arrayFill, unsigned int arrayStart){}

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
	String header = "hits,MC1um,MC2.5um,MC10um,AMC1um,AMC2.5um,AMC10um,";
	header += "NC03um,NC05um,NC10um,NC25um,NC50um,NC100um";
	return header;
}

String Plantower::logUpdate(){

	String dataLogLocal = "";											//Log sample number, in flight time																							
    dataLogLocal += String(nTot) + ",";  
    
    if ((millis()-goodLogAge <= logRate)&&goodLog){                     //If data is in the buffer, log it
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

void Plantower::getData(float dataPtr[], unsigned int arrayFill){		//Will populate data into a provided array
	unsigned int i = 0;
	uint32_t dataArray[12] = {PMSdata.pm10_standard,PMSdata.pm25_standard,PMSdata.pm100_standard,PMSdata.pm10_env,PMSdata.pm25_env,PMSdata.pm100_env,PMSdata.particles_03um,PMSdata.particles_05um,PMSdata.particles_10um,PMSdata.particles_25um,PMSdata.particles_50um,PMSdata.particles_100um};
	
	while ((i<arrayFill)&&(i<12)){										//Will fill array or provide all data, whichever comes first
		dataPtr[i]=dataArray[i];
		
		i++;
	}
}

void Plantower::getData(float dataPtr[], unsigned int arrayFill, unsigned int arrayStart){
	unsigned int i = arrayStart;										//Same process as above, but a starting point in the external array can be chosen
	uint32_t dataArray[12] = {PMSdata.pm10_standard,PMSdata.pm25_standard,PMSdata.pm100_standard,PMSdata.pm10_env,PMSdata.pm25_env,PMSdata.pm100_env,PMSdata.particles_03um,PMSdata.particles_05um,PMSdata.particles_10um,PMSdata.particles_25um,PMSdata.particles_50um,PMSdata.particles_100um};
	
	while ((i<arrayFill)&&((i-arrayStart)<12)){		
		dataPtr[i]=dataArray[i-arrayStart];
		
		i++;
	}
}



//////////SPS//////////



SPS::SPS(Stream* ser) : OPC(ser) {}										//Initialize stream using OPC constructor

void SPS::powerOn()                                			            //SPS Power on command. This sends and recieves the power on frame
{
  s->write(0x7E);                                                       //Send startup frame
  s->write((byte)0x00);
  s->write((byte)0x00);                                                 //This is the actual command
  s->write(0x02);
  s->write(0x01);
  s->write(0x03);
  s->write(0xF9);
  s->write(0x7E);

  delay (100);
  for (unsigned int q = 0; q<7; q++) data = s->read();                  //Read the response bytes
  data = 0;
}

void SPS::powerOff()                              		                //SPS Power off command. This sends and recieves the power off frame
{
  s->write(0x7E);                                                       //Send shutdown frame
  s->write((byte)0x00);
  s->write(0x01);                                                       //This is the actual command
  s->write((byte)0x00);
  s->write(0xFE);
  s->write(0x7E);

  delay(100);
  for (unsigned int q = 0; q<7; q++) data = s->read();                  //Read the response bytes
  data = 0;
}

void SPS::clean()                                		                //SPS Power off command. This sends and recieves the power off frame
{
  s->write(0x7E);                                                       //Send clean frame
  s->write((byte)0x00);
  s->write(0x56);                                                       //This is the actual command
  s->write((byte)0x00);
  s->write(0xA9);
  s->write(0x7E);

  delay(100); 
  for (unsigned int q = 0; q<7; q++) data = s->read();                  //Read the response bytes
  data = 0;
}

void SPS::initOPC()                            			  		        //SPS initialization code. Requires input of SPS serial stream.
{
	OPC::initOPC();														//Calls original init
	
    powerOn();                                       	                //Sends SPS active measurement command
    delay(100);
    clean();                                              		        //Sends fan clean command.
}

String SPS::CSVHeader(){												//Returns a data header in CSV formate
	String header = "hits,MC-1um,MC-2.5um,MC-4.0um,MC-10um,NC-0.5um,NC-1um,NC-2.5um,NC-4.0um,NC-10um,Avg. PM";
	return header;
}

String SPS::logUpdate(){                          				        //This function will parse the data and form loggable strings.
    String dataLogLocal = String(nTot);   
    if (readData()){                                                    //Read the data and determine the read success.
       goodLog = true;                                                  //This will establish the good log inidicators.
       goodLogAge = millis();
       badLog = 0;
       nTot++;

   for(unsigned short k = 0; k<4; k++){                                 //This loop will populate the data string with mass concentrations.
      if (k==0) {
         dataLogLocal += ',' + String(m.MCF[k],6) + ',';                //Each bin from the sensor includes all of the particles from the bin
        } else {                                                        //below it.
         dataLogLocal += String(m.MCF[k],6) + ',';            		    
        }
   }

   for(unsigned short k = 0; k<5; k++){                                 //This loop will populate the data string with number concentrations.
        dataLogLocal += String(n.NCF[k],6) + ',';
   }
    dataLogLocal += String(a.ASF,6);                                    //This adds the average particle size to the end of the bin.
    
  } else {
	 badLog ++;
	 if (badLog >= 5) goodLog = false;									//Good log situation the same as in the Plantower code
	 dataLogLocal += ",-,-,-,-,-,-,-,-,-,-";							//If there is bad data, the string is populated with failure symbols.              
	 if ((millis()-goodLogAge)>=resetTime) {							//If the age of the last good log exceeds the automatic reset trigger,
		 powerOff();													//the system will cycle and clean the dust bin.
		 delay (2000);
		 powerOn();
		 delay (100);
		 clean();
		 delay(20500);
		 goodLogAge = millis();
	 }
	}
	return dataLogLocal;
  }

bool SPS::readData(){                                     		      	//SPS data request. The system will pull data and ensure the accuracy.                                                                   
  s->write(0x7E);                                                       //The read data function will return true if the data request is successful.
  s->write((byte)0x00);
  s->write(0x03);                                                       //This is the actual command
  s->write((byte)0x00);
  s->write(0xFC);
  s->write(0x7E);

 if (! s->available()) return false;                                    //If the given serial connection is not available, the data request will fail.

   if (s->peek() != 0x7E){                                              //If the sent start byte is not as expected, the data request will fail.
     for (unsigned short j = 0; j<60; j++) data = s->read();            //The data buffer will be wiped to ensure the next data pull isn't corrupt.
     data = 0;
    return false;
   }

  if (s->available() < 47){                                             //If there are not enough data bytes available, the data request will fail. This
    return false;                                                       //will not clear the data buffer, because the system is still trying to fill it.
  }

    for(unsigned short j = 0; j<5; j++){                                //This will populate the system information array with the data returned by the                  
        systemInfo[j] = s->read();                                      //by the system about the request. This is not the actual data, but will provide
        if (j != 0) checksum += systemInfo[j];                          //information about the data. The information is also added to the checksum.
    }

   if (systemInfo[3] != (byte)0x00){                                    //If the system indicates a malfunction of any kind, the data request will fail.
     for (unsigned short j = 0; j<60; j++) data = s->read();            //Any data that populates the main array will be thrown out to prevent future corruption.
     data = 0;
    return false;
   }

byte stuffByte = 0;
  for(unsigned short buffIndex = 0; buffIndex < 40; buffIndex++){       //This loop will populate the buffer with the data bytes.
    buffers[buffIndex] = s->read();
      if (buffers[buffIndex] == 0x7D) {                                 //This hex indicates that byte stuffing has occurred. The
        stuffByte = s->read();                                          //series of if statements will determine the original value
        if (stuffByte == 0x5E) buffers[buffIndex] = 0x7E;				//based on the following hex and replace the data.
        if (stuffByte == 0x5D) buffers[buffIndex] = 0x7D;
        if (stuffByte == 0x31) buffers[buffIndex] = 0x11;
        if (stuffByte == 0x33) buffers[buffIndex] = 0x13;
    }
    checksum += buffers[buffIndex];                                     //The data is added to the checksum.
  }

    SPSChecksum = s->read();                                            //The provided checksum byte is read.
    data = s->read();                                                   //The end byte of the data is read.

    if (data != 0x7E){                                                  //If the end byte is bad, the data request will fail.
       for (unsigned short j = 0; j<60; j++) data = s->read();          //At this point, there likely isn't data to throw out. However,
       data = 0;                                                        //The removal is completed as a redundant measure to prevent corruption.
       return false;
    }

    checksum = checksum & 0xFF;                                         //The local checksum is calculated here. The LSB is taken by the first line.
    checksum = ~checksum;                                               //The bit is inverted by the second line.

    if (checksum != SPSChecksum){                                       //If the checksums are not equal, the data request will fail.  
      for (unsigned short j = 0; j<60; j++) data = s->read();           //Just to be certain, any remaining data is thrown out to prevent corruption.
      data = 0;
      checksum = 0;
      return false;
    }
    
   checksum = 0;
   data = 0;

    for (unsigned short j = 0; j<16; j++) MassC[j] = buffers[j];        //The mass concentration data is removed from the buffer.
    for (unsigned short j = 0; j<20; j++) NumC[j] = buffers[j+16];      //The number concentration data is removed from the buffer.
    for (unsigned short j=0; j<4; j++) AvgS[j] = buffers[j+36];         //The average size data is removed from the buffer.

																		//The data is sent in reverse. This will flip the order of every four bytes
	unsigned short flip = 0;                                            //Index of array to flip- I KNOW THIS IS REDUNDANT FOR THE FLIP IN THE FOR LOOP. However, I want flip to continue to increase.
	unsigned short result = 0;                                          //Index of array that will be the result

	for (unsigned short flipMax = 4; flipMax<21; flipMax+=4){           //This will loop through the main flipping mechanism
		result = flipMax - 1;                                           //The result starts one lower than the flipMax because of counting from zero
		for (flip; flip<flipMax; flip++){                               //Flipping mechanism. This flips the results of the data request. IGNORE THIS WARNING.
			if (flipMax < 5)  a.ASA[flip] = AvgS[result];               //Flips average size. Average size only has four bytes.
			if (flipMax < 17) m.MCA[flip] = MassC[result];              //Flips mass. Mass has four less bytes than number.
			n.NCA[flip] = NumC[result];                                 //Flips number count.
			result--;
		}
	}
  return true;                                                          //If the reading is successful, the function will return true.
}

void SPS::getData(float dataPtr[], unsigned int arrayFill){				//Populate an array with the collected data
	unsigned int i = 0;													//Below, an array is populated with particle data
	float dataArray[10] = {m.MCF[0],m.MCF[1],m.MCF[2],m.MCF[3],n.NCF[0],n.NCF[1],n.NCF[2],n.NCF[3],n.NCF[4],a.ASF};
	
	while ((i<arrayFill)&&(i<10)){
		dataPtr[i]=dataArray[i];										//particle data is passed to external array
		
		i++;
	}
}

void SPS::getData(float dataPtr[], unsigned int arrayFill, unsigned int arrayStart){
	unsigned int i = arrayStart;
	float dataArray[10] = {m.MCF[0],m.MCF[1],m.MCF[2],m.MCF[3],n.NCF[0],n.NCF[1],n.NCF[2],n.NCF[3],n.NCF[4],a.ASF};
	
	while ((i<arrayFill)&&((i-arrayStart)<10)){
		dataPtr[i]=dataArray[i-arrayStart];								//The process is the same as above, but you can pass data later into the input array
		
		i++;
	}
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
  SPI.beginTransaction(SPISettings(700000, MSBFIRST, SPI_MODE1));
 
  do{																	//Cycle to attempt power on
	  inData[0] = SPI.transfer(0x03);									//Power signal byte
	  delay(10);
	  loopy++;
	  
	  if (loopy > 20){													//If 20 attempts to communicate fail, then turn off and back on
		digitalWrite(CS, HIGH);                                          
		SPI.endTransaction();
		delay(1000);
		loopy = 0;
		SPI.beginTransaction(SPISettings(700000, MSBFIRST, SPI_MODE1));
		digitalWrite(CS,LOW);
		bail++;
	  }
  } while ((inData[0] != 0xF3)||(bail <= 5));							//If the system successfully activates, or fails to power on after 120 attempts, give up

  inData[1] = SPI.transfer(0x03);										//Control bytes
  delay(10);
  inData[2] = SPI.transfer(0x01);
  delay (10);

  digitalWrite(CS, HIGH);                                          
  SPI.endTransaction();
  
  if (bail >= 5) {														//give up :(
	  return;
  }
  
  if (inData[1] != 0x03){												//Restart attempt
    delay(5000);
    powerOn();
  }
}

void R1::powerOff(){													//This is the power down sequence
  byte inData[3] = {0}; 
  unsigned short loopy = 0; 
  unsigned short bail = 0;
  
  
  digitalWrite(CS,LOW);													//Open communication
  SPI.beginTransaction(SPISettings(700000, MSBFIRST, SPI_MODE1));
 
  do{																	//Power down cycle attempts, same system as power on
	  inData[0] = SPI.transfer(0x03);
	  delay(10);
	  loopy++;
	  
	  if (loopy > 20){
		digitalWrite(CS, HIGH);                                          
		SPI.endTransaction();
		delay(1000);
		loopy = 0;
		SPI.beginTransaction(SPISettings(700000, MSBFIRST, SPI_MODE1));
		digitalWrite(CS,LOW);
		bail++;
	  }
  } while ((inData[0] != 0xF3)||(bail <= 5));

  inData[1] = SPI.transfer(0x00);										//Control bytes
  delay(10);
  inData[2] = SPI.transfer(0x00);
  delay (10);

  digitalWrite(CS, HIGH);                                          
  SPI.endTransaction();
  
  if (bail >= 5) {
	  return;
  }
  
  if (inData[1] != 0x03){
    delay(5000);
    powerOff();
  }
}

void R1::initOPC(){
	OPC::initOPC();														//Calls original init

	SPI.begin();        											 	//Intialize SPI in Arduino
	digitalWrite(CS,HIGH);												//Pull the pin up so there is no data leakage
	delay(2500);
	powerOn();															
}

String R1::CSVHeader(){													//Returns a data header in CSV formate
	String header = "hits,Bin0,Bin1,Bin2,Bin3,Bin4,Bin5,Bin6,Bin7,Bin8,Bin9,";
	header += "Bin10,Bin11,Bin12,Bin13,Bin14,Bin15,Bin1 Time,Bin3 Time,";
	header += "Bin5 Time,Bin7 Time,Flow Rate,Temp,Humidity,Sample Period,";
	header += "PMA,PMB,PMC";
	return header;
}

String R1::logUpdate(){													//If the log is successful, each bin will be logged.
	String dataLogLocal = String(nTot);
	if (readData()){
	   goodLog = true;                                                  
       goodLogAge = millis();
       badLog = 0;
       nTot++;
	
		for (int x = 0; x<27; x++){										//Data log in CSV
			if(x == 20){
				dataLogLocal += ',';
				dataLogLocal += String(sfr.floatOut,6);
				
			} else if(x == 23){
				dataLogLocal += ',';
				dataLogLocal += String(sp.floatOut,6);
				
			} else if(x == 24){
				dataLogLocal += ',';
				dataLogLocal += String(a.floatOut,6);
				
			} else if(x == 25){
				dataLogLocal += ',';
				dataLogLocal += String(b.floatOut,6);				
				
			} else if(x == 26){
				dataLogLocal += ',';
				dataLogLocal += String(c.floatOut,6);				
				
			} else {
				dataLogLocal += ',';
				dataLogLocal += String(data[x]);
			}
		}
	} else {
		badLog ++;
		if (badLog >= 5) goodLog = false;								//Good log situation the same as in the Plantower code
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

bool R1::readData(){													//Data reading system
  digitalWrite(CS,LOW);
  SPI.beginTransaction(SPISettings(700000, MSBFIRST, SPI_MODE1));

  byte test = 0x00;
  byte raw[64] = {0};
  unsigned short loopy = 0;
  unsigned short bail = 0;
  
  do{																	//Attempt to communicate
	  delay(10);
	  test = SPI.transfer(0x30);
	  loopy++;
  
	  if (loopy > 20){
		digitalWrite(CS, HIGH);                                          
		SPI.endTransaction();
		delay(1000);
		loopy = 0;
		SPI.beginTransaction(SPISettings(700000, MSBFIRST, SPI_MODE1));
		digitalWrite(CS,LOW);
		bail++;
		if (bail >= 5) return false;
	  }
	  
  } while (test != 0xF3);
  
	for (unsigned short i = 0; i < 64; i++){							//Pull data
		delayMicroseconds(20);
		raw[i] = SPI.transfer(0x30); 
	 }
	 
  digitalWrite(CS,HIGH);
  SPI.endTransaction();													//End communciation
  
  for (unsigned short x=0; x<16; x++){									//Convert bytes into data
    data[x] = bytes2int(raw[(x*2)], raw[(x*2+1)]);
   }

  for (int x = 16; x<20; x++){
    data[x] = raw[x+16];
  }

  for (unsigned short x = 0; x<4; x++){
    sfr.byteIn[x] = raw[x+36];
  }

  data[20] = bytes2int(raw[40], raw[41]);
  data[21] = bytes2int(raw[42], raw[43]);

  for (unsigned short x = 0; x<4; x++){
    sp.byteIn[x] = raw[x+44];
  }

  data[22] = raw[48];
  data[23] = raw[49];

  for (unsigned short x = 0; x<4; x++){
    a.byteIn[x] = raw[x+50];
  }

  for (unsigned short x = 0; x<4; x++){
    b.byteIn[x] = raw[x+54];
  }

  for (unsigned short x = 0; x<4; x++){
    c.byteIn[x] = raw[x+58];
  }

  data[24] = bytes2int(raw[62],raw[63]);
  
  return (data[24] == CalcCRC(raw,62));									//Return results of the checksum

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
																		//I have not set this up yet
/*void R1::getData(float dataPtr[], unsigned int arrayFill){			//This fills an array with the data instead of using CSV format
	unsigned int i = 0;
	
	while ((i<arrayFill)&&(i<27)){
		dataPtr[i]=com[i];
		
		i++;
	}
}

void R1::getData(float dataPtr[], unsigned int arrayFill, unsigned int arrayStart){
	unsigned int i = arrayStart;										//This allows you to fill with the latter parts of the data
	
	while ((i<arrayFill)&&((i-arrayStart)<27)){							//Data cannot fill past the size of the array or the total amount of potential data
		dataPtr[i]=com[i-arrayStart];
		
		i++;
	}
}*/



//////////HPM//////////



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
  String localDataLog = String(nTot) + ",";								//This system only works when data is not being automatically sent.
  
  if (readData()){														//If the data is successfully read, it will be logged
    nTot++;
    goodLog = true;
    badLog = 0;
    goodLogAge = millis();

    localDataLog += String(localData.PM1_0) + "," + String(localData.PM2_5) + "," + String(localData.PM4_0) + "," + String(localData.PM10_0);
    
  } else {																//Otherwise, the data string will be populated with error symbols and
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

void HPM::getData(float dataPtr[], unsigned int arrayFill){				//Data is saved directly into an array
	unsigned int i = 0;
	uint32_t dataArray[4] = {localData.PM1_0,localData.PM2_5,localData.PM4_0,localData.PM10_0};
	
	while ((i<arrayFill)&&(i<4)){
		dataPtr[i]=dataArray[i];
		
		i++;
	}	
}			

void HPM::getData(float dataPtr[], unsigned int arrayFill, unsigned int arrayStart){
	unsigned int i = arrayStart;										//Data is still saved into an array, but the starting point can be chosen.
	uint32_t dataArray[4] = {localData.PM1_0,localData.PM2_5,localData.PM4_0,localData.PM10_0};
	
	while ((i<arrayFill)&&((i-arrayStart)<4)){
		dataPtr[i]=dataArray[i-arrayStart];
		
		i++;
	}	
}	
