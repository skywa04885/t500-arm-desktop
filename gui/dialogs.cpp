//
// Created by lrieff on 09-12-20.
//

#include "dialogs.h"

void showErrorDialog(GtkWidget *parent, const char *title, const char *msg)
{
    GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(parent), flags,
        GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE, "%s", title);
    gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", msg);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}
