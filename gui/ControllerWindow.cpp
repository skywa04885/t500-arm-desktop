//
// Created by lrieff on 09-12-20.
//

#include "ControllerWindow.h"

extern ControllerWindow g_ControllerWindow;
extern GtkApplication *g_App;
extern Driver g_Driver;

ControllerWindow::ControllerWindow(int32_t width, int32_t height):
    m_Width(width), m_Height(height)
{

}

ControllerWindow &ControllerWindow::build(uint8_t motorCount, GtkApplication *app)
{
    // Allocates the memory for the motors
    this->m_GTKMotors = reinterpret_cast<ControllerWindowGTKMotor *>(reinterpret_cast<ControllerWindowGTKMotor **>(malloc(
            sizeof(ControllerWindowGTKMotor) * motorCount)));

    // Creates the window and sets the title and default size
    this->m_Window_Root = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(this->m_Window_Root), T500_ARM_DESKTOP_CONTROLLER_WINDOW_TITLE);
    gtk_window_set_default_size(GTK_WINDOW(this->m_Window_Root), this->m_Width, this->m_Height);

    // Creates the root box ( contains everything ), and adds it to the window
    this->m_Box_Root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(this->m_Window_Root), this->m_Box_Root);

    /**********************************
     * Creates the header
     **********************************/

    // Creates the banner, and adds it to the root box
    this->m_Image_Banner = gtk_image_new_from_file("../assets/banner.jpg");
    gtk_box_pack_start(GTK_BOX(this->m_Box_Root), this->m_Image_Banner, false, false, 0);

    /**********************************
     * Creates the motor controller
     **********************************/

    // Adds the motor box to the final window
    this->m_ScrolledWindow_Motors = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(this->m_ScrolledWindow_Motors), 500);
    gtk_box_pack_start(GTK_BOX(this->m_Box_Root), this->m_ScrolledWindow_Motors, false, false, 0);

    this->m_Box_Motors = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(m_ScrolledWindow_Motors), this->m_Box_Motors);

    // Adds the motors
    for (uint8_t i = 0; i < motorCount; ++i)
    {
        auto *args = new ControllerWindowMovePressedArgs {
                .motor = i,
                .instance = this
        };

        ControllerWindowGTKMotor &gtkMotor = this->m_GTKMotors[i];

        // Creates the root box
        gtkMotor.motorRootBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

        // Creates the title label
        char *titleLabelText = new char[64];
        sprintf(titleLabelText, "Stepper %d", i);
        gtkMotor.titleLabel = gtk_label_new(titleLabelText);
        gtk_box_pack_start(GTK_BOX(this->m_Box_Motors), gtkMotor.titleLabel, false, false, 10);

        // Creates the sub-boxes, for the buttons, moving and status
        gtkMotor.motorButtonBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_box_pack_start(GTK_BOX(gtkMotor.motorRootBox), gtkMotor.motorButtonBox, false, false, 5);
        gtkMotor.motorMoveBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_box_pack_start(GTK_BOX(gtkMotor.motorRootBox), gtkMotor.motorMoveBox, false, false, 5);
        gtkMotor.motorStatusBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_box_pack_start(GTK_BOX(gtkMotor.motorRootBox), gtkMotor.motorStatusBox, false, false, 5);

        // Creates the elements of the button sub-box
        gtkMotor.motorEnableButton = gtk_button_new_with_label("Enable");
        gtk_box_pack_start(GTK_BOX(gtkMotor.motorButtonBox), gtkMotor.motorEnableButton, false, false, 5);
        gtkMotor.motorDisableButton = gtk_button_new_with_label("Disable");
        gtk_box_pack_start(GTK_BOX(gtkMotor.motorButtonBox), gtkMotor.motorDisableButton, false, false, 5);

        // Creates the elements of the move sub-box
        gtkMotor.motorPositionInput = gtk_entry_new();
        gtk_entry_set_max_length(GTK_ENTRY(gtkMotor.motorPositionInput), 10);
        gtk_entry_set_text(GTK_ENTRY(gtkMotor.motorPositionInput), "0");
        gtk_entry_set_input_purpose(GTK_ENTRY(gtkMotor.motorPositionInput), GTK_INPUT_PURPOSE_DIGITS);
        gtk_box_pack_start(GTK_BOX(gtkMotor.motorMoveBox), gtkMotor.motorPositionInput, true, true, 5);

        gtkMotor.motorMoveButton = gtk_button_new_with_label("Move");
        g_signal_connect(gtkMotor.motorMoveButton, "pressed", G_CALLBACK(ControllerWindow::onMovePressed), args);
        gtk_box_pack_start(GTK_BOX(gtkMotor.motorMoveBox), gtkMotor.motorMoveButton, false, false, 5);

        // Creates the rotating icon
        gtkMotor.motorMovingSpinner = gtk_spinner_new();
        gtk_spinner_stop(GTK_SPINNER(gtkMotor.motorMovingSpinner));
        gtk_box_pack_end(GTK_BOX(gtkMotor.motorStatusBox), gtkMotor.motorMovingSpinner, false, false, 5);

        // Creates the status elements
        gtkMotor.motorPositionLabel = gtk_label_new("Pos: *");
        gtk_box_pack_start(GTK_BOX(gtkMotor.motorStatusBox), gtkMotor.motorPositionLabel, false, false, 5);

        gtkMotor.motorSpsLabel = gtk_label_new("Sps: *");
        gtk_box_pack_start(GTK_BOX(gtkMotor.motorStatusBox), gtkMotor.motorSpsLabel, false, false, 5);

        // Adds the motor to the final window
        gtkMotor.motorSeparator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
        gtk_box_pack_start(GTK_BOX(this->m_Box_Motors), gtkMotor.motorRootBox, false, false, 0);
        if (i + 1 != motorCount)
            gtk_box_pack_start(GTK_BOX(this->m_Box_Motors), gtkMotor.motorSeparator, false, false, 0);
    }

    /**********************************
     * Creates the footer
     **********************************/

    // Creates the footer
    this->m_Box_Footer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    char *footerDeviceMessage = new char[128];
    sprintf(footerDeviceMessage, "%s on %s", g_Driver.getDeviceID().c_str(), g_Driver.getPath().c_str());

    this->m_Label_Footer = gtk_label_new("Developed by Luke A.C.A. Rieff at Fannst Software");
    this->m_Label_FooterDevice = gtk_label_new(footerDeviceMessage);

    gtk_box_pack_start(GTK_BOX(this->m_Box_Footer), this->m_Label_FooterDevice, false, false, 10);
    gtk_box_pack_end(GTK_BOX(this->m_Box_Footer), this->m_Label_Footer, false, false, 10);

    // Adds the footer to the result
    gtk_box_pack_end(GTK_BOX(this->m_Box_Root), this->m_Box_Footer, false, false, 10);

    return *this;
}

