//
// Created by lrieff on 09-12-20.
//

#include "ConnectWindow.h"
#include "ControllerWindow.h"

extern ControllerWindow g_ControllerWindow;
extern GtkApplication *g_App;
extern Driver g_Driver;

speed_t intToSpeedType(uint32_t n)
{
    switch (n) {
        case 9600: return B9600;
        case 19200: return B19200;
        case 38400: return B38400;
        case 57600: return B57600;
        case 115200: return B115200;
        case 230400: return B230400;
        case 460800: return B460800;
        case 500000: return B500000;
        case 576000: return B576000;
        case 921600: return B921600;
        case 1000000: return B1000000;
        case 1152000: return B1152000;
        case 1500000: return B1500000;
        case 2000000: return B2000000;
        case 2500000: return B2500000;
        case 3000000: return B3000000;
        case 3500000: return B3500000;
        case 4000000: return B4000000;
        default: throw std::runtime_error("BAUD rate not supported !");
    }
}

ConnectWindow::ConnectWindow(int32_t width, int32_t height):
    m_Width(width), m_Height(height)
{}

ConnectWindow &ConnectWindow::build(GtkApplication *app)
{
    // Creates the main window
    this->m_Window_Root = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(this->m_Window_Root), T500_ARM_DESKTOP_INITIAL_WINDOW_TITLE);
    gtk_window_set_default_size(GTK_WINDOW(this->m_Window_Root), this->m_Width, this->m_Height);
    gtk_window_set_resizable(GTK_WINDOW(this->m_Window_Root), false);

    // Creates the root box ( contains everything ), and adds it to the window
    this->m_Box_Root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(this->m_Window_Root), this->m_Box_Root);

    // Creates the path
    this->m_Box_Path = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    this->m_Label_Path = gtk_label_new("Path: ");
    this->m_Entry_Path = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(this->m_Entry_Path), "/dev/ttyACM0");

    gtk_box_pack_start(GTK_BOX(this->m_Box_Path), this->m_Label_Path, false, false, 0);
    gtk_box_pack_start(GTK_BOX(this->m_Box_Path), this->m_Entry_Path, false, false, 0);
    gtk_box_pack_start(GTK_BOX(this->m_Box_Root), this->m_Box_Path, false, false, 0);

    // Creates the baud
    this->m_Box_Baud = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    this->m_Label_Baud = gtk_label_new("Baud: ");
    this->m_Entry_Baud = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(this->m_Entry_Baud), "576000");

    gtk_box_pack_start(GTK_BOX(this->m_Box_Baud), this->m_Label_Baud, false, false, 0);
    gtk_box_pack_start(GTK_BOX(this->m_Box_Baud), this->m_Entry_Baud, false, false, 0);
    gtk_box_pack_start(GTK_BOX(this->m_Box_Root), this->m_Box_Baud, false, false, 0);

    // Creates the continue button
    this->m_Button_Continue = gtk_button_new_with_label("Continue");
    g_signal_connect(this->m_Button_Continue, "pressed",
                     G_CALLBACK(&ConnectWindow::onContinuePress), reinterpret_cast<void *>(this));
    gtk_box_pack_start(GTK_BOX(this->m_Box_Root), this->m_Button_Continue, false, false, 0);

    // Creates the progress bar
    this->m_ProgressBar_Connecting = gtk_progress_bar_new();
    gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(this->m_ProgressBar_Connecting), true);
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(this->m_ProgressBar_Connecting), "Press continue");
    gtk_box_pack_end(GTK_BOX(this->m_Box_Root), this->m_ProgressBar_Connecting, false, false, 0);

    return *this;
}

void ConnectWindow::onContinuePress(GtkButton *button, gpointer udata)
{
    ConnectWindow &instance = *reinterpret_cast<ConnectWindow *>(udata);

    g_Driver.setPath(gtk_entry_get_text(GTK_ENTRY(instance.m_Entry_Path)));
    const char *baud = gtk_entry_get_text(GTK_ENTRY(instance.m_Entry_Baud));

    std::cout << "Attempting to connect to STM32 on path: '" << g_Driver.getPath() << "', with BAUD rate: " << baud << std::endl;

    // Attempts to convert the baud rate, to an actual valid speed, if not
    //  throw error message
    try {
        g_Driver.setBaud(intToSpeedType(std::stoi(baud)));
    } catch (const std::runtime_error &e)
    {
        showErrorDialog(instance.m_Window_Root, "BAUD error", e.what());
        return;
    }

    // Attempts to connect to the device, and shows an error
    //  if anything goes wrong
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(instance.m_ProgressBar_Connecting), 0.2);
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(instance.m_ProgressBar_Connecting), "Connecting to driver ...");

    try {
        g_Driver.connect();
    } catch (const std::runtime_error &e)
    {
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(instance.m_ProgressBar_Connecting), 0.0);
        gtk_progress_bar_set_text(GTK_PROGRESS_BAR(instance.m_ProgressBar_Connecting), "Failure");
        showErrorDialog(instance.m_Window_Root, "Driver Error", e.what());
        return;
    }

    // Gets the required information from the device, and sets the global configuration
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(instance.m_ProgressBar_Connecting), 0.4);
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(instance.m_ProgressBar_Connecting), "Talking to driver ...");

    g_Driver.getInfo();
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(instance.m_ProgressBar_Connecting), 0.8);
    std::string message = "Connected to: " + g_Driver.getDeviceID();
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(instance.m_ProgressBar_Connecting), message.c_str());

    std::cout << "Connected to: '" << g_Driver.getDeviceID()
        << "', " << (uint32_t) g_Driver.getMotorCount() << " motors detected" << std::endl;

    // Closes the current window
    g_ControllerWindow.build(g_Driver.getMotorCount(), g_App).launch();
    gtk_window_close(GTK_WINDOW(instance.m_Window_Root));
}

ConnectWindow &ConnectWindow::launch(void)
{
    gtk_widget_show_all(this->m_Window_Root);
    return *this;
}