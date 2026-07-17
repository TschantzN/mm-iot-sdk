/*
 * Copyright 2021-2023 Morse Micro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Example app utilizing the LWIP "raw" API to handle UDP broadcast
 * packets.
 *
 * @note It is assumed that you have followed the steps in the @ref
 * GETTING_STARTED guide and are therefore familiar with how to build, flash,
 * and monitor an application using the MM-IoT-SDK framework.
 *
 * # Operating Modes
 *
 * This example application supports a number of modes that can be set by
 * writing the key @c udp_broadcast.mode in the config store with the relevant
 * mode.
 *
 * Config store value | Mode
 * -------------------|-----
 * tx                 | @ref UDP_BC_TX_MODE
 * rx                 | @ref UDP_BC_RX_MODE
 *
 * ## Transmit Mode {#UDP_BC_TX_MODE}
 *
 * In this mode the application demonstrates how to transmit a number of UDP
 * broadcast packets. To view the packets the application can use @c tcpdump on
 * the AP.
 *
 * @note The reason for two copies of the packet in @c tcpdump is due to the
 * fact that the STA first transmits the packet to the AP and then the AP
 * broadcasts it to the network. This expected behavior.
 *
 * Example output from @c tcpdump :
 *
 * @code
 * root@morsemicro:~ $ tcpdump -A -i wlan0 -n "broadcast"
 * tcpdump: verbose output suppressed, use -v[v]... for full protocol decode
 * listening on wlan0, link-type EN10MB (Ethernet), snapshot length 262144 bytes
 * 01:51:49.865347 0c:bf:74:00:01:29 > ff:ff:ff:ff:ff:ff Null Unnumbered, xid,
 * Flags [Response], length 6: 01 00
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
 * In receive mode the application demonstrates reception of UDP broadcast
 * packets. In this mode a callback function is registered with LWIP. This
 * callback function will get executed every time that packet is received. In
 * this case the application have some additional logic that looks for a
 * specific packet format but this need not be the case.
 *
 * As mentioned above, the application has some additional logic to look for
 * specific payloads in the broadcast packets. The application uses this to
 * blink the LEDs on any connected devices. The payload has the following
 * format:
 *
 * @code
 * +-----+--------------+--------------+       +----------------+
 * | Key | Color data 0 | Color data 1 | ..... | Color data n-1 |
 * +-----+--------------+--------------+       +----------------+
 * @endcode
 * > @b n is the number of devices. Key is a 32-bit little-endian number.
 *
 * By default the application will process the color data for @ref
 * DEFAULT_UDP_BROADCAST_ID. However this can be configured by setting @c
 * udp_broadcast.id in the config store.
 *
 * To generate this payload a python script @c udp_broadcast_server.py has been
 * provided in the udp_broadcast/tools directory. You can configure your Morse
 * Micro AP into bridge mode so that you can access devices on the HaLow
 * network, see user guide for AP on how to do this. Once that is set up you can
 * run the python script to start sending broadcast packets.
 *
 * @code
 * ./udp_broadcast_server.py
 * @endcode
 *
 * > There is a help menu for the python script that you can view for
 * configuration settings. @code > ./udp_broadcast_server.py -h @endcode
 *
 * # Configuration
 *
 * See @ref APP_COMMON_API for details of WLAN and IP stack configuration.
 * Additional configuration options for this application can be found in the
 * config.hjson file.
 */

#include "mmconfig.h"
#include "mmosal.h"
#include "mmhal_wlan.h"
#include "mmwlan.h"
#include <endian.h>
#include <string.h>

#include "mm_app_common.h"
#include "mmdrv.h"
#include "mmpkt.h"
#include "stm32u5xx_hal.h"

// --- SPI ---
#define SPI_PAYLOAD_SIZE 1400

SPI_HandleTypeDef hspi1;
uint8_t spi_rx_buffer[SPI_PAYLOAD_SIZE];
volatile bool spi_packet_received = false;
// ---------------------
/* --- Configurations d'Injection Raw 802.11 --- */

/** Frequence cible */
#define RAW_TX_FREQ_MHZ 916

/** Index de canal (par convention, offset) */
#define RAW_TX_CHAN_IDX 0

/** Adresse MAC de destination (ici Broadcast) */
#define DEST_MAC_B1 0xFF
#define DEST_MAC_B2 0xFF
#define DEST_MAC_B3 0xFF
#define DEST_MAC_B4 0xFF
#define DEST_MAC_B5 0xFF
#define DEST_MAC_B6 0xFF

