


#include <signal.h>
#include <rte_eal.h>
#include <rte_ethdev.h>


#include "include/MaoConstant.h"




struct port_runtime {
    unsigned short port_id;
    unsigned short lcore_id;
    unsigned xxx tx_buffer;
    unsigned xxx rx_buffer;
    unsigned char assigned; // avoid to read unused port
};


unsigned char need_shutdown = 0;
struct port_runtime port_runtimes[Mao_MAX_ETHPORTS];

static void sys_signal_processor(int signal) {
    switch(signal) {
        case SIGINT:
        case SIGTERM:
            need_shutdown = 1;
            break;
    }
}

void init_timer_subsystem() {
    // may need
    ;
}

void init_power_subsystem() {
    // may need
    ;
}

void check_lcore_config() {
    // todo
    ;
}

void init_port() {
    struct rte_eth_conf rte_port_config = {
            .rxmode = {
                    .max_rx_pkt_len = RTE_ETHER_MAX_LEN, // FIXME: Mao: Does it support JUMBO Frame?
                    .offloads = DEV_RX_OFFLOAD_CHECKSUM // DEV_RX_OFFLOAD_CRC_STRIP | DEV_RX_OFFLOAD_CHECKSUM
            },
            .txmode = {
                    .offloads = DEV_TX_OFFLOAD_IPV4_CKSUM | DEV_TX_OFFLOAD_TCP_CKSUM | DEV_TX_OFFLOAD_UDP_CKSUM,
            }
    };
    
    RTE_ETH_FOREACH_DEV(port_id) {
        rte_eth_dev_configure(port_id, Mao_RX_QUEUE_NUM_PER_PORT, Mao_TX_QUEUE_NUM_PER_PORT, &rte_port_config);
        rte_eth_dev_adjust_nb_rx_tx_desc(port_id, )
    }
}

int main (int argc, char ** argv) {

    // register callback for system signal, e.g. Ctrl+C or else
    signal(SIGINT, sys_signal_processor);
    signal(SIGTERM, sys_signal_processor);

    int ret;
    printf("Mao: Init EAL...\n");
    ret = rte_eal_init(argc, argv);
    if (ret < 0) {
        RTE_LOG(ERR, Mao, "Init EAL: Fail, invalid EAL parameters\n");
        rte_exit(EXIT_FAILURE, "Invalid EAL parameters\n");
    }

    init_timer_subsystem();
    init_power_subsystem();

    check_lcore_config();
    init_lcore();

    check_port_config();
    init_port();

    launch_port();

    wait_for_complete();


    clean(); // wait for detail

    goto cleanup;

cleanup:
    printf("Mao: clean EAL...\n");
    ret = rte_eal_cleanup();
    printf("Mao: clean EAL %u\n", ret);
    return 0;
}