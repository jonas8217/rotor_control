#include "rotor_control.cpp"

int main(int argc, char *argv[]) {

    SERIAL_PORT = open(PORT, O_RDWR);

    struct termios tty;
    if (setup_USB_UART_connection(&tty) != 0) {
        return -1;
    }

    if(argc > 1) { // test for commands
        if (std::strcmp(argv[1], "reset") == 0) {
            basic_message_get_debug(CMD_RESTART_DEVICE);
            printf("Reset sent successfully\n");
        }
        else if (std::strcmp(argv[1], "read")  == 0) {
            double angle_output[2];
            get_angles_100(angle_output);
            if (argc == 2) {
                printf("Motor_1 angle: %.2f, Motor_2 angle: %.2f\n", angle_output[0], angle_output[1]);
            }
            else { // if another argument is given after "read" (any argument) print a stripped down version of the angles for good interfacing with other programs
                printf("%.2f,%.2f",angle_output[0], angle_output[1]);
            }
        }
        else if (std::strcmp(argv[1], "stop")  == 0) {
            basic_message_get_debug(CMD_STOP);
            printf("Stop sent successfully\n");
        }
        else if (std::strcmp(argv[1], "set-position")  == 0) {
            double angles[2];
            angles[0] = std::stod(argv[2]);
            angles[1] = std::stod(argv[3]);
            set_angles(angles);
            printf("Angle %f %f set successfully\n", angles[0], angles[1]);
        }
        else if (std::strcmp(argv[1], "set-power")  == 0) {
            int p1 = std::stoi(argv[2]);
            int p2 = std::stoi(argv[3]);
            set_motor_power(p1,p2);
            printf("Power set successfully\n");
        }
        // This command has the possibilty of running the rotor against one of the motors internal endstops, this may lock up one of the two motors or possibly damage it
        else if (std::strcmp(argv[1], "set-direction")  == 0) { 
            if (argc != 4){
                printf("Movement duration not provided, direction not set\n");    
            }
            int d = std::stoi(argv[2]);    // movement direction Left:0 Right:1 Up:2 Down:3
            double t = std::stod(argv[3]); // time on seconds (decimal allowed)
            set_motor_direction(d==0,d==1,d==2,d==3); // only one direction per message
            printf("Direction set successfully, running for %.3f\n", t);
            usleep((int)t*1000000); // Microsecond sleep
            set_motor_direction(0,0,0,0); // Stop the motors
            printf("Rotor stopped\n");
        }

    } 
    else if (DO_CONTROL) {

    }
    else { // Testing area
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

        // get_angles_100(angle_output);
        // printf("Motor_1 angle: %.2f, Motor_2 angle: %.2f\n", angle_output[0], angle_output[1]);
        // set_motor_direction(0,0,1,0);
        // set_motor_power(0,40);

        // int t = 8;
        // printf("sleeping for %i seconds\n", t);
        // sleep(t);

        // get_angles_100(angle_output);
        // printf("Motor_1 angle: %.2f, Motor_2 angle: %.2f\n", angle_output[0], angle_output[1]);

        // printf("\nStopping rotor\n");
        // stop_rotor();
    }

    return 0;
}