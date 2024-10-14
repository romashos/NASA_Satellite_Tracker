#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <windows.h>
#include <tlhelp32.h>

#include "curl/curl.h"
#include "custom.h"
#include "slre.h"
#include "cJSON.h"

#ifdef _WIN32
#include <Windows.h>  
#else
#include <unistd.h>   
#endif

static Satellite* current_session_items[15]; // array of satellites for current session
static int current_sat_count = 0; // keeps track of the amount of satellites added to current_session_items
extern char* satellite_data_str;
extern char* serialized_satellite_data_str;
extern char* sat_orbital_period;
extern char* sat_orbital_energy;
extern bool ready_to_send;

//Curl callback function to send satellite data
static size_t write_callback(char* data, size_t size, size_t nmemb, void* clientp)
{
    size_t data_realsize = size * nmemb;
    if (satellite_data_str != NULL) {
        free(satellite_data_str);
        satellite_data_str = NULL;
    }

    satellite_data_str = (char*)malloc(data_realsize + 1);

    memcpy(satellite_data_str, data, data_realsize);
    satellite_data_str[data_realsize] = '\0';

    size_t realsize = size * nmemb;
    struct memory* mem = (struct memory*)clientp;

    char* ptr = realloc(mem->response, mem->size + realsize + 1);
    if (!ptr)
        return 0;

    mem->response = ptr;
    memcpy(&(mem->response[mem->size]), data, realsize);
    mem->size += realsize;
    mem->response[mem->size] = 0;

    return realsize;
}

char* get_time() {
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);
    static char s[64];
    // Custom format string without locale-specific formatting
    size_t ret = strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", tm);

    if (ret > 0) {
        return s;
    }
    else {
        return NULL;
    }
}

void print_help() {
    int user_ans;
    printf("Press 1 to view available satellites for analysis. This option allows you to view raw data or add satellites to current session. \n");
    printf("Press 2 to view satellites in the current session.\n");
    printf("Press 3 to start Mongo App and save satellite record to database in real time. Note that both applications will have to run for at least 6 minutes. \n" 
        "You will then be able to view the positions and perform visual analysis on the satellites through Satellite Viewer.\n");
    printf("Press 4 to launch Satellite Viewer.\n");
    printf("Press 5 to exit.\n");
}

void add_to_current_session_items(Satellite* s) {

    if (current_sat_count > 4) {
        printf("ERROR: too many items. Review your saved satellites and delete extras. Max of 5 allowed.\n");
        return;
    }

    if (find_satellite_by_id(s->id)) {
        printf("ERROR: possible duplicate. Satellite %s already exists in the saved items list.\n", s->name);
        return;
    }

    current_session_items[current_sat_count] = s;
    current_sat_count++;

    printf("\nSuccessfully added satellite %s to the list of saved satellites for the current session.\n", s->name);
}

