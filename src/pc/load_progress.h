#pragma once
#include <atomic>

extern std::atomic<int>  gParallelLoadTotal;
extern std::atomic<int>  gParallelLoadDone;
extern std::atomic<bool> gParallelLoadActive;
