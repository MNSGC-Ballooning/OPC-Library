# OPCSensor Library
This is the repository for the OPCSensor library. Currently the Sensirion SPS30, Alphasense R1, Honeywell HPMAC0115-004, and the Plantower 5003 are integrated in this library.
The library contains a separate read me that provides an in depth explanation of the library and its functionality.

## Sensor Overview
The Plantower PMS 5003 runs the read data function as fast as possible, and can record new data every 2.3 seconds. The PMS5003 serial is 9600 baud. The Plantower has 7 data points, logging the number of hits, Number Concentrations 0.3um, 0.5um, 1.0um, 2.5um, 5.0um, 10.0um.
 
The Sensirion SPS 30 runs the read data function with the record data function, and can record new data every 1 seconds. The SPS30 serial is 115200 baud. The SPS 30 is configured for UART communication. I2C communication is not supported. The SPS 30 has 11 data points, logging the number of hits, Mass Concentrations 1um, 2.5um, 4.0um, 10um, Number Concentrations 0.5um, 1um, 2.5um, 4.0um, 10um, Average Particle size.

The Alphasense R1 runs the read data function with the log update function, and can record new data every 1 seconds. The R1 runs on SPI. As of the latest update, the R1 can only run on the primary SPI bus. The Alphasense R1 has 28 data points, logging the number of hits,Number Concentrations 00.4um, 00.7um, 01.1um, 01.5um, 01.9um, 02.4um, 03um, 04um, 05um, 06um, 07um, 08um, 09um, 10um, 11um, 12um, 12.4um and more.

The Honeywell HPMA115S0-004 runs the read data function with the log update function, and can record new data every 1 seconds. The HPM serial is 9600 baud. The HPM has 5 data points, logging the number of hits, Mass Concentrations 1um, 2.5um, 4.0um, 10um.

## Important Notes
Each bin will contain the particles of the bin below it in addition to the particles exclusively within the bin. With the exclusion of the R1, this will not be done within the code.

In order to operate the OPCs, an object must be created (with class Plantower, SPS, R1, or HPM) AND .initOPC() must be called. After this, the particle counters will be active and ready for use.

With the Plantower, .readData() must be called separately as fast as possible before a .logUpdate() is called. With every other OPC, .logUpdate() will call .readData() automatically.

The data is passed from .getData() through a float array.

Any other data from the sensors, such as particle counter statuses or other data arrangements, are not stored.

Any questions, comments, or concerns should be directed towards Nathan Pharis <nathan.pharis@gmail.com> or the current library maintainer.

The Plantower code is a modified version of code that a team member found on Github. The original source is not known.

This code is written with the purpose of conducting high altitude ballooning experiments on micron level particles. As a result, some functions of the sensors have been discarded as unnecessary, or are not clearly laid out. Feel free to modify the code to suit your specific needs.

Data from the first 30 seconds of powering on the sensors will not be reliable, because the fans must reach operating speed.

## Commands
### All OPC:
 - construct with a reference to the serial port name (&serialName), and separately begin the serial connection.
 - .getTot() - returns total number of hits (int)
 - .getLogQuality() - returns the quality of the log (bool)
 - .powerOn() - used to start full system (void) (called by initOPC)
 - .powerOff() - used to end measurements (void)
 - .initOPC() - will initialize the OPC (void)
 - .CSVHeader() - will provide a header for the logUpdate data string (String)
 - .logUpdate() - will return a data string in CSV format (String)
 - .readData() - will read the data and return a bool indicating success (bool)
 - .getData(array, arraySize) - will save data as floats into the passed arrays (void)
 - .getData(array, arraySize, arrayStart) - will save data as floats into passed arrays starting deeper in both the data and the provided array (void)
 - .setReset(int) - will manually set the automatic bad log reset time (void). The default is 20 minutes of constantly poor logging. This will cause a time delay 
					of approximately 20 seconds in the code operation.

### Classes:
#### Plantower
- constructed with an additional unsigned integer representing the log rate in milliseconds.
- .passiveMode() - will require requests from the microcontroller to send data (void) (Not functional)
- .activeMode() - will spam data like there is no tomorrow (void)

#### SPS
- .clean() - used to clean the system (void) (called by initOPC)

#### R1
- constructed with a slave pin instead of a serial line.

#### HPM
- .autoSendOn() - will automatically send data to the microcontroller (void) (Not configured with logUpdate, must call readData as fast as possible)
- .autoSendOff() - will take requests from the microcontroller to send data (void) (Recommended) (called by initOPC)
