#include <stdint.h>
#include "ProtocolRoTxCfg.h"

#include <stdio.h>
#include <string.h>

#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>

#include <chrono>
#include <iostream>
#include <cstring>

#define PORT "/dev/ttyUSB0"
#define BAUDRATE B115200

#define MAX_RETURN_MSG_LEN 43 // specifically in set and get config otherwise ususally 12 bytes, only other exception is GET_OUTS which returns 2 bytes
#define TRANSMIT_MSG_LEN 13
#define NOMINAL_RETURN_MSG_BYTES 12

bool DEBUG = true;

int SERIAL_PORT;
uint8_t READ_BUF[MAX_RETURN_MSG_LEN];
uint8_t WRITE_BUF[TRANSMIT_MSG_LEN];


enum MSG_TYPE {
    CMD_GET_MOTOR_ANGLES,       // Get current motors positions
    CMD_GET_MOTOR_ANGLES_100,   // Get current motors positions. 0.01 resolution
    CMD_SET_ANGLES,             // Move motors to position.
    CMD_CFG_GET,                // (has not been setup) Get settings value. isSketchValue determines, if response provides value for current running settings or for prepared settings to be applied in bulk. Passing fieldId = 0 in response returns maximum fiedlId in fieldValue.f_word
    CMD_GET_SOFT_HARD,          // Get START and STOP settings (IMMEDIATELY/SOFTLY)
    CMD_SET_SOFT_HARD,          // Set start/stop immediately or softly settings.
    CMD_RESTART_DEVICE,         // Restarts device after 5 seconds. Payload restartConfirmValue must be set to: rotxMagicRestartDevice
    CMD_STOP,                   // Stop motors immediately.
    CMD_MOTORS,                 // Command motors move (left/right etc.)
    CMD_POWER,                  // Set motors power (0-100%). (Applied immediately, without stoping current move)
} msg_type;

const bool MSG_HAS_INPUT[] = {0,0,1,0,0,1,0,0,1,1};

const uint8_t MSG_ARRAYS[][13] = {
    { 0x57,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1f,0x20 },
    { 0x57,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x6f,0x20 },
    { 0x57,0x00,0x00,0x00,0x00,0x0a,0x00,0x00,0x00,0x00,0x0a,0x2f,0x20 },
    { 0x57,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xef,0x20 },
    { 0x57,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xa1,0x20 },
    { 0x57,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xa2,0x20 },
    { 0x57,0xef,0xbe,0xad,0xde,0x00,0x00,0x00,0x00,0x00,0x00,0xee,0x20 },
    { 0x57,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0f,0x20 },
    { 0x57,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x14,0x20 },
    { 0x57,0x00,0x00,0x00,0x00,0x4d,0x00,0x00,0x00,0x00,0x42,0xf7,0x20 }
};



