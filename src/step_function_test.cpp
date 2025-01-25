#include "rotor_control.cpp"

#include <iostream>
#include <fstream>
#include <chrono>


double angles[2];
int p_az,p_el = 0; // power in azimuth and elevation
int L,R,U,D = 0; // directions to move: left, right, up, down

int main(int argc, char *argv[]) {

    SERIAL_PORT = open(PORT, O_RDWR);

    struct termios tty;
    if (setup_USB_UART_connection(&tty) != 0) {
        return -1;
    }



    // Do zigzaggin pattern for optimal time utilization and gather the data all at the same time (maybe warm up the system first)
    // zigzag pattern UP RIGHT UP LEFT UP ... UP LEFT DOWN RIGHT DOWN LEFT DOWN ...

    //1 Set power 100 (and lower for more testing?)
    //2 goto staring position for the measurements
    //3 do a measurement moving in the indicated direction
    //4 goto the next setpoint which should be relatively close to the position the next measurement should be made from (TODO Need to figure out how far the rotor should move for steady state)
    // repeat from step 3 until the upper bound for elevation has been reached

    set_motor_power(60,60); // set initial power to go to starting position


    // --- START --- This section incapsulates one step response test

    std::ofstream data_file((std::string)"test_data/" + "step_response_test_test" + ".csv");
    data_file << "t,p_az,p_el,a_az,a_el" << std::endl;

    angles[0] = 0;
    angles[1] = 0;
    set_angles(angles); // goto staring position

    p_el,p_az = 100;
    set_motor_power(p_el,p_az); // set power to testing value (100%)
    // Initial test to check distance covered for steady state
    
    // begin collecting data
    std::chrono::time_point t_start = std::chrono::system_clock::now();
    // save current angle and commanded power
    get_angles_100(angles);
    double t = (std::chrono::system_clock::now() - t_start).count();
    data_file << t << "," << p_az*(L-R) << "," << p_el*(U-D) << "," << angles[0] << "," << angles[1] << std::endl;

    U = 1;
    set_motor_direction(L,R,U,D); // start moving up
    // collect data points in regular intervals
    while (t < 5) {
        get_angles_100(angles);
        // Time is taken after angles are retrieved from the rotor, may lead to inaccurate delays
        double t = (std::chrono::system_clock::now() - t_start).count();
        data_file << t << "," << p_az*(L-R) << "," << p_el*(U-D) << "," << angles[0] << "," << angles[1] << std::endl;
        sleep(0.05); // TODO make sure interval is long enough
    }
    
    U = 0;
    set_motor_direction(L,R,U,D); // stop the rotor after 5 seconds
    data_file.close();
    // --- END ---


    return 0;
    // reapeting reading section TODO : ...

}