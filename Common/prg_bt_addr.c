//=============================================================================
// Table of Bluetooth addresses of all ROBO TX Controllers which are used
// for Bluetooth communicating programs. It should be adjusted to the concrete
// set of ROBO TX Controllers, planned to be used for Bluetooth communication.
//
// Disclaimer - Exclusion of Liability
//
// This software is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. It can be used and modified by anyone
// free of any license obligations or authoring rights.
//=============================================================================

#include "ROBO_TX_PRG.H"

UCHAR8 bt_address_table[BT_CNT_MAX][BT_ADDR_LEN] =
{
    {0x00, 0x13, 0x7B, 0x53, 0x10, 0xE7}, // Bluetooth-Adresse von ROBO TX 1 (MASTER)
    {0x00, 0x13, 0x7B, 0x52, 0xB2, 0x11}  // Bluetooth-Adresse von ROBO TX 2 (SLAVE)
};