int setup_USB_UART_connection(struct termios *tty) {
    // Info from this blog
    // https://blog.mbedded.ninja/programming/operating-systems/linux/linux-serial-ports-using-c-cpp/

    if (SERIAL_PORT < 0) {
        printf("Error %i from open: %s\n", errno, strerror(errno));
        return -1;
    }


    if(tcgetattr(SERIAL_PORT, tty) != 0) {
        printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
        return -1;
    }
    tty->c_cflag &= ~PARENB; // No parity bit
    tty->c_cflag &= ~CSTOPB; // Only one stop bit
    tty->c_cflag &= ~CSIZE; // Clear all the size bits then set:
    tty->c_cflag |= CS8; // 8 bits per byte
    
    tty->c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control 
    tty->c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)
    tty->c_lflag &= ~ICANON; // Disable cononical mode (don't process input)
    
    tty->c_lflag &= ~ECHO; // Disable echo
    tty->c_lflag &= ~ECHOE; // Disable erasure
    tty->c_lflag &= ~ECHONL; // Disable new-line echo

    tty->c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
    tty->c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off software flow ctrl
    tty->c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); // Disable any special handling of received bytes

    tty->c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty->c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed

    // temporaily set both to 0 to be able clear buffers
    tty->c_cc[VTIME] = 0;
    tty->c_cc[VMIN] = 0;

    // Baudrate is 9600, could possible be made faster TODO 
    cfsetispeed(tty, BAUDRATE);
    cfsetospeed(tty, BAUDRATE);

    if (tcsetattr(SERIAL_PORT, TCSANOW, tty) != 0) {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
        return -1;
    }

    read(SERIAL_PORT, &READ_BUF, MAX_RETURN_MSG_LEN); // clear buffer
    // Then setup for real configuration

    // Either wait for 12 bytes or for 1 decisecond to stop doing a blocking read, whichever comes first
    tty->c_cc[VTIME] = 1; // Block for 1 decisecond.
    tty->c_cc[VMIN] = 12;  // Block until 12 bytes are recieved
    // buffer setup for returned commands to allow for fast control-loop

    if (tcsetattr(SERIAL_PORT, TCSANOW, tty) != 0) {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
        return -1;
    }

    printf("USB-UART connection established\n");

    return 0;
}

void send(MSG_TYPE type) {
    if (!MSG_HAS_INPUT[type]) {
        for (int i = 0; i < TRANSMIT_MSG_LEN; i++) {
            WRITE_BUF[i] = MSG_ARRAYS[type][i];
        }
    } 
    
    if (DEBUG) {
        printf("Transmitting: ");
        for (int i = 0; i < TRANSMIT_MSG_LEN; i++) {
            printf("0x%02x ",WRITE_BUF[i]);
        }
        printf("\n");
    }

    write(SERIAL_PORT, WRITE_BUF, TRANSMIT_MSG_LEN);

    // clear WRITE_BUF to make sure no mistakes are made while testing message functions
    for (int i = 0; i < TRANSMIT_MSG_LEN; i++) {
        WRITE_BUF[i] = 0x00;
    }
}

void recv(int expected_return_bytes) {
    
    int num_bytes = 0;
    if (expected_return_bytes > NOMINAL_RETURN_MSG_BYTES) {
        printf("TODO, implement this functionality (reading more than 12 bytes per message)\n"); // the serial read should be set up to work with this, but it requires a little work in this implementation
    } 
    else {
        num_bytes = read(SERIAL_PORT, &READ_BUF, MAX_RETURN_MSG_LEN);
    }

    if (num_bytes < 0) {
        printf("Error reading: %s", strerror(errno));
    }
    if (num_bytes < expected_return_bytes) {
        printf("Return message incomplete expected: %i, Recieved: %i\n",expected_return_bytes, num_bytes);
    }

    if (DEBUG) {
        printf("Recieved:     ");
        for (int i = 0; i < expected_return_bytes; i++) {
            printf("0x%02x ",READ_BUF[i]);
        }
        printf("\n");
    }
}

void send_recv(MSG_TYPE type, int expected_return_bytes) {
    send(type);
    recv(expected_return_bytes);
}

void basic_message_get_debug(MSG_TYPE type) {
    send_recv(type, NOMINAL_RETURN_MSG_BYTES);
    if (!DEBUG) { // will already be printed if debug is on
        printf("Read bytes:\n");
        for (int i = 0; i < NOMINAL_RETURN_MSG_BYTES; i++) {
            printf("0x%02x ",READ_BUF[i]);
        }
        printf("\n");
    }
}

void get_angles(double* angle_output) {
    
    send_recv(CMD_GET_MOTOR_ANGLES, NOMINAL_RETURN_MSG_BYTES);

    // angle = StrToInt(receivedAngle) * divisor - 360 * divisor
    int a1 = READ_BUF[1+0]*1000 + READ_BUF[1+1]*100 + READ_BUF[1+2]*10 + READ_BUF[1+3]*1; // convert to integer
    int a2 = READ_BUF[6+0]*1000 + READ_BUF[6+1]*100 + READ_BUF[6+2]*10 + READ_BUF[6+3]*1;
    angle_output[0] = (a1 - 3600) / (double)READ_BUF[ 5]; // do the math
    angle_output[1] = (a2 - 3600) / (double)READ_BUF[10];
}

