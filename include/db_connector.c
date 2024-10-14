#include "db_connector.h"
#include "cJSON.h"
#include "custom.h"

#define MY_MONGODB_URI "mongodb+srv://Admin:BVkPfGqEYCgKQz3j@sofiiaapi.vtswuz7.mongodb.net/?retryWrites=true&w=majority&appName=sofiiaAPI"
bson_error_t error;

void start_db_connection ()
{
    const char* uri_string = MY_MONGODB_URI;
    mongoc_uri_t* uri;
    bson_t* command, reply;
    mongoc_database_t* database;
    char* str;

    char** collection_names;
    unsigned i;

    bool retval;

    logger("Starting Mongo Database service...");

    mongoc_init();

    uri = mongoc_uri_new_with_error(uri_string, &error);
    if (!uri) {
        fprintf(stderr,
            "failed to parse URI: %s\n"
            "error message:       %s\n",
            uri_string,
            error.message);
        
        size_t size = strlen(error.message) + 100;
        char* log_entry = (char*)malloc(size);

        char* log = "Error parsing database URI:";
        snprintf(log_entry, size, "%s %s", log, error.message);
        logger(log_entry);
    }

    client = mongoc_client_new_from_uri(uri);
    if (!client) {

        size_t size = strlen(error.message) + 100;
        char* log_entry = (char*)malloc(size);

        char* log = "Error registering database client:";
        snprintf(log_entry, size, "%s %s", log, error.message);
        logger(log_entry);

        return EXIT_FAILURE;
    }

    command = BCON_NEW("ping", BCON_INT32(1));
    retval = mongoc_client_command_simple(client, "admin", command, NULL, &reply, &error);
    if (!retval) {
        size_t size = strlen(error.message) + 100;
        char* log_entry = (char*)malloc(size);

        char* log = "Error pinging database: ";
        snprintf(log_entry, size, "%s %s", log, error.message);
        logger(log_entry);

        return EXIT_FAILURE;
    }
    str = bson_as_json(&reply, NULL);

    logger("Pinged your deployment. You are now successfully connected to Satellite Mongo Database.");

    database = mongoc_client_get_database(client, "satellite_record");

    if ((collection_names = mongoc_database_get_collection_names_with_opts(
        database, NULL, &error))) {

        char* log = "Successfully retrieved collection:";

        for (i = 0; collection_names[i]; i++) {

            size_t size = strlen(collection_names[i]) + 102;
            char* entry = (char*)malloc(size);

            snprintf(entry, size, "%s %s", log, collection_names[i]);
            logger(entry);
        }
    }
    else {

        size_t size = strlen(error.message) + 100;
        char* log_entry = (char*)malloc(size);

        char* log = "Error retrieving collections:";
        snprintf(log_entry, size, "%s %s", log, error.message);
        logger(log_entry);
    }
}

//note for future self - see why when I declare bson variables outside if, memory allocation fails
void insert_into_collection(Collection col, Satellite* sat) {

    if (col == GEN) {

        bson_t* doc;
        bson_oid_t oid;
        doc = bson_new();
        bson_oid_init(&oid, NULL);
        mongoc_collection_t* collection;

        BSON_APPEND_OID(doc, "_id", &oid);

        collection = mongoc_client_get_collection(client, "satellite_record", "general");
        
        BSON_APPEND_UTF8(doc, "Name", sat->name);

        char str_id[10];
        sprintf(str_id, "%d", sat->id);
        BSON_APPEND_UTF8(doc, "Satellite ID", str_id);

        char str_year[10];
        sprintf(str_year, "%d", sat->year);
        BSON_APPEND_UTF8(doc, "Launch year", str_year);

        BSON_APPEND_UTF8(doc, "Registered", get_time());

        if (!mongoc_collection_insert_one(
            collection, doc, NULL, NULL, &error)) {

            size_t size = strlen(error.message) + 100;
            char* log_entry = (char*)malloc(size);

            char* log = "Error appending to database:";
            snprintf(log_entry, size, "%s %s", log, error.message);
            logger(log_entry);
        }
        else {
            logger("Inserted into GENERAL collection.");
        }

        bson_destroy(doc);
        mongoc_collection_destroy(collection);

    } else if (col == EPH) {
        logger("Line142");
        bson_t* doc;
        bson_oid_t oid;
        doc = bson_new();
        bson_oid_init(&oid, NULL);
        mongoc_collection_t* collection;

        BSON_APPEND_OID(doc, "_id", &oid);

        bson_t child;

        collection = mongoc_client_get_collection(client, "satellite_record", "ephemeris");

        BSON_APPEND_DOCUMENT_BEGIN(doc, "Satellite", &child);
        BSON_APPEND_UTF8(&child, "Name", sat->name);
        char str_id[10];
        sprintf(str_id, "%d", sat->id);
        BSON_APPEND_UTF8(&child, "ID", str_id);
        bson_append_document_end(doc, &child);

        char mmotion_arr[10];
        sprintf(mmotion_arr, "%2.13f", sat->mean_motion);
        BSON_APPEND_UTF8(doc, "Mean Motion", mmotion_arr);

        BSON_APPEND_UTF8(doc, "Personal Notes", sat->notes);

        char orbper_arr[10];
        sprintf(orbper_arr, "%2.13f", sat->orbital_period);
        BSON_APPEND_UTF8(doc, "Orbital Period", orbper_arr);

        char orben_arr[10];
        sprintf(orben_arr, "%2.13f", sat->orbital_energy);
        BSON_APPEND_UTF8(doc, "Orbital Energy", orben_arr);

        BSON_APPEND_UTF8(doc, "Registered", get_time());

        if (!mongoc_collection_insert_one(
            collection, doc, NULL, NULL, &error)) {

            size_t size = strlen(error.message) + 100;
            char* log_entry = (char*)malloc(size);

            char* log = "Error appending to database:";
            snprintf(log_entry, size, "%s %s", log, error.message);
            logger(log_entry);
        }
        else {
            logger("Inserted into EPHEMERAL collection.");
        }

        bson_destroy(doc);
        mongoc_collection_destroy(collection);
    }
}

