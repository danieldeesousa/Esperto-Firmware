/*
  ******************************************************************************
  * @file    esperto_stble.h
  * @author  Daniel De Sousa
  * @version V2.0.0
  * @date    24-July-2018
  * @brief   This library was developed as a driver for STBLE BlueNRG based modules
  ******************************************************************************
*/

#ifndef __ESPERTO_STBLE_H
#define __ESPERTO_STBLE_H

#include "samd_bluenrg_ble.h"
#include "samd_hal.h"

#ifdef __cplusplus
 extern "C" {
#endif
	#include "STBlueNRG/inc/bluenrg_utils.h"
	#include "STBlueNRG/inc/hal_types.h"
	#include "STBlueNRG/inc/ble_status.h"
	#include "STBlueNRG/inc/hci.h"
	#include "STBlueNRG/inc/hci_le.h"
	#include "STBlueNRG/inc/bluenrg_aci.h"
	#include "STBlueNRG/inc/gp_timer.h"
	#include "STBlueNRG/inc/bluenrg_gap.h"
	#include "STBlueNRG/inc/sm.h"
#ifdef __cplusplus
}
#endif // __cplusplus

#define COPY_UUID_128(uuid_struct, uuid_15, uuid_14, uuid_13, uuid_12, uuid_11, uuid_10, uuid_9, uuid_8, uuid_7, uuid_6, uuid_5, uuid_4, uuid_3, uuid_2, uuid_1, uuid_0) \
  do {\
    uuid_struct[0] = uuid_0; uuid_struct[1] = uuid_1; uuid_struct[2] = uuid_2; uuid_struct[3] = uuid_3; \
    uuid_struct[4] = uuid_4; uuid_struct[5] = uuid_5; uuid_struct[6] = uuid_6; uuid_struct[7] = uuid_7; \
    uuid_struct[8] = uuid_8; uuid_struct[9] = uuid_9; uuid_struct[10] = uuid_10; uuid_struct[11] = uuid_11; \
    uuid_struct[12] = uuid_12; uuid_struct[13] = uuid_13; uuid_struct[14] = uuid_14; uuid_struct[15] = uuid_15; \
  }while(0)

#define COPY_UART_SERVICE_UUID(uuid_struct)  COPY_UUID_128(uuid_struct,0x6E, 0x40, 0x00, 0x01, 0xB5, 0xA3, 0xF3, 0x93, 0xE0, 0xA9, 0xE5, 0x0E, 0x24, 0xDC, 0xCA, 0x9E)
#define COPY_UART_TX_CHAR_UUID(uuid_struct)  COPY_UUID_128(uuid_struct,0x6E, 0x40, 0x00, 0x02, 0xB5, 0xA3, 0xF3, 0x93, 0xE0, 0xA9, 0xE5, 0x0E, 0x24, 0xDC, 0xCA, 0x9E)
#define COPY_UART_RX_CHAR_UUID(uuid_struct)  COPY_UUID_128(uuid_struct,0x6E, 0x40, 0x00, 0x03, 0xB5, 0xA3, 0xF3, 0x93, 0xE0, 0xA9, 0xE5, 0x0E, 0x24, 0xDC, 0xCA, 0x9E)

#define  ADV_INTERVAL_MIN_MS  50
#define  ADV_INTERVAL_MAX_MS  100

#endif // __ESPERTO_STBLE_H
