/*
 * Copyright 2021-2023 Morse Micro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Example app utilizing the LWIP "raw" API to handle UDP broadcast packets.
 *
 * @note It is assumed that you have followed the steps in the @ref GETTING_STARTED guide and are
 * therefore familiar with how to build, flash, and monitor an application using the MM-IoT-SDK
 * framework.
 *
 * # Operating Modes
 *
 * This example application supports a number of modes that can be set by writing the key @c
 * udp_broadcast.mode in the config store with the relevant mode.
 *
 * Config store value | Mode
 * -------------------|-----
 * tx                 | @ref UDP_BC_TX_MODE
 * rx                 | @ref UDP_BC_RX_MODE
 *
 * ## Transmit Mode {#UDP_BC_TX_MODE}
 *
 * In this mode the application demonstrates how to transmit a number of UDP broadcast packets. To
 * view the packets the application can use @c tcpdump on the AP.
 *
 * @note The reason for two copies of the packet in @c tcpdump is due to the fact that the STA first
 *       transmits the packet to the AP and then the AP broadcasts it to the network. This expected
 *       behavior.
 *
 * Example output from @c tcpdump :
 *
 * @code
 * root@morsemicro:~ $ tcpdump -A -i wlan0 -n "broadcast"
 * tcpdump: verbose output suppressed, use -v[v]... for full protocol decode
 * listening on wlan0, link-type EN10MB (Ethernet), snapshot length 262144 bytes
 * 01:51:49.865347 0c:bf:74:00:01:29 > ff:ff:ff:ff:ff:ff Null Unnumbered, xid, Flags [Response],
 * length 6: 01 00
 * ...
 * 01:51:49.902936 ARP, Request who-has 192.168.1.2 tell 192.168.1.2, length 28
 * ..........t..)..............
 * 01:51:49.902975 ARP, Request who-has 192.168.1.2 tell 192.168.1.2, length 28
 * ..........t..)..............
 * 01:51:51.432422 IP 192.168.1.2.1337 > 0.0.0.0.0: UDP, length 28
 * E..8.......
 * .........9...$..G'day World, packet no. 00..
 * 01:51:51.432440 IP 192.168.1.2.1337 > 0.0.0.0.0: UDP, length 28
 * E..8.......
 * .........9...$..G'day World, packet no. 00..
 * 01:52:01.309673 IP 192.168.1.2.1337 > 0.0.0.0.0: UDP, length 28
 * E..8.......	.........9...$..G'day World, packet no. 01..
 * 01:52:01.309700 IP 192.168.1.2.1337 > 0.0.0.0.0: UDP, length 28
 * E..8.......	.........9...$..G'day World, packet no. 01..
 * 01:52:11.186521 IP 192.168.1.2.1337 > 0.0.0.0.0: UDP, length 28
 * E..8.................9...$..G'day World, packet no. 02..
 * 01:52:11.186549 IP 192.168.1.2.1337 > 0.0.0.0.0: UDP, length 28
 * E..8.................9...$..G'day World, packet no. 02..
 * @endcode
 *
 * ## Receive Mode {#UDP_BC_RX_MODE}
 *
 * In receive mode the application demonstrates reception of UDP broadcast packets. In this mode a
 * callback function is registered with LWIP. This callback function will get executed every time
 * that packet is received. In this case the application have some additional logic that looks for a
 * specific packet format but this need not be the case.
 *
 * As mentioned above, the application has some additional logic to look for specific payloads in
 * the broadcast packets. The application uses this to blink the LEDs on any connected devices. The
 * payload has the following format:
 *
 * @code
 * +-----+--------------+--------------+       +----------------+
 * | Key | Color data 0 | Color data 1 | ..... | Color data n-1 |
 * +-----+--------------+--------------+       +----------------+
 * @endcode
 * > @b n is the number of devices. Key is a 32-bit little-endian number.
 *
 * By default the application will process the color data for @ref DEFAULT_UDP_BROADCAST_ID. However
 * this can be configured by setting @c udp_broadcast.id in the config store.
 *
 * To generate this payload a python script @c udp_broadcast_server.py has been provided in the
 * udp_broadcast/tools directory. You can configure your Morse Micro AP into bridge mode so that you
 * can access devices on the HaLow network, see user guide for AP on how to do this. Once that is
 * set up you can run the python script to start sending broadcast packets.
 *
 * @code
 * ./udp_broadcast_server.py
 * @endcode
 *
 * > There is a help menu for the python script that you can view for configuration settings. @code
 * > ./udp_broadcast_server.py -h @endcode
 *
 * # Configuration
 *
 * See @ref APP_COMMON_API for details of WLAN and IP stack configuration. Additional configuration
 * options for this application can be found in the config.hjson file.
 */

#include <string.h>
#include <endian.h>
#include "mmosal.h"
#include "mmwlan.h"
#include "mmconfig.h"

