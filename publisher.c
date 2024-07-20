#include <zmq.h>
#include <assert.h>
#include <ctype.h>
#include <time.h>
#include <windows.h>

#include "include/custom.h"
#include "cJSON.h"
#include "curl/curl.h"

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

#define API_KEY "8edD3mWx97g31QaQ2v6J7ZzyoMtpFYQQNnWQEDg5" //might not need it      

int main() {
    //setup ZMQ socket
    void* context = zmq_ctx_new();
    void* publisher = zmq_socket(context, ZMQ_PUB);
    int rc = zmq_bind(publisher, "tcp://*:3000");
    assert(rc == 0);

    //setup curl & get satellite data
    CURL* curl;
    CURLcode res;
    char* query_result = NULL;
    size_t query_result_len = 0;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://tle.ivanstanojevic.me/api/tle/25544");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));

        curl_easy_cleanup(curl);
    }


    while (1) {
        char* message = "Hello from publisher";
        zmq_send(publisher, message, strlen(message) + 1, 0);
    }

    zmq_close(publisher);
    zmq_ctx_destroy(context);

    return 0;
}