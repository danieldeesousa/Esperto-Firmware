/*
  ******************************************************************************
  * @file    esperto_mxa30102_spo2.cpp
  * @author  Daniel De Sousa
  * @version V2.1.1
  * @date    26-August-2018
  * @brief  
  ******************************************************************************
*/

#include "arduino.h"
#include "esperto_max30102_spo2.h"

static void findPeaks(int32_t *locationIndices, int32_t *numPeaks,  int32_t  *inputData, int32_t size, int32_t minHeight, int32_t minDistance, int32_t maxNumPeaks);
static void findPeaksAboveHeight(int32_t *locationIndices, int32_t *numPeaks,  int32_t  *inputData, int32_t size, int32_t minHeight);
static void removeClosePeaks(int32_t *locationIndices, int32_t *numPeaks, int32_t *inputData, int32_t minDistance);
static void sortAscend(int32_t *inputData, int32_t size);
static void sortDescend(int32_t  *inputData, int32_t *pn_indx, int32_t size);

static int32_t infraredRaw[BUFFER_SIZE];
static int32_t redRaw[BUFFER_SIZE];

/**
* \brief        Calculate Spo2
* \par          Details
*               By detecting  peaks of PPG cycle and corresponding AC/DC of red/infra-red signal, the absorptionRatios for the SPO2 is computed.
*               Since this algorithm is aiming for Arm M0/M3. formaula for SPO2 did not achieve the accuracy due to register overflow.
*               Thus, accurate SPO2 is precalculated and save longo uch_spo2_table[] per each absorptionRatios.
*
* \param[in]    *infraredBuffer     - IR sensor data buffer
* \param[in]    infraredBufferLen   - IR sensor data buffer length
* \param[in]    *redBuffer          - Red sensor data buffer
* \param[out]   *spO2Value          - Calculated SpO2 value
* \param[out]   *spO2Valid          - 1 if the calculated SpO2 value is valid
*/
void calcSpO2(uint32_t *infraredBuffer, int32_t infraredBufferLen, uint32_t *redBuffer, int32_t *spO2Value, int8_t *spO2Valid)
{
  int32_t k;

  // Calculates DC mean from infrared values
  uint32_t infraredDCMean = 0; 
  for (k = 0; k < infraredBufferLen; k++) 
  {
	infraredDCMean += infraredBuffer[k];
  }
  infraredDCMean =infraredDCMean/infraredBufferLen;
    
  // Remove DC and invert signal so that we can use peak detector as valley detector
  // Note: we use a valley detecter as an infrared valley represents a heart beat
  // HbO2 absorbs infrared. Increase in Hb02 (heart beat) is decrease in infrared reflection
  for (k = 0; k < infraredBufferLen; k++)  
  {
	infraredRaw[k] = -1*(infraredBuffer[k] - infraredDCMean) ; 
  }
    
  // Calculate 4 pt Moving Average - smoothes out the curve
  for(k = 0; k < BUFFER_SIZE-MA4_SIZE; k++)
  {
    infraredRaw[k] = (infraredRaw[k] + infraredRaw[k+1] + infraredRaw[k+2] + infraredRaw[k+3])/(int)4;        
  }
  
  // Calculate threshold (the average of all of the averages)
  int32_t threshold = 0; 
  for (k = 0; k < BUFFER_SIZE; k++)
  {
    threshold += infraredRaw[k];
  }
  threshold = threshold/(BUFFER_SIZE);
  
  // Limit the threshold
  if(threshold < 30)
  {
	threshold = 30; // minimum threshold allowed
  }	  
  else if(threshold > 60)
  {
	threshold = 60; // max threshold allowed
  } 

  // Ensure infrared location buffer is cleared
  int32_t infraredValleyLocations[15];
  for (k = 0; k < 15; k++) 
  {
	infraredValleyLocations[k] = 0;
  }
  
  // Use peak detector as valley detector since we flipped signal
  int32_t numValleys;
  findPeaks(infraredValleyLocations, &numValleys, infraredRaw, BUFFER_SIZE, threshold, 4, 15 );

  // Load raw value again for SPO2 calculation
  for (k = 0; k < infraredBufferLen; k++)  
  {
	infraredRaw[k] =  infraredBuffer[k]; 
	redRaw[k] =  redBuffer[k]; 
  }
  
  // Calculate absorptoin ratios
  int32_t absorptionRatios[5]; 
  int32_t absorptionRatioCount = 0; 
  int32_t redAC, infraredAC;
  int32_t absorptionNume, absorptionDenom;
  
  // Clear ratio buffer
  for(k = 0; k < 5; k++)
  {
	absorptionRatios[k]=0;
  }

  // Check if any valley locations are out of range
  for (k = 0; k < numValleys; k++)
  {
    if (infraredValleyLocations[k] > BUFFER_SIZE)
	{
      *spO2Value =  -999; 
      *spO2Valid  = 0; 
      return;
    }
  }
  
  // Find max between two valley locations and use absorption ratios betwen AC compoent of Ir & Red and DC compoent of Ir & Red for SPO2 
  int32_t redDCMax = -16777216; 
  int32_t infraredDCMax = -16777216; 
  int32_t redDCMaxIndex, infraredDCMaxIndex; 
  for (k = 0; k < numValleys-1; k++)
  {
	// Check if maximums are too close
    if (infraredValleyLocations[k+1] - infraredValleyLocations[k] > 3)
	{
	  // Update maximums if points exist between the two adjacent maximums
      for (int32_t i = infraredValleyLocations[k]; i < infraredValleyLocations[k+1]; i++)
	  {
		if (infraredRaw[i] > infraredDCMax) 
		{
		  infraredDCMax = infraredRaw[i]; 
		  infraredDCMaxIndex = i;
		}
		if (redRaw[i] > redDCMax)
		{
		  redDCMax = redRaw[i]; 
		  redDCMaxIndex = i;
		}
      }
	  
	  // Calculate AC component for red
      redAC = (redRaw[infraredValleyLocations[k+1]] - redRaw[infraredValleyLocations[k]])*(redDCMaxIndex - infraredValleyLocations[k]);
      redAC = redRaw[infraredValleyLocations[k]] + redAC/(infraredValleyLocations[k+1] - infraredValleyLocations[k]); 
      redAC = redRaw[redDCMaxIndex] - redAC;    // subracting linear DC compoenents from raw 
      
	  // Calculate AC component for infrared
	  infraredAC = (infraredRaw[infraredValleyLocations[k+1]] - infraredRaw[infraredValleyLocations[k]])*(infraredDCMaxIndex - infraredValleyLocations[k]);
      infraredAC = infraredRaw[infraredValleyLocations[k]] + infraredAC/(infraredValleyLocations[k+1] - infraredValleyLocations[k]); 
      infraredAC = infraredRaw[redDCMaxIndex] - infraredAC;      // subracting linear DC compoenents from raw 
      
	  // Calculate absorption ratio
	  absorptionNume = (redAC*infraredDCMax) >> 7;
      absorptionDenom = (infraredAC*redDCMax) >> 7;
      if (absorptionDenom > 0  && absorptionRatioCount < 5 && absorptionNume != 0)
      {   
        absorptionRatios[absorptionRatioCount]= (absorptionNume*100)/absorptionDenom;
        absorptionRatioCount++;
      }
    }
  }
  
  // Choose median value since PPG signal varies from beat to beat
  int32_t ratioAverage; 
  sortAscend(absorptionRatios, absorptionRatioCount);
  int32_t middleIndex = absorptionRatioCount/2;

  // Determine the average ratio
  if (middleIndex > 1)
  {
	// Calculate median
	ratioAverage = (absorptionRatios[middleIndex-1] + absorptionRatios[middleIndex])/2;
  }
  else
  {
	ratioAverage = absorptionRatios[middleIndex];
  }

  if(ratioAverage > 2 && ratioAverage < 184)
  {
	// Valid SpO2 found - use look up table
    *spO2Value = uch_spo2_table[ratioAverage] ;
    *spO2Valid  = 1;
  }
  else
  {
	// SpO2 is out of range
    *spO2Value = -999 ;
    *spO2Valid = 0; 
  }
}