void view_available_satellites() {

    printf("ISS (ZARYA) - 25544\n");
    printf("PROXIMA I - 43694\n");
    printf("PROXIMA II - 43696\n");
    printf("CENTAURI-1 - 43809\n");
    printf("WORLDVIEW-1 (WV-1) - 32060\n");
    printf("KKS-1 (KISEKI) - 33499\n");
    printf("ZHUHAI-1 02 (CAS-4B) - 42759\n");
    printf("DEIMOS-1 - 35681\n");
    printf("NORSAT 1 - 42826\n");
    printf("NORSAT 2 - 42828\n");

    printf("\n");
    printf("Enter satellite id and '-d' to view its raw data, or id and '-s' to add to your current session. \n");
    printf("Enter -c to view currently saved satellites.\n");
    printf("Note that the record of the saved satellites will be destroyed if you close the window. \n--> ");

    char user_ans[128]; 
    scanf("%s", user_ans);

    int sat_id;
    if (strchr(user_ans, '-c') == NULL) {
        sat_id = extract_sat_id_from_user_ans(user_ans);
        get_individual_satellite(sat_id);
    }
    else if (strchr(user_ans, '-c') != NULL) {
        view_current_session_items();
    } 

    if (strchr(user_ans, '-s') != NULL) {
        Satellite* s = raw_data_decomposer();

        printf("Would you like to calculate orbital period of the satellite?\n--> ");
        char user_ans_period[4];
        scanf("%s", user_ans_period);

        if (strchr(user_ans_period, 'ye') != NULL) {
            Body body;
            initialize_body(&body);
            calc_orbital_period(s, body);
            s->orbital_period = atof(sat_orbital_period);
            printf("The orbital period of the satellite %s is %f seconds. \n", s->name, s->orbital_period);

        }
        else {
            s->orbital_period = 0;
        }

        printf("Would you like to calculate orbital energy of the satellite?\n--> ");
        char user_ans_energy[4];
        scanf("%s", &user_ans_energy);

        if (strchr(user_ans_energy, 'ye') != NULL) {
            Body body;
            initialize_body(&body);
            calc_orbital_energy(s, body);
            char* ptr;
            s->orbital_energy = atof(sat_orbital_energy, &ptr);
            printf("The orbital energy of the satellite %s is %f Joules \n", s->name, s->orbital_energy);
        }
        else {
            s->orbital_energy = 0;
        }

        printf("Would you like to add personalized notes to this satellite?\n--> ");
        char user_ans_notes[4];
        scanf("%s", &user_ans_notes);

        while (getchar() != '\n');

        if (strstr(user_ans_notes, "no") == NULL) {
            printf("Please enter your notes. Do not press Enter unless done.\n--> ");
            fgets(s->notes, sizeof(s->notes), stdin);
            s->notes[strcspn(s->notes, "\n")] = 0; 
        }
        else {
            strcpy(s->notes, "");
        }

        //Add the retrieved values to a string so we can send over ZMQ to db_handler
        snprintf(serialized_satellite_data_str, 500, "%s, %f, %f, %s", serialized_satellite_data_str, s->orbital_period, s->orbital_energy, s->notes);
        ready_to_send = true; // flag that the string is ready to be sent over zmq

        add_to_current_session_items(s);

        printf("\nCurrent session items: \n");
        view_current_session_items();

        if (!isProgramRunning("mongo_app.exe")) {
            printf("Mongo App will open in a second to save your current session items into a database. You can minimize the Mongo App window. \n");
            Sleep(3000);
            system("start mongo_app.exe");
        }
    }
    else if (strchr(user_ans, '-d') != NULL) {
        printf("%s\n", satellite_data_str);
    }
    else {
        printf("Argument not entered. Try again. \n");
    }
}

void get_individual_satellite(int satellite_id) {

    CURLcode res;
    curl = curl_easy_init();
    struct memory satellite_info = { 0 };

    if (curl) {
        switch (satellite_id) {

        // Zarya (NORAD ID 25544) is a module of Space Station, thus the two names are used interchangeably in the project. 
        // An interesting read here:
        // https://nssdc.gsfc.nasa.gov/nmc/spacecraft/display.action?id=1998-067A#:~:text=Because%20Zarya%20was%20the%20first,will%20also%20be%20maintained%20here.
        case 25544: 
            curl_easy_setopt(curl, CURLOPT_URL, "https://tle.ivanstanojevic.me/api/tle/25544");
            break;
        case 43694:
            curl_easy_setopt(curl, CURLOPT_URL, "https://tle.ivanstanojevic.me/api/tle/43694");
            break;
        case 43696:
            curl_easy_setopt(curl, CURLOPT_URL, "https://tle.ivanstanojevic.me/api/tle/43696");
            break;
        case 43809:
            curl_easy_setopt(curl, CURLOPT_URL, "https://tle.ivanstanojevic.me/api/tle/43809");
            break;
        case 32060:
            curl_easy_setopt(curl, CURLOPT_URL, "https://tle.ivanstanojevic.me/api/tle/32060");
            break;
        case 33499:
            curl_easy_setopt(curl, CURLOPT_URL, "https://tle.ivanstanojevic.me/api/tle/33499");
            break;
        case 42759:
            curl_easy_setopt(curl, CURLOPT_URL, "https://tle.ivanstanojevic.me/api/tle/42759");
            break;
        default:
            printf("Invalid satellite id.\n");
            break;
        case 35681:  
            curl_easy_setopt(curl, CURLOPT_URL, "https://tle.ivanstanojevic.me/api/tle/35681");
            break;
        case 42826:
            curl_easy_setopt(curl, CURLOPT_URL, "https://tle.ivanstanojevic.me/api/tle/42826");
            break;
        case 42828:
            curl_easy_setopt(curl, CURLOPT_URL, "https://tle.ivanstanojevic.me/api/tle/42828");
            break;
        }

        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&satellite_info);

        res = curl_easy_perform(curl);

        free(satellite_info.response);
        curl_easy_cleanup(curl);
    }
}