#include "mmipal.h"
#include "lwip/icmp.h"
#include "lwip/tcpip.h"
#include "lwip/udp.h"
#include "lwip/netif.h"


#include "mm_app_common.h"

/* Application default configurations. */

/** Number of broadcast packet to transmit */
#define DEFAULT_BROADCAST_PACKET_COUNT 100
/** UDP port to bind too. */
#define DEFAULT_UDP_PORT 1337
/** Interval between successive packet transmission. */
#define DEFAULT_PACKET_INTERVAL_MS 100
/** Maximum length of broadcast tx packet payload */
#define BROADCAST_PACKET_MAX_TX_PAYLOAD_LEN 35
/** Format string to use for the tx packet payload */
#define BROADCAST_PACKET_TX_PAYLOAD_FMT "G'day World, packet no. %lu."
/** Default mode for the application */
#define DEFAULT_UDP_BROADCAST_MODE TX_MODE
/** Default ID used in the rx metadata. */
#define DEFAULT_UDP_BROADCAST_ID 0

/** Key used to identify received broadcast packets. */
#define MMBC_KEY 0x43424d4d

/** Enumeration of the various broadcast modes that can be used. */
enum udp_broadcast_mode
{
    /** Transmit mode. Application will transmit a set amount of broadcast packets. */
    TX_MODE,
    /** Receive mode. Application will listen for any broadcast packets and process any that start
     * with @ref MMBC_KEY */
    RX_MODE
};

/** UDP broadcast rx payload format. */
PACK_STRUCT_STRUCT struct udp_broadcast_rx_payload
{
    /** Key used to identify payload.*/
    uint32_t key;

    /** Flexible array member used to access color data for each ID. */
    struct
    {
        /** Red intensity. */
        uint8_t red;
        /** Green intensity. */
        uint8_t green;
        /** Blue intensity. */
        uint8_t blue;
    } data[];
};

/** Struct used in rx mode for storing state. */
struct udp_broadcast_rx_metadata
{
    /** The last time in milliseconds that a valid payload was received. */
    uint32_t last_rx_time_ms;
    /** ID of the device, used to retrieve data from the payload. */
    uint32_t id;
};

/** Global data structure used in RX mode to record metadata. */
static struct udp_broadcast_rx_metadata rx_metadata = { 0 };


static volatile bool is_network_ready = false;

/* Callback pour savoir quand la connexion est prete (~50ms) */
static void link_status_callback(const struct mmipal_link_status *link_status)
{
    if (link_status->link_state == MMIPAL_LINK_UP) {
        printf("\n>>> CONNECTE A OPENWRT ! Ligne UP. <<<\n");
        is_network_ready = true;
    }
}

/**
 * Callback function to handle received data from the UDP pcb.
 *
 * @warning Be aware that @c addr might point into the pbuf @c p so freeing this pbuf can make
 *          @c addr invalid, too.
 *
 * @param arg   User supplied argument used to store a reference to the global rx_metadata struct.
 * @param pcb   The udp_pcb which received data
 * @param p     The packet buffer that was received
 * @param addr  The remote IP address from which the packet was received
 * @param port  The remote port from which the packet was received
 */
static void udp_raw_recv(void *arg,
                         struct udp_pcb *pcb,
                         struct pbuf *p,
                         const ip_addr_t *addr,
                         u16_t port)
{
    LWIP_UNUSED_ARG(pcb);
    LWIP_UNUSED_ARG(addr);
    LWIP_UNUSED_ARG(port);

    if (p == NULL)
    {
        return;
    }

    struct udp_broadcast_rx_metadata *metadata = (struct udp_broadcast_rx_metadata *)arg;
    struct udp_broadcast_rx_payload *payload = (struct udp_broadcast_rx_payload *)p->payload;
    uint32_t current_time_ms = mmosal_get_time_ms();

    /* This is the minimum length we need to prevent reading off the end of the payload. */
    uint32_t min_payload_len =
        sizeof(payload->key) + (sizeof(payload->data[0]) * (metadata->id + 1));

    if (p->len < min_payload_len)
    {
        printf("Payload length to short. Len: %u. Min len: %lu\n", p->len, min_payload_len);
        goto exit;
    }

    if (le32toh(payload->key) != MMBC_KEY)
    {
        printf("Invalid payload received.\n");
        goto exit;
    }

    printf("Valid payload received. \n"
           "    Time since last: %lums\n"
           "    Data recieved: 0x%02x%02x%02x\n\n",
           (current_time_ms - metadata->last_rx_time_ms),
           payload->data[metadata->id].red,
           payload->data[metadata->id].green,
           payload->data[metadata->id].blue);

    metadata->last_rx_time_ms = current_time_ms;

    mmhal_set_led(LED_RED, payload->data[metadata->id].red);
    mmhal_set_led(LED_GREEN, payload->data[metadata->id].green);
    mmhal_set_led(LED_BLUE, payload->data[metadata->id].blue);

exit:
    pbuf_free(p);
}

