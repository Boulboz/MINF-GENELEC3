/* ************************************************************************** */
/** gestion SPI via machine d'état
 *
 *  @Company
 *    ETML-ES - SCA
 *
 *  @File Name
 *    Mc32Ex8_2_spi_sm.c
 *
 *  @Summary
 *    gestion SPI via machine d'état
 *
 *  @Description
 *    gestion SPI via machine d'état
 *    Testé pour fonctionner avec LM70 sur SPI1
 * 
 *   Création 14.03.2017 SCA
 *  
*/

#include "Mc32Ex8_2_spi_sm.h"
#include "bsp.h"
#include "peripheral/spi/plib_spi.h"
#include "system/clk/sys_clk.h" // pour SYS_CLK_PeripheralFrequencyGet()
#include "system/int/sys_int.h"


//byte bidon pour envoi lorsqu'uniquement une lecture est demandée
//le spi étant full-duplex, une lecture implique forcément une écriture simultanée
#define DUMMY_BYTE  0x81    
#define KitSpi1 (SPI_ID_1)

uint32_t ConfigReg;     // pour lecture de SPI1CON
uint32_t BaudReg;       // pour lecture de SPI1BRG
APP_STATES_SPI spiState;


//fonction à appeler 1x au démarrage pour init.
//code repris de la génération du driver par Hamony 1.08
void SPI_Init(void)
{
    PLIB_SPI_Disable(KitSpi1);

   PLIB_SPI_BufferClear(KitSpi1);
   PLIB_SPI_StopInIdleDisable(KitSpi1);
   PLIB_SPI_PinEnable(KitSpi1, SPI_PIN_DATA_OUT);
   PLIB_SPI_CommunicationWidthSelect(KitSpi1, SPI_COMMUNICATION_WIDTH_8BITS);
   // LM70 MAX 6.25 MHz choix 5 MHz
   PLIB_SPI_BaudRateSet(KitSpi1, SYS_CLK_PeripheralFrequencyGet(CLK_BUS_PERIPHERAL_1), 5000000);
   // Config polarité traitement des signaux SPI
   // pour input à confirmer
   // Polarité clock OK
   // Phase output à confirmer
   PLIB_SPI_InputSamplePhaseSelect(KitSpi1, SPI_INPUT_SAMPLING_PHASE_IN_MIDDLE );
   PLIB_SPI_ClockPolaritySelect(KitSpi1, SPI_CLOCK_POLARITY_IDLE_LOW);
   PLIB_SPI_OutputDataPhaseSelect(KitSpi1, SPI_OUTPUT_DATA_PHASE_ON_IDLE_TO_ACTIVE_CLOCK);
   PLIB_SPI_MasterEnable(KitSpi1);
   PLIB_SPI_FramedCommunicationDisable(KitSpi1);
   PLIB_SPI_FIFOEnable(KitSpi1);     // Enhenced buffer mode

   PLIB_SPI_Enable(KitSpi1);

   // Contrôle de la configuration
   ConfigReg = SPI1CON;
   BaudReg = SPI1BRG;  
   
//   // action de configuration
//   CS_LM70 = 0;
//   spi_read1(0xFF);
//   spi_read1(0xFF);
//   spi_read1(0);  // pour écrire 0
//   spi_read1(0);  // pour écrire 0 - pour sortir le LM70 du shutdown
//
//   //Fin de transmission
//   CS_LM70 = 1;
   
   
   spiState.state = SPI_STATE_IDLE;
}

//Ecriture.
//Comme le SPI est obligatoirement full-duplex,
//les données reçues ne seront pas traitées
void SPI_StartWrite(uint32_t nBytes, uint8_t* pBytesToWrite)
{
    uint32_t n;
    spiState.state = SPI_STATE_BUSY_WRITE;

    CS_LM70 = 0;
    
    for(n = 0 ; n<nBytes ; n++)
    {
        PLIB_SPI_BufferWrite(SPI_ID_1, pBytesToWrite);
    }
 
}

