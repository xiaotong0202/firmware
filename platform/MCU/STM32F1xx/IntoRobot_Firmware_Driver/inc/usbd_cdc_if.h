/**
 ******************************************************************************
 * @file    USB_Device/CDC_Standalone/Inc/usbd_cdc_interface.h
 * @author  MCD Application Team
 * @version V1.0.2
 * @date    13-November-2015
 * @brief   Header for usbd_cdc_interface.c file.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
 *
 * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *        http://www.st.com/software_license_agreement_liberty_v2
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBD_CDC_IF_H
#define __USBD_CDC_IF_H

#ifdef	__cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc.h"

/* Exported types ------------------------------------------------------------*/
typedef void (*linecoding_bitrate_handler)(uint32_t bitrate);

/* Exported constants --------------------------------------------------------*/
extern USBD_CDC_ItfTypeDef  USBD_CDC_fops;
extern SDK_QUEUE USB_Rx_Queue;
extern USBD_HandleTypeDef  USBD_Device;
extern USBD_CDC_LineCodingTypeDef LineCoding;


/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void SetLineCodingBitRateHandler(linecoding_bitrate_handler handler);


#ifdef	__cplusplus
}
#endif



#endif /* __USBD_CDC_IF_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
