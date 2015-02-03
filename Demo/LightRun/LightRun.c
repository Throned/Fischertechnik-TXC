//=============================================================================
// Demo program "Light run".
//
// Can be run under control of the ROBO TX Controller
// firmware in download (local) mode.
// Switches one after another six lamps connected to the outputs O1...O6.
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

#define BEG_LAMP_IDX    0
#define END_LAMP_IDX    5

static unsigned long cur_time;
static unsigned long prev_time;
static enum {ON_1, PAUSE_1, ON_2, PAUSE_2} stage;
static int lamp_idx;
static int n_loops;
static int wait;


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
    prev_time = 0;
    stage = ON_1;
    lamp_idx = BEG_LAMP_IDX;
    n_loops = 3;
    wait = 200;
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
    
    // Get the current value of the system time
    cur_time = p_ta->hook_table.GetSystemTime(TIMER_UNIT_MILLISECONDS);

    while (1)
    {
        switch (stage)
        {
            case ON_1:
                if (prev_time == 0)
                {
                    // Switch the current lamp on
                    p_ta->output.duty[lamp_idx] = LIGHT_ON;

                    // Store the current value of the system time
                    prev_time = cur_time;
                }
                else
                {
                    // The lamp should be on for the number of milliseconds from the variable "wait"
                    if (cur_time - prev_time >= wait)
                    {
                        // Switch the current lamp off
                        p_ta->output.duty[lamp_idx] = LIGHT_OFF;

                        if (lamp_idx + 1 <= END_LAMP_IDX)
                        {
                            prev_time = 0;
                            lamp_idx++; // switch to the next lamp
                            continue;
                        }
                        else
                        {
                            // Store the current value of the system time
                            prev_time = cur_time;

                            stage = PAUSE_1;
                        }
                    }
                }
                return rc;

            case PAUSE_1:
                // All lamps should be off for the number of milliseconds from the variable "wait"
                if (cur_time - prev_time >= wait)
                {
                    prev_time = 0;
                    stage = ON_2;
                }
                else
                {
                    return rc;
                }

            case ON_2:
                if (prev_time == 0)
                {
                    // Switch the current lamp on
                    p_ta->output.duty[lamp_idx] = LIGHT_ON;

                    // Store the current value of the system time
                    prev_time = cur_time;
                }
                else
                {
                    // The lamp should be on for the number of milliseconds from the variable "wait"
                    if (cur_time - prev_time >= wait)
                    {
                        // Switch the current lamp off
                        p_ta->output.duty[lamp_idx] = LIGHT_OFF;

                        if (lamp_idx - 1 >= BEG_LAMP_IDX)
                        {
                            prev_time = 0;
                            lamp_idx--; // switch to the previous lamp
                            continue;
                        }
                        else
                        {
                            // Store the current value of the system time
                            prev_time = cur_time;

                            stage = PAUSE_2;
                        }
                    }
                }
                return rc;

            case PAUSE_2:
                // All lamps should be off for 1 second
                if (cur_time - prev_time >= 1000)
                {
                    if (--n_loops <= 0)
                    {
                        rc = 0; // stop program
                    }
                    else
                    {
                        wait /= 2;
                        prev_time = 0;
                        stage = ON_1;
                        continue;
                    }
                }
                return rc;

            default:
                return rc;
        }
    }
    return rc;
}
