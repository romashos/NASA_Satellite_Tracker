nasa_satellite_tracker

OVERVIEW 
The project utilizes publish - subscribe ZMQ messaging to communicate data from NASA satellites from publisher to subscribers (these binaries mimic the outside computers receiving and processing the data).
The publisher uses NASA API to send data to subscribers on a given topic every minute.
First subscriber writes data blob into the database (MongoDB). 
Second subscriber utilizes .py libs (__________________) to visualize data. 
Third subscriber analyzes data (also via .py libs ____________) for standard deviation, mean, median, etc.

FEATURES / CHARACTERISTICS: 
GUI: ________ 
Python libs: _____________ 
C library for HTTP requests: curl
Communication: ZMQ (tcp, PUB-SUB, REQ-RES) 
Database: MongoDB 
Language of programming: C, .py 
Build tool: CMake 

* This repository contains everything you need to successfully build the three executables (all libraries installed & prebuilt for you). However, the installation section below also outlines the necessary libraries and tools required (just in case). If for any reason you decide to rebuild these libraries yourself, note the paths for .lib and header files in CMakeLists.txt, or tweak them accordingly. 

** The built apps contain a command line user interaction mechanism. This "interface" exists FOR DEMO PURPOSES such that the user can explore the capabilities of each of the apps. In real life, these apps will be reconfigured to launch and run as background processes without user intervention. For example, the processes will be writing data to database, sending zmq messages, processing satellite API data, etc.

INSTALLATION 
1. Download & build ZMQ C library (I installed using vcpkg manager). 
2. Download & build the curl library (awesome Windows guide here: https://www.youtube.com/watch?v=q_mXVZ6VJs4). 
3. Download & build the cJSON library (https://github.com/DaveGamble/cJSON)
4. Download & build the mongoc and libbson libraries (https://github.com/mongodb/mongo-c-driver)
5. Download & build the GTK library (https://github.com/wingtk/gvsbuild). Important: do NOT install the pre-built GTK. At the moment of creating of Satellite Tracker, the prebuilt GTK zip missed some crucial header files. Also, as the authors state, the pre-built package is not supported. 
6. Download Python for developers (https://www.python.org/downloads/release/python-3126/). Add to included directories in VS.
7. For all builds, use CMake (in the project root, run these commands in this order: mkdir build; cd build; cmake ..; cmake --build . --config Release)
8. You might get an error when trying to launch either one of the built binaries that certain run time libraries (.dll) are missing. Locate it in you zmq vcpkg folder or library build dir's and copy to the same directory as the executable to resolve the error. Alternatively, you can also find them in .backup-dll directory, which I created specifically for such event. 

NOTES
1. Libraries used are platform agnostic, however, the project was built on Windows and currently does not have Linux support. 
