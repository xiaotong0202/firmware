/**
 ******************************************************************************
 * @file    usart_hal.c
 * @author  Satish Nair, Brett Walach
 * @version V1.0.0
 * @date    12-Sept-2014
 * @brief
 ******************************************************************************
  Copyright (c) 2013-2015 Particle Industries, Inc.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation, either
  version 3 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "hw_config.h"
#include "usart_hal.h"
#include "uart.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>


uart_t* _uart = nullptr;

void HAL_USART_Initial(HAL_USART_Serial serial)
{

}

void HAL_USART_Begin(HAL_USART_Serial serial, uint32_t baud)
{
    HAL_USART_BeginConfig(serial, baud, UART_8N1, 0); // Default serial configuration is 8N1
}

void HAL_USART_BeginConfig(HAL_USART_Serial serial, uint32_t baud, uint32_t config, void *ptr)
{
    if(uart_get_debug() == UART1) {
        uart_set_debug(UART_NO);
    }

    if (_uart) {
        free(_uart);
    }

    _uart = uart_init(UART1, baud, UART_8N1, 0, 1);

    if(uart_tx_enabled(_uart)) {
        uart_set_debug(UART1);
    }
}

void HAL_USART_End(HAL_USART_Serial serial)
{
}

uint32_t HAL_USART_Write_Data(HAL_USART_Serial serial, uint8_t data)
{
    if(!_uart || !uart_tx_enabled(_uart)) {
        return 0;
    }

    uart_write_char(_uart, data);
    return 1;
}

uint32_t HAL_USART_Write_NineBitData(HAL_USART_Serial serial, uint16_t data)
{
    return 1;
}

int32_t HAL_USART_Available_Data(HAL_USART_Serial serial)
{
    return 0;
}

int32_t HAL_USART_Available_Data_For_Write(HAL_USART_Serial serial)
{
    if(!_uart || !uart_tx_enabled(_uart)) {
        return 0;
    }

    return static_cast<int>(uart_tx_free(_uart));
}

int32_t HAL_USART_Read_Data(HAL_USART_Serial serial)
{
    return 0;
}

int32_t HAL_USART_Peek_Data(HAL_USART_Serial serial)
{
    return 0;
}

void HAL_USART_Flush_Data(HAL_USART_Serial serial)
{
}

bool HAL_USART_Is_Enabled(HAL_USART_Serial serial)
{
    return false;
}

void HAL_USART_Half_Duplex(HAL_USART_Serial serial, bool Enable)
{
}