//Lecture/écriture.
//Comme le SPI est obligatoirement full-duplex,
//des données sont reçues simultanément à l'envoi
void SPI_StartReadWrite(uint32_t nBytes, uint8_t* pBytesToWrite)
{
    uint32_t n;
    spiState.state = SPI_STATE_BUSY_READ_WRITE;

    CS_LM70 = 0;
    
    for(n = 0 ; n<nBytes ; n++)
    {
        PLIB_SPI_BufferWrite(SPI_ID_1, pBytesToWrite);
    }
  
}

//Lecture.
//Comme le SPI est obligatoirement full-duplex,
//il faut envoyer des données bidons pour faire une lecture
void SPI_StartRead(uint32_t nBytes)
{
    uint32_t n;
    spiState.state = SPI_STATE_BUSY_READ;

    CS_LM70 = 0;
    
    for(n = 0 ; n<nBytes ; n++)
    {
        PLIB_SPI_BufferWrite(SPI_ID_1, DUMMY_BYTE);    
    }    
}

//pour obtenir l'état interne de la SM spi
SPI_STATES SPI_GetState (void)
{
    return spiState.state;
}

//lecture d'un byte dans buffer réception
uint8_t SPI_ReadByte(void)
{
    uint8_t retVal;      
    
    retVal = PLIB_SPI_BufferRead(SPI_ID_1);
    
    if (PLIB_SPI_ReceiverFIFOIsEmpty(SPI_ID_1))
    {
        spiState.state = SPI_STATE_IDLE;
    }
    
    return(retVal);
}

//fonction à appeler périodiquement pour gestion SPI
//gestion de la machine d'état du SPI
void SPI_DoTasks(void)
{
    switch (spiState.state)
    {
        case SPI_STATE_UNINITIALIZED:
        {
            break;
        }
        case SPI_STATE_IDLE:
        {
            break;
        }
        case SPI_STATE_IDLE_READ_DATA_AVAILABLE:
        {
            break;
        }
        case SPI_STATE_BUSY_WRITE:
        {
            if (!PLIB_SPI_IsBusy(SPI_ID_1) && !PLIB_SPI_ReceiverFIFOIsEmpty(SPI_ID_1))
            {
               CS_LM70 = 1;
               PLIB_SPI_BufferRead(SPI_ID_1);
    
                if (PLIB_SPI_ReceiverFIFOIsEmpty(SPI_ID_1))
                {
                    spiState.state = SPI_STATE_IDLE;
                }
            }
   
            break;
        }
        case SPI_STATE_BUSY_READ_WRITE:
        {
            if (!PLIB_SPI_IsBusy(SPI_ID_1) && !PLIB_SPI_ReceiverFIFOIsEmpty(SPI_ID_1))
            {
                CS_LM70 = 1;
                spiState.state = SPI_STATE_IDLE_READ_DATA_AVAILABLE;
            }  
            
            break;
        }
        case SPI_STATE_BUSY_READ:
        {
            if (!PLIB_SPI_IsBusy(SPI_ID_1) && !PLIB_SPI_ReceiverFIFOIsEmpty(SPI_ID_1))
            {
                CS_LM70 = 1;
                spiState.state = SPI_STATE_IDLE_READ_DATA_AVAILABLE;
            }  
            break;
        }
    }
 
}

void LM70_ConvRawToDeg( int16_t RowTemp, float *PTemp)
{
    float TempLoc;

    RowTemp = RowTemp / 32;  // par division pour assurrer le signe
    // Bit poid faible = 0.25 degré
    TempLoc = RowTemp * 0.25;
    *PTemp = TempLoc;
}

void APP_SPI_UpdateState (SPI_STATES NewState)
{
    spiState.state  = NewState;
    
}
/* *****************************************************************************
 End of File
 */
