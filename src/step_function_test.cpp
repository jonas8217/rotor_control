#include "rotor_control.cpp"

#include <iostream>
#include <fstream>
#include <chrono>

void do_step_response(double az_setpoint, double el_setpoint, bool L, bool R, bool U, bool D) {
    double angles_setpoint[2];
    angles_setpoint[0] = az_setpoint;
    angles_setpoint[1] = el_setpoint;
    // power in azimuth and elevation
    int p_az = 100;
    int p_el = 100;

    time_t rawtime;
    struct tm *timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, sizeof(buffer), "%Y_%m_%d_%H_%M_%S", timeinfo);
    std::string str(buffer);

    std::ofstream data_file;
    data_file.open((std::string) "test_data/" + buffer + "_Az_" + std::to_string(R) + "_El_" + std::to_string(U) + "_" + std::to_string((int)az_setpoint) + "_" + std::to_string((int)el_setpoint) + ".csv");
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

    set_motor_power(p_el, p_az);  // set power to testing value (100%)

    printf("Waiting for 4 second to settle after moving\n");
    sleep(4);  // wait for the rotor to settle
    printf("Running step response - Set-point_ Az %.2f El %.2f, Power: Az %d El %d\n", az_setpoint, el_setpoint, p_az * (L - R), p_el * (U - D));

    // begin collecting data
    get_angles_100(angles);
    std::chrono::time_point<std::chrono::system_clock> t_start = std::chrono::system_clock::now();
    // save current angle and commanded power
    double t = 0.0;
    data_file << 0 << "," << 0 << "," << 0 << "," << angles[0] << "," << angles[1] << std::endl;

    set_motor_direction(L, R, U, D);  // start moving up
    // collect data points in regular intervals
    while (t < 5) {
        get_angles_100(angles);
        // Time is taken after angles are retrieved from the rotor, may lead to inaccurate delays
        t = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - t_start).count();
        t /= 1000000;
        // printf("Current time t: %f\n", t);
        // printf("p_el: %d, U: %d, D: %d\n", p_el, U, D);
        data_file << t << "," << p_az * (L - R) << "," << p_el * (U - D) << "," << angles[0] << "," << angles[1] << std::endl;

        // rotor commands are blocking so no need to sleep due to "get_angles_100"
        // sleep(0.01);
    }

    set_motor_direction(0, 0, 0, 0);  // stop the rotor after 5 seconds
    data_file.close();
}

int main(int argc, char *argv[]) {
    if (setup_USB_UART_connection() != 0) return -1;

    // Do zigzaggin pattern for optimal time utilization and gather the data all at the same time (maybe warm up the system first)
    // zigzag pattern UP RIGHT UP LEFT UP ... UP LEFT DOWN RIGHT DOWN LEFT DOWN ...

    // 1 Set power 100 (and lower for more testing?)
    // 2 goto staring position for the measurements
    // 3 do a measurement moving in the indicated direction
    // 4 goto the next setpoint which should be relatively close to the position the next measurement should be made from (TODO Need to figure out how far the rotor should move for steady state)
    //  repeat from step 3 until the upper bound for elevation has been reached

    //               Az, El, L, R, U, D
    do_step_response(0, 90, 0, 0, 1, 0);  // Pointing up      , moving backwards (positive elevation)
    do_step_response(0, 90, 0, 0, 0, 1);  // Pointing up      , moving forwards (negative elevation)
    do_step_response(0, 90, 1, 0, 0, 0);  // Pointing up      , moving left/clockwise(from above, negative azimuth)
    do_step_response(0, 90, 0, 1, 0, 0);  // Pointing up      , moving right/counter-clockwise(from above, positive azimuth)
    // elevation 5 degrees above the horizon to not meet the motor endstop
    do_step_response(0, 5, 0, 0, 1, 0);  // Pointing forwards, moving up (positive elevation)
    // cannot move downwards
    do_step_response(0, 5, 1, 0, 0, 0);  // Pointing forwards, moving left/clockwise(from above, negative azimuth)
    do_step_response(0, 5, 0, 1, 0, 0);  // Pointing forwards, moving right/counter-clockwise(from above, positive azimuth)

    return 0;
    // reapeting reading section TODO : ...
}
