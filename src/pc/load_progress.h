#pragma once
#include <atomic>
#include <cstdint>

enum LoadPhase : int {
    LOAD_PHASE_IDLE    = 0,
    LOAD_PHASE_PARSE   = 1,
    LOAD_PHASE_COMPUTE = 2,
    LOAD_PHASE_INSERT  = 3,
    LOAD_PHASE_DONE    = 4,
};

extern std::atomic<int>      gParallelLoadTotal;
extern std::atomic<int>      gParallelLoadDone;
extern std::atomic<bool>     gParallelLoadActive;
extern std::atomic<int>      gLoadPhase;
extern std::atomic<int>      gLoadThreadCount;
extern std::atomic<int64_t>  gLoadElapsedMs;
extern std::atomic<int64_t>  gLoadEtaMs;
