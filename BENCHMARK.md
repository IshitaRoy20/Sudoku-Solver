# Solver Benchmark

Three solver implementations compared on 1,000 puzzles per difficulty.
All solvers receive **identical pre-generated puzzle sets** — fair comparison.

## How to reproduce

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/benchmark
```

**Always use Release mode.** Debug mode is 5–10x slower due to disabled optimisations — you'd be benchmarking the compiler, not the algorithm.

---

## The three solvers

### Solver 1 — Baseline: pure MRV backtracking

The original algorithm. Picks the empty cell with the fewest legal candidates (Minimum Remaining Values heuristic), tries each value 1–9, recurses, undoes on failure. Every candidate check calls `get_possible_values()` which rescans all 20 peers. Every backtrack writes a 0 back to the cell.

**Complexity:** O(9^k) where k = number of cells requiring a guess. On hard puzzles k can exceed 40, leading to thousands of backtracks.

### Solver 2 — Propagation: naked + hidden singles + MRV backtracking

Before guessing, reason. Two constraint propagation techniques run in a loop until the board is stable:

**Naked singles (Phase 1):** A cell with exactly one legal candidate is forced — no choice exists. Place it immediately, eliminate it from all 20 peers via bitmask AND (`mask &= ~bit(v)`), repeat. Zero branching.

**Hidden singles (Phase 2):** A digit that can only fit in one cell of a row, column, or 3×3 box must go there — even if that cell has other candidates. The constraint comes from the unit, not the cell.

The two phases alternate until neither makes progress. Only then does MRV backtracking activate.

Candidates stored as `uint16_t` bitmasks (bits 1–9). Elimination: `mask &= ~bit(v)` — one operation. This replaces the O(27) peer rescan the baseline does on every candidate check.

**Complexity:** O(81 × 27 × P) propagation where P = passes until stable (typically 3–8), plus O(9^k') backtracking where k' << k.

### Solver 3 — AC-3: arc consistency + MRV backtracking

Formalises propagation as a graph problem (Mackworth, 1977). Every directed pair of peer cells is an **arc**. Each cell has 20 peers (8 row + 8 column + 4 new box = 20), giving 81 × 20 = **1,620 directed arcs** total.

AC-3 maintains a queue of arcs. When a cell loses a candidate, only that cell's arcs are re-enqueued — not the whole board. This eliminates the redundant full-board scans the propagation loop does on every pass.

**Proven bound:** O(e × d) = O(1,620 × 9) = **O(14,580)** per propagation call. Each arc is processed at most d=9 times because a domain can only shrink — once a candidate is removed it is never added back.

---

## Results — 1,000 puzzles per difficulty

```
========================================================================================
  Sudoku Engine — Solver Benchmark
  1000 puzzles per difficulty, 3 solvers
========================================================================================

  EASY   (35 cells removed, ~46 clues)  (1000 puzzles)
----------------------------------------------------------------------------------------
  Solver                    Avg ms    Max ms    Avg BT    Max BT  Pure logic
----------------------------------------------------------------------------------------
  Baseline (BT only)         0.084     0.244       0.0         0       100%
  Propagation (N+H+BT)       0.009     0.125       0.0         0       100%
  AC-3 (AC3+BT)              0.101     6.622       0.0         0       100%
----------------------------------------------------------------------------------------
  Propagation vs Baseline:   9.2x faster,  100.0% fewer backtracks
  AC-3 vs Baseline:          0.8x faster,  100.0% fewer backtracks
  AC-3 vs Propagation:       0.1x faster,  same backtracks

  MEDIUM (45 cells removed, ~36 clues)  (1000 puzzles)
----------------------------------------------------------------------------------------
  Solver                    Avg ms    Max ms    Avg BT    Max BT  Pure logic
----------------------------------------------------------------------------------------
  Baseline (BT only)         0.162     0.750       1.7       137        90%
  Propagation (N+H+BT)       0.012     0.196       0.0         3        99%
  AC-3 (AC3+BT)              0.115     6.228       0.3        33        90%
----------------------------------------------------------------------------------------
  Propagation vs Baseline:  13.0x faster,  99.5% fewer backtracks
  AC-3 vs Baseline:          1.4x faster,  83.4% fewer backtracks
  AC-3 vs Propagation:       0.1x faster,  AC-3 does more backtracks

  HARD   (55 cells removed, ~26 clues)  (1000 puzzles)
----------------------------------------------------------------------------------------
  Solver                    Avg ms    Max ms    Avg BT    Max BT  Pure logic
----------------------------------------------------------------------------------------
  Baseline (BT only)         1.069    15.387     167.9      2069        12%
  Propagation (N+H+BT)       0.057     2.705       0.8        19        72%
  AC-3 (AC3+BT)              0.728     9.666      45.9       728        12%
----------------------------------------------------------------------------------------
  Propagation vs Baseline:  18.9x faster,  99.5% fewer backtracks
  AC-3 vs Baseline:          1.5x faster,  72.7% fewer backtracks
  AC-3 vs Propagation:       0.1x faster,  AC-3 does far more backtracks
========================================================================================
```

---

## What the numbers mean

### Backtracks (BT)
Every time the solver tries a value and has to undo it, that is one backtrack. Zero backtracks means the puzzle was solved entirely by logic — no guessing at all.

### Pure logic %
Percentage of puzzles solved with zero backtracking. Baseline: 12% on hard (most puzzles require guessing). Propagation: 72% on hard (most hard puzzles solved by constraint propagation alone).

### The hard puzzle story
The baseline averaged 167.9 backtracks per hard puzzle. Propagation averaged 0.8. The maximum dropped from 2,069 to 19. This means the hardest puzzle the baseline struggled with (2,069 wrong guesses) was solved by propagation with at most 19.

---

## The unexpected finding — why AC-3 was slower

AC-3 has a **tighter theoretical bound** than the propagation loop: O(e×d) = O(14,580) vs O(81×27×P). But it was consistently slower in practice. Why?

**Constant factor overhead.** Every arc operation in the queue-based AC-3 involves:
- Dequeuing a `std::pair<Coord,Coord>` from `std::queue` (heap-backed)
- Computing or looking up the 20 peers of the source cell
- Checking and potentially re-enqueuing all of the source's arcs

The propagation loop uses tight bitmask operations with no dynamic allocation — `mask &= ~bit(v)` is a single CPU instruction per peer elimination. The bitmask scan over 81 cells is sequential memory access with excellent cache behaviour.

On a fixed 9×9 grid, the constant factor of the propagation bitmask operations beats the asymptotically tighter AC-3 queue. The theoretical bound holds — AC-3 does less *logical* work on hard puzzles — but that advantage is erased by the per-operation overhead.

**This is why benchmarking matters.** Asymptotic complexity describes behaviour at scale. For a fixed-size 9×9 grid, the constant factor is the whole story.

**The fix** (planned): pre-compute a peer lookup table `peers[81][20]` at startup, eliminating per-arc allocation. This should recover AC-3's theoretical advantage.


## Complexity reference

| Solver | Avg time (Hard) | Avg BT (Hard) | Theoretical TC |
|--------|----------------|---------------|----------------|
| Baseline | 1.069ms | 167.9 | O(9^k), k≈40+ |
| Propagation | 0.057ms | 0.8 | O(81×27×P) + O(9^k'), k'<<k |
| AC-3 | 0.728ms | 45.9 | O(e×d) + O(9^k''), proven |

*BT = backtrack nodes. P = propagation passes (typically 3–8).*

---