/**
 * Set a receive callback for the UDP PCB. This callback will be called when receiving a datagram
 * for the pcb.
 *
 * @param pcb UDP protocol control block to register the callback for
 */
static void udp_broadcast_rx_start(struct udp_pcb *pcb)
{
    mmconfig_read_uint32("udp_broadcast.id", &(rx_metadata.id));

    LOCK_TCPIP_CORE();
    udp_recv(pcb, udp_raw_recv, &rx_metadata);
    UNLOCK_TCPIP_CORE();
}

/**
 * Broadcast a udp packet every @ref DEFAULT_PACKET_INTERVAL_MS until @ref
 * DEFAULT_BROADCAST_PACKET_COUNT packets have been sent.
 *
 * @note If the parameters are set in the config store they will be used.
 *
 * @param pcb UDP protocol control block to use for transmission
 */
static void udp_broadcast_tx_start(struct udp_pcb *pcb)
{
    uint32_t count = 0;

    // 10ms = 100 Tirs par seconde ! (Ideal pour stream video UDP)
    //uint32_t packet_interval_ms = 10;

    ip_set_option(pcb, SOF_BROADCAST);

    // Cible Broadcast : C'est CA qui desactive les ACKs Wi-Fi !
    ip_addr_t dest_ip;
    IP4_ADDR(ip_2_ip4(&dest_ip), 192, 168, 12, 255);

    while (1)
        {
            // On tire une rafale de 5 paquets (5 x 1400 = 7000 octets d'un coup !)
            for (int i = 0; i < 10; i++) {
                struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, 1400, PBUF_RAM);
                if (p == NULL) {
                    break; // Plus de mémoire temporaire, on arrête la rafale
                }

                snprintf((char *)p->payload, p->len, "BACHELOR VIDEO PAYLOAD %lu", count);

                LOCK_TCPIP_CORE();
                udp_sendto(pcb, p, &dest_ip, 1337);
                UNLOCK_TCPIP_CORE();

                pbuf_free(p);
                count++;
            }

            if (count % 500 == 0) {
                printf("Stream Video Haute Vitesse... (%lu frames expediees)\n", count);
            }

            // On dort le minimum absolu (1 tick) pour laisser la couche MAC travailler
            mmosal_task_sleep(1);
        }
}

/**
 * Initialize the UDP protocol control block. Binds to @ref DEFAULT_UDP_PORT
 *
 * @note If the parameters are set in the config store they will be used.
 *
 * @return Reference to the pcb is successfully initialized else NULL
 */
static struct udp_pcb *init_udp_pcb(void)
{
    struct udp_pcb *pcb = NULL;
    LOCK_TCPIP_CORE();
    pcb = udp_new();
    if (pcb != NULL) {
        udp_bind(pcb, IP_ANY_TYPE, 1337);
    }
    UNLOCK_TCPIP_CORE();
    return pcb;
}
/**
 * Get the mode from config store.
 *
 * @return translates the value of @c udp_broadcast.mode into a @ref udp_broadcast_mode, if no valid
 *         mode is set @ref DEFAULT_UDP_BROADCAST_MODE is returned.
 */
static enum udp_broadcast_mode get_mode(void)
{
    enum udp_broadcast_mode mode = DEFAULT_UDP_BROADCAST_MODE;
    char mode_str[32];
    if (mmconfig_read_string("udp_broadcast.mode", mode_str, sizeof(mode_str)) > 0)
    {
        if (strcasecmp(mode_str, "tx") == 0)
        {
            mode = TX_MODE;
        }
        else if (strcasecmp(mode_str, "rx") == 0)
        {
            mode = RX_MODE;
        }
        else
        {
            printf("Unknown mode: %s. Reverting to default.\n", mode_str);
        }
    }

    return mode;
}

/**
 * Main entry point to the application. This will be invoked in a thread once operating system
 * and hardware initialization has completed. It may return, but it does not have to.
 */
void app_init(void)
{
    printf("\n\n--- BroadCastclassic  (Built " __DATE__ " " __TIME__ ") ---\n\n");

    app_wlan_init();

    /* On ecoute l'etat du reseau */
    mmipal_set_link_status_callback(link_status_callback);

    printf("Connexion a l'AP OpenWrt en cours...\n");
    app_wlan_start();

    mmwlan_ate_override_rate_control(MMWLAN_MCS_2, MMWLAN_BW_2MHZ, MMWLAN_GI_NONE);
    printf("forcage OK : 2 MHz / MCS 2 force.\n");

    /* On attend poliment que la connexion soit etablie (Fini le Failure 15 !) */
    while (!is_network_ready) {
        mmosal_task_sleep(10);
    }

    /* Le feu est vert, on lache le torrent de donnees */
    struct udp_pcb *pcb = init_udp_pcb();
    if (pcb != NULL) {
        udp_broadcast_tx_start(pcb);
    }

    /* Hack pour le compilateur */
    (void)get_mode;
    (void)udp_broadcast_rx_start;
}
