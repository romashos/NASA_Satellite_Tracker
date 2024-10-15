#include "analyzer_core.h"
          
GdkPixbuf* base_pixbuf = NULL;
cairo_surface_t* overlay_surface = NULL;

static int visual_counter = 5;

                                                                        /*      Date helper functions       */

static void format_date(char* buffer, size_t buffer_size, struct tm* tm_info) {
    strftime(buffer, buffer_size, "%Y-%m-%d", tm_info);

}

static void get_date_string(time_t t, char* buffer, size_t buffer_size) {
    struct tm tm = *localtime(&t);
    strftime(buffer, buffer_size, "%Y-%m-%d", &tm);
}

static char* get_yesterdays_date() {
    static char date_str[11];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    tm.tm_mday--;
    mktime(&tm);
    strftime(date_str, sizeof(date_str), "%Y-%m-%d", &tm);

    return date_str;
}

bson_t* create_time_period_query(int days) {
    bson_t* query = bson_new();
    time_t now;
    time(&now);

    // Calculate the start date by subtracting 'days' days from now
    time_t start_time = now - (days * 24 * 60 * 60);

    char start_date[11];
    char end_date[11];

    // Format the dates
    get_date_string(start_time, start_date, sizeof(start_date));
    get_date_string(now, end_date, sizeof(end_date));

    // Create the BSON query with date range
    bson_t* date_range = BCON_NEW(
        "$gte", BCON_UTF8(start_date),
        "$lte", BCON_UTF8(end_date)
    );

    BSON_APPEND_DOCUMENT(query, "Registered", date_range);

    return query;
}

char* get_time_period(enum Date date) {
    time_t now;
    struct tm* tm_info;
    char* result = malloc(256);

    if (result == NULL) {
        return NULL;
    }
      
    time(&now);
    tm_info = localtime(&now);

    switch (date) {
    case TODAY:
        format_date(result, 256, tm_info);
        break;

    case YESTERDAY:
        tm_info->tm_mday--;
        mktime(tm_info);
        format_date(result, 256, tm_info);
        break;

    case WEEK: {
        struct tm start = *tm_info;
        struct tm end = *tm_info;
        char start_date[256], end_date[256];

        start.tm_mday -= 6;
        mktime(&start); 
        end.tm_mday = start.tm_mday + 6;
        mktime(&end); 

        if (end.tm_mday != tm_info->tm_mday || end.tm_mon != tm_info->tm_mon || end.tm_year != tm_info->tm_year) {
            end = *tm_info;
        }

        format_date(start_date, sizeof(start_date), &start);
        format_date(end_date, sizeof(end_date), &end);

        snprintf(result, 256, "Week: %s - %s", start_date, end_date);
        break;
    }

    case MONTH: {

        time_t now;
        time(&now);

        char end_date[11];
        get_date_string(now, end_date, sizeof(end_date));

        time_t start_time = now - (30 * 24 * 60 * 60); // 30 days ago
        char start_date[11];
        get_date_string(start_time, start_date, sizeof(start_date));

        snprintf(result, 256, "Month: %s - %s", start_date, end_date);
        break;
    }

    case YEAR: {
        time_t now;
        time(&now);

        char end_date[11];
        get_date_string(now, end_date, sizeof(end_date));

        time_t start_time = now - (365 * 24 * 60 * 60); // 365 days ago
        char start_date[11];
        get_date_string(start_time, start_date, sizeof(start_date));

        snprintf(result, 256, "Year: %s - %s", start_date, end_date);
        break;
    }

    default:
        snprintf(result, 256, "Unknown period");
        break;
    }

    return result;
}

                                                                        /*      Callback functions for UI elements       */

void on_combo_box_change_date(GtkComboBox* combo_box, gpointer user_data) {

    satellite_full_str = "";
    free_hash_table(table);

    gchar* active_date_text = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo_box));

    if (active_date_text != NULL) {
        gchar* text_to_display = g_hash_table_lookup(combo_text_map, active_date_text);
        if (text_to_display != NULL) {
            gtk_label_set_text(GTK_LABEL(date_text), text_to_display);

            if (strcmp(text_to_display, get_time_period(TODAY))==0) {
                date_flag = TODAY;
                gtk_label_set_text(GTK_LABEL(sat_position_display_explained), "The most recent satellite position recorded is displayed.");
                logger("Registered date flag: TODAY");
            }
            else if (strstr(text_to_display, get_time_period(YESTERDAY))) {
                date_flag = YESTERDAY;
                gtk_label_set_text(GTK_LABEL(sat_position_display_explained), "The most recent satellite position recorded is displayed.");
                logger("Registered date flag: YESTERDAY");
            }
            else if (strstr(text_to_display, get_time_period(WEEK))) {
                date_flag = WEEK;
                gtk_label_set_text(GTK_LABEL(sat_position_display_explained), "");
                logger("Registered date flag: WEEK");
            }
            else if (strstr(text_to_display, get_time_period(MONTH))) {
                date_flag = MONTH;
                gtk_label_set_text(GTK_LABEL(sat_position_display_explained), "");
                logger("Registered date flag: MONTH");
            }
            else if (strstr(text_to_display, get_time_period(YEAR))) {
                date_flag = YEAR;
                gtk_label_set_text(GTK_LABEL(sat_position_display_explained), "");
                logger("Registered date flag: YEAR");
            }
            else {
                date_flag = TODAY;
                gtk_label_set_text(GTK_LABEL(sat_position_display_explained), "The most recent satellite position recorded is displayed.");
                logger("Registered date flag: TODAY");
            }

            gtk_label_set_line_wrap(GTK_LABEL(sat_position_display_explained), TRUE);
            gtk_label_set_max_width_chars(GTK_LABEL(sat_position_display_explained), 35);
            gtk_widget_modify_font(sat_position_display_explained, font_desc);
        }
        g_free(active_date_text);
    }

    switch (satellite_flag) {
    case 0:
        logger("Registered satellite flag: 25544");
        get_sat_info_from_name_and_date("25544");
        break;

    case 1:
        logger("Registered satellite flag: 43694");
        get_sat_info_from_name_and_date("43694");
        break;

    case 2:
        logger("Registered satellite flag: 43696");
        get_sat_info_from_name_and_date("43696");
        break;

    case 3:
        logger("Registered satellite flag: 43809");
        get_sat_info_from_name_and_date("43809");
        break;

    case 4:
        logger("Registered satellite flag: 32060");
        get_sat_info_from_name_and_date("32060");
        break;

    case 5:
        logger("Registered satellite flag: 33499");
        get_sat_info_from_name_and_date("33499");
        break;

    case 6:
        logger("Registered satellite flag: 42759");
        get_sat_info_from_name_and_date("42759");
        break;

    case 7:
        logger("Registered satellite flag: 35681");
        get_sat_info_from_name_and_date("35681");
        break;

    case 8:
        logger("Registered satellite flag: 42826");
        get_sat_info_from_name_and_date("42826");
        break;

    case 9:
        logger("Registered satellite flag: 42828");
        get_sat_info_from_name_and_date("42828");
        break;

    default:
        logger("Satellite flag is out of range");
        break;
    }

    // Update the labels with new text
    set_label_position_text();

   // Draw new position markings
    calculate_and_draw_position_markings();
}

