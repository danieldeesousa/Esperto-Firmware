/*
  ******************************************************************************
  * @file    Esperto_V2.ino
  * @author  Daniel De Sousa
  * @version V2.0.6
  * @date    15-Aug-2018
  * @brief   Main Esperto Watch application
  * @note    Last revision: Timer refactoring
  ******************************************************************************
*/
#include "esperto_mpu9250.h"
#include "esperto_rtc.h"
#include "esperto_fram.h"
#include "esperto_timer.h"
#include "esperto_stble.h"
#include "esperto_max30102.h"
#include "esperto_watch.h"
#include <U8g2lib.h>

// BLE variables
char dateBT[INFO_BUFFER_SIZE];      // Date info MM/DD/YYYY
char timeBT[INFO_BUFFER_SIZE];      // Time info HH:MM:SS MM
char callBT[INFO_BUFFER_SIZE];      // Caller number info
char textBT[INFO_BUFFER_SIZE];      // Text number info
uint8_t bleRXBuffer[BLE_DATA_SIZE]; // Buffer containing incoming BLE data
uint8_t bleRXBbufferLen = 0;        // Size of incoming data
uint8_t bleConnectionState = false; // Status of BLE connection

// Heart Rate Variables
int heartRateAvg; // Average heart rate which will we be displayed
uint16_t spO2Avg; // Average SpO2 which will be stored, displayed, and sent

// MPU9250
uint16_t stepCount = 0;    // Total number of steps taken
uint16_t dmpStepCount = 0; // Total number of steps taken calculated by the Digital Motion Processor

// FRAM Definitions
uint16_t countFRAM = 0;  // Address where the latest data will be written to in the FRAM
bool isDataSent = 0;     // Determines if initial data was sent for the current BLE connection

// 1Hz Timer ISR -- 1Hz timer which times writing to FRAM, BLE, display
volatile uint8_t ISR_CTR = 0;         // General 1 second counter
volatile uint8_t STANDBY_CTR = 0;     // How many seconds the device has not been in standby mode
volatile uint8_t NOTIF_CTR = 0;       // Number of seconds notifications have shown on screen
volatile uint8_t DATA_CTR = 0;        // Track how many seconds between data writes
volatile bool updateDisplay_flag = 0; // Control when display is updated
void ISR_timer3(struct tc_module *const module_inst) 
{ 
  ISR_CTR++;
  STANDBY_CTR++;
  NOTIF_CTR++;
  DATA_CTR++;
  updateDisplay_flag = 1; // Set flag to update display
}
SAMDtimer timer3_1Hz = SAMDtimer(3, ISR_timer3, 1e6); // 1Hz timer interrupt

// Power management definitions
float inputVoltage; // Input voltage measured (USB, battery) 

// Bootloader setup
void setup(){
  
  // Used for debugging purposes
  SerialUSB.begin(9600);
  
  // Initialize display and display boot screen
  u8g2.begin();
  u8g2.firstPage();
  do {
    u8g2.drawXBMP(32, 0, 64, 64, boot);
  } while ( u8g2.nextPage() );
  
  // Setup MPU9250
  initMPU9250();
  
  // Initialize Real Time Clock
  rtc.begin();

  // Turn on and setup heart rate sensor
  heartRateSensor.begin(Wire, I2C_SPEED_STANDARD); // 100 KHz
  heartRateSensor.setup(); // configure sensor with default settings
  heartRateSensor.setPulseAmplitudeRed(0x0A); // red LED to low to indicate sensor is running
  heartRateSensor.setPulseAmplitudeGreen(0); // turn off Green LED

  // Initialize FRAM 
  fram.begin();
  
  // Initialize STPBTLE-RF
  BLEsetup();
}

