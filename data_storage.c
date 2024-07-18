#include <zmq.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "custom.h"

int main() {
    void* context = zmq_ctx_new();
    void* subscriber = zmq_socket(context, ZMQ_SUB);
    int rc = zmq_connect(subscriber, "tcp://localhost:3000");
    assert(rc == 0);
    zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, "", 0); // Subscribe to all topics

    while (1) {
        char buffer[1024];
        char* current_time = GetTime();

        zmq_recv(subscriber, buffer, 100, 0);
        printf("%s : Subscriber 2 received: %s\n", current_time, buffer);
     }

    zmq_close(subscriber);
    zmq_ctx_destroy(context);
    return 0;
}
