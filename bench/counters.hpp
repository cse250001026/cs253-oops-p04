#ifndef BENCH_COUNTERS_HPP
#define BENCH_COUNTERS_HPP

// Operation counters for the benchmark harness (Analyst, D6).
//
// Structure A, Structure B and the oracle call the BENCH_* macros at the
// points listed below. In a normal build (no -DBENCH_COUNT) every macro
// expands to nothing, so timed runs pay zero overhead. The counting build
// is compiled with -DBENCH_COUNT and is never used for timing.
//
// Expected values for each counter are predicted in REPORT.md §9.2; a
// measured count that differs from the prediction is a bug, not noise.
//
// Requires C++17 (inline variable).

#include <cstdint>

struct OpCounters {
    // Shared by A, B and the oracle
    uint64_t half_edges_scanned = 0;  // one per AdjEdge read in the DFS/BFS traversal loop
                                      // (A: DFS loop only, not the later bcc_vertices pass)
    uint64_t vertices_visited   = 0;  // one per vertex first discovered (oracle: in every BFS)
    uint64_t max_stack_depth    = 0;  // deepest explicit DFS stack seen (A and B only)

    // Structure A only
    uint64_t edge_stack_pushes  = 0;  // one per edge id pushed for BCCs

    // Structure B only
    uint64_t uf_finds           = 0;  // one per DSU::find() call, both DSUs, incl. get_highest()
    uint64_t uf_find_steps      = 0;  // one per parent pointer followed to the root
    uint64_t uf_unions          = 0;  // one per tree edge marked in markCyclePath()

    // Oracle only
    uint64_t bfs_runs           = 0;  // one per count_comp() call, including the first
                                      // (no deletion) call: 1 + V + E' in total

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

#endif // BENCH_COUNTERS_HPP
