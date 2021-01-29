


#include <signal.h>
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_malloc.h>
#include <include/MaoMacroTool.h>


#include "include/MaoConstant.h"




//struct port_runtime {
//    unsigned short port_id;
//    unsigned short lcore_id;
////    unsigned xxx tx_buffer;
////    unsigned xxx rx_buffer;
//    unsigned char assigned; // avoid to read unused port
//};

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

// all runtime variables

struct lcore_runtime lcore_runtime_config[Mao_MAX_LCORE];
struct port_runtime port_runtime_config[Mao_MAX_ETHPORTS];
struct rte_mempool *rx_pktmbuf_pool[Mao_MAX_SOCKET]; // RX buffer
struct rte_eth_dev_tx_buffer *tx_buffers[Mao_MAX_ETHPORTS]; // TX buffer bind to per port.


unsigned char volatile need_shutdown = 0; // volatile is necessary.
//struct port_runtime port_runtimes[Mao_MAX_ETHPORTS];




static void sys_signal_processor(int signal) {
    switch (signal) {
        case SIGINT: // ctrl+C
            need_shutdown = 1;
            RTE_LOG(INFO, Mao, "get SIGINT signal %d, need_shutdown %d.\n", signal, need_shutdown);
            break;
        case SIGTERM: // kill
            need_shutdown = 1;
            RTE_LOG(INFO, Mao, "get SIGTERM signal %d, need_shutdown %d.\n", signal, need_shutdown);
            break;
        default:
            RTE_LOG(INFO, Mao, "get default signal %d, need_shutdown %d.\n", signal, need_shutdown);
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

void init_lcore_port_runtime_config() {

    unsigned int i;
    for (i = 0; i < ARRAY_SIZE(port_runtime_config); i++) {
        port_runtime_config[i].rx_lcore_id = Mao_LCORE_ID_INVALID;
        port_runtime_config[i].tx_lcore_id = Mao_LCORE_ID_INVALID;
    }

    // now, we do not need to init lcore_runtime_config manually.
};

void debug_inject_lcore_config() {
    lcore_runtime_config[0].enable = Mao_LCORE_ENABLED;
    lcore_runtime_config[0].nb_rx_port++;
    lcore_runtime_config[0].rx_port_ids[0] = 0;
    lcore_runtime_config[0].nb_tx_port++;
    lcore_runtime_config[0].tx_port_ids[0] = 0;
//    lcore_runtime_config[0].tx_buffers[0] = rte_zmalloc_socket("lcore0_tx_buffer",
//        RTE_ETH_TX_BUFFER_SIZE(Mao_MAX_PACKET_BURST), 0,rte_eth_dev_socket_id(0));

    lcore_runtime_config[1].enable = Mao_LCORE_ENABLED;
    lcore_runtime_config[1].nb_rx_port++;
    lcore_runtime_config[1].rx_port_ids[0] = 1;
    lcore_runtime_config[1].nb_tx_port++;
    lcore_runtime_config[1].tx_port_ids[0] = 1;
//    lcore_runtime_config[1].tx_buffers[0] = rte_zmalloc_socket("lcore1_tx_buffer",
//        RTE_ETH_TX_BUFFER_SIZE(Mao_MAX_PACKET_BURST), 0,rte_eth_dev_socket_id(1));
}

void load_lcore_config() {
    // todo : load from config file or CLI params

    //    debug_inject_lcore_config();
}

void complement_lcore_config() {

    unsigned int port_socket_id;
    unsigned int lcore_id;
    unsigned int port_id;
    RTE_ETH_FOREACH_DEV(port_id) {
        if (Mao_PORT_DISABLED_MANUALLY != port_runtime_config[port_id].enable) {

            port_socket_id = rte_eth_dev_socket_id(port_id);

            // complement RX lcore
            if (Mao_LCORE_ID_INVALID == port_runtime_config[port_id].rx_lcore_id) {
                // First, try to assign port to same socket.
                unsigned int select_lcore_id = Mao_LCORE_ID_INVALID;
                for (lcore_id = 0; lcore_id < ARRAY_SIZE(lcore_runtime_config); lcore_id++) {
                    if (Mao_LCORE_ID_INVALID == lcore_runtime_config[lcore_id].enable
                        || rte_lcore_to_socket_id(lcore_id) != port_socket_id) {
                        continue;
                    }
                    if (lcore_runtime_config[lcore_id].nb_rx_port < Mao_MAX_PORTS_PER_LCORE) {
                        select_lcore_id = lcore_id;
                        break;
                    }
                }
                if (Mao_LCORE_ID_INVALID == select_lcore_id) {
                    // Fallback, try to assign port to other socket.
                    for (lcore_id = 0; lcore_id < ARRAY_SIZE(lcore_runtime_config); lcore_id++) {
                        if (Mao_LCORE_ID_INVALID == lcore_runtime_config[lcore_id].enable) {
                            continue;
                        }
                        if (lcore_runtime_config[lcore_id].nb_rx_port < Mao_MAX_PORTS_PER_LCORE) {
                            select_lcore_id = lcore_id;
                            break;
                        }
                    }
                }

                if (Mao_LCORE_ID_INVALID != select_lcore_id) {
                    lcore_runtime_config[select_lcore_id].enable = Mao_LCORE_ENABLED;
                    lcore_runtime_config[select_lcore_id].rx_port_ids[lcore_runtime_config[select_lcore_id].nb_rx_port] = port_id;
                    lcore_runtime_config[select_lcore_id].nb_rx_port++;

                    port_runtime_config[port_id].enable = Mao_PORT_ENABLED;
                    port_runtime_config[port_id].rx_lcore_id = select_lcore_id;
                } else {
                    RTE_LOG(WARNING, Mao,
                            "Caution, fail to complement rx_lcore for port %d, its enable state is %d, rx_lcore %d, tx_lcore %d\n",
                            port_id, port_runtime_config[port_id].enable, port_runtime_config[port_id].rx_lcore_id,
                            port_runtime_config[port_id].tx_lcore_id);
                }
            }


            // complement TX lcore
            if (Mao_LCORE_ID_INVALID == port_runtime_config[port_id].tx_lcore_id) {
                // First, try to assign port to same socket.
                unsigned int select_lcore_id = Mao_LCORE_ID_INVALID;
                for (lcore_id = 0; lcore_id < ARRAY_SIZE(lcore_runtime_config); lcore_id++) {
                    if (Mao_LCORE_ID_INVALID == lcore_runtime_config[lcore_id].enable
                        || rte_lcore_to_socket_id(lcore_id) != port_socket_id) {
                        continue;
                    }
                    if (lcore_runtime_config[lcore_id].nb_tx_port < Mao_MAX_PORTS_PER_LCORE) {
                        select_lcore_id = lcore_id;
                        break;
                    }
                }
                if (Mao_LCORE_ID_INVALID == select_lcore_id) {
                    // Fallback, try to assign port to other socket.
                    for (lcore_id = 0; lcore_id < ARRAY_SIZE(lcore_runtime_config); lcore_id++) {
                        if (Mao_LCORE_ID_INVALID == lcore_runtime_config[lcore_id].enable) {
                            continue;
                        }
                        if (lcore_runtime_config[lcore_id].nb_tx_port < Mao_MAX_PORTS_PER_LCORE) {
                            select_lcore_id = lcore_id;
                            break;
                        }
                    }
                }

                if (Mao_LCORE_ID_INVALID != select_lcore_id) {
                    lcore_runtime_config[select_lcore_id].enable = Mao_LCORE_ENABLED;
                    lcore_runtime_config[select_lcore_id].tx_port_ids[lcore_runtime_config[select_lcore_id].nb_tx_port] = port_id;
                    lcore_runtime_config[select_lcore_id].nb_tx_port++;

                    port_runtime_config[port_id].enable = Mao_PORT_ENABLED;
                    port_runtime_config[port_id].tx_lcore_id = select_lcore_id;
                } else {
                    RTE_LOG(WARNING, Mao,
                            "Caution, fail to complement tx_lcore for port %d, its enable state is %d, rx_lcore %d, tx_lcore %d\n",
                            port_id, port_runtime_config[port_id].enable, port_runtime_config[port_id].rx_lcore_id,
                            port_runtime_config[port_id].tx_lcore_id);
                }
            }
        }
    }
}

unsigned char check_lcore_config() {
    //FIXME: may be enhanced.

    unsigned int i, j;
    for (i = 0; i < ARRAY_SIZE(lcore_runtime_config); i++) {
        if (lcore_runtime_config[i].nb_rx_port > Mao_MAX_PORTS_PER_LCORE ||
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

void show_lcore_port_runtime_config();


void init_lcore() {

}

void init_rx_pktmbuf_mem_pool() {

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

    char tmp_name[64];
    unsigned int socket_id;
    unsigned int lcore_id;
    for (lcore_id = 0; lcore_id < ARRAY_SIZE(lcore_runtime_config); lcore_id++) {

        // just create pool on the sockets to be use.

        if (Mao_LCORE_ENABLED == lcore_runtime_config[lcore_id].enable) {
            socket_id = rte_lcore_to_socket_id(lcore_id);
            if (0 == rx_pktmbuf_pool[socket_id]) {
                snprintf(tmp_name, sizeof(tmp_name), "rx-pktmbuf-pool-%d", socket_id);
                rx_pktmbuf_pool[socket_id] = rte_pktmbuf_pool_create(tmp_name, Mao_PACKET_MEMPOOL_PACKET_NUMBER,
                    Mao_PACKET_MEMPOOL_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE, socket_id);
            }
        }
    }
}

void init_tx_buffer() {

    // Now, tx buffer is closer to lcore.
    // By the way, we recommend that lcore is closer to port. That is what complement_lcore_config does.

    unsigned int ret;
    char tmp_name[64];
    unsigned int port_id;
    unsigned int socket_id;
    RTE_ETH_FOREACH_DEV(port_id) {
        if (Mao_PORT_DISABLED_MANUALLY != port_runtime_config[port_id].enable) {
            socket_id = rte_lcore_to_socket_id(port_runtime_config[port_id].tx_lcore_id);

            snprintf(tmp_name, sizeof(tmp_name), "tx-buffer-port-%d", port_id);
            tx_buffers[port_id] = rte_zmalloc_socket(tmp_name, RTE_ETH_TX_BUFFER_SIZE(Mao_MAX_PACKET_BURST), 0,
                                                     socket_id);

            ret = rte_eth_tx_buffer_init(tx_buffers[port_id], Mao_MAX_PACKET_BURST);
            if (ret == 0) {
                RTE_LOG(INFO, Mao, "Init tx_buffer for port %u, socket %d, tx_buffer %s, OK.\n", port_id, socket_id,
                        tmp_name);
            } else {
                RTE_LOG(ERR, Mao, "Fail, unable to init tx_buffer for port %u, socket %d, tx_buffer %s, ret: %d, %s.\n",
                        port_id, socket_id, tmp_name, ret, rte_strerror(-ret));
            }
        }
    }
}


void init_port() {

    // todo: iterate every lcore to init all rx & tx queues which it takes responsibility. 2021.01.22


    // FIXME: local_port_conf.intr_conf.rxq = 1;
    struct rte_eth_conf rte_port_config = {
            .rxmode = {
                    .max_rx_pkt_len = RTE_ETHER_MAX_LEN, // FIXME: Mao: Does it support JUMBO Frame?
                    .offloads = DEV_RX_OFFLOAD_CHECKSUM // DEV_RX_OFFLOAD_CRC_STRIP | DEV_RX_OFFLOAD_CHECKSUM
            },
            .txmode = {
                    .offloads = DEV_TX_OFFLOAD_IPV4_CKSUM | DEV_TX_OFFLOAD_TCP_CKSUM |
                                DEV_TX_OFFLOAD_UDP_CKSUM, // FIXME: to add DEV_TX_OFFLOAD_MBUF_FAST_FREE
            }
    };

    uint16_t nb_ports = rte_eth_dev_count_avail();
    RTE_LOG(INFO, Mao, "Port count %d\n", nb_ports);


    uint16_t nb_rx_desc = Mao_RX_DESC_PER_PORT;
    uint16_t nb_tx_desc = Mao_TX_DESC_PER_PORT;
    unsigned int ret;
    uint64_t port_id;
    unsigned int tx_socket_id;
    unsigned int rx_socket_id;
    RTE_ETH_FOREACH_DEV(port_id) {

        if (Mao_PORT_DISABLED_MANUALLY != port_runtime_config[port_id].enable) {

            struct rte_eth_dev_info port_dev_info;
            ret = rte_eth_dev_info_get(port_id, &port_dev_info);
            if (ret != 0) {
                RTE_LOG(WARNING, Mao, "Fail, unable to get dev info for port %lu, ignore this port. ret %d.\n", port_id,
                        ret);
            }

            rte_eth_dev_configure(port_id, Mao_RX_QUEUE_PER_PORT, Mao_TX_QUEUE_PER_PORT, &rte_port_config);
            rte_eth_dev_adjust_nb_rx_tx_desc(port_id, &nb_rx_desc, &nb_tx_desc);


//        //todo: init port's tx buffer & setup port's tx queue
//        //fixme: avoid O(n^2)
//        unsigned int lcore_id;
//        for (lcore_id = 0; lcore_id < ARRAY_SIZE(lcore_runtime_config); lcore_id++) {
//            //fixme: to check rte_lcore_is_enabled()
//            struct lcore_runtime* lcore_rt = &lcore_runtime_config[lcore_id];
//
//            unsigned int ret;
//            unsigned int i;
//            for (i = 0; i < lcore_rt->nb_tx_port; i++) {
//                if (lcore_rt->tx_port_ids[i] == port_id) {
//
//
//
//
//                    break;
//                }
//            }
//        }

            //FIXME: TODO fill txmode.offload, and so on
            port_dev_info.default_txconf;

            // Mao: now, one port - one queue(0) - one tx lcore.
            tx_socket_id = rte_lcore_to_socket_id(port_runtime_config[port_id].tx_lcore_id);
            ret = rte_eth_tx_queue_setup(port_id, 0, nb_tx_desc, tx_socket_id, &port_dev_info.default_txconf);
            if (ret == 0) {
                RTE_LOG(INFO, Mao, "Init setup tx queue for port %lu, socket %d, queue 0, OK.\n", port_id,
                        tx_socket_id);
            } else {
                RTE_LOG(ERR, Mao, "Fail, unable to Init setup tx queue for port %lu, socket %d, queue 0, ret %d, %s.\n",
                        port_id, tx_socket_id, ret, rte_strerror(-ret));
            }






//            //todo: init port's rx queue
//            for (lcore_id = 0; lcore_id < ARRAY_SIZE(lcore_runtime_config); lcore_id++) {
//                //fixme: to check rte_lcore_is_enabled()
//                struct lcore_runtime *lcore_rt = &lcore_runtime_config[lcore_id];
//
//                unsigned int ret;
//                unsigned int i;
//                for (i = 0; i < lcore_rt->nb_rx_port; i++) {
//                    if (lcore_rt->rx_port_ids[i] == port_id) {
//
//
//
//                        break;
//                    }
//                }
//            }

            //FIXME: TODO fill rxmode.offload, and so on
            port_dev_info.default_rxconf;

            // Mao: now, one port - one queue(0) - one rx lcore.
            rx_socket_id = rte_lcore_to_socket_id(port_runtime_config[port_id].rx_lcore_id);
            ret = rte_eth_rx_queue_setup(port_id, 0, nb_rx_desc, rx_socket_id,
                                         &port_dev_info.default_rxconf, rx_pktmbuf_pool[rx_socket_id]);
            if (ret == 0) {
                RTE_LOG(INFO, Mao, "Init setup rx queue for port %lu, socket %d, queue 0, OK.\n", port_id,
                        rx_socket_id);
            } else {
                RTE_LOG(ERR, Mao, "Fail, unable to Init setup rx queue for port %lu, socket %d, queue 0, ret %d, %s.\n",
                        port_id, rx_socket_id, ret, rte_strerror(-ret));
            }
        }
    }
}

void launch_port() {
    unsigned int ret;
    unsigned int port_id;
    RTE_ETH_FOREACH_DEV(port_id) {
        if (Mao_PORT_ENABLED == port_runtime_config[port_id].enable) {
            ret = rte_eth_dev_start(port_id);
            if (ret != 0) {
                RTE_LOG(ERR, Mao, "Fail, unable to start port %d, ret %d, %s.\n", port_id, ret, rte_strerror(-ret));
                continue;
            }

            ret = rte_eth_promiscuous_enable(port_id);
            if (ret != 0) {
                RTE_LOG(WARNING, Mao, "Caution, unable to set promiscuous for port %d, ret %d, %s.\n", port_id, ret,
                        rte_strerror(-ret));
            }

            // todo: init spinlock for rx interrupt.
        }
    }
}

int debug_loop() {
    // TEST PASS: RTE_LOG(INFO, Mao, "here is lcore %d, socket %d.\n", rte_lcore_id(), rte_socket_id());

    struct lcore_runtime *me = &lcore_runtime_config[rte_lcore_id()];
    if (Mao_LCORE_ENABLED != me->enable || (me->nb_rx_port == 0 && me->nb_tx_port == 0)) {
        return 0;
    }


//    RTE_LOG(INFO, Mao, "checked need shutdown %d, lcore %d, socket %d.\n", need_shutdown, rte_lcore_id(), rte_socket_id());
    while (!need_shutdown) {
//        if (need_shutdown) {
//            RTE_LOG(INFO, Mao, "IF need shutdown %d, lcore %d, socket %d.\n", need_shutdown, rte_lcore_id(), rte_socket_id());
//        }
        ;
    }
//    RTE_LOG(INFO, Mao, "while need shutdown %d, lcore %d, socket %d.\n", need_shutdown, rte_lcore_id(), rte_socket_id());
    return 0;
}

void launch_lcore() {
    rte_eal_mp_remote_launch(debug_loop, NULL, CALL_MAIN); /* lcore handler also executed by main core. */
}

void wait_for_complete() {

    // now, we use CALL_MAIN, so main lcore will first finish debug_loop, then come here to wait others.

    unsigned int lcore_id;
    unsigned int ret;
    RTE_LCORE_FOREACH_WORKER(lcore_id) {
        // TEST PASS: RTE_LOG(INFO, Mao, "To wait lcore %d, socket %d.\n", lcore_id, rte_lcore_to_socket_id(lcore_id));
        ret = rte_eal_wait_lcore(lcore_id);
        if (ret != 0) {
            RTE_LOG(WARNING, Mao, "Caution, wait_lcore returns %d, %s.\n", ret, rte_strerror(-ret));
        }
    }

}

void stop_port() {
    unsigned int port_id;
    unsigned ret;
    RTE_ETH_FOREACH_DEV(port_id) {
        ret = rte_eth_dev_stop(port_id);
        if (ret != 0) {
            RTE_LOG(WARNING, Mao, "Caution, stop device returns %d, %s.\n", ret, rte_strerror(-ret));
        }

        ret = rte_eth_dev_close(port_id); // may report 'Operation not permitted'.
        if (ret != 0) {
            RTE_LOG(WARNING, Mao, "Caution, close device returns %d, %s.\n", ret, rte_strerror(ret > 0 ? ret : -ret));
        }
    }
}

int main(int argc, char **argv) {

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


    init_lcore_port_runtime_config();
    load_lcore_config();
    complement_lcore_config(); // Mao: Important

    // FIXME: Caution, need to check if any port's tx_lcore/rx_lcore is invalid.
    ret = check_lcore_config();
    if (ret < 0) {
        RTE_LOG(ERR, Mao, "Fail, invalid lcore config.\n");
        rte_exit(EXIT_FAILURE, "Invalid lcore config");
    }

    show_lcore_port_runtime_config();

    init_lcore();

    init_rx_pktmbuf_mem_pool();
    init_tx_buffer();

//    check_port_config();
    init_port();

    launch_port();

    // todo :monitor link status in background

    launch_lcore();

    wait_for_complete();

    stop_port();

    goto cleanup;

    cleanup:
    printf("Mao: clean EAL...\n");
    ret = rte_eal_cleanup();
    printf("Mao: clean EAL %u\n", ret);
    return 0;
}