#ifdef _WIN32

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#undef near
#undef far

#include <PR/ultratypes.h>

#include "../engine/surface_load.h"
#include "../game/object_list_processor.h"
#include "../../include/surface_terrains.h"
#include "../game/macro_special_objects.h"

typedef struct {
    s16 *vertex_data;
    s16 i0, i1, i2;
    s16 surface_type;
    s8  room;
    s16 force;
    s8  flags;
    s8  has_force;
} TriDesc;

typedef struct {
    struct Surface *scratch;
    const TriDesc  *descs;
    u8             *valid;
    s32             start;
    s32             count;
} FillBatch;

#define BATCH_SIZE 512

static void CALLBACK fill_batch_cb(PTP_CALLBACK_INSTANCE inst, void *ctx, PTP_WORK work) {
    FillBatch *b = (FillBatch *)ctx;
    s32 end = b->start + b->count;
    LARGE_INTEGER t0, t1, freq;
    (void)inst; (void)work;
    QueryPerformanceCounter(&t0);
    for (s32 i = b->start; i < end; i++) {
        const TriDesc *d = &b->descs[i];
        struct Surface tmp;
        memset(&tmp, 0, sizeof(tmp));
        b->valid[i] = (u8)surface_fill_from_data(&tmp, d->vertex_data, d->i0, d->i1, d->i2);
        if (b->valid[i]) {
            b->scratch[i] = tmp;
        }
    }
    QueryPerformanceCounter(&t1);
    QueryPerformanceFrequency(&freq);
    fprintf(stderr, "[surface-worker] batch %d tris %d..%d done in %.2f ms\n",
            b->start / BATCH_SIZE, b->start, end - 1,
            (double)(t1.QuadPart - t0.QuadPart) * 1000.0 / (double)freq.QuadPart);
}

static s32 no_cam_collision_flag(s16 type) {
    switch (type) {
        case SURFACE_NO_CAM_COLLISION:
        case SURFACE_NO_CAM_COLLISION_77:
        case SURFACE_NO_CAM_COL_VERY_SLIPPERY:
        case SURFACE_SWITCH:
            return SURFACE_FLAG_NO_CAM_COLLISION;
        default:
            return 0;
    }
}