void initialize_body(Body* body) {
    body->gravitational_constant = 6.67430e-11;
    body->earth_mass = 5.972e24;
    body->satellite_mass = 3000; //taken as a constant for demonstration purposes, does not reflect real mass
    body->pi = 3.141592653589793;
}

// Accesses the global satellite_data_str and converts the data into struct. 
// Additionally, populates a serialized_satellite_data_str to send to the mongo_app via zmq. 
Satellite* raw_data_decomposer() {

    Satellite* sat = (Satellite*)malloc(sizeof(Satellite));
    memset(sat, 0, sizeof(Satellite));

    size_t size = 150; //random number to ensure whatever data we get, it for sure fits into our string
    serialized_satellite_data_str = (char*)malloc(size);

    cJSON *json = cJSON_Parse(satellite_data_str);
    if (json == NULL) {
        fprintf(stderr, "Error parsing JSON while creating a satellite struct \n");
    }

    cJSON* id = cJSON_GetObjectItem(json, "satelliteId");
    if (id != NULL && cJSON_IsNumber(id)) {
        sat->id = id->valueint;
    }

    // extract year from line 1
    cJSON* line1 = cJSON_GetObjectItem(json, "line1"); 
    char* year_line;
    if (line1 != NULL && cJSON_IsString(line1)) {
        size_t length = strlen(line1->valuestring) + 1; 
        year_line = malloc(length);
        strncpy(year_line, line1->valuestring, length);
        year_line[length - 1] = '\0';
    }

    char* year = malloc(4);
    memcpy(year, &year_line[8], 3);
    year[4] = '\0';

    sat->year = atof(year);

    // extract mean motion
    cJSON* line2 = cJSON_GetObjectItem(json, "line2");

    char* mmotion_line;
    if (line2 != NULL && cJSON_IsString(line2)) {
        size_t length = strlen(line2->valuestring) + 1;
        mmotion_line = malloc(length);
        strncpy(mmotion_line, line2->valuestring, length);
        mmotion_line[length - 1] = '\0';
    }

    char* mmotion = malloc(8);
    memcpy(mmotion, &mmotion_line[51], 7);

    sat->mean_motion = atof(mmotion);

    // extract name
    cJSON* name = cJSON_GetObjectItem(json, "name");
    if (name != NULL && cJSON_IsString(name)) {
        strcpy(sat->name, name->valuestring);
        sat->name[sizeof(sat->name) - 1] = '\0';
    }

    //Add the retrieved values to a string so we can send over ZMQ to mongo_app
    snprintf(serialized_satellite_data_str, size, "%d,%s,%s, %s", sat->id, year, mmotion, sat->name);

    cJSON_Delete(json);
    return sat;
};

void delete_from_saved_satellites(int sat_id) {
    int i;
    bool found = false; 

    for (i = 0; i < current_sat_count; i++) {
        if (current_session_items[i] != NULL && current_session_items[i]->id == sat_id) {
            found = true;
            printf("Satellite %s with id %d deleted successfully from saved satellites list.\n", current_session_items[i]->name, sat_id);

            for (int j = i; j < current_sat_count - 1; j++) {
                current_session_items[j] = current_session_items[j + 1];
            }
            current_session_items[current_sat_count - 1] = NULL;
            current_sat_count--; 
            break; 
        }
    }

    if (!found) {
        printf("Satellite with id %d not found in the list.\n", sat_id);
    }
}

