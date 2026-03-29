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

#include "FreeRTOS.h"
#include "stream_buffer.h"
#include "usbd_core.h"
#include "usbd_cdc.h"
volatile uint32_t debug_usb_irq_count = 0;

// Déclaration de la variable globale pour le tuyau (Stream Buffer)
StreamBufferHandle_t video_stream_buffer;

// Déclaration de la poignée (Handle) USB pour le STM32
USBD_HandleTypeDef hUsbDeviceFS;
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



/* ====================================================================
 * INTERFACE USB CDC (Virtual COM Port)
 * ==================================================================== */

// Le buffer temporaire utilisé par le matériel USB
__ALIGN_BEGIN uint8_t UserRxBufferFS[2048] __ALIGN_END;
__ALIGN_BEGIN uint8_t UserTxBufferFS[2048] __ALIGN_END;

static int8_t CDC_Init_FS(void) {
    USBD_CDC_SetTxBuffer(&hUsbDeviceFS, UserTxBufferFS, 0);

    USBD_CDC_SetRxBuffer(&hUsbDeviceFS, UserRxBufferFS);
    USBD_CDC_ReceivePacket(&hUsbDeviceFS);
    return (USBD_OK);
}

static int8_t CDC_DeInit_FS(void) { return (USBD_OK); }
/* Configuration par défaut du Port Serie Virtuel (115200 bauds, 8N1) */
static uint8_t LineCoding[7] = {
    0x00, 0xC2, 0x01, 0x00, /* Baudrate 115200*/
    0x00,                   /* 1 Stop Bit */
    0x00,                   /* Parity : None */
    0x08                    /* Data bits : 8 */
};

static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length)
{
    /* Si Windows modifie le baudrate via le gestionnaire de périphérique */
    if (cmd == 0x20) { // Commande CDC_SET_LINE_CODING
        memcpy(LineCoding, pbuf, 7);
    }
    /* Si Windows demande quel est le baudrate actuel */
    else if (cmd == 0x21) { // Commande CDC_GET_LINE_CODING
        memcpy(pbuf, LineCoding, 7);
    }

    return (USBD_OK);
}

static int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t *Len)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // SECURITE : On vérifie que le buffer vidéo existe avant de pousser dedans
    if (video_stream_buffer != NULL) {
        xStreamBufferSendFromISR(video_stream_buffer, Buf, *Len, &xHigherPriorityTaskWoken);
    }

    USBD_CDC_SetRxBuffer(&hUsbDeviceFS, &Buf[0]);
    USBD_CDC_ReceivePacket(&hUsbDeviceFS);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

    return (USBD_OK);
}

static int8_t CDC_TransmitCplt_FS(uint8_t *Buf, uint32_t *Len, uint8_t epnum) {
    return (USBD_OK);
}

// La structure officielle de STMicroelectronics pour la classe CDC
USBD_CDC_ItfTypeDef USBD_Interface_fops_FS = {
    CDC_Init_FS,
    CDC_DeInit_FS,
    CDC_Control_FS,
    CDC_Receive_FS,
	CDC_TransmitCplt_FS
};

extern PCD_HandleTypeDef hpcd_USB_OTG_FS;

void OTG_FS_IRQHandler(void)
{
	debug_usb_irq_count++;
    HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
}

/** Enumeration of the various broadcast modes that can be used. */
enum udp_broadcast_mode
{
    /** Transmit mode. Application will transmit a set amount of broadcast packets. */
    TX_MODE,
    /** Receive mode. Application will listen for any broadcast packets and process any that start
     * with @ref MMBC_KEY */
    RX_MODE
};

static volatile bool is_network_ready = false;

