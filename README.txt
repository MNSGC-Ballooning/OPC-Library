Optical Particle Counter Library (OPCSensor)

University of Minnesota - Candler MURI, MNSGC High Altitude Ballooning Team
Written Summer 2019

These are the files for the OPC library (OPCSensor.h). This will run any
optical particle counters used for MURI that do not have independent logging capabilities.
All particle counters will need to be run in loops of different speeds.
Serial begin must be called separately for systems that run through a serial port.
The library has been optimized for Teensy 3.5/3.6, but should work on any system.
A count of total successful hits precedes the data from the sensors.



----------Sensor Overview----------




The Plantower PMS 5003 runs the read data function as fast as possible, and can
record new data every 2.3 seconds. The PMS5003 serial is 9600 baud. The Plantower has
6 data points.
 
The Sensirion SPS 30 runs the read data function with the record data function, and
can record new data every 1 seconds. The SPS30 serial is 115200 baud. The
SPS 30 is configured for UART communication. I2C communication is not supported.
The SPS 30 has 10 data points.

The Alphasense R1 runs the read data function with the log update function,
and can record new data every 1 seconds. The R1 runs on SPI. As of the latest
update, the R1 can only run on the primary SPI bus. The Alphasense R1 has 
16 data points.

The Honeywell HPMA115S0-004 runs the read data function with the log update function,
and can record new data every 1 seconds. The HPM serial is 9600 baud. The HPM
has 4 data points.

The Plantower logs the number of hits, 03um, 05um, 10um, 25um, 50um, 100um.
The SPS 30 logs the number of hits, Mass Concentrations 1um, 2.5um, 4.0um, 10um, Number Concentrations 0.5um, 1um, 2.5um, 4.0um, 10um, Average Particle size.
The Alphasense R1 logs the number of hits, 00.4um, 00.7um, 01.1um, 01.5um, 01.9um, 02.4um, 03um, 04um, 05um, 06um, 07um, 08um, 09um, 10um, 11um, 12um, 12.4um.
The HPM logs the number of hits, 1um, 2.5um, 4.0um, 10um.



----------Important Notes----------



Each bin will contain the particles of the bin below it in addition to the particles exclusively within the bin.
With the exclusion of the R1, this will not be done within the code.

In order to operate the OPCs, an object must be created (with class Plantower, SPS, R1, or HPM)
AND .initOPC() must be called. After this, the particle counters will be
active and ready for use.

With the Plantower, .readData() must be called separately as fast as possible before a .logUpdate() is called.
With every other OPC, .logUpdate() will call .readData() automatically.

The data is passed from .getData() through a float array.

Any other data from the sensors, such as particle counter statuses or other data arrangements, are not stored.

Any questions, comments, or concerns should be directed towards Nathan Pharis <nathan.pharis@gmail.com> or
the current library maintainer.

The Plantower code is a modified version of code that a team member found on Github. The original source is not known.

This code is written with the purpose of conducting high altitude ballooning experiments on micron level particles. As
a result, some functions of the sensors have been discarded as unnecessary, or are not clearly laid out. Feel free to
modify the code to suit your specific needs.



----------Commands for OPCSensor Library----------



All OPC:
 - construct with a reference to the serial port name (&serialName), and separately begin the serial connection.
 - .getTot() - returns total number of hits (int)
 - .getLogQuality() - returns the quality of the log (bool)
 - .initOPC() - will initialize the OPC (void)
 - .CSVHeader() - will provide a header for the logUpdate data string (String)
 - .logUpdate() - will return a data string in CSV format (String)
 - .readData() - will read the data and return a bool indicating success (bool)
 - .getData(array, arraySize) - will save data as floats into the passed arrays
 - .getData(array, arraySize, arrayStart) - will save data as floats into passed arrays starting at the array start
 - .setReset(int) - will manually set the automatic bad log reset time. (void) The default is 20 minutes of constantly poor logging. This will cause a time delay 
					of approximately 20 seconds in the code operation.

Classes:
Plantower
- add an additional log rate in milliseconds as a second parameter to the constructor

SPS
- .powerOn() - used to start measurements (void) (called by initOPC)
- .powerOff() - used to end measurements (void)
- .clean() - used to clean the system (void) (called by initOPC)

R1
- constructed with a slave pin instead of a serial line
- .powerOn() - used to start full system (void) (called by initOPC)
- .powerOff() - used to end measurements (void)

HPM
- .powerOn() - used to turn on the system and to start measurements (void) (called by initOPC)
- .powerOff() - used to end measurements and to turn off the system (void)
