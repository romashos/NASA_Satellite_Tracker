#ifndef CUSTOM_H
#define CUSTOM_H

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "curl/curl.h"

#ifdef _WIN32
#include <Windows.h>  // Include Windows.h for Windows
#else
#include <unistd.h>   // Include unistd.h for Unix-like systems
#endif

#define foreach(item, array) \
    for(int keep = 1, \
            count = 0,\
            size = sizeof (array) / sizeof *(array); \
        keep && count != size; \
        keep = !keep, count++) \
      for(item = (array) + count; keep; keep = !keep)

CURL* curl;
static char* satellite_data_str = NULL;
char* serialized_satellite_data_str;
bool ready_to_send; //toggle the flag to send the prepared string over zmq
static char* sat_orbital_period = NULL;
static char* sat_orbital_energy = NULL;

typedef struct {
    double gravitational_constant;
    double earth_mass;
    double satellite_mass;
    double pi;

} Body;

typedef struct {

    int id;
    int year;
    double mean_motion;
    char name[100];
    char notes[350];
    double orbital_period;
    double orbital_energy;

} Satellite;

struct string {
    char* ptr;
    size_t len;
};

struct memory {
    char* response;
    size_t size;
};

//callback function for curl to send satellite data
static size_t write_callback(char* data, size_t size, size_t nmemb, void* clientp);

void print_help();

char* get_time();

void view_available_satellites();

void get_individual_satellite(int satellite_id);

void add_to_current_session_items(Satellite* s);

void view_current_session_items();

void delete_from_saved_satellites(int sat_id);

void initialize_body(Body* body);

Satellite* raw_data_decomposer();

void calc_orbital_period(Satellite* sat, Body body); //T = 2π / n

void calc_orbital_energy(Satellite* sat, Body body); //E = -GMm / (2r)

bool find_satellite_by_id(int sat_id);

int extract_sat_id_from_user_ans(char user_ans[128]);

void save_txt(Satellite* sat);

void calc_orbital_period(Satellite* sat, Body body);

void calc_orbital_energy(Satellite* sat, Body body);

int isProgramRunning(char* programName);

#endif