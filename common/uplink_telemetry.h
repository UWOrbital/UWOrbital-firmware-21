#ifndef UPLINK_TELEMETRY_H
#define UPLINK_TELEMETRY_H

struct uplink_telemetry {
    // Encrypted Channel
    struct uplink_data u_d;
    struct selfie_pass s_p;
    struct emergency_control e_c;
    struct uplink_QEYnet u_q;

    // Unencrypted Channel
    struct uplink_aro u_a;
    struct unencrypted_command u_c;
};

// Commands to send data 
struct uplink_data {
    void send_tle_data(int tle_data);       // Send the TLE data
    void request_data();                    // Request for telemetry data
}

// Commands from MCC for selfie_pass
struct selfie_pass {
    void new_selfie_pass(float longitude, float attitude, int time, int roll_angle, int passcode);     // Some parameters to take the selfie
}

// Commands during emergencies
struct emergency_control {
    void reboot_obc();                      // CDH emergency command to reboot the OBC
    void power_cutoff(int cutoff_type);     // EPS emergency command to cut off power to some payload/subsystem
    void detumble();                        // ADCS Emergency command to instruct manual detumbling
}

// Commands for the QEYnet project
struct uplink_QEYnet {
    void ping();                        // Ping the OBC to check if it is still connected to the laser
    void send_packet(int packet);       // Send a packet and wait for a response packet
    void idle();                        // Instruct the module to enter idle mode
    void laser_on();                    // Instruct the laser to turn on
    void laser_off();                   // Instruct the laser to turn off
}

// Commands when 48 hours has passed since the last encrypted communication
struct unencrypted_command {
    void set_new_key(int key);          // Set up new encryption keys with the CubeSat
}

// Commands for the ARO
struct uplink_aro {
    void establish_connection(int passcode);    // Send the passcode to establish connection
    void take_selfie();                         // Instruct the CubeSat to take a selfie
}


#endif