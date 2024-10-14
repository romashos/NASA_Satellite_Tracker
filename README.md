
OVERVIEW 
The project NASA Satellite Tracker was created as a fun side project to demonstrate the ability to implement
various communication or GUI libraries in C language.  It uses modular approach and thus, contains three executables.
Two of these (satellite_viewer.exe and message_publisher.exe) include user interaction layer (GUI and command line 
interface, correspondingly). 
The project was created for practical purposes only, and I did not intend it to be designed for a practical 
real-life application. However, it could easily be implemented at schools or for educational expo's / demonstrations, or simply
used by astronomy enthusiasts like myself :) 

- message_publisher.exe
This binary uses curl and ZMQ to query for the satellite TLE data string & process it as per user request. The user can save the satellite struct in the current session,
save the data to .txt file, view raw jSON string, save to MongoDB, or delete the satellite from the current session. There is also a parallel thread running in the background, 
querying and sending position data for 10 specific satellites to the mongo_app every 5 minutes. It has 2 ZMQ sockets to perform this job. They operate in PUB-SUB & REQ-RES patterns.

- mongo_app.exe
This binary receives the current session satellites and position data from the message_publisher, where it processes it and saves to the satellite_tracker database, 
into collections ephemeris, position, and general. Just as the message_publisher, mongo_app has 2 ZMQ sockets operating in PUB-SUB & REQ-RES patterns to receive data from the publisher.
It is an independent binary that receives messages, processes them and saves to the database. It is designed to run as a background process. Important events or errors are logged in the 
logger.txt - a journal file used to track the performance of the app. 

- satellite_viewer.exe
GUI application, which is designed to be used by a regular user who does not have the development background. It also uses logger functionality to track errors and run process; 
unlike message_publisher & mongo_app, it contains C and Python code (graphs & histogram data analysis). Provided that message_publisher & mongo_app were run for some time to accumulate data,
it will display the graphs and visualize satellite position on the map. If no position is available, the longitude and latitude values are set to 0.
                     
FEATURES / CHARACTERISTICS: 
GUI: GTK library
Python libs: default Python libs and header files at the installation path 
C library for HTTP requests: curl
Communication: ZMQ (over tcp, using PUB-SUB & REQ-RES) 
Database service: MongoDB 
Language of programming: C, .py 
Build tool: CMake 

*  This repository contains everything you need to successfully build the three executables (all libraries installed are prebuilt for you). 
   However, the installation section below also outlines the necessary libraries and tools required (just in case). 
   If for any reason you decide to rebuild these libraries yourself, note the paths for .lib and header files in CMakeLists.txt, or tweak them accordingly. 
** message_publisher.exe contains a command line user interaction mechanism. This "interface" exists FOR DEMO PURPOSES 
   such that the user can explore the capabilities of the app while it is doing its job sending TLE data to the mongo_app.exe. It could potentially be slightly reconfigured to 
   launch and run as background process just like mongo_app.exe does (without user intervention). For example, the process will be sending zmq messages & processing 
   satellite API data ONLY.

INSTALLATION 
1. Download & build ZMQ C library (I installed using vcpkg manager). 
2. Download & build the curl library (awesome Windows guide here: https://www.youtube.com/watch?v=q_mXVZ6VJs4). 
3. Download & build the cJSON library (https://github.com/DaveGamble/cJSON)
4. Download & build the mongoc and libbson libraries (https://github.com/mongodb/mongo-c-driver)
5. Download & build the GTK library (https://github.com/wingtk/gvsbuild). Important: do NOT install the pre-built GTK. 
   At the moment of creating the Satellite Tracker, the prebuilt GTK zip missed some crucial header files. Also, as the authors state, the pre-built package is not supported. 
6. Download Python for developers (https://www.python.org/downloads/release/python-3126/). Add to included directories in VS (or whatever IDE you are using).
7. For all builds, use CMake (in the project root, run these commands in this order: mkdir build; cd build; cmake ..; cmake --build . --config Release)

NOTES
1. Libraries used are platform agnostic, however, the project was built on Windows and currently does not have Linux support. 


FUTURE WORK
- Bug fixes
- Option to display satellites in the satellite_viewer that the user has added to the current session items 
  in the message_publisher