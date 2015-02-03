//=============================================================================
// Demo program "Stop and go via Bluetooth (button part)".
//
// Can be run under control of the ROBO TX Controller
// firmware in download (local) mode.
// Starts and stops motor, connected to outputs M1 on other
// ROBO TX Controller, by means of the button connected to
// the input I8. Pulses from the motor are calculated by the
// counter C1 on other ROBO TX Controller. The motor is
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

#define MOTOR_NUMBER    1
#define BUTTON_NUMBER   8
#define BUTTON_IDX      (BUTTON_NUMBER - 1)

#define BT_CHANNEL      1

// Bluetooth address of other ROBO TX Controller
static UCHAR8 * bt_address = bt_address_table[1];

static enum
{
    CONNECT,
    PAUSE_1,
    PAUSE_2,
    START_RECEIVE,
    SEND_REQUEST,
    WAIT_REPLY,
    PAUSE_3,
    EXIT
} stage;

static INT16 prev_button_state;
static INT16 remote_counter_value;
static unsigned long timer;
static enum bt_commands_e command;
static CHAR8 command_status;
static CHAR8 receive_command_status;
static bool was_receive;
static char str[128];


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
    was_receive = TRUE;

    if (p_data->status == BT_MSG_INDICATION)
    {
        TA * p_ta = &p_ta_array[TA_LOCAL];
        UCHAR8 counter;

        // Format of a received message should be:
        // byte 0  : counter number(1...N_CNT)
        // byte 1-2: counter value(0...0xFFFF)
        counter = p_data->msg[0];
        if (counter >= 1 && counter <= N_CNT)
        {
            p_ta->hook_table.memcpy(&remote_counter_value, &p_data->msg[1],
                sizeof(remote_counter_value));
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

    // Configure input I8 to "Digital 5 kOhm" mode
    p_ta->config.uni[BUTTON_IDX].mode = MODE_R;
    p_ta->config.uni[BUTTON_IDX].digital = TRUE;

    // Inform firmware that configuration was changed
    p_ta->state.config_id += 1;

    // Emulate button press in order to run at program start-up on
    // program branch where the button release is noticed
    prev_button_state = 1;

    remote_counter_value = 0;

    // Connect to the controller with bt_address via Bluetooth channel BT_CHANNEL
    stage = CONNECT;
    command = CMD_CONNECT;
    command_status = -1;
    p_ta->hook_table.BtConnect(BT_CHANNEL, bt_address, BtCallback);
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
        case CONNECT:
            if (command_status >= 0)
            {
                if (BtDisplayCommandStatus(p_ta, bt_address, BT_CHANNEL, command, command_status))
                {
                    if (command_status != BT_SUCCESS)
                    {
                        stage = PAUSE_3;
                    }
                    else
                    {
                        stage = PAUSE_1;

                        // Start receive from Bluetooth channel BT_CHANNEL
                        command = CMD_START_RECEIVE;
                        receive_command_status = -1;
                        p_ta->hook_table.BtStartReceive(BT_CHANNEL, BtReceiveCallback);
                    }
                    timer = 0;
                }
            }
            break;

        case PAUSE_1: // to let a user to notice the "Connected" display output
            if (++timer >= 3000) // wait for 3 seconds
            {
                stage = PAUSE_2;
                timer = 0;
            }
            else
            {
                break;
            }

        case PAUSE_2: // to let a user to notice the "Started receiving" display output
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
                        // Unsuccessful result of BtStartReceive() - make PAUSE_3 (to let a user to
                        // notice this display output) and exit the program
                        stage = PAUSE_3;
                        timer = 0;
                    }
                }
                break;
            }
            else if (++timer >= 3000) // wait for 3 seconds
            {
                stage = START_RECEIVE;
            }
            else
            {
                break;
            }

        case START_RECEIVE:
        case WAIT_REPLY:
            if (was_receive || command_status >= 0)
            {
                if (command_status >= 0 && command_status != BT_SUCCESS)
                {
                    // Usually if we have come here, then this means the other controller
                    // has disconnected from us
                    if (BtDisplayCommandStatus(p_ta, bt_address, BT_CHANNEL, command, command_status))
                    {
                        stage = PAUSE_3;
                        timer = 0;
                    }
                }
                else
                {
                    UCHAR8 msg[3];
                    INT16 duty;

                    was_receive = FALSE;

                    // Read current status of the button input
                    INT16 cur_button_state = p_ta->input.uni[BUTTON_IDX];

                    // Start motor if button on input I8 is pressed, otherwise stop it
                    duty = (cur_button_state) ? DUTY_MAX : 0;

                    if (!p_ta->hook_table.IsDisplayBeingRefreshed(p_ta)) // wait until display is refreshed
                    {
                        // Program should be executed until counter reaches 1000
                        if (remote_counter_value >= 1000)
                        {
                            p_ta->hook_table.sprintf(str, "Motor M%d reached position 1000", MOTOR_NUMBER);
                            p_ta->hook_table.DisplayMsg(p_ta, str);
                            duty = 0;
                            stage = PAUSE_3;
                            timer = 0;
                        }
                        else if (prev_button_state != cur_button_state)
                        {
                            if (prev_button_state && !cur_button_state) // if button was released
                            {
                                p_ta->hook_table.sprintf(str,
                                    "Press button I%d to run motor M%d on other TXC",
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
                    }

                    // Prepare BT message
                    msg[0] = MOTOR_NUMBER;                                 // motor number
                    p_ta->hook_table.memcpy(&msg[1], &duty, sizeof(duty)); // motor duty

                    // Send BT message
                    stage = (stage != PAUSE_3) ? SEND_REQUEST : stage;
                    command = CMD_SEND;
                    command_status = -1;
                    p_ta->hook_table.BtSend(BT_CHANNEL, sizeof(msg), msg, BtCallback);
                }
            }
            break;

        case SEND_REQUEST:
            if (command_status >= 0)
            {
                if (command_status == BT_SUCCESS)
                {
                    // Wait for the reply to the sent BT message
                    stage = WAIT_REPLY;
                    command = CMD_NO_CMD;
                    command_status = -1;
                }
                else
                {
                    if (BtDisplayCommandStatus(p_ta, bt_address, BT_CHANNEL, command, command_status))
                    {
                        stage = PAUSE_3;
                        timer = 0;
                    }
                }
            }
            break;

        case PAUSE_3: // to let a user to notice the last display output
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