/* Callback pour savoir quand la connexion est prete*/
static void link_status_callback(const struct mmipal_link_status *link_status)
{
    if (link_status->link_state == MMIPAL_LINK_UP) {
        printf("\n>>> CONNECTE A OPENWRT <<<\n");
        is_network_ready = true;
    }
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
    err_t err;

    ip_set_option(pcb, SOF_BROADCAST);
    ip_addr_t dest_ip;
    IP4_ADDR(ip_2_ip4(&dest_ip), 192, 168, 12, 255); // Adresse IP Broadcast

    printf("Demarrage de la passerelle USB -> UDP (Video Stream)...\n");

    while (1)
    {
        struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, 1400, PBUF_RAM);

        if (p == NULL) {
            mmosal_task_sleep(1);
            continue;
        }

        // LE CHANGEMENT EST ICI : On attend 1000 ticks (1 seconde) au lieu de l'infini
        size_t received = xStreamBufferReceive(video_stream_buffer, p->payload, 1400, 1000);

        if (received > 0) {
            p->len = received;
            p->tot_len = received;

            LOCK_TCPIP_CORE();
            err = udp_sendto(pcb, p, &dest_ip, 1337);
            UNLOCK_TCPIP_CORE();
        } else {
            // Timeout de 1 seconde atteint : on affiche l'état matériel !
            printf("--- DIAGNOSTIC USB (IRQ : %lu) ---\n", debug_usb_irq_count);

            // 1. Vérification de l'horloge interne 48 MHz
            if (__HAL_RCC_GET_FLAG(RCC_FLAG_HSI48RDY)) {
                printf(" [OK] Oscillateur HSI48   : Allume et stable\n");
            } else {
                printf(" [XX] Oscillateur HSI48   : EN PANNE\n");
            }

            // 2. Vérification du CRS (Clock Recovery System)
            // C'est LE juge de paix. Si le STM32 entend le PC, il lève le flag SYNCF (Bit 1)
            uint32_t crs_status = CRS->ISR;
            if (crs_status & CRS_ISR_ESYNCF) {
                printf(" [OK] Synchro PC (CRS)    : VERROUILLEE (Le PC parle a la carte !)\n");
                // On efface le flag pour voir si le PC continue de parler
                __HAL_RCC_CRS_CLEAR_FLAG(RCC_CRS_FLAG_ESYNC);
            } else if (crs_status & CRS_ISR_SYNCERR) {
                printf(" [XX] Synchro PC (CRS)    : ERREUR DE SYNCHRONISATION\n");
            } else {
                printf(" [..] Synchro PC (CRS)    : En attente du signal PC...\n");
            }

            // 3. Vérification de l'alimentation des broches
            printf(" [INFO] RCC_CCIPR1 (ICLK) : 0x%08lX\n", RCC->CCIPR1);
            printf("------------------------------------\n");
        }

        pbuf_free(p);
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
 * Main entry point to the application. This will be invoked in a thread once operating system
 * and hardware initialization has completed. It may return, but it does not have to.
 */
extern USBD_DescriptorsTypeDef FS_Desc;

void app_init(void)
{
    printf("\n\n--- Passerelle USB Video (Built " __DATE__ " " __TIME__ ") ---\n\n");

    /* 1. CREATION DU BUFFER */
    video_stream_buffer = xStreamBufferCreate(2048, 1);
    if (video_stream_buffer == NULL) {
        printf("!!! Plus de RAM pour le buffer !!!\n");
        return;
    }

    /* 2. DEMARRAGE DE L'USB (UNE SEULE FOIS !) */
    printf("Initialisation du port USB CDC...\n");
    if (USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS) != USBD_OK) {
        printf("!!! CRASH USBD_Init !!!\n");
    }
    if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_CDC) != USBD_OK) {
        printf("!!! CRASH USBD_RegisterClass !!!\n");
    }
    USBD_CDC_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS);
    USBD_Start(&hUsbDeviceFS);
    printf("Port USB allume ! Branchez le cable au PC.\n");

    /* 3. DEMARRAGE DU WI-FI */
    app_wlan_init();
    mmipal_set_link_status_callback(link_status_callback);

    printf("Connexion a l'AP OpenWrt en cours...\n");
    app_wlan_start();

    mmwlan_ate_override_rate_control(MMWLAN_MCS_2, MMWLAN_BW_2MHZ, MMWLAN_GI_NONE);

    while (!is_network_ready) {
        mmosal_task_sleep(10);
    }

    /* 4. DEMARRAGE DE LA BOUCLE UDP */
    struct udp_pcb *pcb = init_udp_pcb();
    if (pcb != NULL) {
        udp_broadcast_tx_start(pcb);
    }
}
