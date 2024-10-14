#include <zmq.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "db_connector.h"

#define CURL_STATICLIB
#define BUFFER_SIZE 1024

#ifdef _DEBUG
#pragma comment(lib, "../lib/curl/libcurl_a_debug.lib")
#else 
#pragma comment (lib, "../lib/curl/libcurl_a.lib")
#endif

DWORD WINAPI TleQueryThread(LPVOID lpParam);
void* tle_getter = NULL;
void* position_getter = NULL;
void* context = NULL;

int main() {
    // Establish connection to MongoDB
    start_db_connection();

    // Set up ZMQ sockets
    context = zmq_ctx_new();
    position_getter = zmq_socket(context, ZMQ_SUB);

    // Position getter receives data for satellites from background thread every 5 minutes
    int rc_position = zmq_connect(position_getter, "tcp://localhost:3000");
    if (rc_position != 0) {
        logger("Failed to connect position getter");
        return 1;
    }
    zmq_setsockopt(position_getter, ZMQ_SUBSCRIBE, "", 0);

    // TLE getter receives data for satellites that the user specifically added to current_session_items
    tle_getter = zmq_socket(context, ZMQ_REQ);
    int rc_tle = zmq_connect(tle_getter, "tcp://localhost:5555");
    if (rc_tle != 0) {
        logger("Failed to connect to TLE getter.");
        system("pause");
        return 1;
    }

    logger("ZMQ connection has been established.");

    HANDLE threads[2];
    DWORD threadID[2];
    int threadParams[2] = { 1, 2 };

    // Create the first thread
    threads[0] = CreateThread(
        NULL,
        0,
        TleQueryThread,
        &threadParams[0],
        0,
        &threadID[0]
    );

    if (threads[0] == NULL) {
        logger("Failed to create thread which receives TLE data for the satellites.");
        return 1;
    }

    while (1) {
        int msg_size = zmq_recv(position_getter, received_from_publisher, sizeof(received_from_publisher) - 1, 0);
        if (msg_size == -1) {
            logger("Failed to receive satellite position data.");
            continue;
        }
        else {
            received_from_publisher[msg_size] = '\0'; 
            logger("Receiving satellite position data...");
            logger(received_from_publisher);

            PositionData* loc = (PositionData*)malloc(sizeof(PositionData));
            insert_into_position_col(loc_struct_assembler());

            free(loc);

            logger("Database service is running...");
            printf("Database service is running... \n");
        }
    }

    // Cleanup
    zmq_close(position_getter);
    zmq_close(tle_getter);
    zmq_ctx_destroy(context);

    system("pause");
    return 0;
}

DWORD WINAPI TleQueryThread(LPVOID lpParam) {
    int threadNumber = *((int*)lpParam);

    while (1) {

        // Send request
        char* acknowledgement = "Mongo App is up & waiting to receive TLE data";
        int rc = zmq_send(tle_getter, acknowledgement, strlen(acknowledgement), 0);
        if (rc == -1) {
            logger("Failed to send request.");
            continue;
        }

        int msg_size = zmq_recv(tle_getter, tle_sat_string, sizeof(tle_sat_string), 0);
        if (msg_size == -1) {
            logger("Failed to receive TLE data");
            continue; 
        }
        else {
            tle_sat_string[msg_size] = '\0'; 
            logger(tle_sat_string);
        }

        Satellite* sat = sat_struct_assembler();
        if (sat == NULL) {
            logger("Failed to assemble satellite structure.");
            continue;
        }
        insert_into_collection(EPH, sat);
        insert_into_collection(GEN, sat);

        free(sat);
    }

    return 0;
}