void load_area_terrain_parallel(s16 index, s16 *data, s8 *surfaceRooms, s16 *macroObjects) {
    s16 *vertex_data = NULL;
    s32 total_tris = 0;
    LARGE_INTEGER t_start, t_pass1, t_alloc, t_dispatch, t_wait, t_insert, t_end, freq;

    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&t_start);
    fprintf(stderr, "[parallel-terrain] area %d: starting load\n", index);

    surface_load_begin();

    {
        s16 *p = data;
        while (1) {
            s16 type = *p++;
            if (TERRAIN_LOAD_IS_SURFACE_TYPE_LOW(type) || TERRAIN_LOAD_IS_SURFACE_TYPE_HIGH(type)) {
                s32 n = *p++;
                total_tris += n;
                p += (3 + surface_has_force(type)) * n;
            } else if (type == TERRAIN_LOAD_VERTICES) {
                s32 nv = *p++;
                p += 3 * nv;
            } else if (type == TERRAIN_LOAD_OBJECTS) {
                p += (s32)get_special_objects_size(p);
            } else if (type == TERRAIN_LOAD_ENVIRONMENT) {
                s32 nr = *p++;
                p += 6 * nr;
            } else if (type == TERRAIN_LOAD_CONTINUE) {
                continue;
            } else if (type == TERRAIN_LOAD_END) {
                break;
            }
        }
    }

    QueryPerformanceCounter(&t_pass1);
    fprintf(stderr, "[parallel-terrain] area %d: pass 1 found %d triangles (%.2f ms)\n",
            index, total_tris,
            (double)(t_pass1.QuadPart - t_start.QuadPart) * 1000.0 / (double)freq.QuadPart);

    if (total_tris == 0) {
        fprintf(stderr, "[parallel-terrain] area %d: no triangles, skipping parallel path\n", index);
        if (macroObjects != NULL && *macroObjects != -1) {
            if (0 <= *macroObjects && *macroObjects < 30)
                spawn_macro_objects_hardcoded(index, macroObjects);
            else
                spawn_macro_objects(index, macroObjects);
        }
        surface_load_end();
        return;
    }

    struct Surface *scratch = (struct Surface *)malloc((size_t)total_tris * sizeof(struct Surface));
    TriDesc        *descs   = (TriDesc *)malloc((size_t)total_tris * sizeof(TriDesc));
    u8             *valid   = (u8 *)calloc((size_t)total_tris, 1);

    if (!scratch || !descs || !valid) {
        free(scratch); free(descs); free(valid);
        abort();
    }

    QueryPerformanceCounter(&t_alloc);
    fprintf(stderr, "[parallel-terrain] area %d: allocated scratch buffers (%zu KB, %.2f ms)\n",
            index, (size_t)total_tris * (sizeof(struct Surface) + sizeof(TriDesc) + 1) / 1024,
            (double)(t_alloc.QuadPart - t_pass1.QuadPart) * 1000.0 / (double)freq.QuadPart);

    {
        s16 *p = data;
        s8  *rooms = surfaceRooms;
        s32  tri_idx = 0;

        while (1) {
            s16 type = *p++;
            if (TERRAIN_LOAD_IS_SURFACE_TYPE_LOW(type) || TERRAIN_LOAD_IS_SURFACE_TYPE_HIGH(type)) {
                s32 n       = *p++;
                s32 hf      = surface_has_force(type);
                s8  flags   = (s8)no_cam_collision_flag(type);
                for (s32 i = 0; i < n; i++) {
                    TriDesc *d      = &descs[tri_idx++];
                    d->vertex_data  = vertex_data;
                    d->i0           = p[0];
                    d->i1           = p[1];
                    d->i2           = p[2];
                    d->surface_type = type;
                    d->room         = (rooms != NULL) ? *rooms++ : 0;
                    d->force        = hf ? p[3] : 0;
                    d->has_force    = (s8)hf;
                    d->flags        = flags;
                    p += 3 + hf;
                }
            } else if (type == TERRAIN_LOAD_VERTICES) {
                s32 nv   = *p++;
                vertex_data = p;
                p += 3 * nv;
            } else if (type == TERRAIN_LOAD_OBJECTS) {
                spawn_special_objects(index, &p);
            } else if (type == TERRAIN_LOAD_ENVIRONMENT) {
                load_environmental_regions(&p);
            } else if (type == TERRAIN_LOAD_CONTINUE) {
                continue;
            } else if (type == TERRAIN_LOAD_END) {
                break;
            }
        }
    }

    {
        s32 num_batches = (total_tris + BATCH_SIZE - 1) / BATCH_SIZE;
        FillBatch *batches = (FillBatch *)malloc((size_t)num_batches * sizeof(FillBatch));
        PTP_WORK  *works   = (PTP_WORK *)malloc((size_t)num_batches * sizeof(PTP_WORK));

        if (!batches || !works) {
            free(batches); free(works);
            free(scratch); free(descs); free(valid);
            abort();
        }

        for (s32 b = 0; b < num_batches; b++) {
            s32 start = b * BATCH_SIZE;
            s32 count = total_tris - start;
            if (count > BATCH_SIZE) count = BATCH_SIZE;
            batches[b].scratch = scratch;
            batches[b].descs   = descs;
            batches[b].valid   = valid;
            batches[b].start   = start;
            batches[b].count   = count;
            works[b] = CreateThreadpoolWork(fill_batch_cb, &batches[b], NULL);
            if (works[b]) SubmitThreadpoolWork(works[b]);
        }

        QueryPerformanceCounter(&t_dispatch);
        fprintf(stderr, "[parallel-terrain] area %d: dispatched %d batches of max %d tris (%.2f ms)\n",
                index, num_batches, BATCH_SIZE,
                (double)(t_dispatch.QuadPart - t_alloc.QuadPart) * 1000.0 / (double)freq.QuadPart);

        for (s32 b = 0; b < num_batches; b++) {
            if (works[b]) {
                WaitForThreadpoolWorkCallbacks(works[b], FALSE);
                CloseThreadpoolWork(works[b]);
            }
        }

        free(works);
        free(batches);

        QueryPerformanceCounter(&t_wait);
        fprintf(stderr, "[parallel-terrain] area %d: all workers done (%.2f ms wall, parallel phase %.2f ms)\n",
                index,
                (double)(t_wait.QuadPart - t_start.QuadPart) * 1000.0 / (double)freq.QuadPart,
                (double)(t_wait.QuadPart - t_dispatch.QuadPart) * 1000.0 / (double)freq.QuadPart);
    }

    for (s32 i = 0; i < total_tris; i++) {
        if (!valid[i]) continue;
        struct Surface *s = alloc_surface();
        const TriDesc  *d = &descs[i];
        *s = scratch[i];
        s->type  = d->surface_type;
        s->force = d->force;
        s->flags = d->flags;
        s->room  = d->room;
        s->object = NULL;
        add_surface(s, FALSE);
    }

    free(valid);
    free(descs);
    free(scratch);

    QueryPerformanceCounter(&t_insert);
    fprintf(stderr, "[parallel-terrain] area %d: serial insertion done, %d surfaces in partition (%.2f ms)\n",
            index, gSurfacesAllocated,
            (double)(t_insert.QuadPart - t_wait.QuadPart) * 1000.0 / (double)freq.QuadPart);

    if (macroObjects != NULL && *macroObjects != -1) {
        if (0 <= *macroObjects && *macroObjects < 30)
            spawn_macro_objects_hardcoded(index, macroObjects);
        else
            spawn_macro_objects(index, macroObjects);
    }

    surface_load_end();

    QueryPerformanceCounter(&t_end);
    fprintf(stderr, "[parallel-terrain] area %d: TOTAL load time %.2f ms (%d surfaces, %d nodes)\n",
            index,
            (double)(t_end.QuadPart - t_start.QuadPart) * 1000.0 / (double)freq.QuadPart,
            gNumStaticSurfaces, gNumStaticSurfaceNodes);
}

#endif