ControllerWindow &ControllerWindow::launch(void)
{
    gtk_widget_show_all(this->m_Window_Root);

    // Creates the updater
    g_timeout_add(101, ControllerWindow::updateStats, reinterpret_cast<void *>(this));
    return *this;
}

gboolean ControllerWindow::updateStats(void *udata)
{
    ControllerWindow &window = *reinterpret_cast<ControllerWindow *>(udata);

    try {
        for (uint8_t i = 0; i < g_Driver.getMotorCount(); ++i)
        {
            auto status = g_Driver.getMotorStats(i);

            sprintf(window.m_GTKMotors[i].motorPositionLabelText, "Pos: %d", status.pos);
            gtk_label_set_text(GTK_LABEL(window.m_GTKMotors[i].motorPositionLabel), window.m_GTKMotors[i].motorPositionLabelText);

            sprintf(window.m_GTKMotors[i].motorSpsLabelText, "SPS: %d", status.current_sps);
            gtk_label_set_text(GTK_LABEL(window.m_GTKMotors[i].motorSpsLabel), window.m_GTKMotors[i].motorSpsLabelText);

            if (status.moving) gtk_spinner_start(GTK_SPINNER(window.m_GTKMotors[i].motorMovingSpinner));
            else gtk_spinner_stop(GTK_SPINNER(window.m_GTKMotors[i].motorMovingSpinner));
        }
    } catch (const std::runtime_error &e)
    {
        std::cerr << "updateStats() failed : "  << e.what() << std::endl;
    }

    return TRUE;
}

void ControllerWindow::onMovePressed(GtkButton *button, gpointer udata)
{
    auto &args = *reinterpret_cast<ControllerWindowMovePressedArgs *>(udata);
    const char *posCString = gtk_entry_get_text(GTK_ENTRY(args.instance->m_GTKMotors[args.motor].motorPositionInput));

    // Parses the position
    int32_t pos;
    try {
        pos = std::stoi(posCString);
    } catch (...)
    {
        std::string message = std::string("Invalid position: ") + posCString;
        showErrorDialog(args.instance->m_Window_Root, "Conversion error", message.c_str());
        return;
    }

    // Sends the move command, and handles errors
    std::cout << "Moving motor " << (uint32_t) args.motor << " to position: " << posCString << std::endl;

    try {
        g_Driver.setMotorPos(args.motor, pos);
    } catch (const std::runtime_error &e)
    {
        showErrorDialog(args.instance->m_Window_Root, "Command error", e.what());
        return;
    }
}
