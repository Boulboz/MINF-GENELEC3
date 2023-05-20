/*******************************************************************************
  MPLAB Harmony Application Source File
  
  Company:
    Microchip Technology Inc.
  
  File Name:
    app.c
  Summary:
    This file contains the source code for the MPLAB Harmony application.
  Description:
    This file contains the source code for the MPLAB Harmony application.  It 
    implements the logic of the application's state machine and it may call 
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2013-2014 released Microchip Technology Inc.  All rights reserved.
Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).
You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.
SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
// DOM-IGNORE-END


// *****************************************************************************
// *****************************************************************************
// Section: Included Files 
// *****************************************************************************
// *****************************************************************************

#include "app.h"
#include "Mc32DriverLcd.h"
#include "Mc32Ex8_2_spi_sm.h"


// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data
  Summary:
    Holds application data
  Description:
    This structure holds the application's data.
  Remarks:
    This structure should be initialized by the APP_Initialize function.
    
    Application strings and buffers are be defined outside this structure.
*/

APP_DATA appData;
APP_STATES_SPI appSPI;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


/* TODO:  Add any necessary local functions.
*/


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )
  Remarks:
    See prototype in app.h.
 */

void APP_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    appData.state = APP_STATE_INIT;
    
    APP_SPI_UpdateState(SPI_STATE_UNINITIALIZED);

    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
}


  //Remarks:
    //See prototype in app.h.


void APP_Tasks ( void )
{
    /* Check the application's current state. */
    switch ( appData.state )
    {
        /* Application's initial state. */
        case APP_STATE_INIT:
        {
            //initaliser LCD
            lcd_init ();
            //Activer le rétro-éclairage
            lcd_bl_on();
            //Ecrire à la première ligne
            lcd_gotoxy(1,1);
            printf_lcd ("Exercice 8_2");
            //Ecrire à la seconde ligne
            lcd_gotoxy(1,2);
            printf_lcd ("Caroline Mieville");
            
            //LM70 init
            SPI_Init();
            //Initialiser Timer
            DRV_TMR0_Start();
                        
           //Prochain état: wait
            APP_UpdateState (APP_STATE_WAIT);
                   
            break;
        }
        
        case APP_STATE_WAIT:
        {
        
            break;
        }
        
        case APP_STATE_SERVICE_TASKS:
        {
            float APP_TempLm70;
            uint8_t msb, lsb;
            int16_t RawTemp;
            BSP_LEDToggle(BSP_LED_2);
         
            SPI_DoTasks();
            
            switch(SPI_GetState())
            {
                case SPI_STATE_IDLE :
                    SPI_StartRead(2);   //starte nouvelle lecture
                    break;

                case SPI_STATE_IDLE_READ_DATA_AVAILABLE :
                    //lit valeur disponible
                    msb = SPI_ReadByte();
                    lsb = SPI_ReadByte();
                    
                    //recomposer une température sur la base de msb et lsb
                    RawTemp = msb;
                    RawTemp = RawTemp << 8;
                    RawTemp = RawTemp | lsb;
                    
                    LM70_ConvRawToDeg( RawTemp, &APP_TempLm70);
                    
                    
                    // Affichage temperature du LM70
                    lcd_gotoxy(1,3);
                    printf_lcd("RawTemp = %.2f°C", APP_TempLm70);
                    break;
                    
                default:
                    break;         
            }
     
           //Prochain état: wait
            APP_UpdateState (APP_STATE_WAIT);                 
    
            break;
        }

        /* TODO: implement your application state machine.*/
        

        /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            
            break;
        }
    }
}

//fonction UpdateState
//permet de mettre à jour le nouvel état du programme (INIT, WAIT, TASK)
void APP_UpdateState (APP_STATES NewState)
{
    appData.state = NewState;
    
}


/*******************************************************************************
 End of File
 */