void on_combo_box_change_satellite(GtkComboBox* satellite_combo_box, gpointer user_data) {

    //Clean global satellite data holder for reuse
    satellite_full_str = "";
    free_hash_table(table);

    //Deal with the satellite displayed text
    char* active_text = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(satellite_combo_box));
    get_sat_info_from_name_and_date(active_text);

    // Update the labels with new text
    set_label_position_text();

    // Draw new position markings
    calculate_and_draw_position_markings();
}
  
void switch_visual(GtkWidget* arrow_button, gpointer user_data) {

    if (arrow_button == button_left2) {
        visual_counter--; 
        if (visual_counter < 0) {

            visual_counter = 5; 
            calculate_and_draw_position_markings();
        }
        else {

            char* image_name = generate_plot_image(visual_counter);
            gtk_image_set_from_file(GTK_IMAGE(image), image_name);
        }

    }
    else if (arrow_button == button_right2) {
        visual_counter++;
        if (visual_counter == 5) {

            calculate_and_draw_position_markings();
        }
        else if (visual_counter > 5) {
            visual_counter = 0; 
            char* image_name = generate_plot_image(visual_counter);
            gtk_image_set_from_file(GTK_IMAGE(image), image_name);
        }
        else {

            char* image_name = generate_plot_image(visual_counter);
            gtk_image_set_from_file(GTK_IMAGE(image), image_name);
        }
    }

    satellite_plot_flag = visual_counter;

}


                                                                        /*      Hash table functions       */   

static HashTable* create_hash_table() {
    HashTable* table = malloc(sizeof(HashTable));
    if (!table) return NULL;

    // Initialize hash table buckets
    for (int i = 0; i < TABLE_SIZE; i++) {
        table->buckets[i] = NULL;
    }

    // Initialize the linked list for tracking insertion order
    table->insertion_order = NULL;

    return table;
}

static unsigned long hash_string(const char* str) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }

    return hash % TABLE_SIZE; // Use modulo to fit within the table size
}

static LinkedListNode* create_linked_list_node(const char* key) {
    LinkedListNode* node = malloc(sizeof(LinkedListNode));
    if (!node) return NULL;
    node->key = strdup(key);
    node->next = NULL;
    return node;
}

static void insert(HashTable* table, const char* key) {
    unsigned long hash = hash_string(key);
    HashEntry* new_entry = malloc(sizeof(HashEntry));
    if (!new_entry) return;

    new_entry->hash = hash;
    new_entry->key = strdup(key);
    new_entry->next = table->buckets[hash];
    table->buckets[hash] = new_entry;

    // Update the linked list to keep track of the insertion order
    LinkedListNode* new_node = create_linked_list_node(key);
    if (table->insertion_order) {
        new_node->next = table->insertion_order;
    }
    table->insertion_order = new_node;
}

static char* get_last_value(HashTable* table) {
    if (!table->insertion_order) return NULL;

    return table->insertion_order->key;
}

static void free_linked_list(LinkedListNode* head) {
    while (head) {
        LinkedListNode* temp = head;
        head = head->next;
        free(temp->key);
        free(temp);
    }
}

static void free_hash_table(HashTable* table) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        HashEntry* entry = table->buckets[i];
        while (entry) {
            HashEntry* prev_entry = entry;
            entry = entry->next;
            free(prev_entry->key);
            free(prev_entry);
        }
    }

    free_linked_list(table->insertion_order);
    free(table);
}

                                                                        /*      Query functions       */

