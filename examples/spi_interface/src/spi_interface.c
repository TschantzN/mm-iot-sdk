/*
 * Copyright 2021-2023 Morse Micro
 *
 * SPDX-License-Identifier: Apache-2.0
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
#include "stm32u5xx_hal.h"

// --- SPI ---
#define SPI_PAYLOAD_SIZE 1400

SPI_HandleTypeDef hspi1;
uint8_t spi_rx_buffer[SPI_PAYLOAD_SIZE];
volatile bool spi_packet_received = false;
// ---------------------


// --- Variables de Profilage DWT ---
volatile uint32_t dwt_start = 0;
volatile uint32_t dwt_end = 0;
volatile uint32_t mcu_cycles = 0;
extern uint32_t SystemCoreClock; // Fourni par le système STM32 (ex: 160000000 pour 160 MHz)

static volatile bool is_network_ready = false;

/* Callback pour savoir quand la connexion est prete*/
static void link_status_callback(const struct mmipal_link_status *link_status)
{
    if (link_status->link_state == MMIPAL_LINK_UP) {
        printf("\n>>> CONNECTE A OPENWRT <<<\n");
        is_network_ready = true;
    }
}



static void udp_unicast_tx_start(struct udp_pcb *pcb)
{
    //err_t err;

    //ip_set_option(pcb, SOF_BROADCAST);
    ip_addr_t dest_ip;
    IP4_ADDR(ip_2_ip4(&dest_ip), 192, 168, 12, 10);

    // --- ACTIVATION DU COMPTEUR DE CYCLES DWT ---
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
   	DWT->CYCCNT = 0;
   	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    printf(">>> PONT SPI-WIFI ACTIVE ! En attente de la Rpi... <<<\n");
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
    uint32_t packet_counter = 0;
        while (1)
        {
            if (spi_packet_received) {

            	struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, SPI_PAYLOAD_SIZE, PBUF_REF);
            	if (p != NULL) {
            	    p->payload = (void *)spi_rx_buffer;

            	    LOCK_TCPIP_CORE();
            	    udp_sendto(pcb, p, &dest_ip, 1337);
            	    UNLOCK_TCPIP_CORE();

            	    pbuf_free(p);
            	}

            	// --- ARRÊT DU CHRONO ---
            	dwt_end = DWT->CYCCNT;
            	mcu_cycles = dwt_end - dwt_start;

            	// Affichage 1 fois tous les 100 paquets (pour ne pas bloquer le CPU avec l'UART)
            	if ((packet_counter++ % 100) == 0) {
            	    // SystemCoreClock vaut 160000000 (160 MHz)
            	    uint32_t freq_mhz = SystemCoreClock / 1000000;
            	    uint32_t time_us = mcu_cycles / freq_mhz;

            		printf("[Profilage] Temps de traitement MCU : %lu us (%lu cycles)\n", time_us, mcu_cycles);
            	}

                spi_packet_received = false;

                // STM32 écoute
                HAL_SPI_Receive_IT(&hspi1, spi_rx_buffer, SPI_PAYLOAD_SIZE);
                HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);

            } else {
               // mmosal_task_sleep(1);
            }
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

void SPI_Slave_Init(void)
{
    // 1. Activer les horloges du SPI1, du Port E(SPI) et port D (Spare GPIO)
    __HAL_RCC_SPI1_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    // D10(PE12), D13(PE13), D12(PE14) et D11(PE15)
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1; // Sur U5, Port E = SPI1 (AF5)
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
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == SPI1) {
    	// Démarrer le chrono
    	dwt_start = DWT->CYCCNT;
    	// mesure
    	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
        spi_packet_received = true;
        //HAL_SPI_Receive_IT(&hspi1, spi_rx_buffer, SPI_PAYLOAD_SIZE); //ca faisait un double receive donc erreur -> donc un peut de lantence en plus
    }
}

void SPI1_IRQHandler(void)
{
    HAL_SPI_IRQHandler(&hspi1);
}


/**
 * Main entry point to the application. This will be invoked in a thread once operating system
 * and hardware initialization has completed. It may return, but it does not have to.
 */
void app_init(void)
{
    printf("\n\n--- RPI -> STM32 -> HALOW ---\n\n");

    SPI_Slave_Init();

    HAL_SPI_Receive_IT(&hspi1, spi_rx_buffer, SPI_PAYLOAD_SIZE);

    // low QoS config
    struct mmwlan_qos_queue_params fpv_qos = {
        .aci = 3,         // ACI 3 = Voice (TOS 0xC0 - LwIP)
        .aifs = 2,        // inter-trame waiting time
        .cw_min = 1,
        .cw_max = 1,
        .txop_max_us = 0
    };

    mmwlan_set_default_qos_queue_params(&fpv_qos, 1);
    mmwlan_set_power_save_mode(MMWLAN_PS_DISABLED);

    app_wlan_init();

    //mmwlan_override_max_tx_power(26);

    mmipal_set_link_status_callback(link_status_callback);

    printf("Connexion a l'AP OpenWrt en cours...\n");

    app_wlan_start();

    mmwlan_ate_override_rate_control(MMWLAN_MCS_1, MMWLAN_BW_8MHZ, MMWLAN_GI_NONE);
    printf("forcage OK : 8 MHz / MCS 1 force.\n");


    while (!is_network_ready) {
        mmosal_task_sleep(10);
    }

    struct udp_pcb *pcb = init_udp_pcb();
    if (pcb != NULL) {
    	pcb->tos = 0xC0;
        udp_unicast_tx_start(pcb);
    }
}
