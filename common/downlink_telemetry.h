#ifndef DOWNLINK_TELEMETRY_H
#define DOWNLINK_TELEMETRY_H

struct downlink_telemetry {
    // Encrypted channel
    struct downlink_data d_d;
    struct downlink_QEYnet d_q;

    // Unencrypted channel
    struct downlink_aro d_a;
}

// Commands to send data
struct downlink_data {
    void send_telemetry();  // The 1 minute health checks
}

// Commands for the QEYnet project
struct downlink_QEYnet {
    void response_ping();                // Response ping after receiving ping() from uplink
    void response_packet(int packet);    // Response packet after receiving send_packet() from uplink
}

// Commands for the ARO
struct downlink_aro {
    void send_selfie(int selfie);       // Send the selfie data along with other metrics
    void selfie_fail();                 // Send notification if the selfie fails
}

#endif