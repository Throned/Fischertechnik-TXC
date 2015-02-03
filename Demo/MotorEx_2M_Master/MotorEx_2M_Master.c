//=============================================================================
// Demo program "Extended motor control mode with two motors on Master".
//
// Can be run under control of the ROBO TX Controller
// firmware in download (local) mode.
// Continuously starts two synchronized motors with changing rotation directions.
// Motors are connected to outputs M1 and M2. Pulses from the motors are
// calculated by the counters C1 and C2. The motors are stopped and rotation
// direction is changed after both counters reach the value of 200.
//
// Disclaimer - Exclusion of Liability
//
// This software is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. It can be used and modified by anyone
// free of any license obligations or authoring rights.
//=============================================================================

#include "ROBO_TX_PRG.h"

#define TA_IDX                  TA_LOCAL
#define MASTER_MOTOR_NUMBER     1
#define MASTER_MOTOR_IDX        (MASTER_MOTOR_NUMBER - 1)
#define SLAVE_MOTOR_NUMBER      2
#define SLAVE_MOTOR_IDX         (SLAVE_MOTOR_NUMBER - 1)

static BOOL32 rotation_direction;
static BOOL32 prev_rotation_direction;


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

    // Configure master motor output to be used as a motor output
    p_ta->config.motor[MASTER_MOTOR_IDX] = TRUE;

    // Configure slave motor output to be used as a motor output
    p_ta->config.motor[SLAVE_MOTOR_IDX] = TRUE;

    // Inform firmware that configuration was changed
    p_ta->state.config_id += 1;

    rotation_direction = FALSE;
    prev_rotation_direction = !rotation_direction;
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

    if (rotation_direction != prev_rotation_direction)
    {
        // Set master motor duty depending on rotation_direction
        p_ta->output.duty[2 * MASTER_MOTOR_IDX + 0] = (rotation_direction) ? DUTY_MAX : 0;
        p_ta->output.duty[2 * MASTER_MOTOR_IDX + 1] = (rotation_direction) ? 0 : DUTY_MAX;

        // Set slave motor duty depending on rotation_direction
        p_ta->output.duty[2 * SLAVE_MOTOR_IDX + 0] = (rotation_direction) ? DUTY_MAX : 0;
        p_ta->output.duty[2 * SLAVE_MOTOR_IDX + 1] = (rotation_direction) ? 0 : DUTY_MAX;

        // Master motor should run until counter reaches 200
        p_ta->output.distance[MASTER_MOTOR_IDX] = 200;

        // Slave motor should run until counter reaches 200
        p_ta->output.distance[SLAVE_MOTOR_IDX] = 200;

        // Link slave motor to master motor
        p_ta->output.master[SLAVE_MOTOR_IDX] = MASTER_MOTOR_IDX + 1;

        p_ta->input.motor_pos_reached[MASTER_MOTOR_IDX] = FALSE;
        p_ta->input.motor_pos_reached[SLAVE_MOTOR_IDX] = FALSE;

        // Start extended motor control command for master motor
        p_ta->output.motor_ex_cmd_id[MASTER_MOTOR_IDX]++;

        // Start extended motor control command for slave motor
        p_ta->output.motor_ex_cmd_id[SLAVE_MOTOR_IDX]++;

        prev_rotation_direction = rotation_direction;
    }

    if (p_ta->input.motor_pos_reached[MASTER_MOTOR_IDX] && p_ta->input.motor_pos_reached[SLAVE_MOTOR_IDX])
    {
        rotation_direction = !rotation_direction;
    }

    return rc;
}
