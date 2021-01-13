


#include <signal.h>
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_malloc.h>


#include "include/MaoConstant.h"




//struct port_runtime {
//    unsigned short port_id;
//    unsigned short lcore_id;
////    unsigned xxx tx_buffer;
////    unsigned xxx rx_buffer;
//    unsigned char assigned; // avoid to read unused port
//};

struct lcore_runtime {
    unsigned int lcore_id;

    unsigned int nb_rx_port;
    unsigned int rx_port_ids[Mao_MAX_PORTS_PER_LCORE];

    unsigned int nb_tx_port;
    unsigned int tx_port_ids[Mao_MAX_PORTS_PER_LCORE];
    struct rte_eth_dev_tx_buffer * tx_buffers[Mao_MAX_PORTS_PER_LCORE];
};


// all runtime variables

struct lcore_runtime lcore_runtime_config[Mao_MAX_LCORE];
struct rte_mempool* rx_pktmbuf_pool[Mao_MAX_SOCKET];

unsigned char need_shutdown = 0;
//struct port_runtime port_runtimes[Mao_MAX_ETHPORTS];




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

void debug_inject_lcore_config() {
    lcore_runtime_config[0].lcore_id = 0;
    lcore_runtime_config[0].nb_rx_port++;
    lcore_runtime_config[0].rx_port_ids[0] = 0;
    lcore_runtime_config[0].nb_tx_port++;
    lcore_runtime_config[0].tx_port_ids[0] = 0;
    lcore_runtime_config[0].tx_buffers[0] = rte_zmalloc_socket("lcore0_tx_buffer",
        RTE_ETH_TX_BUFFER_SIZE(Mao_MAX_PACKET_BURST), 0,rte_eth_dev_socket_id(0));

    lcore_runtime_config[1].lcore_id = 1;
    lcore_runtime_config[1].nb_rx_port++;
    lcore_runtime_config[1].rx_port_ids[0] = 1;
    lcore_runtime_config[1].nb_tx_port++;
    lcore_runtime_config[1].tx_port_ids[0] = 1;
    lcore_runtime_config[1].tx_buffers[0] = rte_zmalloc_socket("lcore1_tx_buffer",
        RTE_ETH_TX_BUFFER_SIZE(Mao_MAX_PACKET_BURST), 0,rte_eth_dev_socket_id(1));
};

unsigned char check_lcore_config() {
    //FIXME: may be enhanced.

    unsigned int i, j;
    for (i = 0; i < Mao_MAX_SOCKET; i++) {
        if (lcore_runtime_config[i].lcore_id >= Mao_MAX_LCORE ||
            lcore_runtime_config[i].nb_rx_port > Mao_MAX_PORTS_PER_LCORE ||
            lcore_runtime_config[i].nb_tx_port > Mao_MAX_PORTS_PER_LCORE)
            return -1;

        for (j = 0; j < lcore_runtime_config[i].nb_rx_port; j++) {
            if (lcore_runtime_config[i].rx_port_ids[j] >= Mao_MAX_ETHPORTS)
                return -1;
        }
        for (j = 0; j < lcore_runtime_config[i].nb_tx_port; j++) {
            if (lcore_runtime_config[i].tx_port_ids[j] >= Mao_MAX_ETHPORTS)
                return -1;
        }
    }
    return 0;
}

void init_lcore() {

}

void init_pktmbuf_mem_pool() {

/*
 * This expression is used to calculate the number of mbufs needed depending on
 * user input, taking into account memory for rx and tx hardware rings, cache
 * per lcore and mtable per port per lcore. RTE_MAX is used to ensure that
 * NB_MBUF never goes below a minimum value of 8192.
 */
//    uint16_t mempool_size = RTE_MAX	( \
//	(nb_ports*nb_rx_queue*nb_rxd + \
//	nb_ports*nb_lcores*MAX_PKT_BURST + \
//	nb_ports*n_tx_queue*nb_txd + \
//	nb_lcores*Mao_PACKET_MEMPOOL_CACHE_SIZE), \
//	(unsigned)8192)

    uint16_t mempoll_size = 8192;

    //fixme: todo
    unsigned int nb_socket = RTE_MIN(rte_socket_count(), Mao_MAX_SOCKET);

    //fixme: just create pool on the sockets to be use.
    char tmp_name[64];
    unsigned i;
    for (i = 0; i < nb_socket; i++) {
        snprintf(tmp_name, sizeof(tmp_name), "rx-pktmbuf-pool-%d", i);
        rx_pktmbuf_pool[i] = rte_pktmbuf_pool_create(tmp_name, Mao_PACKET_MEMPOOL_PACKET_NUMBER, Mao_PACKET_MEMPOOL_CACHE_SIZE,0, RTE_MBUF_DEFAULT_BUF_SIZE, i);
    }
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

    uint16_t nb_ports = rte_eth_dev_count_avail();
    RTE_LOG(INFO, Mao, "Port count %d\n", nb_ports);


    uint16_t nb_rx_desc = Mao_RX_DESC_PER_PORT;
    uint16_t nb_tx_desc = Mao_TX_DESC_PER_PORT;
    uint64_t port_id;
    RTE_ETH_FOREACH_DEV(port_id) {
        rte_eth_dev_configure(port_id, Mao_RX_QUEUE_PER_PORT, Mao_TX_QUEUE_PER_PORT, &rte_port_config);
        rte_eth_dev_adjust_nb_rx_tx_desc(port_id, &nb_rx_desc, &nb_tx_desc);
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
        RTE_LOG(ERR, Mao, "Fail, invalid EAL parameters\n");
        rte_exit(EXIT_FAILURE, "Invalid EAL parameters\n");
    }

    init_timer_subsystem();
    init_power_subsystem();

    debug_inject_lcore_config();
    ret = check_lcore_config();
    if (ret < 0) {
        RTE_LOG(ERR, Mao, "Fail, invalid lcore config.\n");
        rte_exit(EXIT_FAILURE, "Invalid lcore config");
    }

    init_lcore();
    init_pktmbuf_mem_pool();

//    check_port_config();
    init_port();

//    launch_port();
//
//    wait_for_complete();
//
//
//    clean(); // wait for detail

    goto cleanup;

cleanup:
    printf("Mao: clean EAL...\n");
    ret = rte_eal_cleanup();
    printf("Mao: clean EAL %u\n", ret);
    return 0;
}