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

#endif // __ESPERTO_STBLE_H
