//=============================================================================
// Demo program "I2cTpa81.c".
//
// Can be run under control of the ROBO TX Controller
// firmware in download (local) mode.
// This example shows how to sense the temperature sensor array (thermopile) of
// the I2C thermopile sensor TPA81. After some initialisation the program enters
// a loop updating the measured values for the sensors every 1000ms.
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
    LOOP_READ_AMB,
    LOOP_READ_AMB_WAIT,
    LOOP_READ_P1,
    LOOP_READ_P1_WAIT,
    LOOP_READ_P2,
    LOOP_READ_P2_WAIT,
    LOOP_READ_P3,
    LOOP_READ_P3_WAIT,
    LOOP_READ_P4,
    LOOP_READ_P4_WAIT,
    LOOP_READ_P5,
    LOOP_READ_P5_WAIT,
    LOOP_READ_P6,
    LOOP_READ_P6_WAIT,
    LOOP_READ_P7,
    LOOP_READ_P7_WAIT,
    LOOP_READ_P8,
    LOOP_READ_P8_WAIT,
    LOOP_CLEAR_PREV_SCREEN,
    LOOP_DISP_RESULT,
    LOOP_WAIT_NEXT_ACTION
} stage;

unsigned int ticks;
unsigned int next_action=0;
UINT16 status=0;
UINT16 value=0;
unsigned char amb=0;
unsigned char p[8]={0,0,0,0,0,0,0,0};
unsigned char pidx=255;

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
    
    if(pidx == 128)
    {
        amb = (unsigned char)value;
    }
    else if(pidx != 255)
    {
        p[pidx] = (unsigned char)value;
    }    

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
    
    ticks++;
    
    while(1)
    {
        switch(stage)
        {
            case INIT_1:
                p_ta->hook_table.I2cRead (0x68, 0x00, 0xA5, I2cCallback);
                stage++;
                return rc;

            case WAIT_1:  
                // waiting for callback
                return rc;
                
            case LOOP_READ_AMB:    
                p_ta->hook_table.I2cRead (0x68, 0x01, 0xA5, I2cCallback);
                pidx = 128;
                stage++;    
                return rc;

            case LOOP_READ_P1:    
                p_ta->hook_table.I2cRead (0x68, 0x02, 0xA5, I2cCallback);
                pidx = 0;
                stage++;    
                return rc;

            case LOOP_READ_P2:    
                p_ta->hook_table.I2cRead (0x68, 0x03, 0xA5, I2cCallback);
                pidx = 1;
                stage++;    
                return rc;

            case LOOP_READ_P3:    
                p_ta->hook_table.I2cRead (0x68, 0x04, 0xA5, I2cCallback);
                pidx = 2;
                stage++;    
                return rc;

            case LOOP_READ_P4:    
                p_ta->hook_table.I2cRead (0x68, 0x05, 0xA5, I2cCallback);
                pidx = 3;
                stage++;    
                return rc;

            case LOOP_READ_P5:    
                p_ta->hook_table.I2cRead (0x68, 0x06, 0xA5, I2cCallback);
                pidx = 4;
                stage++;    
                return rc;

            case LOOP_READ_P6:    
                p_ta->hook_table.I2cRead (0x68, 0x07, 0xA5, I2cCallback);
                pidx = 5;
                stage++;    
                return rc;

            case LOOP_READ_P7:    
                p_ta->hook_table.I2cRead (0x68, 0x08, 0xA5, I2cCallback);
                pidx = 6;
                stage++;    
                return rc;

            case LOOP_READ_P8:    
                p_ta->hook_table.I2cRead (0x68, 0x09, 0xA5, I2cCallback);
                pidx = 7;
                stage++;    
                return rc;

            case LOOP_READ_AMB_WAIT:    
            case LOOP_READ_P1_WAIT:    
            case LOOP_READ_P2_WAIT:    
            case LOOP_READ_P3_WAIT:    
            case LOOP_READ_P4_WAIT:    
            case LOOP_READ_P5_WAIT:    
            case LOOP_READ_P6_WAIT:    
            case LOOP_READ_P7_WAIT:    
            case LOOP_READ_P8_WAIT:    
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
                    p_ta->hook_table.sprintf(str, "Amb.Temp.: %d C\n%d, %d, %d, %d\n%d, %d, %d, %d", 
                                                  amb, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
                    p_ta->hook_table.DisplayMsg(p_ta, str);
                    next_action = ticks + 1000;
                    stage++;
                }
                return rc;    
                
            case LOOP_WAIT_NEXT_ACTION:
                if(ticks >= next_action)
                {
                    stage = LOOP_READ_AMB;
                }
                return rc;  
        }
    }    

    return rc;
}