char* satellite_query(enum SatelliteKey key) {

    bson_error_t error;
    const bson_t* reply;
    mongoc_cursor_t* cursor;
    bson_t* doc;
    bson_t* filter;
    char* str = "{ \
                  \"_id\": { \
                    \"$oid\": \"0\" \
                  }, \
                  \"Name\": \"UNKNOWN\", \
                  \"Satellite ID\": \"00000\", \
                  \"Latitude\": \"0.0\", \
                  \"Longitude\": \"0.0\", \
                  \"Altitude\": \"0.0\", \
                  \"Azimuth\": \"0.0\", \
                  \"Elevation\": \"0.0\", \
                  \"Right Ascension\": \"0.0\", \
                  \"Declination\": \"0.0\", \
                  \"Registered\": \"0\" \
                }";

    bson_t* opts;
    bson_t* query = bson_new();
    bson_t* document;

    mongoc_client_set_error_api(client, 2);
    mongoc_collection_t* collection = mongoc_client_get_collection(client, "satellite_record", "position");
    
    char* date_str;
    switch (date_flag) {
    case YESTERDAY:
        opts = BCON_NEW(
            "limit", BCON_INT64(1), "sort", "{", "Registered", BCON_INT32(-1), "}");
        date_str = get_yesterdays_date();
        BSON_APPEND_REGEX(query, "Registered", date_str, "i");

        break;

    case WEEK:
        opts = NULL;
        query = create_time_period_query(7);

        break;

    case MONTH:
        opts = NULL;
        query = create_time_period_query(30);

        break;

    case YEAR:
        opts = NULL;
        query = create_time_period_query(365);

        break;

    default:
        opts = BCON_NEW(
            "limit", BCON_INT64(1), "sort", "{", "Registered", BCON_INT32(-1), "}");

        char date_str[11];
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        mktime(&tm);
        strftime(date_str, sizeof(date_str), "%Y-%m-%d", &tm);
        BSON_APPEND_REGEX(query, "Registered", date_str, "i");

        break;
    }

    switch (key) {
    case ISS:
        BSON_APPEND_UTF8(query, "Satellite ID", "25544");
        break;
    case PROXIMA1:
        BSON_APPEND_UTF8(query, "Satellite ID", "43694");
        break;
    case PROXIMA2:
        BSON_APPEND_UTF8(query, "Satellite ID", "43696");
        break;
    case CENTAURI1:
        BSON_APPEND_UTF8(query, "Satellite ID", "43809");
        break;
    case WV1:
        BSON_APPEND_UTF8(query, "Satellite ID", "32060");
        break;
    case KKS1:
        BSON_APPEND_UTF8(query, "Satellite ID", "33499");
        break;
    case DEIMOS1:
        BSON_APPEND_UTF8(query, "Satellite ID", "35681");
        break;
    case ZHUHAI1:
        BSON_APPEND_UTF8(query, "Satellite ID", "42759");
        break;
    case NORSAT1:
        BSON_APPEND_UTF8(query, "Satellite ID", "42826");
        break;
    case NORSAT2:
        BSON_APPEND_UTF8(query, "Satellite ID", "42828");
        break;
    }

    cursor = mongoc_collection_find_with_opts(collection, query, opts, NULL);

    table = create_hash_table();
    if (opts == NULL) {

        if (mongoc_cursor_next(cursor, &doc)) {
            do {
                str = bson_as_canonical_extended_json(doc, NULL);
                if (str != NULL) {
                    insert(table, str);
                    bson_free(str); 
                }
                else {
                    logger("Error retrieving data from database - null JSON string.");
                }

            } while (mongoc_cursor_next(cursor, &doc));

            if (mongoc_cursor_error_document(cursor, &error, &reply)) {
                str = bson_as_json(reply, NULL);

                size_t size = strlen(str) + strlen(error.message) + 30;
                char* log_entry = (char*)malloc(size);
                char* log = "Cursor Failure & Reply: ";
                snprintf(log_entry, size, "%s %s %s", log, str, error.message);
                logger(log_entry);

                bson_free(str);
                free(log_entry);
            }
        }
        else {
            logger("No data to retrieve records from the database for the specified time period.");
        }

    }
    else {

        if (mongoc_cursor_next(cursor, &doc)) {

            do {
                str = bson_as_canonical_extended_json(doc, NULL);
                if (str == NULL) {
                    logger("Error retrieving data from database - null JSON string.");
                }
            } while (mongoc_cursor_next(cursor, &doc));

        } else {
            if (mongoc_cursor_error_document(cursor, &error, &reply)) {
                str = bson_as_json(reply, NULL);

                size_t size = strlen(str) + strlen(error.message) + 30;
                char* log_entry = (char*)malloc(size);
                char* log = "Cursor Failure & Reply: ";
                snprintf(log_entry, size, "%s %s %s", log, str, error.message);
                logger(log_entry);

                bson_free(str);
            }
            else {
                logger("No satellite data to retrieve for selected date.");
            }
        }   
    }

    bson_destroy(query);
    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(collection);
    mongoc_cleanup();

    return str;
};

static void get_sat_info_from_name_and_date(char* active_text) {

    if (strstr(active_text, "25544")) {
        satellite_full_str = satellite_query(ISS);
        satellite_flag = 0;
    }
    else if (strstr(active_text, "43694")) {
        satellite_full_str = satellite_query(PROXIMA1);
        satellite_flag = 1;
    }
    else if (strstr(active_text, "43696")) {
        satellite_full_str = satellite_query(PROXIMA2);
        satellite_flag = 2;
    }
    else if (strstr(active_text, "43809")) {
        satellite_full_str = satellite_query(CENTAURI1);
        satellite_flag = 3;
    }
    else if (strstr(active_text, "32060")) {
        satellite_full_str = satellite_query(WV1);
        satellite_flag = 4;
    }
    else if (strstr(active_text, "33499")) {
        satellite_full_str = satellite_query(KKS1);
        satellite_flag = 5;
    }
    else if (strstr(active_text, "42759")) {
        satellite_full_str = satellite_query(ZHUHAI1);
        satellite_flag = 6;
    }
    else if (strstr(active_text, "35681")) {
        satellite_full_str = satellite_query(DEIMOS1);
        satellite_flag = 7;
    }
    else if (strstr(active_text, "42826")) {
        satellite_full_str = satellite_query(NORSAT1);
        satellite_flag = 8;
    }
    else if (strstr(active_text, "42828")) {
        satellite_full_str = satellite_query(NORSAT2);
        satellite_flag = 9;
    }
}

                                                                        /*      Text helper functions       */

static void set_label_position_text() {

    char* text_longitude = process_json_for_label_text(satellite_full_str, "Longitude");
    size_t size_lon = strlen(text_longitude) + 15;
    char* text_lon = (char*)malloc(size_lon);
    snprintf(text_lon, size_lon, "%s: %s", "Longitude", text_longitude);
    gtk_label_set_text(GTK_LABEL(satellite_text_long), text_lon);

    char* text_latitude = process_json_for_label_text(satellite_full_str, "Latitude");
    size_t size_lat = strlen(text_latitude) + 15;
    char* text_lat = (char*)malloc(size_lat);
    snprintf(text_lat, size_lat, "%s: %s", "Longitude", text_latitude);
    gtk_label_set_text(GTK_LABEL(satellite_text_lat), text_lat);
}

