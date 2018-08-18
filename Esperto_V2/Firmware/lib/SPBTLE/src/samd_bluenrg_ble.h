/*
  ******************************************************************************
  * @file    samd_bluenrg_ble.h
  * @author  Daniel De Sousa
  * @version V2.1.0
  * @date    18-August-2018
  * @brief   
  * @note    Revision: Cleanup
  ******************************************************************************
*/

#ifndef __SAMD_BLUENRG_BLE_H
#define __SAMD_BLUENRG_BLE_H

#include "samd_hal.h"

#ifdef __cplusplus
extern "C" {
#endif 

#define SYSCLK_FREQ 80000000
#define SPI_HandleTypeDef uint8_t*


void BNRG_SPI_Init(void);
void BlueNRG_RST(void);
uint8_t BlueNRG_DataPresent(void);
void    BlueNRG_HW_Bootloader(void);
int32_t BlueNRG_SPI_Read_All(uint8_t *buffer, uint8_t buff_size);
int32_t BlueNRG_SPI_Write(uint8_t* data1, uint8_t* data2, uint8_t Nb_bytes1, uint8_t Nb_bytes2);

void Hal_Write_Serial(const void* data1, const void* data2, int32_t n_bytes1, int32_t n_bytes2);
void HAL_SPI_TransmitReceive_Opt(const uint8_t *pTxData, uint8_t *pRxData, uint8_t Size);
void HAL_SPI_Transmit_Opt(const uint8_t *pTxData, uint8_t Size);
void HAL_SPI_Receive_Opt(uint8_t *pRxData, uint8_t Size);
void Enable_SPI_IRQ(void);
void Disable_SPI_IRQ(void);
void Clear_SPI_IRQ(void);
void Clear_SPI_EXTI_Flag(void);
 
#ifdef __cplusplus
}
#endif

#endif /* __SAMD_BLUENRG_BLE_H */