#ifndef ANALYZER_CORE_H
#define ANALYZER_CORE_H

#include <stdio.h>
#include <stdlib.h>
#include <bson/bson.h>
#include <mongoc.h>
#include <Python.h>

#include "db_connector.h"
#include "cJSON.h"
#include <gtk/gtk.h>
#include <cairo.h>
#include <cairo-pdf.h>

#ifdef _DEBUG
#pragma comment(lib, "../lib/curl/libcurl_a_debug.lib")
#else 
#pragma comment (lib, "../lib/curl/libcurl_a.lib")
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define TABLE_SIZE 1000

GtkWidget* window;

typedef struct HashEntry {
    unsigned long hash;
    char* key;
    struct HashEntry* next;
} HashEntry;

typedef struct LinkedListNode {
    char* key;
    struct LinkedListNode* next;
} LinkedListNode;

typedef struct HashTable {
    HashEntry* buckets[TABLE_SIZE];
    LinkedListNode* insertion_order;
} HashTable;

GtkWidget* date_text;
GtkWidget* satellite_text_long;
GtkWidget* satellite_text_lat;
GHashTable* combo_text_map;
GtkWidget* button_left2;
GtkWidget* button_right2;
GtkWidget* image;

char* satellite_full_str;
HashTable* table;
HashTable* python_table;

extern GdkPixbuf* base_pixbuf;
extern cairo_surface_t* overlay_surface;

enum Date {
    TODAY,
    YESTERDAY,
    WEEK,
    MONTH,
    YEAR
};

int date_flag;

enum SatelliteKey {
    ISS,
    PROXIMA1,
    PROXIMA2,
    CENTAURI1,
    WV1,
    KKS1,
    ZHUHAI1,
    DEIMOS1,
    NORSAT1,
    NORSAT2
};

enum Plot {
    HIST,                // Elevation (subjective perceivement of the altitude of the satellite, depends on observer POV) over time
    TIME_SERIES_LAT,     // Latitude vs time
    TIME_SERIES_LONG,    // Longitude vs time
    ROLL_STAT,           // Rolling mean and Rolling std for altitude
    DECL_AND_ALT_CORR,   // Correlation between the declination angle and altitude of the satellite
    MAP
};

int satellite_flag;
int satellite_plot_flag;

// Helper function to format a date into a string
void format_date(char* buffer, size_t buffer_size, struct tm* tm_info);

// Get the time period as a string for label text below the combo box
char* get_time_period(enum Date date);

// Use this when you add satellite text for display
void on_combo_box_change_date(GtkComboBox* combo_box, gpointer user_data);

// dynamically print Latitude and Longitude, and draw positions on map
void on_combo_box_change_satellite(GtkComboBox* combo_box, gpointer user_data);

// Switch visuals in image carousel on arrow button click. Has a counter that keeps track of the image displayed.
void switch_visual(GtkWidget* arrow_button, gpointer user_data);

// Retrieve necessary satellite data
char* satellite_query(enum SatelliteKey key);

char* process_json_for_label_text(char* string, char* parameter);

char* get_yesterdays_date();

// Function to create a new linked list node
unsigned long hash_string(const char* str);

// Free the linked list
HashTable* create_hash_table();

// Modify the insert function to update the linked list
void insert(HashTable* table, const char* key);

// Modify the free_hash_table function to also free the linked list
void free_hash_table(HashTable* table);

// Function to get the last value based on insertion order
char* get_last_value(HashTable* table);

void get_sat_info_from_name_and_date(char* active_text);

void set_label_position_text();

void update_image_with_dots(GtkWidget* image_widget);

void load_and_initialize_image();

void clear_overlay();

void draw_dot_on_overlay(double lon, double lat);

// Helper function that combines smaller functions to draw position markings, and also monitors date and satellite flags
static void calculate_and_draw_position_markings();

// Draw plots on button click
char* generate_plot_image(enum Plot plot_type);

char* call_python_module_histogram(PyObject* pValue);

// Parse data from satellite string and add to the list of Python objects
PyObject* get_data_for_python(char* parameter);

void download_txt(GtkWidget* arrow_button, gpointer user_data);

void download_xml(GtkWidget* arrow_button, gpointer user_data);

void download_pdf(GtkWidget* arrow_button, gpointer user_data);

// If the most recent satellite data is not available for download, a warning pops up
void no_sat_data_found_alert_modal(GtkWindow* parent);

static void draw_position_dot_for_pdf(char* text_longitude, char* text_latitude,
    cairo_t* cr, double image_x, double image_y, double image_width, double image_height);

#endif