char* process_json_for_label_text(char* string, char* parameter) {

    cJSON* json = cJSON_Parse(string);
    if (json == NULL) {
        logger("Error parsing JSON for label");

        char* str = "{ \
                  \"_id\": { \
                    \"$oid\": \"0\" \
                  }, \
                  \"Name\": \"UNKNOWN\", \
                  \"Satellite ID\": \"00000\", \
                  \"Latitude\": \"0.0\", \
                  \"Longitude\": \"0.0\", \
                  \"Altitude\": \"0.0\", \
                  \"Azimuth\": \"0.0\", \
                  \"Elevation\": \"0.0\", \
                  \"Right Ascension\": \"0.0\", \
                  \"Declination\": \"0.0\", \
                  \"Registered\": \"0\" \
                }";

        json = cJSON_Parse(str);
    }

    cJSON* value = cJSON_GetObjectItemCaseSensitive(json, parameter);
    if (value == NULL || !cJSON_IsString(value)) {
        logger("Error: Parameter not found or not a string.");
        cJSON_Delete(json);
        return NULL;
    }

    char* extracted_str = value->valuestring;
    if (strstr(extracted_str, "UNKNOWN") != NULL) {
        switch (satellite_flag) {
        case 0:
            extracted_str = "ISS (SPACE STATION)";
            break;

        case 1:
            extracted_str = "PROXIMA I";
            break;

        case 2:
            extracted_str = "PROXIMA II";
            break;

        case 3:
            extracted_str = "CENTAURI I";
            break;

        case 4:
            extracted_str = "WORLVIEW I";
            break;

        case 5:
            extracted_str = "KKS-I (KISEKI)";
            break;

        case 6:
            extracted_str = "ZHUHAI I";
            break;

        case 7:
            extracted_str = "DEIMOS I";
            break;

        case 8:
            extracted_str = "NORSAT I";
            break;

        case 9:
            extracted_str = "NORSAT II";
            break;
        default:
            extracted_str = "ERROR";
            break;
        }
    }

    size_t size = strlen(extracted_str) + 1;
    char* text = (char*)malloc(size);

    snprintf(text, size, "%s", extracted_str);

    cJSON_Delete(json);
    return text;
}

                                                                        /*      Drawing functions       */
static void clear_overlay(void) {

    cairo_surface_destroy(overlay_surface);
    overlay_surface = NULL;

    // Reinitialize the overlay surface
    base_pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(image));
    int width = gdk_pixbuf_get_width(base_pixbuf);
    int height = gdk_pixbuf_get_height(base_pixbuf);

    // Create a new overlay surface
    overlay_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t* cr = cairo_create(overlay_surface);
    cairo_set_source_rgba(cr, 0, 0, 0, 0);
    cairo_paint(cr);
    cairo_destroy(cr);

    // Create a new pixbuf from the base image
    GdkPixbuf* new_pixbuf = gdk_pixbuf_get_from_surface(overlay_surface, 0, 0, width, height);

    if (new_pixbuf) {
        logger("Created new pixbuf from surface");
        gtk_image_set_from_pixbuf(GTK_IMAGE(image), new_pixbuf);
        g_object_unref(new_pixbuf); 
    }
    else {
        logger("Failed to create new pixbuf from surface");
    }
}

void draw_dot_on_overlay(double lon, double lat) {

    cairo_t* cr = cairo_create(overlay_surface);
    int width = gdk_pixbuf_get_width(base_pixbuf);
    int height = gdk_pixbuf_get_height(base_pixbuf);

    double x = (lon + 180) * (width / 360.0);
    double y = (90 - lat) * (height / 180.0);

    cairo_arc(cr, x, y, 5, 0, 2 * M_PI); // Draw a circle with radius 5
    cairo_set_source_rgb(cr, 1, 0, 0); // Red color
    cairo_fill(cr); // Fill the circle

    cairo_destroy(cr);
}

void update_image_with_dots(GtkWidget* image_widget) {
    if (!base_pixbuf || !overlay_surface) {
        logger("Base image or overlay surface not initialized");
        return;
    }

    int width = gdk_pixbuf_get_width(base_pixbuf);
    int height = gdk_pixbuf_get_height(base_pixbuf);

    // Create a combined surface to draw the base image and the overlay
    cairo_surface_t* combined_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t* cr = cairo_create(combined_surface);

    // Draw the base image on the combined surface
    gdk_cairo_set_source_pixbuf(cr, base_pixbuf, 0, 0);
    cairo_paint(cr);

    // Draw the overlay surface
    cairo_set_source_surface(cr, overlay_surface, 0, 0);
    cairo_paint(cr);

    // Clean up
    cairo_destroy(cr);

    // Create a new GdkPixbuf from the combined Cairo surface
    GdkPixbuf* new_pixbuf = gdk_pixbuf_get_from_surface(combined_surface, 0, 0, width, height);
    if (new_pixbuf) {
        gtk_image_set_from_pixbuf(GTK_IMAGE(image_widget), new_pixbuf);
        g_object_unref(new_pixbuf); // Unreference the new pixbuf to avoid memory leaks
    }
    else {
        g_printerr("Failed to create new pixbuf from surface\n");
    }

    // Clean up Cairo surface
    cairo_surface_destroy(combined_surface);
}


