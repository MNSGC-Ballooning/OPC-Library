Optical Particle Counter Library

University of Minnesota - Candler MURI
Written July 2019

These are the files for the OPC library. (OPCSensor.h) This will run any
optical particle counters used for MURI that do not have independent logging capabilities.
All particle counters will need to be run in loops of different speeds.
Serial begin must be called separately for systems that run through a serial port.
The library has been optimized for Teensy 3.5/3.6, but should work on any system.

The PMS 5003 runs the read data function as fast as possible, and can
record new data every 2.3 seconds. The PMS5003 serial is 9600 baud. The Plantower has
6 data points saved in a uint16_t data type.
 
The SPS 30 runs the read data function with the record data function, and
can record new data every 1 seconds. The SPS30 serial is 115200 baud. The
SPS 30 is configured for UART communication. I2C communication is not supported.
The SPS 30 has 10 data points saved in a float data type.

The Alphasense R1 runs the read data function with the log update function,
and can record new data every 1 seconds. The R1 runs on SPI. As of the latest
update, the R1 can only run on the primary SPI bus. The Alphasense R1 has 
16 data points saved in a uint16_t data type.

The Plantower logs the number of hits, 03um, 05um, 10um, 25um, 50um, 100um.
The SPS 30 logs the number of hits, Mass Concentrations 1um, 2.5um, 4.0um, 10um, Number Concentrations 0.5um, 1um, 2.5um, 4.0um, 10um, Average Particle size.
The Alphasense R1 logs the number of hits, 00.4um, 00.7um, 01.1um, 01.5um, 01.9um, 02.4um, 03um, 04um, 05um, 06um, 07um, 08um, 09um, 10um, 11um, 12um, 12.4um.

----------Important notes----------

In order to operate the OPCs, an object must be created (with class Plantower, SPS, or R1)
AND .initOPC() must be called. After this, the particle counters will be
active and ready for use.

With the Plantower, readData must be called separately as fast as possible before a logUpdate is called.
With every other OPC, logUpdate will call readData automatically.

A full list of the available commands exists in the "Keywords" text file.

The data is passed from getData through an array.