/* Adresse MAC de destination (MAC du routeur OpenWrt)
#define DEST_MAC_B1 0x12
#define DEST_MAC_B2 0x34
#define DEST_MAC_B3 0x56
#define DEST_MAC_B4 0x78
#define DEST_MAC_B5 0x9A
#define DEST_MAC_B6 0xBC

*/

/** Structure d'en-tete MAC 802.11 Data (100% brute) */
struct __attribute__((packed)) custom_80211_data_hdr {
  uint16_t frame_control;
  uint16_t duration;
  uint8_t addr1[6]; // RA (Receiver Address / Destination)
  uint8_t addr2[6]; // TA (Transmitter Address / Source)
  uint8_t addr3[6]; // BSSID
  uint16_t seq_ctrl;
};

// --- Variables de Profilage DWT ---
volatile uint32_t dwt_start = 0;
volatile uint32_t dwt_end = 0;
volatile uint32_t mcu_cycles = 0;
extern uint32_t
    SystemCoreClock; // Fourni par le système STM32 (ex: 160000000 pour 160 MHz)

static void raw_80211_tx_start(void) {
  // --- ACTIVATION DU COMPTEUR DE CYCLES DWT ---
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

  uint8_t my_mac[6] = {0};
  mmhal_read_mac_addr(my_mac);

  printf(">>> PONT SPI-WIFI RAW INJECTION ACTIVE ! <<<\n");
  printf("Canal fixe: %d MHz\n", RAW_TX_FREQ_MHZ);
  printf("En attente de la Rpi sur le SPI...\n");

  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
  uint32_t packet_counter = 0;

  while (1) {
    if (spi_packet_received) {

      // Allocation d'un paquet brut pour l'injection
      struct mmpkt *pkt =
          mmdrv_alloc_mmpkt_for_tx(MMDRV_PKT_CLASS_DATA_TID0, 0, 0);
      if (pkt != NULL) {
        struct mmpktview *view = mmpkt_open(pkt);
        // Reserve de la place pour le payload SPI + l'entete 802.11
        uint8_t *data = mmpkt_append(view, sizeof(struct custom_80211_data_hdr) +
                                              SPI_PAYLOAD_SIZE);
        if (data != NULL) {
          struct custom_80211_data_hdr *hdr =
              (struct custom_80211_data_hdr *)data;
          uint8_t *payload = data + sizeof(struct custom_80211_data_hdr);

          // --- Header 802.11 Data basique ---
          hdr->frame_control =
              htole16(0x0008); // Data frame (Type 2, Subtype 0)
          hdr->duration = 0;
          // RA (Receiver) = Broadcast ou MAC du Routeur
          hdr->addr1[0] = DEST_MAC_B1;
          hdr->addr1[1] = DEST_MAC_B2;
          hdr->addr1[2] = DEST_MAC_B3;
          hdr->addr1[3] = DEST_MAC_B4;
          hdr->addr1[4] = DEST_MAC_B5;
          hdr->addr1[5] = DEST_MAC_B6;
          // TA (Transmitter) = MAC du MCU
          memcpy(hdr->addr2, my_mac, 6);
          // BSSID = MAC du MCU (On cree un reseau ad-hoc implicite)
          memcpy(hdr->addr3, my_mac, 6);
          // Sequence Control
          hdr->seq_ctrl = htole16((packet_counter % 4096) << 4);

          // --- Copie du Payload SPI ---
          memcpy(payload, spi_rx_buffer, SPI_PAYLOAD_SIZE);

          // --- Configuration des Metadonnees du Paquet ---
          struct mmdrv_tx_metadata *meta = mmdrv_get_tx_metadata(pkt);
          memset(meta, 0, sizeof(*meta));
          meta->vif_id = 0;                   // VIF par defaut
          meta->flags = MMDRV_TX_FLAG_NO_ACK; // Pas de ACK, on broadcast
                                              // "aveuglement" pour la latence
          meta->tid = 3;                      // Equivalent a Voice / Video
          meta->enc = 0;                      // Pas de chiffrement

          mmpkt_close(&view);
          // --- Emission vers le transceiver ---
          mmdrv_tx_frame(pkt, false);
        } else {
          // Si on ne peut pas allouer l'espace, on libere
          mmpkt_close(&view);
          mmpkt_release(pkt);
        }
      }

      // --- ARRÊT DU CHRONO ---
      dwt_end = DWT->CYCCNT;
      mcu_cycles = dwt_end - dwt_start;

      // Affichage 1 fois tous les 100 paquets (pour ne pas bloquer le CPU avec
      // l'UART)
      if ((packet_counter % 100) == 0) {
        uint32_t freq_mhz = SystemCoreClock / 1000000;
        uint32_t time_us = mcu_cycles / freq_mhz;
        printf(
            "[Profilage RAW] Temps de traitement MCU : %lu us (%lu cycles)\n",
            time_us, mcu_cycles);
      }
      packet_counter++;

      spi_packet_received = false;

      // STM32 écoute
      HAL_SPI_Receive_IT(&hspi1, spi_rx_buffer, SPI_PAYLOAD_SIZE);
      HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);

    } else {
      // mmosal_task_sleep(1);
    }
  }
}

