#include <time.h>

#ifdef _WIN32
#include <Windows.h>  // Include Windows.h for Windows
#else
#include <unistd.h>   // Include unistd.h for Unix-like systems
#endif

int* SimulationData() {

    static int simulation_data[100];
    int i, count = 1;

    for (i = 0; i < 100; i++) {
        int random_num = rand();
        simulation_data[i] = random_num;
    }

    return simulation_data;
}

char* GetTime() {
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);
    static char s[64]; // Make s static to avoid memory allocation issues
    size_t ret = strftime(s, sizeof(s), "%c", tm);

    if (ret > 0) {
        return s; 
    }
    else {
        return NULL; 
    }
}