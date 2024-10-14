#include <string.h>
#include <stdio.h>

#include "satellite_viewer.h"

int main(int argc, char** argv) {
    GtkApplication* app = gtk_application_new("org.example.satellite_viewer", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    start_db_connection();

    return g_application_run(G_APPLICATION(app), argc, argv);
}
