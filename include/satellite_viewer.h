#ifndef SATELLITE_VIEWER_H
#define SATELLITE_VIEWER_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <gtk/gtk.h>

#include "analyzer_core.h"

static void activate(GtkApplication* app, gpointer user_data) {

    //GtkWidget* window = gtk_application_window_new(app);
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Satellite Viewer");
    gtk_window_set_default_size(GTK_WINDOW(window), 1200, 600);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

    // Create a main horizontal box
    GtkWidget* hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    gtk_container_add(GTK_CONTAINER(window), hbox);

    // Create a vertical box for the dropdowns and buttons
    GtkWidget* vbox_main = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20); // coo coo
    GtkWidget* vbox1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20); // the cause of all evil is here at 20

    gtk_box_pack_start(GTK_BOX(hbox), vbox1, FALSE, FALSE, 20);

    // Create and add the first dropdown
    GtkWidget* combo_box1 = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_box1), "SPACE STATION - 25544");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_box1), "PROXIMA I - 43694");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_box1), "PROXIMA II - 43696");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_box1), "CENTAURI-I - 43809");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_box1), "WORLDVIEW-I (WV-I)-32060");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_box1), "KKS-I (KISEKI)-33499");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_box1), "ZHUHAI-I 02 (CAS-4B)-42759");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_box1), "DEIMOS-I - 35681");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_box1), "NORSAT-I - 42826");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_box1), "NORSAT-II - 42828");

    gtk_combo_box_set_active(combo_box1, 0);
    gtk_box_pack_start(GTK_BOX(vbox1), combo_box1, FALSE, FALSE, 10);

    // Create and add a label to display text below the satellite combo box
    satellite_full_str = satellite_query(ISS);

    char* text_longitude = process_json_for_label_text(satellite_full_str, "Longitude");
    size_t size_lon = strlen(text_longitude) + 15;
    char* text = (char*)malloc(size_lon);
    snprintf(text, size_lon, "%s: %s", "Longitude", text_longitude);
    satellite_text_long = gtk_label_new(text);
    gtk_box_pack_start(GTK_BOX(vbox1), satellite_text_long, FALSE, FALSE, 0);

    char* text_latitude = process_json_for_label_text(satellite_full_str, "Latitude");
    text = "";
    size_t size_lat = strlen(text_latitude) + 15;
    text = (char*)malloc(size_lat);
    snprintf(text, size_lat, "%s: %s", "Latitude", text_latitude);
    satellite_text_lat = gtk_label_new(text);
    gtk_box_pack_start(GTK_BOX(vbox1), satellite_text_lat, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(vbox_main), vbox1, FALSE, FALSE, 0); // coo coo

    // Create and add the second dropdown
    GtkWidget* date_combo_box = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(date_combo_box), "Today");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(date_combo_box), "Yesterday");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(date_combo_box), "This week");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(date_combo_box), "This month");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(date_combo_box), "This year");
    gtk_combo_box_set_active(date_combo_box, 0);
    gtk_box_pack_start(GTK_BOX(vbox1), date_combo_box, FALSE, FALSE, 0);

    // Create and add a label to display text below the date combo box
    date_text = gtk_label_new(get_time_period(TODAY));
    gtk_box_pack_start(GTK_BOX(vbox1), date_text, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(vbox_main), vbox1, FALSE, FALSE, 0); // coo coo

    GtkWidget* vbox2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    image = gtk_image_new_from_file("../../assets/map.jpg");

    load_and_initialize_image();
    draw_dot_on_overlay(atof(text_longitude), atof(text_latitude));
    update_image_with_dots(image);

    gtk_box_pack_start(GTK_BOX(vbox2), image, TRUE, TRUE, 10);

    // Create and add the second set of arrow buttons
    GtkWidget* button_box2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    button_left2 = gtk_button_new_from_icon_name("go-previous", GTK_ICON_SIZE_BUTTON);
    button_right2 = gtk_button_new_from_icon_name("go-next", GTK_ICON_SIZE_BUTTON);
    gtk_box_pack_start(GTK_BOX(button_box2), button_left2, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(button_box2), button_right2, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox2), button_box2, FALSE, FALSE, 10);

    gtk_box_pack_start(GTK_BOX(vbox_main), vbox2, FALSE, FALSE, 0); // coo coo

    // Add the vertical box with the image and second set of buttons to the main horizontal box
    //gtk_box_pack_start(GTK_BOX(hbox), vbox2, TRUE, TRUE, 10);
    gtk_box_pack_start(GTK_BOX(hbox), vbox_main, TRUE, TRUE, 10);

    // Create a horizontal box for the buttons below the image
    GtkWidget* button_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget* dwnl_pdf = gtk_button_new_with_label("PDF");
    GtkWidget* dwnl_txt = gtk_button_new_with_label("TXT");
    GtkWidget* dwnl_xml = gtk_button_new_with_label("XML");
    gtk_box_pack_start(GTK_BOX(button_hbox), dwnl_pdf, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(button_hbox), dwnl_txt, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(button_hbox), dwnl_xml, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(vbox2), button_hbox, FALSE, FALSE, 10);

    // Initialize the hash table for combo box item to text mapping
    combo_text_map = g_hash_table_new(g_str_hash, g_str_equal);

    // Define the mapping for combo box 2
    g_hash_table_insert(combo_text_map, "Today", get_time_period(TODAY));
    g_hash_table_insert(combo_text_map, "Yesterday", get_time_period(YESTERDAY));
    g_hash_table_insert(combo_text_map, "This week", get_time_period(WEEK));
    g_hash_table_insert(combo_text_map, "This month", get_time_period(MONTH));
    g_hash_table_insert(combo_text_map, "This year", get_time_period(YEAR));

    date_flag = TODAY;
    satellite_plot_flag = MAP;

    // Connect the signal handler for combo box selection change
    g_signal_connect(combo_box1, "changed", G_CALLBACK(on_combo_box_change_satellite), NULL);
    g_signal_connect(date_combo_box, "changed", G_CALLBACK(on_combo_box_change_date), NULL);
    g_signal_connect(button_left2, "clicked", G_CALLBACK(switch_visual), NULL);
    g_signal_connect(button_right2, "clicked", G_CALLBACK(switch_visual), NULL);

    g_signal_connect(dwnl_pdf, "clicked", G_CALLBACK(download_pdf), NULL);
    g_signal_connect(dwnl_txt, "clicked", G_CALLBACK(download_txt), NULL);
    g_signal_connect(dwnl_xml, "clicked", G_CALLBACK(download_xml), NULL);

    // Connect signals
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);
}

#endif