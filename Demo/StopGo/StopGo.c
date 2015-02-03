//=============================================================================
// Demo program "Stop and go".
//
// Can be run under control of the ROBO TX Controller
// firmware in download (local) mode.
// Starts and stops motor connected to outputs M1 by means
// of the button connected to the input I8. Pulses from the
// motor are calculated by the counter C1. The motor is
// stopped after the counter reaches the value of 1000.
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
//#define TA_IDX          TA_EXT_1
#define MOTOR_NUMBER    1
#define MOTOR_IDX       (MOTOR_NUMBER - 1)
#define BUTTON_NUMBER   8
#define BUTTON_IDX      (BUTTON_NUMBER - 1)

static INT16 prev_button_state;
static char str[128];


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

    // Configure button input to "Digital 5 kOhm" mode
    p_ta->config.uni[BUTTON_IDX].mode = MODE_R;
    p_ta->config.uni[BUTTON_IDX].digital = TRUE;

    // Inform firmware that configuration was changed
    p_ta->state.config_id += 1;

    // Reset counter
    p_ta->input.cnt_resetted[MOTOR_IDX] = FALSE;
    p_ta->output.cnt_reset_cmd_id[MOTOR_IDX]++;

    // Emulate button press in order to run at program start-up on
    // program branch where the button release is noticed
    prev_button_state = 1;
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
    INT16 cur_button_state;

    if (p_ta->input.cnt_resetted[MOTOR_IDX]) // wait until counter is resetted
    {
        if (!p_ta->hook_table.IsDisplayBeingRefreshed(p_ta)) // wait until display is refreshed
        {
            // Read current status of the button input
            cur_button_state = p_ta->input.uni[BUTTON_IDX];
    
            if (prev_button_state != cur_button_state)
            {
                if (prev_button_state && !cur_button_state) // if button was released
                {
                    p_ta->hook_table.sprintf(str, "Press button I%d to run motor M%d",
                        BUTTON_NUMBER, MOTOR_NUMBER);
                    p_ta->hook_table.DisplayMsg(p_ta, str);
                }
                else // if button was pressed
                {
                    // Drop all pop-up messages from display and return to the main frame
                    p_ta->hook_table.DisplayMsg(p_ta, NULL);
                }
                prev_button_state = cur_button_state;
            }

            // Start motor if button is pressed, otherwise stop it
            p_ta->output.duty[2 * MOTOR_IDX] = (cur_button_state) ? DUTY_MAX : 0;
        }
        if (!p_ta->hook_table.IsDisplayBeingRefreshed(p_ta)) // wait until display is refreshed
        {
            // Program should be executed until counter reaches 1000
            if (p_ta->input.counter[MOTOR_IDX] >= 1000)
            {
                p_ta->hook_table.sprintf(str, "Motor M%d reached position 1000", MOTOR_NUMBER);
                p_ta->hook_table.DisplayMsg(p_ta, str);
                rc = 0; // stop program
            }
        }
    }
    return rc;
}