void get_angles_100(double* angle_output) { // unlike in CMD_GET_MOTOR_ANGLES the devisor is set to a constant 100, this makes room for one more byte of data for the angles
    
    send_recv(CMD_GET_MOTOR_ANGLES_100, NOMINAL_RETURN_MSG_BYTES);

    // angle = StrToInt(receivedAngle) * divisor - 360 * divisor
    int a1 = READ_BUF[1+0]*10000 + READ_BUF[1+1]*1000 + READ_BUF[1+2]*100 + READ_BUF[1+3]*10 + READ_BUF[1+4]*1; // convert to integer
    int a2 = READ_BUF[6+0]*10000 + READ_BUF[6+1]*1000 + READ_BUF[6+2]*100 + READ_BUF[6+3]*10 + READ_BUF[6+4]*1;
    angle_output[0] = (a1 - 36000) / 100.0; // do the math
    angle_output[1] = (a2 - 36000) / 100.0;
}

void setup_write_buffer_for_input(MSG_TYPE type) {
    for (int i = 0; i < TRANSMIT_MSG_LEN; i++) {
        WRITE_BUF[i] = MSG_ARRAYS[type][i];
    }
}

void get_soft_hard(bool s1, bool s2) {
    
    basic_message_get_debug(CMD_GET_SOFT_HARD);
}

void set_soft_hard(bool s1, bool s2) {
    
    setup_write_buffer_for_input(CMD_SET_SOFT_HARD);

    WRITE_BUF[5] = s1; WRITE_BUF[10] = s2;

    send(CMD_SET_SOFT_HARD);
}

void set_angles(double* angle_input) {

    int a1 = angle_input[0] * 10 + 3600;

    // angleToSend = IntToString(360 * divisor + (desiredAngle * divisor))
    std::string s1 = std::to_string((int)(360 * 10 + (angle_input[0] *10))); // do the math and convert to string
    std::string s2 = std::to_string((int)(360 * 10 + (angle_input[1] *10)));

    setup_write_buffer_for_input(CMD_SET_ANGLES);

    WRITE_BUF[1+0] = s1[0]; WRITE_BUF[1+1] = s1[1]; WRITE_BUF[1+2] = s1[2]; WRITE_BUF[1+3] = s1[3]; // convert to chars for the write buffer
    WRITE_BUF[6+0] = s2[0]; WRITE_BUF[6+1] = s2[1]; WRITE_BUF[6+2] = s2[2]; WRITE_BUF[6+3] = s2[3];

    send_recv(CMD_SET_ANGLES, NOMINAL_RETURN_MSG_BYTES);
}

void stop_rotor() {
    send_recv(CMD_STOP, NOMINAL_RETURN_MSG_BYTES);
}

void set_motor_direction(bool L, bool R, bool U, bool D) { // TODO might change input method 
    // sets the direction of the motor in which to apply the power setting from CMD_POWER
    
    // mmCmdStop = 0x00

    // mmCmdLeft = 0x01
    // mmCmdRight = 0x02
    // mmCmdUp = 0x04
    // mmCmdDown = 0x08
    // mmCmdLeftUp = 0x05
    // mmCmdRightUp = 0x06
    // mmCmdLeftDown = 0x09
    // mmCmdRightDown = 0x0A

    // half a byte indicating the desired direction
    // X  X  X  X
    // L  R  U  D
    // Either left or right and either up or down, alternatively all zero for stop

    if (L && R || U && R) {
        printf("Cannot command both directions at once!");
        return;
    }

    setup_write_buffer_for_input(CMD_MOTORS);

    std::cout << (L<<0) << (R<<1) << (U<<2) << (D<<3) << std::endl;

    uint8_t cmd_byte = ((L<<0) + (R<<1) + (U<<2) + (D<<3));

    std::cout << (int)cmd_byte << std::endl;

    WRITE_BUF[1] = cmd_byte;

    send(CMD_MOTORS);
}

