#include <iostream>
#include <gtk/gtk.h>
#include "./gui/ControllerWindow.h"
#include "./gui/ConnectWindow.h"

GtkApplication *g_App;

ConnectWindow g_InitialWindow(200, 40);
ControllerWindow g_ControllerWindow(600, 700);
Driver g_Driver;

void activate(GtkApplication *, gpointer udata)
{
    g_InitialWindow.build(g_App).launch();
}

int main(int32_t argc, char **argv) {
    g_App = gtk_application_new("nl.fannst.t500_desktop", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(g_App, "activate", G_CALLBACK(activate), NULL);

    int32_t status = g_application_run(G_APPLICATION(g_App), argc, argv);
    g_object_unref(g_App);
    return status;
}
