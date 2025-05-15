#include "rotor_control.cpp"

#include <iostream>
#include <fstream>
#include <chrono>


double angles_setpoint[2];
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

    set_motor_power(80,80); // set initial power to go to starting position


    // --- START --- This section incapsulates one step response test

    std::ofstream data_file;
    data_file.open((std::string)"test_data/" + "step_response_" +  + ".csv");
    data_file << "t,p_az,p_el,a_az,a_el" << std::endl;

    angles_setpoint[0] = 0;
    angles_setpoint[1] = 45;
    set_angles(angles_setpoint); // goto staring position

    double angles[2];
    get_angles(angles);
    printf("Moving to setpoint %.2f, %.2f ...\n", angles_setpoint[0], angles_setpoint[1]);
    // wait for it to be done moving
    while (abs(angles[0] - angles_setpoint[0]) > 1 || abs(angles[1] - angles_setpoint[1]) > 1) {
        sleep(5);
        get_angles(angles);
        printf("Current position: %.2f,%.2f\n",angles[0], angles[1]);
    }

    p_el = 100;
    p_az = 100;
    set_motor_power(p_el,p_az); // set power to testing value (100%)

    printf("Waiting for 1 second to settle after moving\n");
    sleep(2); // wait for the rotor to settle

    // begin collecting data
    get_angles_100(angles);
    std::chrono::time_point<std::chrono::system_clock> t_start = std::chrono::system_clock::now();
    // save current angle and commanded power
    double t = 0.0;
    data_file << 0 << "," << p_az*(L-R) << "," << p_el*(U-D) << "," << angles[0] << "," << angles[1] << std::endl;

    U = 1;
    set_motor_direction(L,R,U,D); // start moving up
    // collect data points in regular intervals
    while (t < 5) {
        get_angles_100(angles);
        // Time is taken after angles are retrieved from the rotor, may lead to inaccurate delays
        t = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - t_start).count();
        t /= 1000000;
        // printf("Current time t: %f\n", t);
        // printf("p_el: %d, U: %d, D: %d\n", p_el, U, D);
        data_file << t << "," << p_az*(L-R) << "," << p_el*(U-D) << "," << angles[0] << "," << angles[1] << std::endl;
        sleep(0.01); // TODO make sure interval is long enough
    }

    U = 0;
    set_motor_direction(L,R,U,D); // stop the rotor after 5 seconds
    data_file.close();
    // --- END ---


    return 0;
    // reapeting reading section TODO : ...

}