// Function to update the display with latest information
// Display is updated at 1HZ or when a BLE message is recieved
void updateDisplay(){
  // Check if there is an incoming phone call
  if (strlen(callBT) >= 10 && bleConnectionState == true)
  {
    u8g2.setFont(u8g2_font_profont11_tf);
    // print time top left corner TODO: pass into update time function
    u8g2.drawStr(0, 10, timeBT);

    u8g2.setFont(u8g2_font_profont22_tf);
    // display call text and phone number
    u8g2.setCursor(40, 38);
    u8g2.print("Call");
    u8g2.drawStr(0, 58, callBT);
  }
  // Check if there is an incoming text
  else if (strlen(textBT) >= 10 && bleConnectionState == true)
  {
    u8g2.setFont(u8g2_font_profont11_tf);
    
    // Print time top left corner
    u8g2.drawStr(0, 10, timeBT);

    u8g2.setFont(u8g2_font_profont22_tf);
    
    // Display text text and phone number
    u8g2.setCursor(40, 38);
    u8g2.print("Text");
    u8g2.drawStr(0, 58, textBT);
  }
  // If no incoming call or text
  else
  {
    u8g2.setFont(u8g2_font_profont11_tf);
    
    // Display date
    u8g2.setCursor(0, 10);
    u8g2.print(String(rtc.getMonth()) + "/" + String(rtc.getDay()) + "/20" + String(rtc.getYear()));

    // Display heart rate
    u8g2.drawXBMP(0, 54, 10, 10, heart);
    if(heartRateAvg > 40){
      u8g2.setCursor(14, 62);
      u8g2.print(String(heartRateAvg) + " bpm");
    }

    // Display steps
    u8g2.setCursor(78, 62);
    u8g2.print(String((stepCount + dmpStepCount)) + " stp");
    u8g2.drawXBMP(64, 54, 10, 10, mountain);
    
    // Display time
    u8g2.setFont(u8g2_font_profont22_tf);
    // Convert time from 24 hour clock to meridian time
    uint8_t hour = rtc.getHours();
    if(hour >= 12){
      u8g2.setCursor(13, 38);
      if(hour > 12)
        hour = hour - 12;
      // Add leading 0's to display string if needed
      if(rtc.getMinutes() < 10)
        u8g2.print(String(hour) + ":0" + String(rtc.getMinutes()) + " PM");
      else
        u8g2.print(String(hour) + ":" + String(rtc.getMinutes()) + " PM");
    }
    else{
      // In case it is 12AM - we do not want 0 showing up as the hour
      if(hour == 0)
        hour = 12;
      // Determine position of time based on how many digits
      if(hour >= 10)
        u8g2.setCursor(13, 38);
      else
        u8g2.setCursor(20, 38);
      // Add leading 0's to display string if needed
      if(rtc.getMinutes() < 10)
        u8g2.print(String(hour) + ":0" + String(rtc.getMinutes()) + " AM");
      else
        u8g2.print(String(hour) + ":" + String(rtc.getMinutes()) + " AM");
    }
  }
  
  // Draw BT logo only if connected
  if(bleConnectionState == true)
    u8g2.drawXBMP(118, 0, 10, 10, BT);

  // Display battery status
  bool isChargeComplete = digitalRead(CHARGE_PIN); // HIGH when charging is complete
  // Display full battery
  if ((inputVoltage >= 3.5 && inputVoltage <= 4.5) || isChargeComplete)
    u8g2.drawXBMP(105, 0, 10, 10, battHigh);
  // Display charging battery - toggle when charging
  else if (inputVoltage > 4.5 && !isChargeComplete){
    if(ISR_CTR%3 == 0)
      u8g2.drawXBMP(105, 0, 10, 10, battLow);
    else if(ISR_CTR%3 == 1)
      u8g2.drawXBMP(105, 0, 10, 10, battMed);
    else
      u8g2.drawXBMP(105, 0, 10, 10, battHigh);
  }
  // Display low battery
  else if (inputVoltage < 3.5)
    u8g2.drawXBMP(105, 0, 10, 10, battLow);
}

// Set Real Time Clock time
void setRTCTime(){
  // Det time (HH:MM:SS) - 24 hour clock
  rtc.setHours(stringToByte(timeBT, 2));
  rtc.setMinutes(stringToByte(timeBT+3, 2));
  rtc.setSeconds(stringToByte(timeBT+6, 2));
}

