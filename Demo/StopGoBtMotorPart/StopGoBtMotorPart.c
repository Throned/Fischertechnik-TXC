//=============================================================================
// Demo program "Stop and go via Bluetooth (motor and counter part)".
//
// Can be run under control of the ROBO TX Controller
// firmware in download (local) mode.
// Starts and stops motor, connected to outputs M1, by means
// of the button, connected to the input I8 on other
// ROBO TX Controller. Pulses from the motor are calculated
// by the counter C1. The motor is stopped after the counter
// reaches the value of 1000.
//
// Disclaimer - Exclusion of Liability
//
// This software is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. It can be used and modified by anyone
// free of any license obligations or authoring rights.
//=============================================================================

#include "ROBO_TX_PRG.h"

#define MOTOR_NUMBER    1
#define MOTOR_IDX       (MOTOR_NUMBER - 1)

#define BT_CHANNEL      1

// Bluetooth address of other ROBO TX Controller
static UCHAR8 * bt_address = bt_address_table[0];

static enum
{
    START_LISTEN,
    PAUSE_1,
    WAIT_CONNECT,
    PAUSE_2,
    PAUSE_3,
    RECEIVE,
    PAUSE_4,
    EXIT
} stage;

static unsigned long timer;
static enum bt_commands_e command;
static CHAR8 command_status;
static CHAR8 receive_command_status;


/*-----------------------------------------------------------------------------
 * Function Name       : BtCallback
 *
 * This callback function is called to inform the program about result (status)
 * of execution of any Bluetooth command except BtStartReceive command.
 *-----------------------------------------------------------------------------*/
static void BtCallback
(
    TA * p_ta_array,
    BT_CB * p_data
)
{
    command_status = p_data->status;
}


/*-----------------------------------------------------------------------------
 * Function Name       : BtReceiveCallback
 *
 * This callback function is called to inform the program about result (status)
 * of execution of BtStartReceive command. It is also called when a message
 * arrives via Bluetooth.
 *-----------------------------------------------------------------------------*/
