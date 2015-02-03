//=============================================================================
// Demo program I2cTemp.c
//
// Can be run under control of the ROBO TX Controller
// firmware in download (local) mode.
// This example shows how to sense the temperature with the I2C temperature
// sensor DS1631. After writing some initialisation bytes the program enters
// a loop updating the measured value for the temperature every 1000ms.
//
// Disclaimer - Exclusion of Liability
//
// This software is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. It can be used and modified by anyone
// free of any license obligations or authoring rights.
//=============================================================================

#include "ROBO_TX_PRG.h"

#define LIGHT_ON        DUTY_MAX
#define LIGHT_OFF       0

static enum
{
    INIT_1,
    WAIT_1,
    INIT_2,
    WAIT_2,
    INIT_3,
    WAIT_3,
    INIT_4,
    WAIT_4,
    LOOP_WRITE,
    LOOP_WRITE_WAIT,
    LOOP_READ,
    LOOP_READ_WAIT,
    LOOP_CLEAR_PREV_SCREEN,
    LOOP_DISP_RESULT,
    LOOP_WAIT_NEXT_ACTION
} stage;

unsigned int ticks;
unsigned int next_action=0;
UINT16 status=0;
UINT16 value=0;

/*-----------------------------------------------------------------------------
 * Function Name       : I2cCallback
 *
 * This callback function is called to inform the program about result (status)
 * of execution of any I2c command.
 *-----------------------------------------------------------------------------*/
static void I2cCallback
(
    TA * p_ta_array,
    I2C_CB * p_data
)
{
    //TA * p_ta = &p_ta_array[TA_LOCAL];

    status = p_data->status;
    value  = p_data->value;

    stage++;
}

/*-----------------------------------------------------------------------------
 * Function Name       : PrgInit
 *
 * This it the program initialization.
 * It is called once.
 *-----------------------------------------------------------------------------*/
void PrgInit
(
    TA * p_ta_array,    // pointer to the array of transfer areas
    int ta_count        // number of transfer areas in array (equal to TA_COUNT)
)
{
    //TA * p_ta = &p_ta_array[TA_LOCAL];

    ticks = 0;
    stage = INIT_1;
}


/*-----------------------------------------------------------------------------
 * Function Name       : PrgTic
 *
 * This is the main function of this program.
 * It is called every tic (1 ms) realtime.
 *-----------------------------------------------------------------------------*/
int PrgTic
(
    TA * p_ta_array,    // pointer to the array of transfer areas
    int ta_count        // number of transfer areas in array (equal to TA_COUNT)
)
{
    int rc = 0x7FFF; // return code: 0x7FFF - program should be further called by the firmware;
                     //              0      - program should be normally stopped by the firmware;
                     //              any other value is considered by the firmware as an error code
                     //              and the program is stopped.
    TA * p_ta = &p_ta_array[TA_LOCAL];

    char str[64];
    int fraction=0;
    unsigned char temp=0;
    char sign = ' ';
    
    ticks++;
    
    while(1)
    {
        switch(stage)
        {
            case INIT_1:
                p_ta->hook_table.I2cWrite (0x4F, 0xAC, 0x02, 0x85, I2cCallback);
                stage++;
                return rc;

            case WAIT_1:  
            case WAIT_2:  
            case WAIT_3:  
            case WAIT_4:  
                // waiting for callback
                return rc;
                
            case INIT_2:    
                p_ta->hook_table.I2cWrite (0x4F, 0xA1, 0x2800, 0x89, I2cCallback);
                stage++;    
                return rc;

            case INIT_3:    
                p_ta->hook_table.I2cWrite (0x4F, 0xA2, 0x0A00, 0x89, I2cCallback);
                stage++;
                return rc;

            case INIT_4:    
                p_ta->hook_table.I2cWrite (0x4F, 0x00, 0x51, 0x84, I2cCallback);
                stage++;
                return rc;
                
            case LOOP_WRITE:
                p_ta->hook_table.I2cWrite (0x4F, 0x00, 0xAA, 0x84, I2cCallback);
                stage++;
                return rc;
                
            case LOOP_WRITE_WAIT:
                // waiting for callback    
                return rc;

            case LOOP_READ:
                p_ta->hook_table.I2cRead (0x4F, 0x00, 0x88, I2cCallback);
                stage++;
                return rc;
                
            case LOOP_READ_WAIT:
                // waiting for callback    
                return rc;
                
            case LOOP_CLEAR_PREV_SCREEN:
                p_ta->hook_table.DisplayMsg(p_ta, NULL);  // clear previous Msg output
                next_action = ticks + 20;
                stage++;
                return rc;
                
            case LOOP_DISP_RESULT:
                if(ticks >= next_action)  // wait for previous Msg output to be cleared
                {
                    if(value & 0x0080) fraction = 5;
                    else fraction = 0; 
                    if(value & 0x8000) 
                    {
                        sign = '-';
                        temp = value >> 8;
                        temp = ~temp;
                    }    
                    else 
                    {
                        sign = '+';
                        temp = value >> 8;
                    }    
                    p_ta->hook_table.sprintf(str, "Temperature: %c%d,%d C", sign, temp, fraction);
                    p_ta->hook_table.DisplayMsg(p_ta, str);
                    next_action = ticks + 1000;
                    stage++;
                }
                return rc;    
                
            case LOOP_WAIT_NEXT_ACTION:
                if(ticks >= next_action)
                {
                    stage = LOOP_WRITE;
                }
                return rc;  
        }
    }    

    return rc;
}
