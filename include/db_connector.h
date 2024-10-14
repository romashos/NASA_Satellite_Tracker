#ifndef DB_CONNECTOR_H
#define DB_CONNECTOR_H

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "curl/curl.h"
#include "custom.h"
#include "mongoc.h"
#include "bson.h"

#ifdef _WIN32
#include <Windows.h>  // Include Windows.h for Windows
#else
#include <unistd.h>   // Include unistd.h for Unix-like systems
#endif

mongoc_client_t* client;
char received_from_publisher [1024];
char tle_sat_string [1024];

typedef enum {
    GEN,
    EPH
} Collection;

typedef struct {
    char* name;
    int id;
    double latitude; // decimal degrees format
    double longitude; // decimal degrees format
    double altitude; // decimal degrees format
    double azimuth; //degrees
    double elevation; //degrees
    double right_ascension; //degrees
    double declination; //degrees

} PositionData;

void start_db_connection();

void insert_into_collection(Collection col, Satellite* sat);

void insert_into_collection(Collection col, Satellite* sat);

void insert_into_position_col(PositionData* loc);

// Logs program execution into a session_log.txt file
void logger(char log_message[150]);

// Processes the string received over ZMQ from msg publisher such that we get a reconstrcuted Satellite object in db handler
Satellite* sat_struct_assembler();

PositionData* loc_struct_assembler(); 

#endif