void logger(char log_message[150]) {

    char* current_time = get_time();

    FILE* file = fopen("session_log.txt", "r");
    if (file != NULL) {
        fclose(file); // file exists
    }
    else {
        FILE* file = fopen("session_log.txt", "w"); //create file otherwise
    }
 
    file = fopen("session_log.txt", "a");
    if (file == NULL) {
        perror("Error opening file");
    }

    size_t size = strlen(current_time) + 102;
    char* log_entry = (char*)malloc(size);

    snprintf(log_entry, size, "%s %s", current_time, log_message);

    fprintf(file, "%s\n", log_entry);
    fclose(file);
}

Satellite* sat_struct_assembler() {

    Satellite* sat = (Satellite*)malloc(sizeof(Satellite));
    char* tokens[7]; 
    int token_count = 0;

    char* token = strtok(tle_sat_string, ",");

    while (token != NULL && token_count <= 6) {
        while (*token == ' ') token++;
        tokens[token_count] = token;
        printf("Token: %s \n", token);
        token_count++;
        token = strtok(NULL, ",");
    }
     
     if (token_count == 7) {

        sat->id = atoi(tokens[0]);
        sat->year = atoi(tokens[1]);
        sat->mean_motion = atof(tokens[2]);

        strncpy(sat->name, tokens[3], sizeof(sat->name) - 1);
        sat->name[sizeof(sat->name) - 1] = '\0'; 

        sat->orbital_period = atof(tokens[4]);
        sat->orbital_energy = atof(tokens[5]);

        strncpy(sat->notes, tokens[6], sizeof(sat->notes) - 1);
        sat->notes[sizeof(sat->notes) - 1] = '\0';

        logger("Processed satellite string.");
    }
    else {
        logger("Not enough data provided.");
    }

    return sat;
}

void insert_into_position_col(PositionData* loc) {

    bson_t* doc;
    bson_oid_t oid;
    doc = bson_new();
    bson_oid_init(&oid, NULL);
    mongoc_collection_t* collection;

    BSON_APPEND_OID(doc, "_id", &oid);

    collection = mongoc_client_get_collection(client, "satellite_record", "position");

    BSON_APPEND_UTF8(doc, "Name", loc->name);

    char str_id[10];
    sprintf(str_id, "%d", loc->id);
    BSON_APPEND_UTF8(doc, "Satellite ID", str_id);

    char str_lat[10];
    sprintf(str_lat, "%f", loc->latitude);
    BSON_APPEND_UTF8(doc, "Latitude", str_lat);

    char str_long[10];
    sprintf(str_long, "%f", loc->longitude);
    BSON_APPEND_UTF8(doc, "Longitude", str_long);

    char str_alt[10];
    sprintf(str_alt, "%f", loc->altitude);
    BSON_APPEND_UTF8(doc, "Altitude", str_alt);

    char str_azimuth[10];
    sprintf(str_azimuth, "%f", loc->azimuth);
    BSON_APPEND_UTF8(doc, "Azimuth", str_azimuth);

    char str_elev[10];
    sprintf(str_elev, "%f", loc->elevation);
    BSON_APPEND_UTF8(doc, "Elevation", str_elev);

    char str_ar[10];
    sprintf(str_ar, "%f", loc->right_ascension);
    BSON_APPEND_UTF8(doc, "Right Ascension", str_ar);

    char str_dec[10];
    sprintf(str_dec, "%f", loc->declination);
    BSON_APPEND_UTF8(doc, "Declination", str_dec);

    BSON_APPEND_UTF8(doc, "Registered", get_time());

    if (!mongoc_collection_insert_one(
        collection, doc, NULL, NULL, &error)) {

        size_t size = strlen(error.message) + 100;
        char* log_entry = (char*)malloc(size);

        char* log = "Error appending to database:";
        snprintf(log_entry, size, "%s %s", log, error.message);
        logger(log_entry);
        free(log_entry);
    }
    else {
        
        char* log = "is inserted into POSITION collection.";
        size_t size = strlen(loc->name) + 39;
        char* entry = (char*)malloc(size);
        snprintf(entry, size, "%s %s", loc->name, log);
        logger(entry);
        free(entry);
    }

    //bson_destroy(doc);
    bson_destroy_with_steal(doc, true, NULL);
    mongoc_collection_destroy(collection);
}

