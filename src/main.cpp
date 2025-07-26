#include "rotor_control.cpp"

#include <thread>
#include <mutex>

void print_help() {
    printf("Commands:\n");
    printf("  restart\n");
    printf("  read\n");
    printf("  stop\n");
    printf("  set-angles [az] [el]\n");
    printf("  set-power  [az] [el]\n");
    // set-direction
    // get-config
}


volatile double angles_reference_input[2] = {0,0};
volatile bool stop = false;
std::mutex m;

void get_reference_input() {
    double angles[2] = {0,0};
    while (true) {
        std::string inp_buff;
        std::getline(std::cin, inp_buff);
        if (inp_buff.find("stop") == 0) {
            stop = true;
            return;
        }
        unsigned idx = inp_buff.find(" ");
        std::string X = inp_buff.substr(0, idx);
        std::string Y = inp_buff.substr(idx+1, inp_buff.length()-(idx+1));
        try {
            angles[0] = std::stod(X);
            angles[1] = std::stod(Y);
            {
                const std::lock_guard<std::mutex> lock(m);
                angles_reference_input[0] = angles[0];
                angles_reference_input[1] = angles[1];
            }
        } catch (std::invalid_argument& e) {
            std::cout << "Incorrect angle input format, needs to be \"X Y\" where X and Y are ascii numbers." << std::endl;
        }
    }
}

int main(int argc, char *argv[]) {
    bool do_control = false;
    if (argc > 1) {  // test for commands
        if (std::strcmp(argv[1], "do-control") == 0) {
            do_control = true;
        } else if (std::strcmp(argv[1], "-h") == 0 || std::strcmp(argv[1], "help") == 0) {
            print_help();
            return 0;
        } else {
            if (setup_USB_UART_connection() != 0) return -1;
        }

        if (std::strcmp(argv[1], "restart") == 0) {
            basic_message_get_debug(CMD_RESTART_DEVICE);
            printf("Restart sent successfully\n");

        } else if (std::strcmp(argv[1], "read") == 0) {
            double angle_output[2];
            get_angles_100(angle_output);
            if (argc == 2) {
                printf("Motor_1 angle: %.2f, Motor_2 angle: %.2f\n", angle_output[0], angle_output[1]);
            } else {  // if another argument is given after "read" (any argument) print a stripped down version of the angles for good interfacing with other programs
                printf("%.2f,%.2f", angle_output[0], angle_output[1]);
            }

        } else if (std::strcmp(argv[1], "stop") == 0) {
            basic_message_get_debug(CMD_STOP);
            printf("Stop sent successfully\n");

        } else if (std::strcmp(argv[1], "set-angles") == 0) {
            double angles[2];
            angles[0] = std::stod(argv[2]);
            angles[1] = std::stod(argv[3]);
            set_angles_100(angles);
            printf("Angle %.2f %.2f set successfully\n", angles[0], angles[1]);

        } else if (std::strcmp(argv[1], "set-power") == 0) {
            int p1 = std::stoi(argv[2]);
            int p2 = std::stoi(argv[3]);
            set_motor_power(p1, p2);
            printf("Power set successfully\n");
        }
        // This command has the possibilty of running the rotor against one of the motors internal endstops, this may lock up one of the two motors or possibly damage the motors
        else if (std::strcmp(argv[1], "set-direction") == 0) {
            if (argc != 4) {
                printf("Movement duration not provided, direction not set\n");
            }
            int d = std::stoi(argv[2]);             // movement direction Right:0 Left:1 Up:2 Down:3
            double t = std::stod(argv[3]);          // time on seconds (decimal allowed)
            
            int RL = abs(1-d*2) == 1 ? 1-d*2 : 0;
            int UD = abs(1-(d-2)*2) == 1 ? 1-(d-2)*2 : 0;
            set_motor_direction(RL, UD);  // only one direction per message
            printf("Direction set successfully, running for %.3f\n", t);
            usleep((int)t * 1000000);         // Microsecond sleep
            set_motor_direction(0, 0);  // Stop the motors
            printf("Rotor stopped\n");

        } else if (std::strcmp(argv[1], "get-config") == 0) {
            if (argc != 3) {
                printf("missing config field value\n");
            }
            get_configuration(std::stoi(argv[2]));
        } else if ((std::strcmp(argv[1], "-h") != 0 && std::strcmp(argv[1], "help") != 0)) {
            print_help();
        }
    }

    if (DO_CONTROL || do_control) {
        if (setup_USB_UART_connection() != 0) return -1;
        
        double angles[2] = {0,0};
        double angles_old[2] = {0,0};
        std::thread thread(get_reference_input);
        while (!stop) {
            {
                const std::lock_guard<std::mutex> lock(m);
                angles[0] = angles_reference_input[0];
                angles[1] = angles_reference_input[1];
            }
            if (angles_old[0] != angles[0] || angles_old[1] != angles[1]) {
                angles_old[0] = angles[0];
                angles_old[1] = angles[1];

                set_angles(angles);

                printf("Angle %.2f %.2f set successfully\n", angles_reference_input[0], angles_reference_input[1]);
            } 
            else {
                usleep(10000);
            }
        }
        thread.join();

    } else if (argc == 1) {  // Testing area
        // print_help();
        if (setup_USB_UART_connection() != 0) return -1;
        double angle_output[2];

        while (true) {
            unsigned long start = std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
            get_angles_100(angle_output);
            printf("Motor_1 angle: %.2f, Motor_2 angle: %.2f\n", angle_output[0], angle_output[1]);
            unsigned long now = std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
            printf("ms duration: %lu \n", now - start);
        }

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
