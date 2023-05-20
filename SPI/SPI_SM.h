/*******************************************************************************
*  _______  ___________  ___  ___   __               _______      _______ 
* |   ____||           ||   \/   | |  |             |   ____|    /       |
* |  |__   `---|  |----`|  \  /  | |  |      ______ |  |__      |   (----`
* |   __|      |  |     |  |\/|  | |  |     |______||   __|      \   \    
* |  |____     |  |     |  |  |  | |  `----.        |  |____ .----)   |   
* |_______|    |__|     |__|  |__| |_______|        |_______||_______/                                                      
* 
* @file SPI_SM.h
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
*******************************************************************************/

#ifndef SPI_SM_H
#define SPI_SM_H

#include <stdint.h>

typedef enum
{
	/* SPI's state machine's states. */
	SPI_STATE_UNINITIALIZED=0,
    SPI_STATE_IDLE,
    SPI_STATE_IDLE_READ_DATA_AVAILABLE,
    SPI_STATE_BUSY_WRITE,        
    SPI_STATE_BUSY_READ_WRITE,
    SPI_STATE_BUSY_READ,
} SPI_STATES;

//fonction à appeler 1x au démarrage pour init.
void SPI_Init(void);

//Ecriture.
//Comme le SPI est obligatoirement full-duplex,
//les données reçues ne seront pas traitées
void SPI_StartWrite(uint8_t nBytes, uint8_t* pBytesToWrite);

//Lecture/écriture.
//Comme le SPI est obligatoirement full-duplex,
//des données sont reçues simultanément à l'envoi
void SPI_StartReadWrite(uint8_t nBytes, uint8_t* pBytesToWrite);

//Lecture.
//Comme le SPI est obligatoirement full-duplex,
//il faut envoyer des données bidons pour faire une lecture
void SPI_StartRead(uint8_t nBytes);

//pour obtenir l'état interne de la SM spi
SPI_STATES SPI_GetState (void);

void SPI_ResetState(void);

//lecture d'un byte dans buffer réception
uint8_t SPI_ReadByte(uint8_t bytePos);

//fonction à appeler périodiquement pour gestion SPI
void SPI_DoTasks(void);

#endif /* SPI_SM_H */