void SPI_Slave_Init(void) {
  // 1. Activer les horloges du SPI1, du Port E(SPI) et port D (Spare GPIO)(go
  // no go jetson)
  __HAL_RCC_SPI1_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  // D10(PE12), D13(PE13), D12(PE14) et D11(PE15)
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

  GPIO_InitStruct.Alternate = GPIO_AF5_SPI1; // Sur U5, Port E = SPI1 (AF5) !
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  // Config SPI1
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_SLAVE;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;

  // CS sur D10
  hspi1.Init.NSS = SPI_NSS_HARD_INPUT;

  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLED;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
  hspi1.Init.CRCPolynomial = 7;

  if (HAL_SPI_Init(&hspi1) != HAL_OK) {
    printf("ERREUR : Echec initialisation SPI1 !\n");
  } else {
    printf("SPI Esclave (SPI1) initialise sur PE12 a PE15 !\n");
  }

  // --- LES 2 LIGNES MANQUANTES POUR LE MODE '_IT' --- mode IT ?
  HAL_NVIC_SetPriority(SPI1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(SPI1_IRQn);

  // Initialisation de la broche Handshake (PD15) (Go no go jetson)
  GPIO_InitTypeDef GPIO_InitStruct_Handshake = {0};
  GPIO_InitStruct_Handshake.Pin = GPIO_PIN_15;
  GPIO_InitStruct_Handshake.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct_Handshake.Pull = GPIO_NOPULL;
  GPIO_InitStruct_Handshake.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct_Handshake);
}
// Quand le STM32 a reçu un paquet
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {
  if (hspi->Instance == SPI1) {
    // Démarrer le chrono
    dwt_start = DWT->CYCCNT;
    // mesure
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
    spi_packet_received = true;
    // HAL_SPI_Receive_IT(&hspi1, spi_rx_buffer, SPI_PAYLOAD_SIZE); //ca faisait
    // un double receive donc erreur -> donc un peut de lantence en plus
  }
}

void SPI1_IRQHandler(void) { HAL_SPI_IRQHandler(&hspi1); }

/**
 * Main entry point to the application. This will be invoked in a thread once
 * operating system and hardware initialization has completed. It may return,
 * but it does not have to.
 */
void app_init(void) {
  printf("\n\n--- PIPELINE JETSON -> STM32 -> WIFI ---\n\n");

  SPI_Slave_Init();

  HAL_SPI_Receive_IT(&hspi1, spi_rx_buffer, SPI_PAYLOAD_SIZE);

  // Configuration d'une QoS ultra-agressive pour le FPV
  struct mmwlan_qos_queue_params fpv_qos = {
      .aci = 3,    // ACI 3 = Voice (Correspond au TOS 0xC0 de LwIP)
      .aifs = 2,   // Temps d'attente inter-trame minimum légal (ultra rapide)
      .cw_min = 1, // Fenêtre de contention quasi-nulle (parle tout de suite)
      .cw_max = 1, // S'il y a collision, ne recule presque pas
      .txop_max_us = 0 // Désactivé
  };

  // À appeler avant mmwlan_sta_enable ou pendant l'init
  mmwlan_set_default_qos_queue_params(&fpv_qos, 1);

  app_wlan_init();

  printf("Fixation du canal a %d MHz...\n", RAW_TX_FREQ_MHZ);
  // On bypass l'association et le mmwlan_sta_enable()
  // On configure le driver pour utiliser un canal fixe (ici RAW_TX_FREQ_MHZ
  // MHz) Parametres: op_chan_freq_hz, pri_1mhz_chan_idx, op_bw_mhz, pri_bw_mhz,
  // is_off_channel
  mmdrv_set_channel(RAW_TX_FREQ_MHZ * 1000000, RAW_TX_CHAN_IDX, 8, 8, true);

  mmwlan_ate_override_rate_control(MMWLAN_MCS_1, MMWLAN_BW_8MHZ,
                                   MMWLAN_GI_NONE);
  printf("Forcage OK : 8 MHz / MCS 1 force.\n");
  mmwlan_set_power_save_mode(MMWLAN_PS_DISABLED); // Pour la latence optimale

  // Demarrage de la boucle d'injection
  raw_80211_tx_start();
}