void load_and_initialize_image() {
    // Load a fresh base image
    GdkPixbuf* new_pixbuf = gdk_pixbuf_new_from_file("../../assets/map.png", NULL);
    if (!new_pixbuf) {
        g_printerr("Failed to load base image\n");
        return;
    }

    // Update the GtkImage with the new base image
    gtk_image_set_from_pixbuf(GTK_IMAGE(image), new_pixbuf);

    // Free the old pixbuf if it exists
    if (base_pixbuf) {
        g_object_unref(base_pixbuf);
    }

    // Set the new pixbuf as the base pixbuf
    base_pixbuf = new_pixbuf;

    // Initialize the overlay surface with the dimensions of the new pixbuf
    int width = gdk_pixbuf_get_width(base_pixbuf);
    int height = gdk_pixbuf_get_height(base_pixbuf);

    // Create a new overlay surface
    if (overlay_surface) {
        cairo_surface_destroy(overlay_surface);
    }
    overlay_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t* cr = cairo_create(overlay_surface);
    cairo_set_source_rgba(cr, 0, 0, 0, 0); // Transparent
    cairo_paint(cr);
    cairo_destroy(cr);
}

static void calculate_and_draw_position_markings() {
    clear_overlay();
    load_and_initialize_image();

    if (date_flag == WEEK || date_flag == MONTH || date_flag == YEAR) {
        logger("Satellite projectory over a selected time period is being drawn.");
        if (!table) {
            logger("Reload the application. Data has not been retrieved and / or saved correctly.");
        }
        else {
            // Pointer to the current node in the insertion order linked list
            LinkedListNode* current_node = table->insertion_order;

            while (current_node) {
                // Get the key from the linked list node
                const char* key = current_node->key;

                // Calculate hash to find the corresponding bucket
                unsigned long hash = hash_string(key);

                // Find the hash entry in the bucket
                HashEntry* entry = table->buckets[hash];
                while (entry) {
                    if (strcmp(entry->key, key) == 0) {

                        char* text_longitude = process_json_for_label_text(entry->key, "Longitude");
                        char* text_latitude = process_json_for_label_text(entry->key, "Latitude");
                        draw_dot_on_overlay(strtod(text_longitude, NULL), strtod(text_latitude, NULL));

                        break;
                    }
                    entry = entry->next;
                }

                // Move to the next node in the linked list
                current_node = current_node->next;
            }
        }
    }
    else {
        logger("Single satellite position is being drawn.");
        char* text_longitude = process_json_for_label_text(satellite_full_str, "Longitude");
        char* text_latitude = process_json_for_label_text(satellite_full_str, "Latitude");
        draw_dot_on_overlay(strtod(text_longitude, NULL), strtod(text_latitude, NULL));
    }

    update_image_with_dots(GTK_WIDGET(image));
}

                                                                        /*      Python script functions       */

