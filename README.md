nasa_satellite_tracker

OVERVIEW 
The project utilizes publish - subscribe ZMQ messaging to communicate data from NASA satellites from publisher (mimics the "satellite") to subscribers (these binaries mimic the outside computers receiving and processing the data).
The publisher uses NASA API to send data to subscribers on a given topic every minute.
First subscriber writes data blob into the database (MongoDB). 
Second subscriber utilizes .py libs (__________________) to visualize data. 
Third subscriber analyzes data (also via .py libs ____________) for standard deviation, mean, median, etc.

FEATURES / CHARACTERISTICS: 
GUI: ________ 
Python libs: _____________ 
C library for HTTP requests: curl
Communication: ZMQ (tcp, PUB-SUB) 
Database: MongoDB 
Language of programming: C, .py 
Build tool: CMake

INSTALLATION 
1. To install the project on your computer, you need to build zmq C library (I installed using vcpkg manager). 
2. Use CMake (in the project root, run these commands in this order: mkdir build; cd build; cmake ..; cmake --build . --config Release)
3. Download & build the curl library (awesome Windows guide here: https://www.youtube.com/watch?v=q_mXVZ6VJs4). 
4. You might get an error when trying to launch either one of the built binaries that libzmq-mt-4_3_5.dll is missing. Locate it in you vcpkg folder and copy to the same directory as the executable to resolve the error. 

NOTES
1. The project was designed to be platform agnostic, as all tools and libraries are such. However, at the current moment it was only tested on Windows in Release Mode. 
