/*
  ******************************************************************************
  * @file    samd_bluenrg_ble.cpp
  * @author  Daniel De Sousa
  * @version V2.0.1
  * @date    24-July-2018
  * @brief   
  * @note    Revision: Cleanup
  ******************************************************************************
*/
  
/* Includes ------------------------------------------------------------------*/

#include <SPI.h>
#include "samd_bluenrg_ble.h"


#ifdef __cplusplus
extern "C" {
#endif

#include "STBlueNRG/inc/gp_timer.h"
#include "STBlueNRG/inc/debug.h"
#include "STBlueNRG/inc/hci.h"

extern volatile uint32_t ms_counter;

// Private defines
#define HEADER_SIZE 5
#define MAX_BUFFER_SIZE 255
#define TIMEOUT_DURATION 15

// Private variables
SPI_HandleTypeDef SpiHandle;
SPIClass *BLESPI = &SPI;

/* Utility to print the log time in the format HH:MM:SS:MSS (DK GUI time format) */
void print_csv_time(void){
  uint32_t ms = ms_counter;
  PRINT_CSV("%02d:%02d:%02d.%03d", ms/(60*60*1000)%24, ms/(60*1000)%60, (ms/1000)%60, ms%1000);
}

/**
 * @brief  Writes data to a serial interface.
 * @param  data1   :  1st buffer
 * @param  data2   :  2nd buffer
 * @param  n_bytes1: number of bytes in 1st buffer
 * @param  n_bytes2: number of bytes in 2nd buffer
 * @retval None
 */
void Hal_Write_Serial(const void* data1, const void* data2, int32_t n_bytes1,
                      int32_t n_bytes2)
{
  struct timer t;
  int ret;
  uint8_t data2_offset = 0;
  
  Timer_Set(&t, CLOCK_SECOND/10);
  
  Disable_SPI_IRQ();
  
  while(1){
    ret = BlueNRG_SPI_Write((uint8_t *)data1,(uint8_t *)data2 + data2_offset, n_bytes1, n_bytes2);
    
    if(ret >= 0){      
      n_bytes1 = 0;
      n_bytes2 -= ret;
      data2_offset += ret;
      if(n_bytes2==0)
        break;
    }
    
    if(Timer_Expired(&t)){
      break;
    }
  }
  
  Enable_SPI_IRQ();
}

/**
 * @brief  Initializes the SPI communication with the BlueNRG
 *         Expansion Board.
 * @param  None
 * @retval None
 */
void BNRG_SPI_Init(void)
{
  pinMode(BNRG_SPI_RESET_PIN,OUTPUT);
  pinMode(BNRG_SPI_CS_PIN,OUTPUT);
  pinMode(BNRG_SPI_EXTI_PIN,INPUT);
  
  BLESPI->begin();
  BLESPI->setDataMode(SPI_MODE0);
  BLESPI->setBitOrder(MSBFIRST);
#if defined(ARDUINO_ARCH_AVR)
  BLESPI->setClockDivider(SPI_CLOCK_DIV8);
  attachInterrupt(0,HCI_Isr,RISING);
#elif defined(ARDUINO_ARCH_SAMD)
  BLESPI->setClockDivider(48);
  attachInterrupt(BNRG_SPI_EXTI_PIN,HCI_Isr,RISING);
#endif
}

/**
 * @brief  Resets the BlueNRG.
 * @param  None
 * @retval None
 */
void BlueNRG_RST(void)
{
  HAL_GPIO_WritePin(BNRG_SPI_RESET_PORT, BNRG_SPI_RESET_PIN, GPIO_PIN_SET);
  HAL_Delay(5);
  HAL_GPIO_WritePin(BNRG_SPI_RESET_PORT, BNRG_SPI_RESET_PIN, GPIO_PIN_RESET);
  HAL_Delay(5);
  HAL_GPIO_WritePin(BNRG_SPI_RESET_PORT, BNRG_SPI_RESET_PIN, GPIO_PIN_SET);
  HAL_Delay(5);
}

/**
 * @brief  Reports if the BlueNRG has data for the host micro.
 * @param  None
 * @retval 1 if data are present, 0 otherwise
 */
uint8_t BlueNRG_DataPresent(void)
{
  if (HAL_GPIO_ReadPin(BNRG_SPI_EXTI_PORT, BNRG_SPI_EXTI_PIN) == GPIO_PIN_SET)
      return 1;
  else  
      return 0;
} 

/**
 * @brief  Activate internal bootloader using pin.
 * @param  None
 * @retval None
 */
void BlueNRG_HW_Bootloader(void)
{
  BlueNRG_RST();
}

/**
 * @brief  Reads from BlueNRG SPI buffer and store data into local buffer.
 * @param  hspi     : SPI handle
 * @param  buffer   : Buffer where data from SPI are stored
 * @param  buff_size: Buffer size
 * @retval int32_t  : Number of read bytes
 */
int32_t BlueNRG_SPI_Read_All(uint8_t *buffer, uint8_t buff_size)
{
  uint16_t byte_count;
  uint8_t len = 0;
  
  const uint8_t header_master[5] = {0x0b, 0x00, 0x00, 0x00, 0x00};
  uint8_t header_slave[5];
  
  HAL_GPIO_WritePin(BNRG_SPI_CS_PORT, BNRG_SPI_CS_PIN, GPIO_PIN_RESET);
  
  // Read the header
  HAL_SPI_TransmitReceive_Opt(header_master, header_slave, HEADER_SIZE);
  
  // Check if device is ready
  if (header_slave[0] == 0x02) {
    
    byte_count = (header_slave[4]<<8)|header_slave[3];
    
    if (byte_count > 0) {
      
      //Avoid reading more data that size of the buffer
      if (byte_count > buff_size)
        byte_count = buff_size;
      
      HAL_SPI_Receive_Opt(buffer, byte_count);
      
      len = byte_count;
    }    
  }
  
  // Release CS line.
  HAL_GPIO_WritePin(BNRG_SPI_CS_PORT, BNRG_SPI_CS_PIN, GPIO_PIN_SET);
  
#ifdef PRINT_CSV_FORMAT
  if (len > 0) {
    print_csv_time();
    for (int i=0; i<len; i++) {
      PRINT_CSV(" %02x", buffer[i]);
    }
    PRINT_CSV("\n");
  }
#endif  
  
  return len;
}

/**
 * @brief  Writes data from local buffer to SPI.
 * @param  hspi     : SPI handle
 * @param  data1    : First data buffer to be written
 * @param  data2    : Second data buffer to be written
 * @param  Nb_bytes1: Size of first data buffer to be written
 * @param  Nb_bytes2: Size of second data buffer to be written
 * @retval Number of read bytes
 */
int32_t BlueNRG_SPI_Write(uint8_t* data1, uint8_t* data2, uint8_t Nb_bytes1, uint8_t Nb_bytes2)
{
  int16_t result = 0;
  uint16_t tx_bytes;
  uint8_t rx_bytes;
  
  const uint8_t header_master[5] = {0x0a, 0x00, 0x00, 0x00, 0x00};
  uint8_t header_slave[5]  = {0x00};
  
  HAL_GPIO_WritePin(BNRG_SPI_CS_PORT, BNRG_SPI_CS_PIN, GPIO_PIN_RESET);
  
  HAL_SPI_TransmitReceive_Opt(header_master, header_slave, HEADER_SIZE);
  
  if(header_slave[0] != 0x02){
    result = -1;
    goto failed; // BlueNRG not awake
  }
  
  rx_bytes = header_slave[1];
  
  if(rx_bytes < Nb_bytes1){
    result = -2;
    goto failed;
  }
  
  HAL_SPI_Transmit_Opt(data1, Nb_bytes1);
  
  rx_bytes -= Nb_bytes1;
  
  if(Nb_bytes2 > rx_bytes){
    tx_bytes = rx_bytes;
  }
  else{
    tx_bytes = Nb_bytes2;
  }
  
  HAL_SPI_Transmit_Opt(data2, tx_bytes);
  
  result = tx_bytes;
  
failed:
  
  // Release CS line
  HAL_GPIO_WritePin(BNRG_SPI_CS_PORT, BNRG_SPI_CS_PIN, GPIO_PIN_SET);
  
  return result;

}

/**
 * @brief  Enable SPI IRQ.
 * @param  None
 * @retval None
 */
void Enable_SPI_IRQ(void)
{
  //HAL_NVIC_EnableIRQ(BNRG_SPI_EXTI_IRQn);  
}

/**
 * @brief  Disable SPI IRQ.
 * @param  None
 * @retval None
 */
void Disable_SPI_IRQ(void)
{ 
  //HAL_NVIC_DisableIRQ(BNRG_SPI_EXTI_IRQn);
}

/**
 * @brief  Clear Pending SPI IRQ.
 * @param  None
 * @retval None
 */
void Clear_SPI_IRQ(void)
{
  //HAL_NVIC_ClearPendingIRQ(BNRG_SPI_EXTI_IRQn);
}

/**
 * @brief  Clear EXTI (External Interrupt) line for SPI IRQ.
 * @param  None
 * @retval None
 */
void Clear_SPI_EXTI_Flag(void)
{  
  //__HAL_GPIO_EXTI_CLEAR_IT(BNRG_SPI_EXTI_PIN);  
}

/* used by the server (L0 and F4, not L4) for the throughput test */
static void SPI_I2S_SendData(SPI_HandleTypeDef *hspi, uint8_t data)
{
  //hspi->Instance->DR = data;
}

static  uint8_t SPI_I2S_ReceiveData(SPI_HandleTypeDef *hspi)
{
  return 0;//hspi->Instance->DR;
}

/**
  * @brief  Transmit and Receive an amount of data in blocking mode 
  * @param  hspi: pointer to a SPI_HandleTypeDef structure that contains
  *                the configuration information for SPI module.
  * @param  pTxData: pointer to transmission data buffer
  * @param  pRxData: pointer to reception data buffer to be
  * @param  Size: amount of data to be sent
  * @retval HAL status
  */
void HAL_SPI_TransmitReceive_Opt(const uint8_t *pTxData, uint8_t *pRxData, uint8_t Size)
{
  uint8_t i;
  for (i = 0; i < Size; i++) {
    *pRxData++ = BLESPI->transfer(*pTxData++);
  }
}

/**
  * @brief  Transmit an amount of data in blocking mode (optimized version)
  * @param  hspi: pointer to a SPI_HandleTypeDef structure that contains
  *                the configuration information for SPI module.
  * @param  pData: pointer to data buffer
  * @param  Size: amount of data to be sent
  * @retval HAL status
  */
void HAL_SPI_Transmit_Opt(const uint8_t *pTxData, uint8_t Size)
{
  uint8_t i;
  
  for (i = 0; i < Size; i++) {
    BLESPI->transfer(*pTxData++);
  }
}

/**
  * @brief  Receive an amount of data in blocking mode (optimized version)
  * @param  hspi: pointer to a SPI_HandleTypeDef structure that contains
  *                the configuration information for SPI module.
  * @param  pData: pointer to data buffer
  * @param  Size: amount of data to be sent
  * @retval HAL status
  */
void HAL_SPI_Receive_Opt(uint8_t *pRxData, uint8_t Size)
{
  uint8_t i;
  
  for (i = 0; i < Size; i++) {
    *pRxData++ = BLESPI->transfer(0xFF);
  }
}

#ifdef __cplusplus
}
#endif