// Set Real Time Clock date
void setRTCDate(){
  // Det date (DD/MM/YYYY)
  rtc.setDay(stringToByte(dateBT, 2));
  rtc.setMonth(stringToByte(dateBT+3, 2));
  rtc.setYear(stringToByte(dateBT+8, 2)); // only get the last two digits of the year
}

// Write data to FRAM
void writeFRAM(){
  // Ensure memory is not full and sensor data is valid
  if(countFRAM < FRAM_SIZE && heartRateAvg > HEARTRATE_MIN_VALID)
  {
     fram.write8(countFRAM, heartRateAvg);
     fram.write8(countFRAM+1, (stepCount + dmpStepCount) >> 8);
     fram.write8(countFRAM+2, (stepCount + dmpStepCount));
     fram.write8(countFRAM+3, spO2Avg);
     countFRAM+=4; // 4 byte alligned memory
  }
  // New data written to FRAM - reset flag
  isDataSent = 0;
}

// Burst transfer data found in FRAM once connected to BLE device
void burstTransferFRAM(){
  uint8_t blePacket[20];
  int i, j;
  
  for(i = 0; i < countFRAM; i+=20) // 20 byte packages
  {
    // Compile data packet
    for(j = 0; (j < 20) && ((i+j) < countFRAM); j++)
    {
      blePacket[j] = fram.read8(i+j);
    }
    // TODO: Possibly fix - add intermediate display if too slow
    writeUARTTX((char*)blePacket, j);
  }
  countFRAM = 0; // Go back to first FRAM address
  isDataSent = true;
}

void calculateSpO2(uint32_t irValue){
    static uint32_t irBuffer[SPO2_LIGHT_BUF_LENGTH];  // infrared sensor buffer
    static uint32_t redBuffer[SPO2_LIGHT_BUF_LENGTH]; // red sensor buffer
    static uint8_t senseBufIndex = 0;                 // index used to determine where latest vals will be assigned in buf
    static uint8_t spO2Buf[SPO2_BUF_LENGTH];          // buffer containing latest valid SpO2 values
    static int8_t sp02BufIndex = 0;                   // index used to determine where latest SpO2 will be assigned in buf
    int32_t spo2;                                     // current SpO2 value
    int8_t validSPO2;                                 // boolean indicator to show if the SpO2 calculation is valid

    // Assign new data points to sensor buffers
    redBuffer[senseBufIndex] = heartRateSensor.getRed();
    irBuffer[senseBufIndex] = irValue;
    
    // Increase current buffer index
    senseBufIndex++;
    senseBufIndex%=SPO2_LIGHT_BUF_LENGTH;

    // After gathering 25 new samples, recalculate SP02
    if(senseBufIndex%25 == 0)
    {
        // Calculate SpO2 and check if valid
        calcSpO2(irBuffer, SPO2_LIGHT_BUF_LENGTH, redBuffer, &spo2, &validSPO2);
        if(validSPO2 && spo2 > SPO2_MIN_VALID){
            // Assign to valid SpO2 array
            spO2Buf[sp02BufIndex] = spo2;
            sp02BufIndex++;
            sp02BufIndex%=SPO2_BUF_LENGTH;
            spO2Avg = 0;
            
            // Determine average SpO2
            for(int i = 0; i < SPO2_BUF_LENGTH; i++){
              spO2Avg += spO2Buf[i];
            }
            spO2Avg/=SPO2_BUF_LENGTH;
        }
    }
}

