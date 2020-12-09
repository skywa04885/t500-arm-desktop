//
// Created by lrieff on 09-12-20.
//

#ifndef T500_ARM_DESKTOP_DRIVER_H
#define T500_ARM_DESKTOP_DRIVER_H

#include <iostream>
#include <string>
#include <cstdint>
#include <cstring>

#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <atomic>

#define COMMAND_FLAG_BYTE				0xD3

/**********************************
 * Commands
 **********************************/

typedef enum __attribute__ (( packed ))
{
    COMMAND_ERROR_MOTOR_NOT_KNOWN = 0,
    COMMAND_ERROR_NOT_KNOWN
} command_error_t;

typedef enum __attribute__ (( packed ))
{
    COMMAND_LOG = 0,
    COMMAND_SET_MOTOR_POS,
    COMMAND_SET_MOTOR_MIN_SPS,
    COMMAND_SET_MOTOR_MAX_SPS,
    COMMAND_SET_MOTOR_ENABLED,
    COMMAND_READ_MOTOR_STATUS,
    COMMAND_READ_INFO
} command_type_t;

typedef struct __attribute__ (( packed ))
{
    int32_t pos;
    uint16_t current_sps;
    unsigned enabled 	: 1;
    unsigned moving 	: 1;
    unsigned reserved	: 6;
} command_response_motor_status_t;

typedef struct __attribute__ (( packed ))
{
    uint8_t motor_count;
    char device_id[8];
} command_arg_info_t;

typedef struct __attribute__ (( packed ))
{
    command_error_t code;
} command_arg_error_t;

typedef struct __attribute__ (( packed ))
{
    uint8_t motor;
    int32_t pos;
} command_arg_set_motor_pos_t;

typedef struct __attribute__ (( packed ))
{
    unsigned error : 1;
    unsigned reserved : 7;
} command_hdr_flags_t;;

typedef struct __attribute__ (( packed ))
{
    command_type_t type;
    command_hdr_flags_t flags;
} command_hdr_t;

typedef struct __attribute__ (( packed ))
{
    uint16_t size;
    uint8_t payload[];
} command_body_t;

typedef struct __attribute__ (( packed )) {
    command_hdr_t hdr;
    command_body_t body;
} command_packet_t;

const char *getCommandErrorString(command_error_t err);

/**********************************
 * Driver
 **********************************/

class Driver {
public:
    Driver();

    inline void setPath(const std::string &path) noexcept
    { this->m_Path = path; }

    inline void setBaud(speed_t baud) noexcept
    { this->m_Baud = baud; }

    inline const std::string &getPath() const noexcept
    { return this->m_Path; }

    Driver &connect(void);
    Driver &getInfo(void);
    Driver &setMotorPos(uint8_t motor, int32_t pos);

    command_response_motor_status_t getMotorStats(uint8_t motor);

    void writeByte(uint8_t b);
    uint8_t readByte(void);

    void writeEncodedByte(uint8_t b);
    uint8_t readEncodedByte(void);

    void writeCommand(command_packet_t *pkt);
    void readCommand(uint8_t *buffer, uint32_t size);

    inline const std::string &getDeviceID() const noexcept
    { return this->m_DeviceID; }

    inline const uint8_t getMotorCount() const noexcept
    { return this->m_MotorCount; }

    ~Driver(void);
private:
    char m_PacketBuffer[512];
    std::string m_Path, m_DeviceID;
    speed_t m_Baud;
    int32_t m_FD;
    bool m_Open;

    std::atomic<bool> m_Lock;

    uint8_t m_MotorCount;
};


#endif //T500_ARM_DESKTOP_DRIVER_H
