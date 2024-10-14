#ifndef MESSAGE_PUBLISHER_H
#define MESSAGE_PUBLISHER_H

#include <stdio.h>
#include <custom.h>

char* satellite_position_data = NULL;

static size_t write_callback_sat_location(char* data, size_t size, size_t nmemb, void* clientp)
{
    size_t data_realsize = size * nmemb;
    if (satellite_position_data != NULL) {
        free(satellite_position_data);
        satellite_position_data = NULL;
    }

    satellite_position_data = (char*)malloc(data_realsize + 1);

    memcpy(satellite_position_data, data, data_realsize);
    satellite_position_data[data_realsize] = '\0';

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


//Retrieves position data for a satellite depending on its NORAD id. 
//Requires these input parameters in this order: NORAD id, Observer's latitide (decimal degrees format),
//Observer's longitude (decimal degrees format), Observer's altitude above sea level in meters, Number of future positions to return.Each second is a position.Limit 300 seconds.
//Reference at https://www.n2yo.com/api/#positions

void sat_location_api_query(char* id) {

    CURLcode res;
    curl = curl_easy_init();
    struct memory location_info = { 0 };

    char* query1 = "https://api.n2yo.com/rest/v1/satellite/positions/";
    char* query2 = "/43.651070/-79.347015/0/1/&apiKey=TN25QC-FP4PA3-NTKEUH-5BTG";

    char entry[2048];
    if (curl) {   
        snprintf(entry, sizeof(entry), "%s%s%s", query1, id, query2);
        curl_easy_setopt(curl, CURLOPT_URL, entry);     
    }

    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback_sat_location);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&location_info);

    res = curl_easy_perform(curl);
    free(location_info.response);

    curl_easy_cleanup(curl);
}

DWORD WINAPI SatelliteQueryThread(LPVOID lpParam);

bool is_complete_json(const char* sat_string) {
    const char* expected_keys[] = { "info", "positions" };
    if (strlen(sat_string) < 2) return false;

    // Check for presence of expected keys
    for (int i = 0; i < sizeof(expected_keys) / sizeof(expected_keys[0]); i++) {
        if (strstr(sat_string, expected_keys[i]) == NULL) {
            return false;
        }
    }
    return true;
}

#endif 