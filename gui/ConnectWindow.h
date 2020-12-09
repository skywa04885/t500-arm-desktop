//
// Created by lrieff on 09-12-20.
//

#ifndef T500_ARM_DESKTOP_CONNECTWINDOW_H
#define T500_ARM_DESKTOP_CONNECTWINDOW_H

#define T500_ARM_DESKTOP_INITIAL_WINDOW_TITLE           "Select Device"

#include <iostream>
#include <cstdio>
#include <memory>

#include <gtk/gtk.h>

#include "../Driver.h"
#include "dialogs.h"

class ConnectWindow {
public:
    ConnectWindow(int32_t width, int32_t height);

    ConnectWindow &build(GtkApplication *app);
    ConnectWindow &launch(void);

    static void onContinuePress(GtkButton *button, gpointer udata);

    ~ConnectWindow(void) = default;
protected:
    int32_t m_Width, m_Height;

    GtkWidget *m_Window_Root;
    GtkWidget *m_Box_Root, *m_Box_Baud, *m_Box_Path;
    GtkWidget *m_Button_Continue;
    GtkWidget *m_Entry_Path, *m_Entry_Baud;
    GtkWidget *m_Label_Path, *m_Label_Baud;
    GtkWidget *m_ProgressBar_Connecting;
};


#endif //T500_ARM_DESKTOP_CONNECTWINDOW_H
