#include <zmq.h>
#include <assert.h>
#include <ctype.h>
#include <time.h>
#include <windows.h>

#include "include/custom.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

int main() {
    //setup ZMQ socket
    void* context = zmq_ctx_new();
    void* publisher = zmq_socket(context, ZMQ_PUB);
    int rc = zmq_bind(publisher, "tcp://*:3000");
    assert(rc == 0);

    //generate simulation data
    //int* array;
    //array = SimulationData(); //need to apply it

    while (1) {
        char* message = "Hello from publisher";
        zmq_send(publisher, message, strlen(message) + 1, 0);
    }

    zmq_close(publisher);
    zmq_ctx_destroy(context);

    return 0;
}