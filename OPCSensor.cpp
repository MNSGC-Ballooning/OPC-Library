//Optical Particle Counter Library

//University of Minnesota - Candler MURI
//Written July 2019

/*This is the definitions file for the OPC library.
This will run any optical particle counters used for MURI.
All particle counters will need to be run in loops of different speeds.
Serial begin must be called separately.

The PMS 5003 runs the read data function as fast as possible, and can
record new data every 2.3 seconds.
 
The SPS 30 runs the read data function with the record data function, and
can record new data every 1 seconds. */

#include "OPCSensor.h"

//OPC//


OPC::OPC(Stream* ser){
	s = ser;
}

int OPC::getTot(){ return nTot; }										//get the total number of data points

bool OPC::getLogQuality(){ return goodLog; }							//get the log quality

void OPC::initOPC(){													//Initialize the serial and OPC variables
	goodLog = false;
	badLog = 0;
	nTot = 1;	
}

String OPC::logUpdate(){												//Placeholders: will always be redefined
	String localDataLog = "OPC not specified!";
	return localDataLog;
}

bool OPC::readData(){
	bool badRead = false;
	return badRead;
}


//PLANTOWER//


Plantower::Plantower(Stream* ser, unsigned int planLog) : OPC(ser) {
		logRate = planLog;
		goodLogAge = 0;
	}

String Plantower::logUpdate(){
	String localDataLog = "";															
    localDataLog += nTot;												//Log sample number, in flight time
    localDataLog += ",";  
    
    if ((millis()-goodLogAge)>=logRate) goodLog = false;
    
    if (goodLog) {
    localDataLog += PMSdata.particles_03um;                             //If data is in the buffer, log it
    localDataLog += ",";
    localDataLog += PMSdata.particles_05um;
    localDataLog += ",";
    localDataLog += PMSdata.particles_10um;
    localDataLog += ",";
    localDataLog += PMSdata.particles_25um;
    localDataLog += ",";
    localDataLog += PMSdata.particles_50um;
    localDataLog += ",";
    localDataLog += PMSdata.particles_100um;

    nTot += 1;                                                   	    //Total samples
	
	} else {
		badLog++;                                                       //If there are five consecutive bad logs, not the state;
		if (badLog >= 5){
		localDataLog += '%' + ',' + 'Q' + ',' + '=' + ',' + '!' + ',' + '@' + ',' + '$';
		}
	}
  return localDataLog;
}

bool Plantower::readData(){
	  if (! s->available()) {
    return false;
  }
  
  if (s->peek() != 0x42) { 												//Read a byte at a time until we get to the special '0x42' start-byte
    s->read();
    return false;
  }
 
  if (s->available() < 32) {  											//Now read all 32 bytes
    return false;
  }
    
  uint8_t buffer[32];    
  uint16_t sum = 0;
  s->readBytes(buffer, 32);
 
  for (uint8_t i=0; i<30; i++) {  										//Get checksum ready
    sum += buffer[i];
  }

  uint16_t buffer_u16[15];
  for (uint8_t i=0; i<15; i++) {
    buffer_u16[i] = buffer[2 + i*2 + 1];
    buffer_u16[i] += (buffer[2 + i*2] << 8);
  }
 
  memcpy((void *)&PMSdata, (void *)buffer_u16, 30);						 //Put it into a nice struct :)
 
  if (sum != PMSdata.checksum) {
    return false;
  }

	goodLog = true;
	goodLogAge = millis();
	badLog = 0;
  return true;
}


//SPS//


SPS::SPS(Stream* ser) : OPC(ser) {}

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
	goodLog = false;
	badLog = 0;
	nTot = 1;
	
    powerOn();                                       	                //Sends SPS active measurement command
    delay(100);
    clean();                                              		        //Sends fan clean command. This takes 10 seconds. Then, an additional 10 seconds
    delay(20500);                                                       //are taken to get a clean, consistent flow through the system.
}

bool SPS::readData(){                                     		      	//SPS data request. The system will pull data and ensure the accuracy.                                                                   
  s->write(0x7E);                                                       //The read data function will return true if the data request is successful.
  s->write((byte)0x00);
  s->write(0x03);                                                       //This is the actual command
  s->write((byte)0x00);
  s->write(0xFC);
  s->write(0x7E);

 if (! s->available()){                                                 //If the given serial connection is not available, the data request will fail.
    return false;
  }

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

    for(unsigned short j = 0; j<16; j++) MassC[j] = buffers[j];         //The mass concentration data is removed from the buffer.
    for(unsigned short j = 0; j<20; j++) NumC[j] = buffers[j+16];       //The number concentration data is removed from the buffer.
    for (unsigned short j=0; j<4; j++) AvgS[j] = buffers[j+36];         //The average size data is removed from the buffer.

  return true;                                                          //If the reading is successful, the function will return true.
}

String SPS::logUpdate(){                          				        //This function will parse the data and form loggable strings.
    String dataLogLocal = nTot;   
    if (readData()){                                                    //Read the data and determine the read success.
       goodLog = true;                                                  //The data is sent in reverse. This will flip the order of every four bytes
       badLog = 0;
       nTot++;
       
unsigned short flip = 0;                                                //Index of array to flip
unsigned short result = 0;                                              //Index of array that will be the result

for (unsigned short flipMax = 4; flipMax<21; flipMax+=4){               //This will loop through the main flipping mechanism
  result = flipMax - 1;                                                 //The result starts one lower than the flipMax because of counting from zero
    for (flip; flip<flipMax; flip++){                                   //Flipping mechanism. This flips the results of the data request.
      if (flipMax < 5)  a.ASA[flip] = AvgS[result];                     //Flips average size. Average size only has four bytes.
      if (flipMax < 17) m.MCA[flip] = MassC[result];                    //Flips mass. Mass has four less bytes than number.
       n.NCA[flip] = NumC[result];                                      //Flips number count.
    result--;
   }
}

   for(unsigned short k = 0; k<4; k++){                                 //This loop will populate the data string with mass concentrations.
      if (k==0) {
         dataLogLocal += ',' + String(m.MCF[k]) + ',';                  //Each bin from the sensor includes all of the particles from the bin
        } else {                                                        //below it. This will show the number of particles that reside invidually
         dataLogLocal += String(m.MCF[k]-m.MCF[k-1]) + ',';             //in each of the four bins.
        }
   }

   for(unsigned short k = 0; k<5; k++){                                 //This loop will populate the data string with number concentrations.
      if (k==0) {
        dataLogLocal += String(n.NCF[k]) + ',';
      } else {                                          
        dataLogLocal += String(n.NCF[k]-n.NCF[k-1]) + ',';              //These bins are compiled in the same manner as the mass bins.     
      }
   }
    dataLogLocal += String(a.ASF);                                      //This adds the average particle size to the end of the bin.
    
  } else {
	 badLog ++;
	 if (badLog >= 5) goodLog = false;									//Good log situation the same as in the Plantower code
	 dataLogLocal += ",-,-,-,-,-,-,-,-,-,-";							//If there is bad data, the string is populated with failure symbols.              
	}
	return dataLogLocal;
  }

