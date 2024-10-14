#include <zmq.h>
#include <assert.h>
#include <ctype.h>
#include <time.h>
#include <windows.h>

#include "cJSON.h"
#include "custom.h"

#include "message_publisher.h"

#define CURL_STATICLIB
#define BUFFER_SIZE 1024

#ifdef _DEBUG
#pragma comment(lib, "../lib/curl/libcurl_a_debug.lib")
#else 
#pragma comment (lib, "../lib/curl/libcurl_a.lib")
#endif

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif   

DWORD WINAPI SatelliteQueryThread(LPVOID lpParam);
void* position_publisher = NULL;
void* tle_requester = NULL;
HANDLE mutex;

int main() {
    //setup ZMQ socket
    void* context = zmq_ctx_new();
    position_publisher = zmq_socket(context, ZMQ_PUB);
    tle_requester = zmq_socket(context, ZMQ_REP);
    int rc_pub = zmq_bind(position_publisher, "tcp://*:3000");
    int rc_req = zmq_bind(tle_requester, "tcp://*:5555");
    if (rc_req != 0) {
        printf("Failed to bind tle_requester in message publisher: %s\n", zmq_strerror(errno));
        return 1;
    }
     
    //initialize variables for later use
    int user_ans;
    int input;
    ready_to_send = false;

    HANDLE threads[2]; 
    DWORD threadID[2]; 
    int threadParams[2] = { 1, 2 };

    // Create the first thread
    threads[0] = CreateThread(
        NULL,                  
        0,                     
        SatelliteQueryThread,         
        &threadParams[0],       
        0,                     
        &threadID[0]            
    );

    if (threads[0] == NULL) {
        fprintf(stderr, "Failed to create thread 1: %d\n", GetLastError());
        return 1;
    }

    printf("Welcome to NASA Satellite data analysis project.\nTo view available satellites for analysis, please press 1. For help, please press 0.\n");

    while (1) {

        printf("--> ");

        if (scanf("%d", &user_ans) != 1) {
            printf("Invalid input! Please enter a digit.\n");
            while (getchar() != '\n');
            continue; 
        }

        switch (user_ans) {
        case 0:
            print_help();
            break;
        case 1:
            view_available_satellites(); // "entry-point"
            break;
        case 2:
            view_current_session_items();
            break;
        case 3:
            system("start mongo_app.exe");
            break;
        case 4: 
            system("start satellite_viewer.exe");
            break;
        case 5:
            printf("Thank you for using NASA Satellite data analysis project");
            zmq_close(position_publisher);
            zmq_close(tle_requester);
            zmq_ctx_destroy(context);
            system("taskkill /IM mongo_app.exe /F");
            exit(0);
            break;
        default:
            print_help();
            break;
        }

        if (serialized_satellite_data_str != NULL && ready_to_send == true) {

            char mongo_app_is_up_acknowldgement[254];
            zmq_recv(tle_requester, mongo_app_is_up_acknowldgement, sizeof(mongo_app_is_up_acknowldgement), 0);
            mongo_app_is_up_acknowldgement[strlen(mongo_app_is_up_acknowldgement)] = '\0';
            printf("%s\n", mongo_app_is_up_acknowldgement);

            zmq_send(tle_requester, serialized_satellite_data_str, strlen(serialized_satellite_data_str), 0);

            serialized_satellite_data_str = NULL;
        }
    }

    WaitForMultipleObjects(2, threads, TRUE, INFINITE);
    CloseHandle(threads[0]);
    CloseHandle(threads[1]);

    return 0;
}

//Get real-time satellite position and send to db handler, which saves data in db.
//Ideally the app never exits, and gets us record of queries for satellite location every 5 minutes (change to bigger time interval as needed)

DWORD WINAPI SatelliteQueryThread(LPVOID lpParam) {
    int threadNumber = *((int*)lpParam);
    while (1) {

        // Call function to get location data for each satellite
        char sat_id_array[10][6] = { "25544" , "43694" , "43696" , "43809" , "32060" , "33499" , "42759" , "35681" , "42826" , "42828" };
        foreach(char* id, sat_id_array) {

            sat_location_api_query(id);

            if (satellite_position_data != NULL) {
                // Check if the JSON string is complete. Sometimes API call returns a cut off jSON which makes the program bail (specifically for Proxima's).
                if (is_complete_json(satellite_position_data)) {
                    zmq_send(position_publisher, satellite_position_data, strlen(satellite_position_data) + 1, 0);
                }
                else {
                    //printf("Received incomplete JSON: %s. This satellite data will NOT be saved. \n", satellite_position_data);
                }
                satellite_position_data = NULL;  
            }
            else {
                printf("Error querying for position data \n");
            }
        }

        Sleep(5 * 60 * 1000);
    }

    return 0;
}