// Calculate users heart rate
void calculateHR(){
  static uint8_t heartRates[ARRAY_SIZE_HR];   // Array containing latest valid HR values
  static uint8_t heartRateIndex = 0;          // Index where latest latest value will be assigned into array
  static uint32_t prevHeartBeat = 0;          // Time at which the last heart beat occurred
  static uint32_t timeLastHRBeat = 0;         // Last time valid heart rate was calculated

  // Check if valid time - LPF
  if(millis()-timeLastHRBeat < 500)
  {
      return;
  }
  
  // Obtain infrared value
  uint32_t irValue = heartRateSensor.getIR();

  // If presence is not detected on sensor, go into standby mode
  // Note: STANDBY_CTR is used in case user wants to check time but not wear device
  if(irValue < IR_STANDBY_THRESH && (STANDBY_CTR >= STANDBY_TIMEOUT))
  {
      // Shutdown display and heart rate sensor
      u8g2.setPowerSave(PERIPH_SHUTDOWN); 
      heartRateSensor.shutDown();

      // Enable motion on wake up interrupt
      imu.enableMotionWakeup(MOTION_WAKE_THRESH, WAKEUP_FREQ);

      // Wait on motion interrupt
      while(digitalRead(INTERRUPT_PIN) != LOW){}

      // Disable interrupt and turn on peripherals
      imu.disableMotionWakeup();
      u8g2.setPowerSave(PERIPH_WAKEUP); 
      heartRateSensor.wakeUp();

      // Reset standby counter
      STANDBY_CTR = 0;
      
      return;
  }
  // Check if valid IR value being used
  else if(irValue > IR_STANDBY_THRESH)
  {
    // Check if heart beat was detected
    if (checkForBeat(irValue))
    {
      // Calculate time difference between 2 beats
      uint32_t heartBeatTimeDiff = millis() - prevHeartBeat;
      prevHeartBeat = millis();
  
      // Use the difference to calculate the heart rate
      float heartRate = 60 / (heartBeatTimeDiff / 1000.0); // 60 s in 1 min, 1000 ms in 1 s
  
      // Only use valid heart rates
      if (heartRate < HEARTRATE_MAX_VALID && heartRate > HEARTRATE_MIN_VALID)
      {
        // Store heart rate
        heartRates[heartRateIndex++] = heartRate;
        heartRateIndex %= ARRAY_SIZE_HR; // use modulus op. to determine current index
  
        // Calcute average heart rate
        heartRateAvg = 0; // reset
        for (int i = 0; i < ARRAY_SIZE_HR; i++){
          heartRateAvg += heartRates[i]; // add up all heart rates
        }
        heartRateAvg /= ARRAY_SIZE_HR; // determine average by dividing by size of array

        // Obtain current time
        timeLastHRBeat = millis();
      }
    }
    
    // Calculate SpO2 - pass in IR value so we do not have to retrieve it again
    calculateSpO2(irValue);

    // Reset standby counter
    STANDBY_CTR = 0;
  }
}

// Calculate steps based on gyroscope values
void countSteps(){
  static float stepMax = 0; // Peak of gyration data
  static float stepMin = 0; // Trough of gyration data
  static float gyroData[3]; // Array storing recent gyration readings
  static float iir_Av = 0;  // IIR value

  // Check for new data in the MPU9250 FIFO
  if ( !imu.fifoAvailable() )
  {
    return; // break out of function
  }
  // Try to update accel and gyro values
  if ( imu.dmpUpdateFifo() != INV_SUCCESS)
  {
    return; // break out of function
  }

  // Obtain recent gyro values
  float gyroX = imu.calcGyro(imu.gx);
  float gyroY = imu.calcGyro(imu.gy);
  float gyroZ = imu.calcGyro(imu.gz);

  // Obtain magnitude of gyration and pass it through IIR filter
  float magGyration = sqrt(sq(gyroX) + sq(gyroY) + sq(gyroZ));
  iir_Av = iirFilter(iir_Av, (int) magGyration);

  // Update recent valid gyro values
  gyroData[0] = gyroData[1];
  gyroData[1] = gyroData[2];
  gyroData[2] = iir_Av; 
  
  // If a peak/max is found - compare min to 100 to remove high freq data
  if(gyroData[1] > gyroData[0] && gyroData[1] > gyroData[2] && stepMin < STEP_MIN_THRESH)
  {
    stepMax = gyroData[1];// update peak value      
    double maxMinDiff = stepMax - stepMin; 
    // If a step is detected
    if (maxMinDiff > STEP_MIN_DIFF_THRESHOLD)
    {
        stepCount+=2; // increase by 2 as 2 steps were detected
        dmpStepCount = imu.dmpGetPedometerSteps();
    }
  }
  // If a trough/min is found
  else if(gyroData[1] < gyroData[0] && gyroData[1] < gyroData[2])
  {
    stepMin = gyroData[1];
  }
}

