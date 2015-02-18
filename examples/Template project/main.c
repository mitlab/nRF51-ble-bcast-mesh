/***********************************************************************************
Copyright (c) Nordic Semiconductor ASA
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright notice, this
  list of conditions and the following disclaimer in the documentation and/or
  other materials provided with the distribution.

  3. Neither the name of Nordic Semiconductor ASA nor the names of other
  contributors to this software may be used to endorse or promote products
  derived from this software without specific prior written permission.

  4. This software must only be used in a processor manufactured by Nordic
  Semiconductor ASA, or in a processor manufactured by a third party that
  is used in combination with a processor manufactured by Nordic Semiconductor.


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
************************************************************************************/

#include "rbc_mesh.h"

#include "nrf_soc.h"
#include "nrf_sdm.h"
#include "app_error.h"
#include "simple_uart.h"
#include "nrf_delay.h"
#include "boards.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
  
/* Debug macros for debugging with logic analyzer */
#define SET_PIN(x) NRF_GPIO->OUTSET = (1 << (x))
#define CLEAR_PIN(x) NRF_GPIO->OUTCLR = (1 << (x))
#define TICK_PIN(x) do { SET_PIN((x)); CLEAR_PIN((x)); }while(0)

/* Aliases for LEDs */
#ifdef BOARD_PCA10000
    #define LED_0 LED_RGB_RED
    #define LED_1 LED_RGB_GREEN
#endif

/**
* @brief General error handler. Sets the LEDs to blink forever
*/
static void error_loop(char* message)
{
    SET_PIN(7);
    while (true)
    {
        simple_uart_putstring((uint8_t*) message);
        nrf_delay_ms(500);
        SET_PIN(LED_0);
        CLEAR_PIN(LED_1);
        nrf_delay_ms(500);
        CLEAR_PIN(LED_0);
        SET_PIN(LED_1);
    }
}    

/**
* @brief Softdevice crash handler, never returns
* 
* @param[in] pc Program counter at which the assert failed
* @param[in] line_num Line where the error check failed 
* @param[in] p_file_name File where the error check failed
*/
void sd_assert_handler(uint32_t pc, uint16_t line_num, const uint8_t* p_file_name)
{
    char str[256];
    sprintf(str, "SD ERROR: PC: %d, LINE: %d, FILE: %s\n", 
        pc, 
        line_num, 
        p_file_name);
    
    error_loop(str);
}

/**
* 
* @param[in] pc Program counter at which the assert failed
* @param[in] line_num Line where the error check failed 
* @param[in] p_file_name File where the error check failed
*/
void sd_assert_handler(uint32_t pc, uint16_t line_num, const uint8_t* p_file_name)
{
    char str[256];
    sprintf(str, "SD ERROR: PC: %d, LINE: %d, FILE: %s\n", 
        pc, 
        line_num, 
        p_file_name);
    
    error_loop(str);
}

/**
* @brief App error handle callback. Called whenever an APP_ERROR_CHECK() fails.
*   Never returns.
* 
* @param[in] error_code The error code sent to APP_ERROR_CHECK()
* @param[in] line_num Line where the error check failed 
* @param[in] p_file_name File where the error check failed
*/
void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{
    SET_PIN(7);
    
    char str[256];
    sprintf(str, "APP ERROR: CODE: %d, LINE: %d, FILE: %s\n", 
        error_code, 
        line_num, 
        p_file_name);
    
    error_loop(str);
}

void HardFault_Handler(void)
{
    error_loop("HARDFAULT\n");
}

/**
* @brief RBC_MESH framework event handler. Defined in rbc_mesh.h. Handles
*   events coming from the mesh. 
*
* @param[in] evt RBC event propagated from framework
*/
void rbc_mesh_event_handler(rbc_mesh_event_t* evt)
{
    switch (evt->event_type)
    {
        case RBC_MESH_EVENT_TYPE_CONFLICTING_VAL:
        case RBC_MESH_EVENT_TYPE_NEW_VAL:
        case RBC_MESH_EVENT_TYPE_UPDATE_VAL:
            break;
    }
}


int main(void)
{
    simple_uart_config(RTS_PIN_NUMBER, TX_PIN_NUMBER, CTS_PIN_NUMBER, RX_PIN_NUMBER, true);
	
    uint32_t error_code = 
        sd_softdevice_enable(NRF_CLOCK_LFCLKSRC_XTAL_75_PPM, sd_assert_handler);
    
    ble_enable_params_t ble_enable_params;
    ble_enable_params.gatts_enable_params.service_changed = 0;
    
    error_code = sd_ble_enable(&ble_enable_params);
    APP_ERROR_CHECK(error_code);
    
    error_code = rbc_mesh_init(0xA541A68F, 38, 1, 100);
    APP_ERROR_CHECK(error_code);
    
    
    /* sleep */
    while (true)
    {
        sd_app_evt_wait();
    }
}