static char* generate_plot_image(enum Plot plot_type)
{
    free_hash_table(table);

    int previous_date_flag = date_flag;
    date_flag = YEAR;

    Py_Initialize();

    // Set the Python module paths
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append(r'..\\..\\assets')");
    PyRun_SimpleString("sys.path.append(r'..\\..\\include\\site-packages')");

    int result = Py_IsInitialized();
    char* log_entry = (char*)malloc(100);
    char* log = "Python was initialized:";
    snprintf(log_entry, 100, "%s %d", log, result);
    logger(log_entry);

    // Import the module
    PyObject* p_ModuleName = PyUnicode_FromString("data_analysis");
    if (p_ModuleName == NULL) {
        logger("Failed to convert module name.");
        return NULL;
    }
    PyObject* p_Module = PyImport_Import(p_ModuleName);
    Py_XDECREF(p_ModuleName);

    char* plot_name;
    char* image_name = "unknown";

    // Get data from Mongo & convert to Py list
    char* placeholder_str = satellite_query(satellite_flag);
    //free(placeholder_str);

    PyObject* timeValues = get_data_for_python("Registered");
    if (timeValues == NULL) {
        logger("Failed to retrieve time values for python module.");
        Py_Finalize();
        return NULL;
    }

    if (p_Module != NULL) {

        logger("Located python module");

        PyObject* altValues = NULL;
        PyObject* pFunc = NULL;
        PyObject* pArgs = NULL;
        PyObject* pValue = NULL;
        PyObject* pPlotType = PyUnicode_FromString("time_series");

        switch (plot_type) {
        case HIST:

            pFunc = PyObject_GetAttrString(p_Module, "elevation_histogram");
            pPlotType = PyUnicode_FromString("histogram");
            if (pFunc && PyCallable_Check(pFunc)) {

                PyObject* elevationValues = get_data_for_python("Elevation");
                    if (elevationValues == NULL) {
                        logger("Failed to retrieve altitude values.");
                        Py_XDECREF(pFunc);
                        break;
                    }

                pArgs = PyTuple_Pack(1, elevationValues, pPlotType);
                    if (pArgs == NULL) {
                        logger("Failed to create Python argument tuple.");
                        Py_XDECREF(timeValues);
                        Py_XDECREF(elevationValues);
                        Py_XDECREF(pFunc);
                    }

                // Call the python function
                PyObject* pValue = PyObject_CallObject(pFunc, pArgs);
                    Py_XDECREF(pArgs);
                    if (pValue == NULL) {
                        logger("Failed to call the Python function.");
                        Py_XDECREF(elevationValues);
                        break;
                    }
                    else {
                        logger("Function called successfully.");
                    }

                image_name = "Histogram.png";

                Py_XDECREF(pFunc);
                Py_XDECREF(pPlotType);
            }
            else {
                logger("Failed to retrieve the Python function for altitude rolling mean and standard deviation.");
                break;
            }
            break;

        case TIME_SERIES_LAT:

            pFunc = PyObject_GetAttrString(p_Module, "lat_vs_time_graph");
            if (pFunc && PyCallable_Check(pFunc)) {

                PyObject* latValues = get_data_for_python("Latitude");
                    if (latValues == NULL) {
                        logger("Failed to retrieve altitude values.");
                        Py_XDECREF(pFunc);
                        break;
                    }

                pArgs = PyTuple_Pack(2, timeValues, latValues, pPlotType);
                    if (pArgs == NULL) {
                        logger("Failed to create Python argument tuple.");
                        Py_XDECREF(timeValues);
                        Py_XDECREF(latValues);
                        Py_XDECREF(pFunc);
                    }

                // Call the python function
                PyObject* pValue = PyObject_CallObject(pFunc, pArgs);
                Py_XDECREF(pArgs);
                    if (pValue == NULL) {
                        logger("Failed to call the Python function.");
                        Py_XDECREF(latValues);
                        break;
                    }
                    else {
                        logger("Function called successfully.");
                    }

                image_name = "Latitude_time_series.png";

                Py_XDECREF(pFunc);
                Py_XDECREF(pPlotType);
            }
            else {
                logger("Failed to retrieve the Python function for altitude rolling mean and standard deviation.");
                break;
            }
            break;

        case TIME_SERIES_LONG:

            pFunc = PyObject_GetAttrString(p_Module, "long_vs_time_graph");
            if (pFunc && PyCallable_Check(pFunc)) {

                PyObject* longValues = get_data_for_python("Longitude");
                if (longValues == NULL) {
                    logger("Failed to retrieve altitude values.");
                    Py_XDECREF(pFunc);
                    break;
                }

                pArgs = PyTuple_Pack(2, timeValues, longValues, pPlotType);
                if (pArgs == NULL) {
                    logger("Failed to create Python argument tuple.");
                    Py_XDECREF(timeValues);
                    Py_XDECREF(longValues);
                    Py_XDECREF(pFunc);
                }

                // Call the python function
                PyObject* pValue = PyObject_CallObject(pFunc, pArgs);
                Py_XDECREF(pArgs);
                if (pValue == NULL) {
                    logger("Failed to call the Python function.");
                    Py_XDECREF(longValues);
                    break;
                }
                else {
                    logger("Function called successfully.");
                }

                image_name = "Longitude_time_series.png";

                Py_XDECREF(pFunc);
                Py_XDECREF(pPlotType);
            }
            else {
                logger("Failed to retrieve the Python function for altitude rolling mean and standard deviation.");
            }
            break;

        case ROLL_STAT:

            pFunc = PyObject_GetAttrString(p_Module, "rolling_mean_and_std_altitude_graph");
            if (pFunc && PyCallable_Check(pFunc)) {

                PyObject* altValues = get_data_for_python("Altitude");
                    if (altValues == NULL) {
                        logger("Failed to retrieve altitude values.");
                        Py_XDECREF(pFunc);
                        break;
                    }

                pArgs = PyTuple_Pack(2, timeValues, altValues, pPlotType);
                    if (pArgs == NULL) {
                        logger("Failed to create Python argument tuple.");
                        Py_XDECREF(timeValues);
                        Py_XDECREF(altValues);
                        Py_XDECREF(pFunc);
                    }

                // Call the python function
                PyObject* pValue = PyObject_CallObject(pFunc, pArgs);
                Py_XDECREF(pArgs);
                if (pValue == NULL) {
                    logger("Failed to call the Python function.");
                    Py_XDECREF(altValues);
                    break;
                }
                else {
                    logger("Function called successfully.");
                }

                image_name = "Altitude_rolling_mean_std.png";

                Py_XDECREF(pFunc);
                Py_XDECREF(pPlotType);
            }
            else {
                logger("Failed to retrieve the Python function for altitude rolling mean and standard deviation.");
            }
            break;

        case DECL_AND_ALT_CORR:

            pFunc = PyObject_GetAttrString(p_Module, "declination_altitude_correlation");

            if (pFunc && PyCallable_Check(pFunc)) {

                PyObject* altValues = get_data_for_python("Altitude");
                    if (altValues == NULL) {
                        logger("Failed to retrieve altitude values.");
                        Py_XDECREF(pFunc);
                        break;
                    }

                PyObject* declValues = get_data_for_python("Declination");
                    if (declValues == NULL) {
                        logger("Failed to retrieve declination values.");
                        Py_XDECREF(declValues);
                        Py_XDECREF(pFunc);
                        break;
                    }

                pArgs = PyTuple_Pack(3, timeValues, altValues, declValues, pPlotType);
                    if (pArgs == NULL) {
                        logger("Failed to create Python argument tuple.");
                        Py_XDECREF(timeValues);
                        Py_XDECREF(altValues);
                        Py_XDECREF(declValues);
                        Py_XDECREF(pFunc);
                    }

                // Call the python function
                PyObject* pValue = PyObject_CallObject(pFunc, pArgs);
                Py_XDECREF(pArgs);
                    if (pValue == NULL) {
                        logger("Failed to call the Python function.");
                        Py_XDECREF(altValues);
                        Py_XDECREF(declValues);
                        break;
                    }
                    else {
                        logger("Function called successfully.");
                    }

                image_name = "Declination_altitude_corr.png";

                Py_XDECREF(pFunc);
                Py_XDECREF(pPlotType);
            }
            else {
                logger("Failed to retrieve the Python function for declination and altitude correlation.");
            }

            break;

        case MAP:

            calculate_and_draw_position_markings();
            break;
        }

        Py_XDECREF(p_Module);
    }
    else {
        logger("Failed to load python module.");
    }
    date_flag = previous_date_flag;

    return image_name;
}

static char* call_python_module_histogram(PyObject* pValue) {

    char* filename = "histogram.png";

    if (pValue != NULL) {
        FILE* f = fopen(filename, "wb");
        if (f) {
            fwrite(PyBytes_AsString(pValue), 1, PyBytes_Size(pValue), f);
            fclose(f);
            logger("Plot successfully created");
        }
        Py_XDECREF(pValue);
    }
    else {
        PyErr_Print();
        logger("Error processing values for plotting");
    }

    return filename;
}