void set_motor_power(int p1, int p2) {
    
    setup_write_buffer_for_input(CMD_POWER);

    WRITE_BUF[5] = p1; WRITE_BUF[10] = p2;

    send_recv(CMD_POWER, NOMINAL_RETURN_MSG_BYTES);
}

void generate_motor_commands(double* angle_input, int* direction_command, int* motor_power) { // TODO create other functions to validate currently chosen input/output format
    
}


int main(int argc, char *argv[]) { 

    SERIAL_PORT = open(PORT, O_RDWR);

    struct termios tty;
    if (setup_USB_UART_connection(&tty) != 0) {
        return -1;
    }

    if(argc == 2) { // test for basic commands
        if (std::strcmp(argv[1], "reset") == 0) {
            basic_message_get_debug(CMD_RESTART_DEVICE);
            printf("Reset sent successfully\n");
        }
        else if (std::strcmp(argv[1], "read")  == 0) {
            double angle_output[2];
            get_angles_100(angle_output);
            printf("Motor_1 angle: %.2f, Motor_2 angle: %.2f\n", angle_output[0], angle_output[1]);
        }
        else if (std::strcmp(argv[1], "stop")  == 0) {
            basic_message_get_debug(CMD_STOP);
            printf("Stop sent successfully\n");
        }
    }
    else { // do control loop ... for now testing area
        double angle_output[2];
        

        // while (true) {
        //     unsigned long start =
        //         std::chrono::system_clock::now().time_since_epoch() / 
        //         std::chrono::milliseconds(1);
        //     get_angles_100(angle_output);
        //     printf("Motor_1 angle: %.2f, Motor_2 angle: %.2f\n", angle_output[0], angle_output[1]);
        //     unsigned long now =
        //         std::chrono::system_clock::now().time_since_epoch() / 
        //         std::chrono::milliseconds(1);
        //     printf("ms duration: %lu \n", now-start);
        // }
        
        
        // double angle_output[2] = {0,0};
        // get_angles(angle_output);
        // printf("Motor_1 angle: %.2f, Motor_2 angle: %.2f\n", angle_output[0], angle_output[1]);

        // get_angles_100(angle_output);
        // printf("Motor_1 angle: %.2f, Motor_2 angle: %.2f\n", angle_output[0], angle_output[1]);

        // angle_output[0] = -24.5; angle_output[1] = 0.3;
        // set_angles(angle_output);

        // set_motor_direction(0,0,1,0);

        // printf("\nWrite buffer: ");
        // for (int i = 0; i < TRANSMIT_MSG_LEN; i++) {
        //     printf("0x%02x ",WRITE_BUF[i]);
        // }
        // printf("\n");


        // basic_message_get_debug(CMD_GET_SOFT_HARD);
        // set_soft_hard(0,0);
        // sleep(0.1);
        // basic_message_get_debug(CMD_GET_SOFT_HARD);

        get_angles_100(angle_output);
        printf("Motor_1 angle: %.2f, Motor_2 angle: %.2f\n", angle_output[0], angle_output[1]);
        set_motor_direction(0,0,0,1);
        set_motor_power(0,40);

        int t = 8;
        printf("sleeping for %i seconds\n", t);
        sleep(t);

        get_angles_100(angle_output);
        printf("Motor_1 angle: %.2f, Motor_2 angle: %.2f\n", angle_output[0], angle_output[1]);

        printf("\nStopping rotor\n");
        stop_rotor();
    }

    return 0;
}