void view_current_session_items() {
    if (current_sat_count != 0) {
        for (int i = 0; i < current_sat_count; i++) { 
            if (current_session_items[i] == NULL) {
                printf("Satellite at index %d is NULL, skipping...\n", i);
                continue; 
            }
            printf("%d satellite in the list: \n", i + 1); 
            printf("%s\n", current_session_items[i]->name);
            printf("%d\n", current_session_items[i]->id);
            printf("%d\n", current_session_items[i]->year);
            printf("%f\n", current_session_items[i]->mean_motion);
            printf("%f\n", current_session_items[i]->orbital_period);
            printf("%f\n", current_session_items[i]->orbital_energy);
            printf("%s\n", current_session_items[i]->notes);
            printf("\n");
        }

        printf("To delete a satellite from a saved list, enter its satellite id and -d. \n");
        printf("To save a satellite record to a text file, enter its satellite id and -s. \n");
        printf("To return to main menu and commit your changes to the database, enter any other key. \n");

        char user_ans[10];
        scanf("%s", user_ans);

        int sat_id = extract_sat_id_from_user_ans(user_ans);
        if (strstr(user_ans, "-d") != NULL) {
            delete_from_saved_satellites(sat_id);
        }
        else if (strstr(user_ans, "-s") != NULL) {
            for (int i = 0; i < current_sat_count; i++) { 
                if (current_session_items[i]->id == sat_id) {
                    save_txt(current_session_items[i]);
                }
            }
        }
    }
    else {
        printf("The list is empty!\n");
    }
}

bool find_satellite_by_id(int sat_id) {

    if (current_sat_count != 0) {

        for (int i = 0; i < current_sat_count; i++) {
            if (current_session_items[i]->id == sat_id) {
                return true;
            }
        }
    }
    return false;
}

int extract_sat_id_from_user_ans(char user_ans[128]) {

    char* str = user_ans;

    //Extract satellite id from user input
    static const char* regex_test = "^\\s*(\\S*)";
    struct slre_cap caps[2];
    int a, b = 0, length = strlen(str);

    a = slre_match(regex_test, str + b, length - b, caps, 2, SLRE_IGNORE_CASE);
    size_t buffer = 10;
    char* extracted_sat_id = (char*)malloc(buffer);
    int sat_id = 0; //set as default

    if (a > 0) {

        int written = snprintf(extracted_sat_id, buffer, "%.*s", caps[0].len, caps[0].ptr);

        if (written < 0) {
            perror("failed extracting the satellite id from input. \n");
            free(buffer);
        }
        else if (written >= buffer) {
            perror("buffer is too small. Satellite id might be truncated. \n");
            free(buffer);
        }
        sscanf(extracted_sat_id, "%d", &sat_id);
    }

    return sat_id;
}

void save_txt(Satellite* s) {

    FILE* fptr;

    char name[150];
    strcpy(name, s->name);
    strcat(name, ".txt");

    fptr = fopen(name, "w");
    fprintf(fptr, "Name: %s\n", s->name);
    fprintf(fptr, "Year: %d\n", s->year);
    fprintf(fptr, "Satellite id: %d\n", s->id);
    fprintf(fptr, "Mean Motion: %f\n", s->mean_motion);
    fprintf(fptr, "Orbital Period: %f\n", s->orbital_period);
    fprintf(fptr, "Orbital Energy: %f\n", s->orbital_energy);
    fprintf(fptr, "Notes: %s\n", s->notes);
    fprintf(fptr, "\n");

    fclose(fptr);

    printf("Satellite data has been written to a .txt file. \n");
}

void calc_orbital_period(Satellite* sat, Body body) {

    double period = (2 * body.pi) / sat->mean_motion;
    if (sat_orbital_period != NULL) {
        free(sat_orbital_period);
        sat_orbital_period = NULL;
    }

    sat_orbital_period = (char*)malloc(16);
    snprintf(sat_orbital_period, (sizeof(double) + 1), "%f", period);
};

void calc_orbital_energy(Satellite* sat, Body body) {

    calc_orbital_period(sat, body);

    double cubed_orb_rad = (body.gravitational_constant * body.earth_mass * *sat_orbital_period) / (4 * pow(body.pi, 2));
    double orb_rad = cbrt(cubed_orb_rad);

    double orbital_energy = -(body.gravitational_constant * body.earth_mass * body.satellite_mass) / (2 * orb_rad);

    if (sat_orbital_energy != NULL) {
        free(sat_orbital_energy);
        sat_orbital_energy = NULL;
    }

    sat_orbital_energy = (char*)malloc(16);
    snprintf(sat_orbital_energy, (sizeof(double) + 1), "%f", orbital_energy);
}

int isProgramRunning(char* programName) {
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;
    int isRunning = 0;

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        return 0; 
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hProcessSnap, &pe32)) {
        do {
            if (_stricmp(pe32.szExeFile, programName) == 0) {
                isRunning = 1; // Program is running
                break;
            }
        } while (Process32Next(hProcessSnap, &pe32));
    }

    CloseHandle(hProcessSnap);
    return isRunning;
}