static PyObject* get_data_for_python(char* parameter) {
  
    PyObject* values = PyList_New(0);

    for (int i = 0; i < TABLE_SIZE; i++) {
        HashEntry* entry = table->buckets[i];
        while (entry) {
            cJSON* json = cJSON_Parse(entry->key);
                if (json == NULL) {
                    entry = entry->next; 
                    continue;
                }

            char* extracted_str = process_json_for_label_text(entry->key, parameter);
            if (strstr(parameter, "Registered") == NULL) {
                double element_value = strtod(extracted_str, NULL);
                PyObject* pyValue = PyFloat_FromDouble(element_value);
                PyList_Append(values, pyValue);
                Py_DECREF(pyValue);
            }
            else {
                PyObject* pyValue = PyUnicode_FromString(extracted_str);
                if (pyValue) {
                    PyList_Append(values, pyValue);
                    Py_DECREF(pyValue);
                }
            }

            free(extracted_str); 
            cJSON_Delete(json); 
            entry = entry->next; 
        }
    }

    // Log the number of retrieved records for our sanity
    Py_ssize_t count = PyList_Size(values);
    char* count_str = (char*)malloc(100 * sizeof(char));
    char* extra_detail = "value count";
    snprintf(count_str, 100, "%s %s: %zd", parameter, extra_detail, count);
    logger(count_str);
    free(count_str);

    return values;
}

                                                                        /*      Download functions       */

void download_txt(GtkWidget* arrow_button, gpointer user_data) {

    char* name = process_json_for_label_text(satellite_full_str, "Name");

    if (strstr(name, "UNKNOWN") != NULL) {
        no_sat_data_found_alert_modal(window);
    }
    else {

        char* full_str[100];
        char* extracted_val = NULL;
        char filename[25];
        size_t size_of_val = 0;

        snprintf(filename, strlen(name) + 5, "%s.txt", name);

        FILE* file = fopen(filename, "r");
        if (file != NULL) {
            fclose(file);
        }
        else {
            FILE* file = fopen(filename, "w");
        }

        file = fopen(filename, "a");
        if (file == NULL) {
            logger("Error opening file to save satellite data in txt format.");
        }

        fprintf(file, "Last captured satellite data for the selected time period \n");

        char parameters[9][15] = { "Name", "Satellite ID", "Longitude", "Latitude", "Altitude", "Azimuth", "Elevation", "Right Ascension", "Declination" };
        foreach(char* param, parameters) {

            extracted_val = process_json_for_label_text(satellite_full_str, param);
            if (extracted_val == NULL) {
                logger("Error processing parameter.");
                continue;
            }

            snprintf(full_str, sizeof(full_str), "%s: %s\n", param, extracted_val);
            fprintf(file, "%s", full_str);

            free(extracted_val);
        }

        fclose(file);
    }
}

