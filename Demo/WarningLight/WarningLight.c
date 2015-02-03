//=============================================================================
// Demo program "Warning light".
//
// Can be run under control of the ROBO TX Controller
// firmware in download (local) mode.
// The lamp, connected to the output O8, blinks depending on
// distance to the ultrasonic sensor, connected to the input I1.
// The shorter the distance, the faster blinks the lamp.
//
// Disclaimer - Exclusion of Liability
//
// This software is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. It can be used and modified by anyone
// free of any license obligations or authoring rights.
//=============================================================================

#include "ROBO_TX_PRG.h"

#define LIGHT_ON    DUTY_MAX
#define LIGHT_OFF   0

#define LAMP_IDX    7
#define SENSOR_IDX  0

static bool is_light_on;
static INT16 cmp_val;


/*-----------------------------------------------------------------------------
 * Function Name       : SetFlashLight
 *
 * Switches the lamp on/off depending on the difference between the current
 * timer value and the value for compare, considering the flash duration time.
 *-----------------------------------------------------------------------------*/
static INT16 SetFlashLight
(
    TA * p_ta,
    INT16 cmp_val,
    INT16 tm_val,
    INT16 tm_diff
)
{
    if (tm_diff == 0)
    {
        // Switch the lamp off
        p_ta->output.duty[LAMP_IDX] = LIGHT_OFF;

        is_light_on = FALSE;
        return tm_val;
    }

    if ((cmp_val + tm_diff) > tm_val)
    {
        return cmp_val;
    }

    // Set new state
    is_light_on = !is_light_on;

    // Switch the lamp on/off
    p_ta->output.duty[LAMP_IDX] = (is_light_on) ? LIGHT_ON : LIGHT_OFF;
    
    return tm_val;
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
    TA * p_ta = &p_ta_array[TA_LOCAL];

    // Configure input I1 to "Ultrasonic sensor" mode
    p_ta->config.uni[SENSOR_IDX].mode = MODE_ULTRASONIC;

    // Inform firmware that configuration was changed
    p_ta->state.config_id += 1;

    is_light_on = FALSE;

    // Get timer value from timer struct in transfer area
    cmp_val = p_ta->timer.Timer10ms;
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
    INT16 tm_val;
    INT16 tm_flash;
    INT16 distance;

    // Read I1 input value from transfer area
    distance = p_ta->input.uni[SENSOR_IDX];

    // Get timer value from timer struct in transfer area
    tm_val = p_ta->timer.Timer10ms;

    // Set flash time dependent on distance
    tm_flash = (distance > 20) ? 0  :           // no interval, light is always off
               (distance > 15) ? 75 :           // interval 750 ms
               (distance > 10) ? 50 :           // interval 500 ms
               (distance > 5)  ? 25 : 10;       // interval 250 ms or 100 ms

    // Setting flash light
    cmp_val = SetFlashLight(p_ta, cmp_val, tm_val, tm_flash);

    return rc;
}