void alarmInterrupt(){
  // Do nothing
}


void powerManage(){
  // Read analog power inputs
  int inputVoltageRaw = analogRead(BATTERY_PIN);
  inputVoltage = 2*(inputVoltageRaw / ADC_RESOLUTION) * REFERENCE_VOLTAGE; // 2* because of voltage divider config

  // Check if peripherals need to be shutdown
  if(inputVoltage < VOLTAGE_SHUTDOWN_THRESH)
  {
    // Shutdown peripherals
    imu.shutDownPower(PERIPH_SHUTDOWN);
    u8g2.setPowerSave(PERIPH_SHUTDOWN); 
    heartRateSensor.shutDown();

    // loop until input power is sufficient for MCU to leave shutdown mode
    while(inputVoltage < VOLTAGE_SHUTDOWN_THRESH)
    {
      // Set MCU to sleep for 1 minute
      rtc.setAlarmSeconds(0);
      rtc.enableAlarm(rtc.MATCH_SS);
      rtc.attachInterrupt(alarmInterrupt);
      // Put MCU to sleep for 1 minute
      rtc.standbyMode();

      // Once MCU is awoken, check input voltage again
      inputVoltageRaw = analogRead(BATTERY_PIN);
      inputVoltage = 2*(inputVoltageRaw / ADC_RESOLUTION) * REFERENCE_VOLTAGE; // 2* because of voltage divider config
      if(inputVoltage >= VOLTAGE_SHUTDOWN_THRESH){
        // Input is sufficient - Turn on peripherals
        imu.shutDownPower(PERIPH_WAKEUP);
        u8g2.setPowerSave(PERIPH_WAKEUP); 
        heartRateSensor.wakeUp();
        break;
      }
    }
  }
}

// Main loop function
void loop(){
  static bool deviceInit = 0; // Prevents time showing up without initial BLE connection
  
  // Process any ACI commands or events from BLE
  bleProcess();
 
  // Check if data is available
  if (bleRXBbufferLen) 
  { 
    if(strncmp((const char*)bleRXBuffer + 2, "/", 1) == 0){
      strcpy(dateBT, (const char*)bleRXBuffer);
      setRTCDate();
    }
    else if(strncmp((const char*)bleRXBuffer + 2, ":", 1) == 0){
      strcpy(timeBT, (const char*)bleRXBuffer);
      setRTCTime();
    }
    else if(strncmp((const char*)bleRXBuffer, "C", 1) == 0){
        strcpy(callBT, (const char*)bleRXBuffer+1);
        NOTIF_CTR = 0;
    }
    else if(strncmp((const char*)bleRXBuffer, "M", 1) == 0){
        strcpy(textBT, (const char*)bleRXBuffer+1);
        NOTIF_CTR = 0;
    }

    // Clear buffer afer reading
    bleRXBbufferLen = 0;

    // Update display
    u8g2.firstPage();
    do {
      updateDisplay();
    } while ( u8g2.nextPage() );

    // Check if FRAM data has been sent during this connection
    if(isDataSent == false)
    {
      burstTransferFRAM();
    }

    deviceInit = 1; // Established initial BLE connection
  }

  // Check to see if notifications need to be cleared - add NULL terminator
  if(NOTIF_CTR >= NOTIF_COUNTER_MAX){
    callBT[0] = '\0';
    textBT[0] = '\0';
  }

  // Update display
  if(updateDisplay_flag && deviceInit)
  {    
    // Check on power inputs
    powerManage();
    
    // Update display
    u8g2.firstPage();
    do {
      updateDisplay();
    } while ( u8g2.nextPage() );
    
    // Reset display flag
    updateDisplay_flag = 0;
  }

  // Calculate steps and heart rate
  countSteps();
  calculateHR(); 
  
  // Determine whether data is sent to FRAM or BLE
  // Write to FRAM every 30 seconds
  if(DATA_CTR >= DATA_INTERVAL && !bleConnectionState)
  {
    writeFRAM();
    DATA_CTR = 0;
  }
  // Write to BLE every 30 seconds if connected and data is valid
  else if(DATA_CTR >= DATA_INTERVAL && bleConnectionState && heartRateAvg > HEARTRATE_MIN_VALID)
  {
    // Compile packet
    uint8_t txBuf[4];
    txBuf[0] = heartRateAvg;
    txBuf[1] = (stepCount+dmpStepCount) >> 8;
    txBuf[2] = (stepCount+dmpStepCount);
    txBuf[3] = spO2Avg;
    writeUARTTX((char*)txBuf, 4);
    DATA_CTR = 0;
  }
}

