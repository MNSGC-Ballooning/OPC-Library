Optical Particle Counter Library

University of Minnesota - Candler MURI
Written July 2019

This is the definitions file for the OPC library.
This will run any optical particle counters used for MURI.
All particle counters will need to be run in loops of different speeds.
Serial begin must be called separately.

The PMS 5003 runs the read data function as fast as possible, and can
record new data every 2.3 seconds.
 
The SPS 30 runs the read data function with the record data function, and
can record new data every 1 seconds.

The Plantower logs the number of hits, 03, 05, 10, 25, 50, 100.
The SPS 30 logs the number of hits, Mass Concentrations PM1, PM2.5, PM4.0, PM10, Number Concentrations PM0.5, PM1, PM2.5, PM4.0, PM10, Average Particle size
