//
// Created by ip on 2/4/21.
//

#ifndef MAO_DPDK_FRAMEWORK_PACKET_PROCESSOR_MAIN_H
#define MAO_DPDK_FRAMEWORK_PACKET_PROCESSOR_MAIN_H

struct lcore_runtime {
    unsigned int enable;

    unsigned int nb_rx_port;
    unsigned int rx_port_ids[Mao_MAX_PORTS_PER_LCORE];

    unsigned int nb_tx_port;
    unsigned int tx_port_ids[Mao_MAX_PORTS_PER_LCORE];
};

// now, one port - one queue.
struct port_runtime {
    unsigned int enable;
    unsigned int rx_lcore_id;
    unsigned int tx_lcore_id;
};

// treat void* as common data.
// 8 bytes in total.
struct port_interrupt_data {
    unsigned char port_id;
    unsigned char queue_id;
    unsigned char reserved[6];
};
#endif //MAO_DPDK_FRAMEWORK_PACKET_PROCESSOR_MAIN_H
