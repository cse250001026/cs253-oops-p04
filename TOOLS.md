# TOOLS

| Tool and version | Used by | Used for | Files affected | What we changed or verified |
|------------------|---------|----------|----------------|-----------------------------|
| Claude, Oct–Nov 2026 | Shubham Sharma (Oracle, D4) | Generating test cases; help with the code structure and failure reporting of the differential test | `tests/oracle_tests.cpp`, `oracle/differential_test.cpp` | Ran the tests against the oracle, Structure A and Structure B; the differential test passes at 10^5 and 10^6 operations (seed 20260928) |
| Claude Code (Claude Opus 5.5), Nov 2026 | Deepak Chahar (Analyst, D6) | Reviewing pull requests against the project spec; help writing the benchmark, cache-profiling and plotting scripts; first drafts of report text and the D6 derivation | `bench/`, `scripts/run_all.sh`, `REPORT.md`, `docs/derivations/D6_250001026.pdf` | Rewrote the report text and D6 in my own words; checked that the harness's B and oracle results agree with A on every input |
| Antigravity (Gemini 3.8 Flash), Nov 2026 | Mayank Gupta (Structure B, D2) | Generating test case suites (fixed edge cases, randomized testing harness, and stress tests); formatting the D2 derivation document; cross-checking operation counter formulas | `src/structB/`, `tests/test_structB.cpp`, `docs/derivations/D2*`, `REPORT.md` | Verified all test assertions against invariants, confirmed D2 derivation steps and bounds against Tarjan (1975), and validated operation counter instrumentation against theoretical predictions. |

## Representative prompts

**Deepak (Claude Code, 11 Nov 2026)**
- "review the pr requests on cse253 oops repo and let me know are there any bugs or mismatches"
- "complete the part to not commit anything"
| Gemini(4 Argon), Oct–Nov 2026 | Sufiyan Ahmad (Architect, D3) | Designing shared graph header (`include/graph.hpp`) and `INTERFACE.md`; drafting `app/block_cut_tree.hpp`, `app/app.cpp`, `app/run_ops.cpp`, and formatting the D3 derivation in LaTeX | `include/graph.hpp`, `INTERFACE.md`, `app/`, `oracle/invarient_checks.hpp`, `REPORT.md`, `docs/derivations/D3_250001068.pdf` | Verified `BlockCutTree` invariants and `separates(v, x, y)` against brute-force BFS in `check_bct_invariants`; tested `bin/app` and `bin/run_ops` with `make test`. |
