#include <vector>
#include <thread>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <atomic>

#include "load_progress.h"

std::atomic<int>     gParallelLoadTotal{0};
std::atomic<int>     gParallelLoadDone{0};
std::atomic<bool>    gParallelLoadActive{false};
std::atomic<int>     gLoadPhase{LOAD_PHASE_IDLE};
std::atomic<int>     gLoadThreadCount{0};
std::atomic<int64_t> gLoadElapsedMs{0};
std::atomic<int64_t> gLoadEtaMs{0};

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
    u16  i0, i1, i2;
    s16  surface_type;
    s8   room;
    s16  force;
    s8   flags;
    s8   has_force;
};

static constexpr s32 MIN_BATCH_SIZE = 1024;

using Clock     = std::chrono::high_resolution_clock;
using TimePoint = Clock::time_point;

static void fill_batch(struct Surface *scratch, const TriDesc *descs, u8 *valid,
                       s32 start, s32 count, TimePoint t0) {
    for (s32 i = start; i < start + count; i++) {
        const TriDesc &d = descs[i];
        struct Surface tmp{};
        valid[i] = (u8)surface_fill_from_data(&tmp, d.vertex_data, (s16)d.i0, (s16)d.i1, (s16)d.i2);
        if (valid[i]) scratch[i] = tmp;
    }
    int done  = gParallelLoadDone.fetch_add(count, std::memory_order_relaxed) + count;
    int total = gParallelLoadTotal.load(std::memory_order_relaxed);
    if (total > 0) {
        int64_t elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - t0).count();
        gLoadElapsedMs.store(elapsed, std::memory_order_relaxed);
        if (done > 0) {
            gLoadEtaMs.store(elapsed * (total - done) / done, std::memory_order_relaxed);
        }
    }
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

    auto t_start = Clock::now();

    gParallelLoadDone.store(0);
    gParallelLoadTotal.store(0);
    gLoadElapsedMs.store(0);
    gLoadEtaMs.store(0);
    gLoadPhase.store(LOAD_PHASE_PARSE);
    gLoadThreadCount.store(0);
    gParallelLoadActive.store(true);

    surface_load_begin();

    s32 vertex_count_pass1 = 0;
    /* ---- Pass 1: count triangles ---- */
    {
        s16 *p = data;
        while (true) {
            s16 type = *p++;
            if (TERRAIN_LOAD_IS_SURFACE_TYPE_LOW(type) || TERRAIN_LOAD_IS_SURFACE_TYPE_HIGH(type)) {
                s32 n = (u16)*p++;
                total_tris += n;
                p += (3 + surface_has_force(type)) * n;
            } else if (type == TERRAIN_LOAD_VERTICES) {
                s16 raw_count = *p;
                s32 nv = (u16)*p++;
                vertex_count_pass1 = nv;
                if ((s32)(s16)raw_count != nv) {
                    fprintf(stderr, "[parallel-terrain] WARN area %d: vertex count stored as s16=%d but raw int=%d "
                            "(likely >32767 overflow). Correct data needs large-vertex-count fix.\n",
                            index, (int)(s16)raw_count, nv);
                }
                fprintf(stderr, "[parallel-terrain] area %d: vertex block count=%d\n", index, nv);
                p += 3 * nv;
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

    {
        int64_t count_ms = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - t_start).count();
        fprintf(stderr, "[parallel-terrain] area %d: count pass done: %d tris (%.0f ms)\n",
                index, total_tris, (double)count_ms);
    }

    if (total_tris == 0) {
        if (macroObjects && *macroObjects != -1) {
            if (0 <= *macroObjects && *macroObjects < 30)
                spawn_macro_objects_hardcoded(index, macroObjects);
            else
                spawn_macro_objects(index, macroObjects);
        }
        gLoadPhase.store(LOAD_PHASE_DONE);
        gParallelLoadActive.store(false);
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

    s32 vertex_count_pass2 = 0;
    /* ---- Pass 2: collect triangle descriptors ---- */
    {
        s16 *p     = data;
        s8  *rooms = surfaceRooms;
        s32  tri_idx = 0;
        s32  oob_skipped = 0;

        while (true) {
            s16 type = *p++;
            if (TERRAIN_LOAD_IS_SURFACE_TYPE_LOW(type) || TERRAIN_LOAD_IS_SURFACE_TYPE_HIGH(type)) {
                s32 n  = (u16)*p++;
                s32 hf = surface_has_force(type);
                s8  fl = (s8)no_cam_collision_flag(type);
                for (s32 i = 0; i < n; i++) {
                    u16 vi0 = (u16)p[0], vi1 = (u16)p[1], vi2 = (u16)p[2];
                    s32 nv = vertex_count_pass2;
                    if ((s32)vi0 >= nv || (s32)vi1 >= nv || (s32)vi2 >= nv) {
                        if (oob_skipped < 16) {
                            fprintf(stderr, "[parallel-terrain] area %d: OOB vertex index tri=%d "
                                    "(%u,%u,%u) nv=%d -- skipping\n",
                                    index, tri_idx, (unsigned)vi0, (unsigned)vi1, (unsigned)vi2, nv);
                        }
                        oob_skipped++;
                        p += 3 + hf;
                        if (rooms) rooms++;
                        continue;
                    }
                    TriDesc &d     = descs[tri_idx++];
                    d.vertex_data  = vertex_data;
                    d.i0           = vi0;
                    d.i1           = vi1;
                    d.i2           = vi2;
                    d.surface_type = type;
                    d.room         = rooms ? *rooms++ : 0;
                    d.force        = hf ? p[3] : 0;
                    d.has_force    = (s8)hf;
                    d.flags        = fl;
                    p += 3 + hf;
                }
            } else if (type == TERRAIN_LOAD_VERTICES) {
                s32 nv  = (u16)*p++;
                vertex_count_pass2 = nv;
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
        if (oob_skipped > 0) {
            fprintf(stderr, "[parallel-terrain] area %d: %d triangles total had OOB vertex indices (nv=%d) and were skipped\n",
                    index, oob_skipped, vertex_count_pass2);
        }
        total_tris = tri_idx;
    }

    /* ---- Parallel compute phase: thread pool ---- */
    gLoadPhase.store(LOAD_PHASE_COMPUTE);

    int hw = (int)std::thread::hardware_concurrency();
    int num_threads = std::max(1, std::min(hw > 2 ? hw - 2 : 1, 16));
    s32 batch_size  = std::max(MIN_BATCH_SIZE, (total_tris + num_threads * 8 - 1) / (num_threads * 8));
    s32 num_batches = (total_tris + batch_size - 1) / batch_size;

    gLoadThreadCount.store(num_threads);

    fprintf(stderr, "[parallel-terrain] area %d: compute start, %d threads, %d batches of %d tris\n",
            index, num_threads, num_batches, batch_size);

    std::atomic<s32> next_batch{0};
    std::vector<std::thread> workers;
    workers.reserve(num_threads);

    for (int t = 0; t < num_threads; t++) {
        workers.emplace_back([&]() {
            for (;;) {
                s32 b = next_batch.fetch_add(1, std::memory_order_relaxed);
                if (b >= num_batches) break;
                s32 start = b * batch_size;
                s32 count = std::min(total_tris - start, batch_size);
                fill_batch(scratch, descs, valid, start, count, t_start);
            }
        });
    }
    for (auto &w : workers) w.join();

    {
        int64_t compute_ms = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - t_start).count();
        fprintf(stderr, "[parallel-terrain] area %d: compute done (%.0f ms total), starting insert of %d tris\n",
                index, (double)compute_ms, total_tris);
    }

    /* ---- Serial insertion into spatial partition ---- */
    gLoadPhase.store(LOAD_PHASE_INSERT);

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

    {
        int64_t insert_ms = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - t_start).count();
        fprintf(stderr, "[parallel-terrain] area %d: insert done (%.0f ms total)\n",
                index, (double)insert_ms);
    }

    if (macroObjects && *macroObjects != -1) {
        if (0 <= *macroObjects && *macroObjects < 30)
            spawn_macro_objects_hardcoded(index, macroObjects);
        else
            spawn_macro_objects(index, macroObjects);
    }

    int64_t total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - t_start).count();
    fprintf(stderr, "[parallel-terrain] area %d: %d tris, %d threads, %d batches of %d, total %.0f ms\n",
            index, total_tris, num_threads, num_batches, batch_size, (double)total_ms);

    gLoadPhase.store(LOAD_PHASE_DONE);
    gLoadEtaMs.store(0);
    gParallelLoadActive.store(false);
    surface_load_end();
}