PositionData* loc_struct_assembler() {

    PositionData* loc = (PositionData*)malloc(sizeof(PositionData));

    cJSON* json = cJSON_Parse(received_from_publisher);
    if (json == NULL) {
        char* log = "Unable to parse satellite data:";
        size_t size = strlen(log) + strlen(received_from_publisher) + 1;
        char* entry = (char*)malloc(size);
        snprintf(entry, size, "%s %s", log, received_from_publisher);
        logger(entry);

        loc->name = "Unknown";
        loc->id = 0;
        loc->altitude = 0;
        loc->latitude = 0;
        loc->longitude = 0;
        loc->azimuth = 0;
        loc->elevation = 0;
        loc->declination = 0;
        loc->right_ascension = 0;
    }
    else {
        cJSON* info = cJSON_GetObjectItemCaseSensitive(json, "info");
        if (info != NULL && cJSON_IsObject(info)) {

            // extract and assign loc->name value
            cJSON* satname = cJSON_GetObjectItemCaseSensitive(info, "satname");
            loc->name = satname->valuestring;

            // extract and assign sat id
            cJSON* satid = cJSON_GetObjectItemCaseSensitive(info, "satid");
            loc->id = satid->valueint;
        }

        cJSON* positions = cJSON_GetObjectItemCaseSensitive(json, "positions");
        if (positions != NULL && cJSON_IsArray(positions)) {

            // extract and assign sat latitude
            cJSON* lat_item = cJSON_GetArrayItem(positions, 0);
            cJSON* satlatitude = cJSON_GetObjectItemCaseSensitive(lat_item, "satlatitude");
            loc->latitude = satlatitude->valuedouble;

            // extract and assign sat longitude
            cJSON* long_item = cJSON_GetArrayItem(positions, 0);
            cJSON* satlongitude = cJSON_GetObjectItemCaseSensitive(long_item, "satlongitude");
            loc->longitude = satlongitude->valuedouble;

            // extract and assign sat altitude
            cJSON* alt_item = cJSON_GetArrayItem(positions, 0);
            cJSON* satlaltitude = cJSON_GetObjectItemCaseSensitive(alt_item, "sataltitude");
            loc->altitude = satlaltitude->valuedouble;

            // extract and assign sat azimuth
            cJSON* az_item = cJSON_GetArrayItem(positions, 0);
            cJSON* satazimuth = cJSON_GetObjectItemCaseSensitive(az_item, "azimuth");
            loc->azimuth = satazimuth->valuedouble;

            // extract and assign sat elevation
            cJSON* el_item = cJSON_GetArrayItem(positions, 0);
            cJSON* elevation = cJSON_GetObjectItemCaseSensitive(el_item, "elevation");
            loc->elevation = elevation->valuedouble;

            // extract and assign right_ascension
            cJSON* ra_item = cJSON_GetArrayItem(positions, 0);
            cJSON* ra = cJSON_GetObjectItemCaseSensitive(ra_item, "ra");
            loc->right_ascension = ra->valuedouble;

            // extract and assign declination
            cJSON* dec_item = cJSON_GetArrayItem(positions, 0);
            cJSON* declination = cJSON_GetObjectItemCaseSensitive(dec_item, "dec");
            loc->declination = declination->valuedouble;

            char* log = "Successfully parsed satellite data for";
            size_t size = strlen(loc->name) + 41;
            char* entry = (char*)malloc(size);
            snprintf(entry, size, "%s %s", log, loc->name);
            logger(entry);
        }

        cJSON_Delete(json);  
    }
    memset(received_from_publisher, 0, sizeof(received_from_publisher));

    return loc;
}

