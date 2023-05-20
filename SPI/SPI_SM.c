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
 * All the SLO2 class contributed to this code
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
#define SPI_KIT SPI_ID_1

/* Select SPI frequency to use */
#define SPI_FREQ 5000000

/* Dummy byte is used when reading only 
 * to shift peripheric internal register */
#define DUMMY_BYTE  0x81

/* Maximal number of bytes to be stored in buffer */
#define SPI_BUFFER_SIZE 2

/*****************************************************************************/

/* State machine variable */
SPI_STATES spiState = SPI_STATE_UNINITIALIZED;

/* Flag if SPI is doing an action */
bool spiBusy = false;

/* Number of datas to read or write */
uint8_t nData = 0;

/* Index of datas to shift pointer */
uint8_t iData = 0;

/* Pointer to the datas to write */
uint8_t* wData;

/* Buffer to store read data */
uint8_t rData[SPI_BUFFER_SIZE];

/*****************************************************************************/

/* One time call function, at the start */
void SPI_Init(void)
{
	PLIB_SPI_Disable(SPI_KIT);
	PLIB_SPI_BufferClear(SPI_KIT);
	PLIB_SPI_StopInIdleDisable(SPI_KIT);
	PLIB_SPI_PinEnable(SPI_KIT, SPI_PIN_DATA_OUT);
	PLIB_SPI_CommunicationWidthSelect(SPI_KIT, SPI_COMMUNICATION_WIDTH_8BITS);
	PLIB_SPI_BaudRateSet(SPI_KIT, SYS_CLK_PeripheralFrequencyGet(CLK_BUS_PERIPHERAL_1), SPI_FREQ);
	PLIB_SPI_InputSamplePhaseSelect(SPI_KIT, SPI_INPUT_SAMPLING_PHASE_IN_MIDDLE );
	PLIB_SPI_ClockPolaritySelect(SPI_KIT, SPI_CLOCK_POLARITY_IDLE_LOW);
	PLIB_SPI_OutputDataPhaseSelect(SPI_KIT, SPI_OUTPUT_DATA_PHASE_ON_IDLE_TO_ACTIVE_CLOCK);
	PLIB_SPI_MasterEnable(SPI_KIT);
	PLIB_SPI_FramedCommunicationDisable(SPI_KIT);
	PLIB_SPI_FIFOEnable(SPI_KIT);
	PLIB_SPI_Enable(SPI_KIT);

	/* Configuration control */
	//ConfigReg = SPI1CON;
	//BaudReg = SPI1BRG;
    
    /* Initialize buffer */
    for(iData = 0; iData < SPI_BUFFER_SIZE; iData++)
        rData[iData] = 0;
    iData = 0;
   
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
			
		case SPI_STATE_BUSY_READ :
            
            spiBusy = PLIB_SPI_IsBusy(SPI_KIT);
            
            if(!PLIB_SPI_ReceiverFIFOIsEmpty(SPI_KIT))
            {
                if(iData < nData)
                {
                    rData[iData] = PLIB_SPI_BufferRead(SPI_ID_1);
                    iData++;
                }
                else
                {
                    SPI_ResetState();
                }            
            }
            else if(!spiBusy)
            {
                PLIB_SPI_BufferWrite(SPI_KIT, DUMMY_BYTE);
            }            
            
			break;
			
		case SPI_STATE_BUSY_WRITE :
            
            spiBusy = PLIB_SPI_IsBusy(SPI_KIT);
            
            if(!spiBusy)
            {
                if(iData < nData)
                {
                    PLIB_SPI_BufferWrite(SPI_KIT, *(wData + iData));
                    iData++;
                }
                else
                {
                    SPI_ResetState();
                }
            }
			break;
			
		case SPI_STATE_BUSY_READ_WRITE :
			break;
			
		case SPI_STATE_IDLE_READ_DATA_AVAILABLE :
			break;			
	} 
}

/*****************************************************************************/

//Ecriture.
//Comme le SPI est obligatoirement full-duplex,
//les données reçues ne seront pas traitées
void SPI_StartWrite(uint8_t nBytes, uint8_t* pBytesToWrite)
{
    nData       = nBytes;
    wData       = pBytesToWrite;
    spiState    = SPI_STATE_BUSY_WRITE;
}

/*****************************************************************************/

//Lecture/écriture.
//Comme le SPI est obligatoirement full-duplex,
//des données sont reçues simultanément à l'envoi
void SPI_StartReadWrite(uint8_t nBytes, uint8_t* pBytesToWrite)
{
    nData       = nBytes;
    wData       = pBytesToWrite;
    spiState    = SPI_STATE_BUSY_READ_WRITE;
}

/*****************************************************************************/

//Lecture.
//Comme le SPI est obligatoirement full-duplex,
//il faut envoyer des données bidons pour faire une lecture
void SPI_StartRead(uint8_t nBytes)
{
    nData       = nBytes;
    spiState    = SPI_STATE_BUSY_READ; 
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
 * SPI_GetState
 * @return Current state of SPI state machine
 */
void SPI_ResetState(void)
{
    iData = 0;
    spiState = SPI_STATE_IDLE;  
}

/*****************************************************************************/

/**
 * SPI_ReadByte
 * @return Byte in reception buffer
 */
uint8_t SPI_ReadByte(uint8_t bytePos)
{
    if(bytePos < 0)
        bytePos = 0;
    else if(bytePos >= SPI_BUFFER_SIZE)
        bytePos = SPI_BUFFER_SIZE - 1;
    
    return rData[bytePos];
}

/*****************************************************************************/