/**
	Find at most maxNumPeaks peaks above minHeight separated by at least minDistance
*/
void findPeaks(int32_t *locationIndices, int32_t *numValleys, int32_t *inputData, int32_t size, int32_t minHeight, int32_t minDistance, int32_t maxNumPeaks)
{
  findPeaksAboveHeight(locationIndices, numValleys, inputData, size, minHeight);
  removeClosePeaks(locationIndices, numValleys, inputData, minDistance);
  *numValleys = min(*numValleys, maxNumPeaks);
}

/**
	Find all peaks above a minimum height
*/
void findPeaksAboveHeight(int32_t *locationIndices, int32_t *numValleys, int32_t *inputData, int32_t size, int32_t minHeight)
{
  int32_t i = 1, width;
  *numValleys = 0;
  
  while (i < size-1)
  {
	// Find left edge of potential peaks
    if (inputData[i] > minHeight && inputData[i] > inputData[i-1])
	{      
      width = 1;
	  // If peak is flat
      while (i + width < size && inputData[i] == inputData[i+width])
	  {
		width++;
	  }
	  // Find right edge of potential peaks
      if (inputData[i] > inputData[i+width] && (*numValleys) < 15 )
	  {      
        locationIndices[(*numValleys)++] = i;    
        // Note: For flat peaks, peak location is left edge
        i += width+1;
      }
      else
	  {
		i += width;
	  }
    }
    else
	{
      i++;
	}
  }
}

