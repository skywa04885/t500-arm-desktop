//
// Created by lrieff on 09-12-20.
//

#include "Driver.h"

const char *getCommandErrorString(command_error_t err)
{
    switch (err)
    {
        case COMMAND_ERROR_MOTOR_NOT_KNOWN: return "Motor unknown";
        case COMMAND_ERROR_NOT_KNOWN: return "Command unknown";
        default: return "Error message not known";
    }
}

Driver::Driver():
    m_Open(false), m_Lock(false)
{}

Driver &Driver::connect(void)
{
    // Connects to the STM32
    this->m_FD = open(this->m_Path.c_str(), O_RDWR);
    if (this->m_FD < 0)
        throw std::runtime_error(std::string("open() failed: ") + strerror(errno));
    this->m_Open = true;

    // Reads the existing settings, and modifies them
    struct termios tty;
    if (tcgetattr(this->m_FD, &tty) != 0)
        throw std::runtime_error(std::string("tcgetattr() failed: ") + strerror(errno));

    tty.c_cflag &= ~PARENB;                 /* No Parity */
    tty.c_cflag &= ~CSTOPB;                 /* One Stop Bit */

    tty.c_cflag &= ~CSIZE;                  /* Clear previous bit-size bits */
    tty.c_cflag |= CS8;                     /* 8-Bit Bytes */

    tty.c_cflag &= ~CRTSCTS;                /* No RTS/CTS Hardware control flow */

    tty.c_cflag |= CREAD;                   /* Turn on read */
    tty.c_cflag |= CLOCAL;                  /* No modem specific signal lines */

    tty.c_lflag &= ~ICANON;                 /* Disable canonical mode */
    tty.c_lflag &= ~ECHO;                   /* Disable Echo */
    tty.c_lflag &= ~ECHOE;                  /* Disable Erasure */
    tty.c_lflag &= ~ECHONL;                 /* Disable newline echo */
    tty.c_lflag &= ~ISIG;                   /* Disable interpretation of INTR, QUIT and SUSP */

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); /* Disable flow control */
    tty.c_iflag &= ~(
            IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL
    ); /* No Special byte handling */

    tty.c_oflag &= ~OPOST;                  /* Prevent special interpretation of output bytes */
    tty.c_oflag &= ~ONLCR;                  /* Prevent newline conversion */

    tty.c_cc[VTIME] = 5;                    /* Block until bytes received */
    tty.c_cc[VMIN] = 100;                     /* One byte per read */

    cfsetispeed(&tty, this->m_Baud);        /* Set input speed */
    cfsetospeed(&tty, this->m_Baud);        /* Set output speed */

    if (tcsetattr(this->m_FD, TCSANOW, &tty) != 0)
        throw std::runtime_error(std::string("tcsetattr() failed: ") + strerror(errno));

    return *this;
}


Driver &Driver::getInfo(void)
{
    auto *pkt = reinterpret_cast<command_packet_t *>(this->m_PacketBuffer);

    // Writes the info command
    pkt->hdr.flags.error = 0;
    pkt->hdr.type = COMMAND_READ_INFO;
    pkt->body.size = 0;
    this->writeCommand(pkt);

    // Reads the response
    this->readCommand(reinterpret_cast<uint8_t *>(pkt), sizeof (this->m_PacketBuffer));
    if (pkt->hdr.flags.error)
    {
        auto *err = reinterpret_cast<command_arg_error_t *>(pkt->body.payload);
        throw std::runtime_error(std::string("Command failed: ") + getCommandErrorString(err->code));
    }

    // Gets the info from the response
    auto *info = reinterpret_cast<command_arg_info_t *>(pkt->body.payload);
    this->m_DeviceID = std::string(info->device_id, 8);
    this->m_MotorCount = info->motor_count;

    return *this;
}

