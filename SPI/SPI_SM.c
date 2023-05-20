/*******************************************************************************
 *  _______  ___________  ___  ___   __               _______      _______ 
 * |   ____||           ||   \/   | |  |             |   ____|    /       |
 * |  |__   `---|  |----`|  \  /  | |  |      ______ |  |__      |   (----`
 * |   __|      |  |     |  |\/|  | |  |     |______||   __|      \   \    
 * |  |____     |  |     |  |  |  | |  `----.        |  |____ .----)   |   
 * |_______|    |__|     |__|  |__| |_______|        |_______||_______/                                                      
 * 
 * @file SPI_SM.c
 * @summary 
 * 
 * Handle SPI by state machine
 * 
 * @authors
 * 
 * Only peoples mentionned contributed to this code
 * The official version is available at : 
 * https://github.com/Miguel-SLO/MINF-GENELEC3.git
 * 
 * @creation 20.05.2023
 * @lastmodif 20.05.2023
 * 
 ******************************************************************************/

#include "SPI_SM.h"
#include "peripheral/spi/plib_spi.h"
#include "system/clk/sys_clk.h" // pour SYS_CLK_PeripheralFrequencyGet()

/*****************************************************************************/

/* Select which SPI to use */
#define SPI_ID SPI_ID_1

/* Select peripheral CS */
#define SPI_CS CS_LM70

/* Select SPI frequency to use */
#define SPI_FREQ 5000000

/* Dummy byte is used when reading only 
 * to shift peripheric internal register */
#define DUMMY_BYTE  0x81

/*****************************************************************************/

/* State machine variable */
SPI_STATES spiState = SPI_STATE_UNINITIALIZED;

/*****************************************************************************/

/* One time call function, at the start */
void SPI_Init(void)
{
	PLIB_SPI_Disable(SPI_ID);
	PLIB_SPI_BufferClear(SPI_ID);
	PLIB_SPI_StopInIdleDisable(SPI_ID);
	PLIB_SPI_PinEnable(SPI_ID, SPI_PIN_DATA_OUT);
	PLIB_SPI_CommunicationWidthSelect(SPI_ID, SPI_COMMUNICATION_WIDTH_8BITS);
	PLIB_SPI_BaudRateSet(SPI_ID, SYS_CLK_PeripheralFrequencyGet(CLK_BUS_PERIPHERAL_1), SPI_FREQ);
	PLIB_SPI_InputSamplePhaseSelect(SPI_ID, SPI_INPUT_SAMPLING_PHASE_IN_MIDDLE );
	PLIB_SPI_ClockPolaritySelect(SPI_ID, SPI_CLOCK_POLARITY_IDLE_LOW);
	PLIB_SPI_OutputDataPhaseSelect(SPI_ID, SPI_OUTPUT_DATA_PHASE_ON_IDLE_TO_ACTIVE_CLOCK);
	PLIB_SPI_MasterEnable(SPI_ID);
	PLIB_SPI_FramedCommunicationDisable(SPI_ID);
	PLIB_SPI_FIFOEnable(SPI_ID);
	PLIB_SPI_Enable(SPI_ID);
  
	spiState = SPI_STATE_IDLE;
}

/*****************************************************************************/

//fonction à appeler périodiquement pour gestion SPI
//gestion de la machine d'état du SPI
void SPI_DoTasks(void)
{
	switch(spiState)
	{
		case SPI_STATE_UNINITIALIZED :
			/* Use SPI_Init() at launch */
			break;
			
		case SPI_STATE_IDLE :
            /* Waiting for a start */
			break;		
			
		case SPI_STATE_BUSY_WRITE :
            
            if(!PLIB_SPI_IsBusy(SPI_ID))
            {
                SPI_CS = 1;
                spiState = SPI_STATE_IDLE;
            }            
			break;
            
		case SPI_STATE_BUSY_READ :	
		case SPI_STATE_BUSY_READ_WRITE :
            
            if(!PLIB_SPI_IsBusy(SPI_ID) && !PLIB_SPI_ReceiverFIFOIsEmpty(SPI_ID))
            {
                SPI_CS = 1;
                spiState = SPI_STATE_IDLE_READ_DATA_AVAILABLE;
            }            
			break;
			
		case SPI_STATE_IDLE_READ_DATA_AVAILABLE :
            
            PLIB_SPI_BufferRead(SPI_ID);    
            if (PLIB_SPI_ReceiverFIFOIsEmpty(SPI_ID))
                spiState = SPI_STATE_IDLE;
			break;			
	} 
}

/*****************************************************************************/

//Lecture.
//Comme le SPI est obligatoirement full-duplex,
//il faut envoyer des données bidons pour faire une lecture
void SPI_StartRead(uint8_t nBytes)
{
	uint8_t iData = 0;
	
	SPI_CS = 0;
	
	for( iData = 0 ; iData < nBytes ; iData++ )
        PLIB_SPI_BufferWrite(SPI_ID, DUMMY_BYTE);	

    spiState = SPI_STATE_BUSY_READ;
}

/*****************************************************************************/

//Ecriture.
//Comme le SPI est obligatoirement full-duplex,
//les données reçues ne seront pas traitées
void SPI_StartWrite(uint8_t nBytes, uint8_t* pBytesToWrite)
{
	uint8_t iData = 0;
	
	SPI_CS = 0;
	
	for( iData = 0 ; iData < nBytes ; iData++ )
        PLIB_SPI_BufferWrite(SPI_ID, *(pBytesToWrite + iData));	
	
    spiState = SPI_STATE_BUSY_WRITE;
}

/*****************************************************************************/

//Lecture/écriture.
//Comme le SPI est obligatoirement full-duplex,
//des données sont reçues simultanément à l'envoi
void SPI_StartReadWrite(uint8_t nBytes, uint8_t* pBytesToWrite)
{
    uint8_t iData = 0;
	
	SPI_CS = 0;
	
	for( iData = 0 ; iData < nBytes ; iData++ )
        PLIB_SPI_BufferWrite(SPI_ID, *(pBytesToWrite + iData));	
	
    spiState = SPI_STATE_BUSY_READ_WRITE;
}

/*****************************************************************************/

/**
 * SPI_GetState
 * @return Current state of SPI state machine
 */
SPI_STATES SPI_GetState(void)
{
    return spiState; 
}

/*****************************************************************************/

/**
 * SPI_UpdateState
 * @return Current state of SPI state machine
 */
void SPI_UpdateState(SPI_STATES NewState)
{
    spiState = NewState; 
}

/*****************************************************************************/

///**
// * SPI_ReadByte
// * @return Byte in reception buffer
// */
//uint8_t SPI_ReadByte(void)
//{
//
//}

/*****************************************************************************/
