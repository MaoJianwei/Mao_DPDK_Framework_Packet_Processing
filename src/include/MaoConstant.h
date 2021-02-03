

#define RTE_LOGTYPE_Mao RTE_LOGTYPE_USER1

// capabilities of framework
#define Mao_MAX_SOCKET 2 // Can you help us to test the case of more than 2 sockets?
#define Mao_MAX_LCORE RTE_MAX_LCORE // RTE_MAX_LCORE 128
#define Mao_MAX_ETHPORTS RTE_MAX_ETHPORTS // RTE_MAX_ETHPORTS 32

// capabilities of applications
#define Mao_MAX_PORTS_PER_LCORE 4




// solid configuration
#define Mao_RX_QUEUE_PER_PORT 1
#define Mao_TX_QUEUE_PER_PORT 1

#define Mao_RX_DESC_PER_PORT 1024
#define Mao_TX_DESC_PER_PORT 1024


// general configuration
#define Mao_MAX_PACKET_BURST 32
#define Mao_PACKET_MEMPOOL_CACHE_SIZE 256


// debug configuration
#define Mao_PACKET_MEMPOOL_PACKET_NUMBER (8192 - 1)




// tools constant
#define Mao_PORT_ID_INVALID ((unsigned int)0xFFFFFFFF)
#define Mao_LCORE_ID_INVALID ((unsigned int)0xFFFFFFFF)

enum mao_port_state {
    // The order is important
    Mao_PORT_STATE_IDLE,
    Mao_PORT_STATE_ENABLE,
    Mao_PORT_STATE_DISABLE_MANUALLY,
    Mao_PORT_STATE_MAX
};
const char * mao_port_state_name[] = {
        "Idle",
        "Enabled",
        "Disabled",
        "Invalid(MAX)"
};


enum mao_lcore_state {
    // The order is important
    Mao_LCORE_STATE_IDLE,
    Mao_LCORE_STATE_ENABLE,
    Mao_LCORE_STATE_DISABLED_MANUALLY,
    Mao_LCORE_STATE_MAX
};
const char * mao_lcore_state_name[] = {
        "Idle",
        "Enabled",
        "Disabled",
        "Invalid(MAX)"
};