/**
	Remove peaks separated by less than a minimum value
*/
void removeClosePeaks(int32_t *locationIndices, int32_t *numPeaks, int32_t *inputData, int32_t minDistance)
{
  int32_t prevNumPeaks, dist;
    
  // Order peaks in descending order
  sortDescend(inputData, locationIndices, *numPeaks);

  // Loop through all peaks
  for (int32_t i = -1; i < *numPeaks; i++ )
  {
    prevNumPeaks = *numPeaks;
    *numPeaks = i + 1;
    for (int32_t j = i + 1; j < prevNumPeaks; j++ )
	{
      dist = locationIndices[j] - (i == -1 ? -1 : locationIndices[i]); // lag-zero peak of autocorr is at index -1
      if ( dist > minDistance || dist < -minDistance )
	  {
		locationIndices[(*numPeaks)++] = locationIndices[j];
	  }
    }
  }

  // Resort indices to ascending order
  sortAscend(locationIndices, *numPeaks);
}

/**
    Sort array in ascending order (insertion sort algorithm)
*/
void sortAscend(int32_t *inputData, int32_t size) 
{
  int32_t temp;
  
  for (int32_t i = 1; i < size; i++) 
  {
    temp = inputData[i];
    for (int32_t j = i; j > 0 && temp < inputData[j-1]; j--)
	{
		inputData[j] = inputData[j-1];
	}
    inputData[j] = temp;
  }
}

/**
	Sort indices in descending order (insertion sort algorithm)
*/ 
void sortDescend(int32_t *inputData, int32_t *pn_indx, int32_t size)
{
  int32_t temp;
  
  for (int32_t i = 1; i < size; i++) 
  {
    temp = pn_indx[i];
    for (int32_t j = i; j > 0 && inputData[temp] > inputData[pn_indx[j-1]]; j--)
	{
		pn_indx[j] = pn_indx[j-1];
	}
    pn_indx[j] = temp;
  }
}