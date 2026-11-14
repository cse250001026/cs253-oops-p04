#ifndef BENCH_COUNTERS_HPP
#define BENCH_COUNTERS_HPP

#include <cstdint>

struct OpCounters {
    uint64_t half_edges_scanned = 0;
    uint64_t vertices_visited   = 0;
    uint64_t max_stack_depth    = 0;

    uint64_t edge_stack_pushes  = 0;

    uint64_t uf_finds           = 0;
    uint64_t uf_find_steps      = 0;
    uint64_t uf_unions          = 0;

    uint64_t bfs_runs           = 0;

    void reset() { *this = OpCounters{}; }
};

inline OpCounters g_ops;

#ifdef BENCH_COUNT
#define BENCH_INC(field)        (++g_ops.field)
#define BENCH_ADD(field, n)     (g_ops.field += (uint64_t)(n))
#define BENCH_MAX(field, v)     do { if ((uint64_t)(v) > g_ops.field) g_ops.field = (uint64_t)(v); } while (0)
#else
#define BENCH_INC(field)        ((void)0)
#define BENCH_ADD(field, n)     ((void)0)
#define BENCH_MAX(field, v)     ((void)0)
#endif

#endif
