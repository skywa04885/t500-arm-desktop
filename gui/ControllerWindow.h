//
// Created by lrieff on 09-12-20.
//

#ifndef T500_ARM_DESKTOP_CONTROLLERWINDOW_H
#define T500_ARM_DESKTOP_CONTROLLERWINDOW_H

#define T500_ARM_DESKTOP_CONTROLLER_WINDOW_TITLE        "T500 Motor-Controller"

#include <cstdio>
#include <gtk/gtk.h>

#include "dialogs.h"
#include "../Driver.h"

typedef struct {
    GtkWidget *motorSeparator;
    GtkWidget *motorRootBox;
    GtkWidget *motorButtonBox, *motorMoveBox, *motorStatusBox;
    GtkWidget *motorEnableButton, *motorDisableButton, *motorMoveButton;
    GtkWidget *motorPositionInput;
    GtkWidget *motorPositionLabel, *motorSpsLabel, *titleLabel;

    char motorPositionLabelText[64];
    char motorSpsLabelText[64];
} ControllerWindowGTKMotor;

class ControllerWindow {
public:
    ControllerWindow(int32_t width, int32_t height);

    ControllerWindow &build(uint8_t motorCount, GtkApplication *app);
    ControllerWindow &launch(void);

    static void onMovePressed(GtkButton *button, gpointer udata);
    static gboolean updateStats(void *udata);

    ~ControllerWindow(void) = default;
protected:
    int32_t m_Width, m_Height;

    GtkWidget *m_Window_Root;
    GtkWidget *m_ScrolledWindow_Motors;
    GtkWidget *m_Box_Root, *m_Box_Motors, *m_Box_Footer;
    GtkWidget *m_Image_Banner;
    GtkWidget *m_Label_Footer, *m_Label_FooterDevice;

    ControllerWindowGTKMotor *m_GTKMotors;
};

typedef struct {
    uint8_t motor;
    ControllerWindow *instance;
} ControllerWindowMovePressedArgs;

#endif //T500_ARM_DESKTOP_CONTROLLERWINDOW_H
