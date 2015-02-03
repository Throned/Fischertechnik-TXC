//=============================================================================
// Helper functions for the programs which use Bluetooth.
//
// Disclaimer - Exclusion of Liability
//
// This software is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. It can be used and modified by anyone
// free of any license obligations or authoring rights.
//=============================================================================

#include "ROBO_TX_PRG.H"

static char str[128];


/*-----------------------------------------------------------------------------
 * Function Name       : BtCommandStatusToString
 *
 * Converts the status of the executed Bluetooth command to a string.
 *-----------------------------------------------------------------------------*/
static char * BtCommandStatusToString
(
    enum bt_commands_e command,
    CHAR8 command_status
)
{
    char * str = "";

    switch (command_status)
    {
        case BT_SUCCESS: // Successful end of command
            switch (command)
            {
                case CMD_CONNECT:
                    str = "Connected";
                    break;
                case CMD_DISCONNECT:
                    str = "Disconnected";
                    break;
                case CMD_START_LISTEN:
                    str = "Started listening";
                    break;
                case CMD_STOP_LISTEN:
                    str = "Stopped listening";
                    break;
                case CMD_START_RECEIVE:
                    str = "Started receiving";
                    break;
                case CMD_STOP_RECEIVE:
                    str = "Stopped receiving";
                    break;
                case CMD_SEND:
                    str = "Message sent";
                    break;
                default:
                    break;
            }
            break;
        case BT_CON_EXIST:
            str = "Already connected";
            break;
        case BT_CON_SETUP:
            str = "Establishing of connection is ongoing";
            break;
        case BT_SWITCHED_OFF:
            str = "Cannot connect/listen, Bluetooth is set to off";
            break;
        case BT_ALL_CHAN_BUSY:
            str = "Cannot connect, no more free Bluetooth channels";
            break;
        case BT_NOT_ROBOTX:
            str = "Cannot connect/listen, device is not a ROBO TX Controller";
            break;
        case BT_CON_TIMEOUT:
            str = "Cannot connect, timeout, no device with such a BT address";
            break;
        case BT_CON_INVALID:
            str = "Connection does not exist";
            break;
        case BT_CON_RELEASE:
            str = "Disconnecting is ongoing";
            break;
        case BT_LISTEN_ACTIVE:
            str = "Listen is already active";
            break;
        case BT_RECEIVE_ACTIVE:
            str = "Receive is already active";
            break;
        case BT_CON_INDICATION:
            str = "Passive connection establishment (incoming connection)";
            break;
        case BT_DISCON_INDICATION:
            str = "Passive disconnection (initiated by remote end)";
            break;
        case BT_MSG_INDICATION:
            str = "Received data (incoming message)";
            break;
        case BT_CHANNEL_BUSY:
            str = "BT channel is busy";
            break;
        case BT_BTADDR_BUSY:
            str = "BT address is already used by another channel";
            break;
        case BT_NO_LISTEN_ACTIVE:
            str = "Cannot connect, no active listen on remote end";
            break;
        default:
            break;
    }
    return str;
}


/*-----------------------------------------------------------------------------
 * Function Name       : BtDisplayCommandStatus
 *
 * Converts the status of the executed Bluetooth command to a string
 * and displays this string on the display of a ROBO TX Controller.
 *-----------------------------------------------------------------------------*/
BOOL32 BtDisplayCommandStatus
(
    TA * p_ta,
    UCHAR8 * bt_address,
    UINT32 channel,
    enum bt_commands_e command,
    CHAR8 command_status
)
{
    BOOL32 rc = FALSE;
    char bt_addr_str[BT_ADDR_STR_LEN + 1];

    if (!p_ta->hook_table.IsDisplayBeingRefreshed(p_ta)) // wait until display is refreshed
    {
        p_ta->hook_table.sprintf(str, "Ch %u, %s\n%s", channel,
            p_ta->hook_table.BtAddrToStr(bt_address, bt_addr_str),
            BtCommandStatusToString(command, command_status));
        p_ta->hook_table.DisplayMsg(p_ta, str);
        rc = TRUE;
    }
    return rc;
}
