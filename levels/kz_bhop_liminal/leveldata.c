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

#include "levels/kz_bhop_liminal/texture.inc.c"
#include "levels/kz_bhop_liminal/areas/1/1/model.inc.c"
#include "levels/kz_bhop_liminal/areas/1/collision.inc.c"
#include "levels/kz_bhop_liminal/areas/1/macro.inc.c"
#define LEVEL_LIGHTING_NUM LEVEL_KZ_BHOP_LIMINAL
#include "levels/kz_bhop_liminal/level_lighting.inc.c"
#undef LEVEL_LIGHTING_NUM
#include "levels/kz_bhop_liminal/entities.inc.c"