void download_xml(GtkWidget* arrow_button, gpointer user_data) {

    char* name = process_json_for_label_text(satellite_full_str, "Name");

    if (strstr(name, "UNKNOWN") != NULL) {
        no_sat_data_found_alert_modal(window);
    }
    else {
        char filename[25];
        snprintf(filename, strlen(name) + 5, "%s.xml", name);

        FILE* file = fopen(filename, "r");
        if (file != NULL) {
            fclose(file);
        }
        else {
            FILE* file = fopen(filename, "w");
        }

        file = fopen(filename, "a");
        if (file == NULL) {
            logger("Error opening file to save satellite data in xml format.");
        }

        fprintf(file, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\n");
        fprintf(file, "<Last captured satellite data for the selected time period>\n");
        fprintf(file, "<satellite>\n");

        char parameters[9][15] = { "Name", "Satellite ID", "Longitude", "Latitude", "Altitude", "Azimuth", "Elevation", "Right Ascension", "Declination" };
        char* extracted_val = NULL;
        char* full_str[100];
        char* tag_content[15];
        char* open_tag[15];
        char* close_tag[15];

        foreach(char* param, parameters) {

            extracted_val = process_json_for_label_text(satellite_full_str, param);
            if (extracted_val == NULL) {
                logger("Error processing parameter.");
                continue;
            }

            snprintf(open_tag, sizeof(param) + 8, "<%s>", param);
            snprintf(tag_content, sizeof(tag_content), "%s", extracted_val);
            snprintf(close_tag, sizeof(param) + 8, "</%s>", param);

            snprintf(full_str, sizeof(full_str)+2, "%s%s%s\n", open_tag, tag_content, close_tag);

            fprintf(file, "%s", full_str);

            free(extracted_val);
        }

        fprintf(file, "</satellite>\n");
        fclose(file);
    }
}

void download_pdf(GtkWidget* arrow_button, gpointer user_data) {

    //Filename
    char* name = process_json_for_label_text(satellite_full_str, "Name");
    bool show_position_data = false;

    char filename[25];
    snprintf(filename, strlen(name) + 5, "%s.pdf", name);

    // Setup page
    cairo_surface_t* surface = cairo_pdf_surface_create(filename, 595, 842); // A4 size 
    cairo_t* cr = cairo_create(surface); 
    cairo_scale(cr, 0.5, 0.5);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    // Set the position where the image is drawn
    double image_x = 80; // X position of the image on the PDF
    double image_y = 50; // Y position of the image on the PDF
    double image_width = 1000; // Width of the image
    double image_height = 490; // Height of the image

    //Map with positions
    cairo_surface_t* image = cairo_image_surface_create_from_png("../../assets/map.png");
    if (cairo_surface_status(image) != CAIRO_STATUS_SUCCESS) {
        logger("Error loading image - check filepath!");
    }
    cairo_set_source_surface(cr, image, image_x, image_y);
    cairo_rectangle(cr, image_x, image_y, image_width, image_height);

    if (satellite_plot_flag == MAP) {

        if (date_flag == WEEK || date_flag == MONTH || date_flag == YEAR) {
            logger("Satellite projectory over a selected time period is being drawn.");
            if (!table) {
                logger("Reload the application. Data has not been retrieved and / or saved correctly.");
            }
            else {
                // Pointer to the current node in the insertion order linked list
                LinkedListNode* current_node = table->insertion_order;

                while (current_node) {
                    // Get the key from the linked list node
                    const char* key = current_node->key;

                    // Calculate hash to find the corresponding bucket
                    unsigned long hash = hash_string(key);

                    // Find the hash entry in the bucket
                    HashEntry* entry = table->buckets[hash];
                    while (entry) {
                        if (strcmp(entry->key, key) == 0) {

                            char* text_longitude = process_json_for_label_text(entry->key, "Longitude");
                            char* text_latitude = process_json_for_label_text(entry->key, "Latitude");

                            draw_position_dot_for_pdf(text_longitude, text_latitude,
                                cr, image_x, image_y, image_width, image_height);

                            break;
                        }
                        entry = entry->next;
                    }

                    // Move to the next node in the linked list
                    current_node = current_node->next;
                }
            }
        }
        else {

            show_position_data = true;
            logger("Single satellite position is being recorded in pdf.");
            char* text_longitude = process_json_for_label_text(satellite_full_str, "Longitude");
            char* text_latitude = process_json_for_label_text(satellite_full_str, "Latitude");

            draw_position_dot_for_pdf(text_longitude, text_latitude,
                cr, image_x, image_y, image_width, image_height);
        }
    }
    else {
        char* plot_name;
        switch (satellite_plot_flag) {
        case HIST:
            plot_name = "Histogram.png";
            break;
        case TIME_SERIES_LAT:
            plot_name = "Latitude_time_series.png";
            break;
        case TIME_SERIES_LONG:
            plot_name = "Longitude_time_series.png";
            break;
        case ROLL_STAT:
            plot_name = "Altitude_rolling_mean_std.png";
            break;
        case DECL_AND_ALT_CORR:
            plot_name = "Declination_altitude_corr.png";
            break;
        }

        image = cairo_image_surface_create_from_png(plot_name);

        if (cairo_surface_status(image) != CAIRO_STATUS_SUCCESS) {
            logger("Error loading image - check filepath!");
        }
        else {
            cairo_set_source_surface(cr, image, image_x, image_y); // Set the source
            cairo_paint(cr); // Paint the image onto the PDF surface
        }
    }

    // Informational text
    double y_position = 650;
    double x_position = 90;
    cairo_set_font_size(cr, 15);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_select_font_face(cr, "Mono", CAIRO_FONT_SLANT_ITALIC, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_move_to(cr, x_position, y_position);
    cairo_show_text(cr, "Most recent satellite data is provided.");
    cairo_move_to(cr, x_position, y_position + 25);
    cairo_show_text(cr, "If any of the parameters are 0, the latest data is not available. " 
        "Run the message publisher module again to get the new data.");

    // Satellite data text
    cairo_set_font_size(cr, 18);
    cairo_set_source_rgb(cr, 0, 0, 0); 

    char* extracted_val = NULL;
    y_position = 725;

    char full_str[100];
    snprintf(full_str, sizeof(full_str), "Satellite positions displayed for period: %s", get_time_period(date_flag));
    cairo_move_to(cr, x_position, y_position);
    cairo_show_text(cr, full_str);
    y_position += 50;

    cairo_select_font_face(cr, "Mono", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);

    if (show_position_data == true) {
        char parameters[9][15] = { "Name", "Satellite ID", "Longitude", "Latitude", "Altitude", "Azimuth", "Elevation", "Right Ascension", "Declination" };
        foreach(char* param, parameters) {
            extracted_val = process_json_for_label_text(satellite_full_str, param);
            char full_str[100];
            if (extracted_val == NULL) {
                logger("Error processing parameter.");
                continue;
            }

            snprintf(full_str, sizeof(full_str), "%s: %s", param, extracted_val);

            cairo_move_to(cr, x_position, y_position);
            cairo_show_text(cr, full_str);
            y_position += 25;

            free(extracted_val);
        }
    }

    y_position = 1625;

    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_ITALIC, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 12);
    cairo_move_to(cr, x_position, y_position);
    cairo_show_text(cr, "Satellite Tracker Project built using modular approach. Sofiia Romashova, 2024");

    logger("Finished generating PDF.");

    //Cleanup
    cairo_surface_destroy(image);
    cairo_destroy(cr);
    cairo_surface_finish(surface);
    cairo_surface_destroy(surface);
}

void no_sat_data_found_alert_modal(GtkWindow* parent)
{
    GtkWidget* dialog, * label, * content_area;
    GtkDialogFlags flags;

    // Create the widgets
    flags = GTK_DIALOG_DESTROY_WITH_PARENT;
    dialog = gtk_dialog_new_with_buttons("Warning",
        parent,
        flags,
        ("_OK"),
        GTK_RESPONSE_NONE,
        NULL);
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    gtk_window_set_default_size(GTK_WINDOW(dialog), 50, 100);

    label = gtk_label_new("No recent data to download for the selected satellite.\n Choose a different satellite or open message publisher to load data.");

    g_signal_connect_swapped(dialog,
        "response",
        G_CALLBACK(gtk_widget_destroy),
        dialog);

    gtk_container_add(GTK_CONTAINER(content_area), label);
    gtk_widget_show_all(dialog);
}

static void draw_position_dot_for_pdf(char* text_longitude, char* text_latitude,
    cairo_t* cr, double image_x, double image_y, double image_width, double image_height) {

    // Draw position dot
    cairo_clip(cr);
    cairo_paint(cr);
    cairo_reset_clip(cr);

    // Convert longitude and latitude to pixel coordinates
    double longitude = strtod(text_longitude, NULL);
    double latitude = strtod(text_latitude, NULL);

    // When you have an image, its width and height are measured in pixels. 
    // You want to map the geographic coordinates to these pixel dimensions.
    double pixel_x = image_x + (longitude + 180) * (image_width / 360); // Adjust mapping based on your needs
    double pixel_y = image_y + (90 - latitude) * (image_height / 180);  // Adjust mapping based on your needs

    // Draw the red dot at the calculated position
    cairo_set_source_rgb(cr, 1, 0, 0); // Set color to red
    cairo_arc(cr, pixel_x, pixel_y, 5, 0, 2 * M_PI); // Draw a circle
    cairo_fill(cr); // Fill the circle
}