command_response_motor_status_t Driver::getMotorStats(uint8_t motor)
{
    auto *pkt = reinterpret_cast<command_packet_t *>(this->m_PacketBuffer);

    // Writes the info command
    pkt->hdr.flags.error = 0;
    pkt->hdr.type = COMMAND_READ_MOTOR_STATUS;
    pkt->body.size = 1;
    pkt->body.payload[0] = motor;
    this->writeCommand(pkt);

    // Reads the response
    this->readCommand(reinterpret_cast<uint8_t *>(pkt), sizeof (this->m_PacketBuffer));
    if (pkt->hdr.flags.error)
    {
        auto *err = reinterpret_cast<command_arg_error_t *>(pkt->body.payload);
        throw std::runtime_error(std::string("Command failed: ") + getCommandErrorString(err->code));
    }


    return *reinterpret_cast<command_response_motor_status_t *>(pkt->body.payload);
}

Driver &Driver::setMotorPos(uint8_t motor, int32_t pos)
{
    auto *pkt = reinterpret_cast<command_packet_t *>(this->m_PacketBuffer);

    // Writes the info command
    pkt->hdr.flags.error = 0;
    pkt->hdr.type = COMMAND_SET_MOTOR_POS;
    pkt->body.size = sizeof (command_arg_set_motor_pos_t);

    auto *args = reinterpret_cast<command_arg_set_motor_pos_t *>(pkt->body.payload);
    args->motor = motor;
    args->pos = pos;

    this->writeCommand(pkt);

    // Reads the response
    this->readCommand(reinterpret_cast<uint8_t *>(pkt), sizeof (this->m_PacketBuffer));
    if (pkt->hdr.flags.error)
    {
        auto *err = reinterpret_cast<command_arg_error_t *>(pkt->body.payload);
        throw std::runtime_error(std::string("Command failed: ") + getCommandErrorString(err->code));
    }

    return *this;
}

void Driver::writeByte(uint8_t b)
{
    if (write(this->m_FD, &b, 1) <= 0)
        throw std::runtime_error(std::string("write() failed: ") + strerror(errno));
}

uint8_t Driver::readByte(void)
{
    uint8_t res;
    if (read(this->m_FD, &res, 1) <= 0)
        throw std::runtime_error(std::string("read() failed: ") + strerror(errno));

    return res;
}

void Driver::writeEncodedByte(uint8_t b)
{
    this->writeByte((b >> 4) & 0x0F);
    this->writeByte(b & 0x0F);
}

uint8_t Driver::readEncodedByte(void)
{
    uint8_t res, temp;

    // Reads the higher nibble
    temp = this->readByte();
    if (temp == COMMAND_FLAG_BYTE) return temp;
    res = (temp & 0x0F) << 4;

    // Reads the lower nibble
    temp = this->readByte();
    if (temp == COMMAND_FLAG_BYTE) return temp;
    res |= (temp & 0x0F);

    return res;
}

void Driver::writeCommand(command_packet_t *pkt)
{
    this->writeByte(COMMAND_FLAG_BYTE);

    // Writes the packet
    for (uint8_t i = 0; i < sizeof (command_packet_t); ++i)
        this->writeEncodedByte(((uint8_t *) pkt)[i]);

    // Writes the body
    for (uint8_t i = 0; i < pkt->body.size; ++i)
        this->writeEncodedByte(pkt->body.payload[i]);

    this->writeByte(COMMAND_FLAG_BYTE);
}

void Driver::readCommand(uint8_t *buffer, uint32_t size)
{
    uint32_t i = 0;
    uint8_t b;

    // Waits for an packet
    while (this->readEncodedByte() != COMMAND_FLAG_BYTE);

    // Starts reading the packet
    while (i < size)
    {
        b = this->readEncodedByte();
        if (b == COMMAND_FLAG_BYTE) break;
        else buffer[i++] = b;
    }
}

Driver::~Driver(void)
{
    if (this->m_Open) close(this->m_FD);
}