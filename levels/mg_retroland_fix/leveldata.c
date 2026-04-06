#include <ultra64.h>
#include "sm64.h"
#include "surface_terrains.h"
#include "moving_texture_macros.h"
#include "level_misc_macros.h"
#include "macro_preset_names.h"
#include "special_preset_names.h"
#include "textures.h"
#include "dialog_ids.h"

#include "make_const_nonconst.h"

#include "levels/mg_retroland_fix/texture.inc.c"
#include "levels/mg_retroland_fix/areas/1/1/model.inc.c"
#include "levels/mg_retroland_fix/areas/1/collision.inc.c"
#include "levels/mg_retroland_fix/areas/1/macro.inc.c"
#define LEVEL_LIGHTING_NUM LEVEL_MG_RETROLAND_FIX
#include "levels/mg_retroland_fix/level_lighting.inc.c"
#undef LEVEL_LIGHTING_NUM
