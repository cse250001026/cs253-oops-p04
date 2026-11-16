CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra
BIN      := bin

DIFF_OPS ?= 1000000
SEED     ?= 20260928

STRUCTB_SRC := $(wildcard src/structB/*.cpp)
HEADERS     := include/graph.hpp src/structA/tarjan_lowlink.hpp oracle/bfs_oracle.hpp \
               $(wildcard src/structB/*.hpp) $(wildcard oracle/*.hpp) bench/counters.hpp \
               $(wildcard app/*.hpp)
SHRINKER    := $(wildcard oracle/failure_shrinker.cpp)
GENERATORS  := $(patsubst adversary/%.cpp,$(BIN)/%,$(wildcard adversary/gen_*.cpp))

TESTS    := $(BIN)/test_structA $(BIN)/test_structB $(BIN)/oracle_tests $(BIN)/differential_test
PROGRAMS := $(TESTS) $(BIN)/bench $(BIN)/bench_count $(GENERATORS)

ifneq ($(wildcard app/app.cpp),)
PROGRAMS += $(BIN)/app
endif
ifneq ($(wildcard app/run_ops.cpp),)
PROGRAMS += $(BIN)/run_ops
endif

.PHONY: all test bench cache figures all-results clean
.DELETE_ON_ERROR:

all: $(PROGRAMS)

$(BIN):
	mkdir -p $(BIN)

$(BIN)/test_structA $(BIN)/test_structB $(BIN)/oracle_tests: $(BIN)/%: tests/%.cpp $(STRUCTB_SRC) $(HEADERS) | $(BIN)
	$(CXX) $(CXXFLAGS) $< $(STRUCTB_SRC) -o $@

$(BIN)/differential_test: oracle/differential_test.cpp $(SHRINKER) $(STRUCTB_SRC) $(HEADERS) | $(BIN)
	$(CXX) $(CXXFLAGS) $< $(SHRINKER) $(STRUCTB_SRC) -o $@

$(BIN)/bench: bench/bench.cpp $(STRUCTB_SRC) $(HEADERS) | $(BIN)
	$(CXX) $(CXXFLAGS) $< $(STRUCTB_SRC) -o $@

$(BIN)/bench_count: bench/bench.cpp $(STRUCTB_SRC) $(HEADERS) | $(BIN)
	$(CXX) $(CXXFLAGS) -DBENCH_COUNT $< $(STRUCTB_SRC) -o $@

$(BIN)/gen_%: adversary/gen_%.cpp | $(BIN)
	$(CXX) $(CXXFLAGS) $< -o $@

$(BIN)/app: app/app.cpp $(STRUCTB_SRC) $(HEADERS) $(wildcard app/*.hpp) | $(BIN)
	$(CXX) $(CXXFLAGS) $< $(STRUCTB_SRC) -o $@

$(BIN)/run_ops: app/run_ops.cpp $(STRUCTB_SRC) $(HEADERS) $(wildcard app/*.hpp) | $(BIN)
	$(CXX) $(CXXFLAGS) $< $(STRUCTB_SRC) -o $@

test: $(TESTS)
	./$(BIN)/test_structA --stress
	./$(BIN)/test_structB --stress
	./$(BIN)/oracle_tests
	./$(BIN)/differential_test $(DIFF_OPS) $(SEED)
	@echo
	@echo "ALL TESTS PASSED"

bench:
	bash bench/run_benchmarks.sh

cache: $(BIN)/bench
	bash bench/cache_profile.sh

figures:
	python3 bench/plot_results.py

all-results:
	bash scripts/run_all.sh

clean:
	rm -rf $(BIN)
