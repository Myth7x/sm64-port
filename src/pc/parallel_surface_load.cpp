#include <future>
#include <vector>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <cstdlib>
#include <cstdio>

#include "load_progress.h"

std::atomic<int>  gParallelLoadTotal{0};
std::atomic<int>  gParallelLoadDone{0};
std::atomic<bool> gParallelLoadActive{false};

#ifndef _LANGUAGE_C
#define _LANGUAGE_C
#endif

extern "C" {
#include <PR/ultratypes.h>
#include "../engine/surface_load.h"
#include "../game/object_list_processor.h"
#include "../../include/surface_terrains.h"
#include "../game/macro_special_objects.h"
}

struct TriDesc {
    s16 *vertex_data;
    s16  i0, i1, i2;
    s16  surface_type;
    s8   room;
    s16  force;
    s8   flags;
    s8   has_force;
};

static constexpr s32 BATCH_SIZE = 512;

static void fill_batch(struct Surface *scratch, const TriDesc *descs, u8 *valid,
                       s32 start, s32 count) {
    auto t0 = std::chrono::high_resolution_clock::now();
    for (s32 i = start; i < start + count; i++) {
        const TriDesc &d = descs[i];
        struct Surface tmp{};
        valid[i] = (u8)surface_fill_from_data(&tmp, d.vertex_data, d.i0, d.i1, d.i2);
        if (valid[i]) scratch[i] = tmp;
    }
    auto t1  = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    fprintf(stderr, "[surface-worker] batch %d tris %d..%d done in %.2f ms\n",
            start / BATCH_SIZE, start, start + count - 1, ms);
    gParallelLoadDone.fetch_add(count, std::memory_order_relaxed);
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

extern "C" void load_area_terrain_parallel(s16 index, s16 *data, s8 *surfaceRooms, s16 *macroObjects) {
    s16 *vertex_data = nullptr;
    s32  total_tris  = 0;

    auto t_start = std::chrono::high_resolution_clock::now();
    fprintf(stderr, "[parallel-terrain] area %d: starting load\n", index);

    gParallelLoadDone.store(0);
    gParallelLoadTotal.store(0);
    gParallelLoadActive.store(true);

    surface_load_begin();

    /* ---- Pass 1: count triangles ---- */
    {
        s16 *p = data;
        while (true) {
            s16 type = *p++;
            if (TERRAIN_LOAD_IS_SURFACE_TYPE_LOW(type) || TERRAIN_LOAD_IS_SURFACE_TYPE_HIGH(type)) {
                s32 n = *p++;
                total_tris += n;
                p += (3 + surface_has_force(type)) * n;
            } else if (type == TERRAIN_LOAD_VERTICES) {
                s32 nv = *p++; p += 3 * nv;
            } else if (type == TERRAIN_LOAD_OBJECTS) {
                p += (s32)get_special_objects_size(p);
            } else if (type == TERRAIN_LOAD_ENVIRONMENT) {
                s32 nr = *p++; p += 6 * nr;
            } else if (type == TERRAIN_LOAD_CONTINUE) {
                continue;
            } else if (type == TERRAIN_LOAD_END) {
                break;
            }
        }
    }

    gParallelLoadTotal.store(total_tris);

    auto t_pass1 = std::chrono::high_resolution_clock::now();
    fprintf(stderr, "[parallel-terrain] area %d: pass 1 found %d triangles (%.2f ms)\n",
            index, total_tris,
            std::chrono::duration<double, std::milli>(t_pass1 - t_start).count());

    if (total_tris == 0) {
        fprintf(stderr, "[parallel-terrain] area %d: no triangles, skipping parallel path\n", index);
        if (macroObjects && *macroObjects != -1) {
            if (0 <= *macroObjects && *macroObjects < 30)
                spawn_macro_objects_hardcoded(index, macroObjects);
            else
                spawn_macro_objects(index, macroObjects);
        }
        surface_load_end();
        return;
    }

    auto *scratch = static_cast<struct Surface *>(std::malloc((size_t)total_tris * sizeof(struct Surface)));
    auto *descs   = static_cast<TriDesc *>(std::malloc((size_t)total_tris * sizeof(TriDesc)));
    auto *valid   = static_cast<u8 *>(std::calloc((size_t)total_tris, 1));

    if (!scratch || !descs || !valid) {
        std::free(scratch); std::free(descs); std::free(valid);
        std::abort();
    }

    auto t_alloc = std::chrono::high_resolution_clock::now();
    fprintf(stderr, "[parallel-terrain] area %d: allocated scratch buffers (%zu KB, %.2f ms)\n",
            index,
            (size_t)total_tris * (sizeof(struct Surface) + sizeof(TriDesc) + 1) / 1024,
            std::chrono::duration<double, std::milli>(t_alloc - t_pass1).count());

    /* ---- Pass 2: collect triangle descriptors ---- */
    {
        s16 *p     = data;
        s8  *rooms = surfaceRooms;
        s32  tri_idx = 0;

        while (true) {
            s16 type = *p++;
            if (TERRAIN_LOAD_IS_SURFACE_TYPE_LOW(type) || TERRAIN_LOAD_IS_SURFACE_TYPE_HIGH(type)) {
                s32 n  = *p++;
                s32 hf = surface_has_force(type);
                s8  fl = (s8)no_cam_collision_flag(type);
                for (s32 i = 0; i < n; i++) {
                    TriDesc &d     = descs[tri_idx++];
                    d.vertex_data  = vertex_data;
                    d.i0           = p[0];
                    d.i1           = p[1];
                    d.i2           = p[2];
                    d.surface_type = type;
                    d.room         = rooms ? *rooms++ : 0;
                    d.force        = hf ? p[3] : 0;
                    d.has_force    = (s8)hf;
                    d.flags        = fl;
                    p += 3 + hf;
                }
            } else if (type == TERRAIN_LOAD_VERTICES) {
                s32 nv  = *p++;
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

    /* ---- Dispatch batches via std::async ---- */
    s32 num_batches = (total_tris + BATCH_SIZE - 1) / BATCH_SIZE;
    std::vector<std::future<void>> futures;
    futures.reserve(num_batches);

    auto t_dispatch = std::chrono::high_resolution_clock::now();
    fprintf(stderr, "[parallel-terrain] area %d: dispatching %d batches of max %d tris (%.2f ms)\n",
            index, num_batches, BATCH_SIZE,
            std::chrono::duration<double, std::milli>(t_dispatch - t_alloc).count());

    for (s32 b = 0; b < num_batches; b++) {
        s32 start = b * BATCH_SIZE;
        s32 count = std::min(total_tris - start, BATCH_SIZE);
        futures.push_back(std::async(std::launch::async, fill_batch, scratch, descs, valid, start, count));
    }

    for (auto &f : futures) f.get();

    auto t_wait = std::chrono::high_resolution_clock::now();
    fprintf(stderr, "[parallel-terrain] area %d: all workers done (%.2f ms wall, parallel phase %.2f ms)\n",
            index,
            std::chrono::duration<double, std::milli>(t_wait - t_start).count(),
            std::chrono::duration<double, std::milli>(t_wait - t_dispatch).count());

    /* ---- Serial insertion into spatial partition ---- */
    for (s32 i = 0; i < total_tris; i++) {
        if (!valid[i]) continue;
        struct Surface *s = alloc_surface();
        const TriDesc  &d = descs[i];
        *s        = scratch[i];
        s->type   = d.surface_type;
        s->force  = d.force;
        s->flags  = d.flags;
        s->room   = d.room;
        s->object = nullptr;
        add_surface(s, FALSE);
    }

    std::free(valid);
    std::free(descs);
    std::free(scratch);

    auto t_insert = std::chrono::high_resolution_clock::now();
    fprintf(stderr, "[parallel-terrain] area %d: serial insertion done, %d surfaces in partition (%.2f ms)\n",
            index, gSurfacesAllocated,
            std::chrono::duration<double, std::milli>(t_insert - t_wait).count());

    if (macroObjects && *macroObjects != -1) {
        if (0 <= *macroObjects && *macroObjects < 30)
            spawn_macro_objects_hardcoded(index, macroObjects);
        else
            spawn_macro_objects(index, macroObjects);
    }

    gParallelLoadActive.store(false);
    surface_load_end();

    auto t_end = std::chrono::high_resolution_clock::now();
    fprintf(stderr, "[parallel-terrain] area %d: TOTAL load time %.2f ms (%d surfaces, %d nodes)\n",
            index,
            std::chrono::duration<double, std::milli>(t_end - t_start).count(),
            gNumStaticSurfaces, gNumStaticSurfaceNodes);
}
