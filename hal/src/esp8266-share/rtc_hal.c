/**
 ******************************************************************************
 * @file    rtc_hal.c
 * @author  Satish Nair, Brett Walach
 * @version V1.0.0
 * @date    12-Sept-2014
 * @brief
 ******************************************************************************
  Copyright (c) 2013-2015 IntoRobot Industries, Inc.  All rights reserved.

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
#include "rtc_hal.h"

void HAL_RTC_Initial(void)
{

}

time_t HAL_RTC_Get_UnixTime(void)
{
    uint32_t rtc_time = system_get_rtc_time();
    uint32_t cal_time = system_rtc_clock_cali_proc();
    time_t time = rtc_time * ((cal_time*1000) >> 12)/1000 / 1000; // sencods
    return time;
}

void HAL_RTC_Set_UnixTime(time_t value)
{

}

void HAL_RTC_Set_Alarm(uint32_t value)
{
}

void HAL_RTC_Set_UnixAlarm(time_t value)
{

}

void HAL_RTC_Cancel_UnixAlarm(void)
{

}