static void BtReceiveCallback
(
    TA * p_ta_array,
    BT_RECV_CB * p_data
)
{
    if (p_data->status == BT_MSG_INDICATION)
    {
        TA * p_ta = &p_ta_array[TA_LOCAL];
        UCHAR8 motor;
        UCHAR8 pwm_chan;
        INT16 duty;
        UCHAR8 msg[3];
        INT16 counter;

        // Format of a received message should be:
        // byte 0  : motor number(1...N_MOTOR)
        // byte 1-2: motor duty(DUTY_MIN...DUTY_MAX)
        motor = p_data->msg[0];
        if (motor >= 1 && motor <= N_MOTOR)
        {
            pwm_chan = (motor - 1) * 2;
            p_ta->hook_table.memcpy(&duty, &p_data->msg[1], sizeof(duty));
            if (duty >= DUTY_MIN && duty <= DUTY_MAX)
            {
                p_ta->output.duty[pwm_chan] = duty;
                p_ta->output.duty[pwm_chan + 1] = 0;
            }

            // Prepare reply BT message
            msg[0] = motor;                                              // counter number
            counter = p_ta->input.counter[motor - 1];
            p_ta->hook_table.memcpy(&msg[1], &counter, sizeof(counter)); // counter value

            // Send BT message
            command_status = -1;
            p_ta->hook_table.BtSend(BT_CHANNEL, sizeof(msg), msg, BtCallback);
        }
    }
    else
    {
        receive_command_status = p_data->status;
    }
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

    // Configure motor output to be used as a motor output
    p_ta->config.motor[MOTOR_IDX] = TRUE;

    // Inform firmware that configuration was changed
    p_ta->state.config_id += 1;

    // Reset counter
    p_ta->input.cnt_resetted[MOTOR_IDX] = FALSE;
    p_ta->output.cnt_reset_cmd_id[MOTOR_IDX]++;

    // Start listen to the controller with bt_address via Bluetooth channel BT_CHANNEL
    stage = START_LISTEN;
    command = CMD_START_LISTEN;
    command_status = -1;
    p_ta->hook_table.BtStartListen(BT_CHANNEL, bt_address, BtCallback);
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

    switch (stage)
    {
        case START_LISTEN:
            if (p_ta->input.cnt_resetted[MOTOR_IDX]) // wait until counter is resetted
            {
                if (command_status >= 0)
                {
                    if (BtDisplayCommandStatus(p_ta, bt_address, BT_CHANNEL, command, command_status))
                    {
                        if (command_status != BT_SUCCESS)
                        {
                            stage = PAUSE_4;
                        }
                        else
                        {
                            stage = PAUSE_1;
                            command_status = -1;
                        }
                        timer = 0;
                    }
                }
            }
            break;

        case PAUSE_1: // to let a user to notice the "Started listening" display output
            if (++timer >= 3000) // wait for 3 seconds
            {
                stage = WAIT_CONNECT;
            }
            else
            {
                break;
            }

        case WAIT_CONNECT:
            if (command_status >= 0)
            {
                if (BtDisplayCommandStatus(p_ta, bt_address, BT_CHANNEL, command, command_status))
                {
                    if (command_status == BT_CON_INDICATION)
                    {
                        stage = PAUSE_2;

                        // Start receive from Bluetooth channel BT_CHANNEL
                        command = CMD_START_RECEIVE;
                        receive_command_status = -1;
                        p_ta->hook_table.BtStartReceive(BT_CHANNEL, BtReceiveCallback);
                    }
                    command_status = -1;
                    timer = 0;
                }
            }
            break;

        case PAUSE_2: // to let a user to notice the "Passive connection establishment
                      // (incoming connection)" display output
            if (++timer >= 3000) // wait for 3 seconds
            {
                stage = PAUSE_3;
                timer = 0;
            }
            else
            {
                break;
            }

        case PAUSE_3: // to let a user to notice the "Started receiving" display output
            if (receive_command_status >= 0)
            {
                if (BtDisplayCommandStatus(p_ta, bt_address, BT_CHANNEL, command, receive_command_status))
                {
                    if (receive_command_status == BT_SUCCESS)
                    {
                        // Successful result of BtStartReceive() - restart the timer to let a user to
                        // notice this display output
                        receive_command_status = -1;
                        timer = 0;
                    }
                    else
                    {
                        // Unsuccessful result of BtStartReceive() - make PAUSE_4 (to let a user to
                        // notice this display output) and exit the program
                        stage = PAUSE_4;
                        timer = 0;
                    }
                }
                break;
            }
            else if (++timer >= 3000) // wait for 3 seconds
            {
                if (!p_ta->hook_table.IsDisplayBeingRefreshed(p_ta)) // wait until display is refreshed
                {
                    // Drop all pop-up messages from display and return to the main frame
                    p_ta->hook_table.DisplayMsg(p_ta, NULL);

                    stage = RECEIVE;
                }
            }
            else
            {
                break;
            }

        case RECEIVE:
            if (command_status >= 0)
            {
                if (command_status != BT_SUCCESS)
                {
                    // Usually if we have come here, then this means the other controller
                    // has disconnected from us

                    // Stop the motor
                    p_ta->output.duty[MOTOR_IDX * 2] = 0;

                    // Reset counter to be prepared for the next time when
                    // other controller connects to us again
                    p_ta->input.cnt_resetted[MOTOR_IDX] = FALSE;
                    p_ta->output.cnt_reset_cmd_id[MOTOR_IDX]++;

                    command_status = -1;

                    /*
                    // This block of code should be used if we want that the program stops when other
                    // ROBO TX Controller disconnects from us
                    if (BtDisplayCommandStatus(p_ta, bt_address, BT_CHANNEL, command, command_status))
                    {
                        stage = PAUSE_4;
                        timer = 0;
                    }
                    */
                }
                command = CMD_NO_CMD;
            }
            break;

        case PAUSE_4: // to let a user to notice the last display output
            if (++timer >= 3000) // wait for 3 seconds
            {
                stage = EXIT;
            }
            else
            {
                break;
            }

        case EXIT:
            if (!p_ta->hook_table.IsDisplayBeingRefreshed(p_ta)) // wait until display is refreshed
            {
                rc = 0; // stop program
            }
            break;

        default:
            break;
    }
    return rc;
}