// Initialize MPU-9250
void initMPU9250(){
  // Verify communication and init default values
  imu.begin();

  // Enable 3DOF gyro and acceleromter
  imu.setSensors(INV_XYZ_GYRO | INV_XYZ_ACCEL); 
  imu.setGyroFSR(2000); // Set gyro to 2000 dps

  // Initialize gyro and step detection features
  unsigned short dmpFeatureMask = 0;
  dmpFeatureMask |= DMP_FEATURE_GYRO_CAL;
  dmpFeatureMask |= DMP_FEATURE_SEND_CAL_GYRO;
  dmpFeatureMask |= DMP_FEATURE_PEDOMETER;
  imu.dmpBegin(dmpFeatureMask);
  imu.dmpSetPedometerSteps(0);
  imu.dmpSetPedometerTime(0);
  
  // Configure interrupt as active-low, using GPIO internal pull-up resistor
  pinMode(INTERRUPT_PIN, INPUT_PULLUP);
  imu.setIntLevel(INT_ACTIVE_LOW);
}

// Initializes BLE and puts it into peripheral mode
void BLEsetup(){
  HCI_Init();
  //Init SPI interface
  BNRG_SPI_Init();
  // Reset BlueNRG/BlueNRG-MS SPI interface
  BlueNRG_RST();

  // Set BD Address
  uint8_t bdaddr[] = {0x12, 0x34, 0x00, 0xE1, 0x80, 0x02};
  aci_hal_write_config_data(CONFIG_DATA_PUBADDR_OFFSET, CONFIG_DATA_PUBADDR_LEN, bdaddr);

  // Initialize GAP
  aci_gatt_init();
  uint16_t service_handle, dev_name_char_handle, appearance_char_handle;
  aci_gap_init_IDB05A1(GAP_PERIPHERAL_ROLE_IDB05A1, 0, 0x07, &service_handle, &dev_name_char_handle, &appearance_char_handle);

  // Initialize BLE stack
  const char *name = "BlueNRG";
  aci_gatt_update_char_value(service_handle, dev_name_char_handle, 0, strlen(name), (uint8_t *)name);

  // Add UART service
  Add_UART_Service();

  // Set output power to +4 dBm
  aci_hal_set_tx_power_level(1, 3);
}

// Process BLE status
void bleProcess() {
  HCI_Process();
  bleConnectionState = connected;
  if (set_connectable) {
    setConnectable();
    set_connectable = 0;
  }
}

// Write to BLE using UART service
void writeUARTTX(char* TXdata, uint8_t datasize){
  aci_gatt_update_char_value(UARTServHandle, UARTRXCharHandle, 0, datasize, (uint8_t *)TXdata);
}

// Recursive Infinite Impulse Response filter
float iirFilter(float iir_Av, int val){
  iir_Av = iir_Av + ((float)val - iir_Av)/16;
  return iir_Av;
}

