//=============================================================================
// Program dispatcher.
// This function is an entry point to any ROBO TX Controller
// program. It is called periodically by the ROBO TX Controller
// firmware in download (local) mode.
//
// Disclaimer - Exclusion of Liability
//
// This software is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. It can be used and modified by anyone
// free of any license obligations or authoring rights.
//=============================================================================

#include "ROBO_TX_PRG.H"


static int PrgDisp
(
    TA * p_ta_array,    // pointer to the array of transfer areas
    int ta_count        // number of transfer areas in array (equal to TA_COUNT)
);


const struct prg_code_intro_s prg_code_intro __attribute__ ((section (".intro"))) =
{
    /* magic            */ PRG_MAGIC,
    /* ta_version       */ {TA_VERSION},
    /* entry            */ (unsigned long) PrgDisp
};


// This function is called periodically by the ROBO TX Controller firmware with the period
// of CALL_CYCLE_MS
static int PrgDisp
(
    TA * p_ta_array,    // pointer to the array of transfer areas
    int ta_count        // number of transfer areas in array (equal to TA_COUNT)
)
{
    TA * p_ta = &p_ta_array[TA_LOCAL];
    
    if (!p_ta->state.pgm_initialized)
    {
        PrgInit(p_ta_array, ta_count);

        p_ta->state.pgm_initialized = TRUE;
    }

    return PrgTic(p_ta_array, ta_count);
}
