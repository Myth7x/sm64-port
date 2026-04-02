#include <ultra64.h>
#include "sm64.h"
#include "segment_symbols.h"
#include "level_commands.h"

#include "levels/scripts.h"

#include "make_const_nonconst.h"

const LevelScript level_script_entry[] = {
    INIT_LEVEL(),
    SLEEP(/*frames*/ 2),
    BLACKOUT(/*active*/ FALSE),
    JUMP(/*target*/ level_main_scripts_entry), // skip intro+menu, go straight to game
};