// Add UART services and characteristics
void Add_UART_Service(void){
  uint8_t uuid[16];

  COPY_UART_SERVICE_UUID(uuid);
  aci_gatt_add_serv(UUID_TYPE_128,  uuid, PRIMARY_SERVICE, 7, &UARTServHandle);

  COPY_UART_TX_CHAR_UUID(uuid);
  aci_gatt_add_char(UARTServHandle, UUID_TYPE_128, uuid, 20, CHAR_PROP_WRITE_WITHOUT_RESP, ATTR_PERMISSION_NONE, GATT_NOTIFY_ATTRIBUTE_WRITE,
                           16, 1, &UARTTXCharHandle);

  COPY_UART_RX_CHAR_UUID(uuid);
  aci_gatt_add_char(UARTServHandle, UUID_TYPE_128, uuid, 20, CHAR_PROP_NOTIFY, ATTR_PERMISSION_NONE, 0,
                           16, 1, &UARTRXCharHandle);
}

// Connect and disconnect to BLE device
void setConnectable(void){
  // Set device to connectable
  const char local_name[] = {AD_TYPE_COMPLETE_LOCAL_NAME, 'E', 's', 'p', 'e', 'r', 't', 'o'};
  hci_le_set_scan_resp_data(0, NULL);
  aci_gap_set_discoverable(ADV_IND,
                           (ADV_INTERVAL_MIN_MS * 1000) / 625, (ADV_INTERVAL_MAX_MS * 1000) / 625,
                           STATIC_RANDOM_ADDR, NO_WHITE_LIST_USE,
                           sizeof(local_name), local_name, 0, NULL, 0, 0);
}

// Recieve BLE data
void Attribute_Modified_CB(uint16_t handle, uint8_t data_length, uint8_t *att_data){
  if (handle == UARTTXCharHandle + 1) {
    int i;
    for (i = 0; i < data_length; i++) {
      bleRXBuffer[i] = att_data[i];
    }
    bleRXBuffer[i] = '\0';
    bleRXBbufferLen = data_length;
  }
}

// Process BLE message
void HCI_Event_CB(void *pckt){
  static uint16_t connectionHandle = 0;
  
  hci_uart_pckt *hci_pckt = (hci_uart_pckt *)pckt;
  hci_event_pckt *event_pckt = (hci_event_pckt*)hci_pckt->data;

  if (hci_pckt->type != HCI_EVENT_PKT)
    return;

  switch (event_pckt->evt) {

    case EVT_DISCONN_COMPLETE:
      {
        // Make the device connectable again
        connected = FALSE;
        set_connectable = TRUE;
      }
      break;

    case EVT_LE_META_EVENT:
      {
        evt_le_meta_event *evt = (evt_le_meta_event *)event_pckt->data;
        switch (evt->subevent)
        {
          case EVT_LE_CONN_COMPLETE:
            {
              evt_le_connection_complete *cc = (evt_le_connection_complete *)evt->data;
              connected = TRUE;
              connectionHandle = cc->handle;
            }
            break;
        }
      }
      break;

    case EVT_VENDOR:
      {
        evt_blue_aci *blue_evt = (evt_blue_aci *)event_pckt->data;
        switch (blue_evt->ecode)
        {
          case EVT_BLUE_GATT_READ_PERMIT_REQ:
            {
              evt_gatt_read_permit_req *pr = (evt_gatt_read_permit_req *)blue_evt->data;
              if (connectionHandle != 0)
                  aci_gatt_allow_read(connectionHandle);
            }
            break;

          case EVT_BLUE_GATT_ATTRIBUTE_MODIFIED:
            {
              evt_gatt_attr_modified_IDB05A1 *evt = (evt_gatt_attr_modified_IDB05A1*)blue_evt->data;
              Attribute_Modified_CB(evt->attr_handle, evt->data_length, evt->att_data);
            }
            break;
        }
      }
      break;
  }
}

// Convert a number in a char array to a byte data type
byte stringToByte(char *src, int numBytes){
  char charBuffer[4];
  memcpy(charBuffer, src, numBytes);
  return (byte)atoi(charBuffer);
}
