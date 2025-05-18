#include "rotor_control.cpp"

#include <iostream>
#include <fstream>
#include <chrono>

void do_step_response(double az_setpoint, double el_setpoint, int RL_pow, int UD_pow) {
    double angles_setpoint[2];
    angles_setpoint[0] = az_setpoint;
    angles_setpoint[1] = el_setpoint;
    // power in azimuth and elevation

    time_t rawtime;
    struct tm *timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, sizeof(buffer), "%Y_%m_%d_%H_%M_%S", timeinfo);
    std::string str(buffer);

    std::ofstream data_file;
    data_file.open((std::string) "test_data/" + buffer + "_az_" + std::to_string(RL_pow) + "_el_" + std::to_string(UD_pow) + "_" + std::to_string((int)az_setpoint) + "_" + std::to_string((int)el_setpoint) + ".csv");
    data_file << "t,p_az,p_el,a_az,a_el" << std::endl;

    set_motor_power(80, 80);      // set initial power to go to starting position
    set_angles(angles_setpoint);  // goto staring position

    double angles[2];
    get_angles(angles);
    printf("Moving to setpoint %.2f, %.2f ...\n", angles_setpoint[0], angles_setpoint[1]);
    // wait for it to be done moving
    while (abs(angles[0] - angles_setpoint[0]) > 0.2 || abs(angles[1] - angles_setpoint[1]) > 0.2) {
        sleep(5);
        get_angles(angles);
        printf("Current position: %.2f,%.2f\n", angles[0], angles[1]);
    }

    set_motor_power(abs(RL_pow), abs(UD_pow));  // set power to testing value (100%)

    printf("Waiting for 4 second to settle after moving\n");
    sleep(4);  // wait for the rotor to settle
    printf("Running step response. Set-point: Az %.2f El %.2f, Power: Az %d El %d\n", az_setpoint, el_setpoint, RL_pow, UD_pow);

    // begin collecting data
    get_angles_100(angles);
    std::chrono::time_point<std::chrono::system_clock> t_start = std::chrono::system_clock::now();
    // save current angle and commanded power
    double t = 0.0;
    data_file << 0 << "," << 0 << "," << 0 << "," << angles[0] << "," << angles[1] << std::endl;

    set_motor_direction(sign(RL_pow), sign(UD_pow));  // start moving up
    // collect data points in regular intervals
    while (t < 5) {
        get_angles_100(angles);
        // Time is taken after angles are retrieved from the rotor, may lead to inaccurate delays
        t = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - t_start).count();
        t /= 1000000;
        // printf("Current time t: %f\n", t);
        // printf("p_el: %d, U: %d, D: %d\n", p_el, U, D);
        data_file << t << "," << RL_pow << "," << UD_pow << "," << angles[0] << "," << angles[1] << std::endl;

        // rotor commands are blocking so no need to sleep due to "get_angles_100"
        // sleep(0.01);
    }

    set_motor_direction(sign(RL_pow), sign(UD_pow));  // stop the rotor after 5 seconds
    data_file.close();
}

int main(int argc, char *argv[]) {
    if (setup_USB_UART_connection() != 0) return -1;

    //               Az, El, RL_pow, UD_pow
    do_step_response( 0, 90,      0,    100);  // Pointing up      , moving backwards (positive elevation)
    do_step_response( 0, 90,      0,   -100);  // Pointing up      , moving forwards (negative elevation)
    do_step_response( 0, 90,   -100,      0);  // Pointing up      , moving left/clockwise(from above, negative azimuth)
    do_step_response( 0, 90,    100,      0);  // Pointing up      , moving right/counter-clockwise(from above, positive azimuth)
    // elevation 5 degrees above the horizon to not meet the motor endstop
    do_step_response( 0,  5,      0,    100);  // Pointing forwards, moving up (positive elevation)
    // do_step_response( 0,  5,      0,   -100);  cannot move downwards 
    do_step_response( 0,  5,   -100,      0);  // Pointing forwards, moving left/clockwise(from above, negative azimuth)
    do_step_response( 0,  5,    100,      0);  // Pointing forwards, moving right/counter-clockwise(from above, positive azimuth)

    return 0;
}
