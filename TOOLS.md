# TOOLS

D4 (Shubham): The test cases in `tests/oracle_tests.cpp` were generated using Claude.
D4 (Shubham): Used Claude for assistance with the structure
and failure reporting of oracle/differential_test.cpp.

| Tool and version | Used by | Used for | Files affected | What we changed or verified |
|------------------|---------|----------|----------------|-----------------------------|
| Claude Code (Claude Opus 5.5), 11 Nov 2026 | Deepak Chahar (Analyst, D6) | Review of the open PRs against the spec; drafts of the benchmark harness and scripts, the counter and cache predictions, the experimental setup, the bug log, and the D6 derivation | `bench/bench.cpp`, `bench/run_benchmarks.sh`, `bench/cache_profile.sh`, `bench/plot_results.py`, `bench/counters.hpp` (comments), `scripts/run_all.sh`, `REPORT.md` §7.4 (bug log), §9.2, §9.3, §10, `docs/derivations/D6_250001026.pdf` | Verified: the harness and scripts were smoke-tested on a copy of main, with B and the oracle agreeing with A on every input. TODO (Deepak): say what you rewrote, and how you checked the D6 derivation. |

### Representative prompts (Deepak, Claude Code, 11 Nov 2026)
- "review the pr requests on cse253 oops repo and let me know are there any bugs or mismatches"
- "complete the part to not commit anything"
