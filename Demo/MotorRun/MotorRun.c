//=============================================================================
// Demo program "Motor run".
//
// Can be run under control of the ROBO TX Controller
// firmware in download (local) mode.
// Controls motor connected to outputs M1 with different
// speeds and in different rotation directions.
//
// Disclaimer - Exclusion of Liability
//
// This software is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. It can be used and modified by anyone
// free of any license obligations or authoring rights.
//=============================================================================

#include "ROBO_TX_PRG.h"

#define TA_IDX          TA_LOCAL
#define MOTOR_NUMBER    1
#define MOTOR_IDX       (MOTOR_NUMBER - 1)

static unsigned long timer;
static enum {DIRECTION_1, PAUSE, DIRECTION_2, OFF} stage;
static int duty;


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
    TA * p_ta = &p_ta_array[TA_IDX];

    // Configure motor output to be used as a motor output
    p_ta->config.motor[MOTOR_IDX] = TRUE;

    // Inform firmware that configuration was changed
    p_ta->state.config_id += 1;

    // Switch motor off
    p_ta->output.duty[2 * MOTOR_IDX + 0] = 0;
    p_ta->output.duty[2 * MOTOR_IDX + 1] = 0;

    timer = 0;
    stage = DIRECTION_1; // motor should start to rotate in the first direction
    duty = 0;
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
    TA * p_ta = &p_ta_array[TA_IDX];
    
    while (1)
    {
        switch (stage)
        {
            case DIRECTION_1:
                if (timer == 0)
                {
                    // Switch motor on in the first direction with the speed from the duty value
                    duty += DUTY_MAX / 4;
                    p_ta->output.duty[2 * MOTOR_IDX + 0] = duty;
                    p_ta->output.duty[2 * MOTOR_IDX + 1] = 0;
                }
                if (++timer >= 2000) // motor should rotate with such a speed for 2 seconds
                {
                    timer = 0;
                    if (duty >= DUTY_MAX)
                    {
                        stage = PAUSE;
                    }
                    else
                    {
                        continue;
                    }
                }
                else
                {
                    return rc;
                }

            case PAUSE:
                if (timer == 0)
                {
                    // Switch motor off
                    p_ta->output.duty[2 * MOTOR_IDX + 0] = 0;
                    p_ta->output.duty[2 * MOTOR_IDX + 1] = 0;
                }
                if (++timer >= 2000) // motor should be stopped for 2 seconds
                {
                    stage = DIRECTION_2;
                    timer = 0;
                }
                else
                {
                    return rc;
                }

            case DIRECTION_2:
                if (timer == 0)
                {
                    // Switch motor on in the second direction with the speed from the duty value
                    p_ta->output.duty[2 * MOTOR_IDX + 0] = 0;
                    p_ta->output.duty[2 * MOTOR_IDX + 1] = duty;
                }
                if (++timer >= 2000) // motor should rotate with such a speed for 2 seconds
                {
                    duty -= DUTY_MAX / 4;
                    if (duty <= 0)
                    {
                        stage = OFF;
                    }
                    else
                    {
                        timer = 0;
                        continue;
                    }
                }
                else
                {
                    return rc;
                }

            case OFF:
                // Switch motor off
                p_ta->output.duty[2 * MOTOR_IDX + 0] = 0;
                p_ta->output.duty[2 * MOTOR_IDX + 1] = 0;

                rc = 0; // stop program
                return rc;

            default:
                return rc;
        }
    }
    return rc;
}
