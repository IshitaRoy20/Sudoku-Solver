#ifndef SRC_SUDOKU_SOLVER_H_
#define SRC_SUDOKU_SOLVER_H_
#include <array>
#include <chrono>
#include <queue>
#include <stdexcept>
#include <string>
#include <utility>

#include "grid.h"

namespace sudoku {

struct SolveStats {
    int         backtracks = 0;
    std::string solved_by;
    double      time_ms    = 0.0;
};

using CandidateBoard = std::array<std::array<uint16_t, 9>, 9>;
using Arc = std::pair<Coord, Coord>;

inline uint16_t bit(int v)       { return static_cast<uint16_t>(1 << v); }
inline int      popcount(uint16_t m) { int n=0; while(m){n+=m&1;m>>=1;} return n; }
inline int      lowest_bit(uint16_t m) { for(int v=1;v<=9;v++) if(m&bit(v)) return v; return 0; }

inline std::array<Coord, 20> get_peers(int r, int c)
{
    std::array<Coord, 20> peers{};
    int idx = 0;

    for (int col = 0; col < 9; col++)
        if (col != c) peers[idx++] = {r, col};

    for (int row = 0; row < 9; row++)
        if (row != r) peers[idx++] = {row, c};

    int br = (r/3)*3, bc = (c/3)*3;
    for (int dr = 0; dr < 3; dr++)
        for (int dc = 0; dc < 3; dc++) {
            int pr = br+dr, pc = bc+dc;
            if (pr != r && pc != c)   // new: not same row AND not same col
                peers[idx++] = {pr, pc};
        }

    return peers;
}

inline CandidateBoard build_candidates(const Grid& grid)
{
    CandidateBoard cands{};
    constexpr uint16_t ALL = 0b1111111110; // bits 1-9

    for (int r = 0; r < GRID_LEN; r++)
        for (int c = 0; c < GRID_LEN; c++)
            cands[r][c] = grid.is_empty({r,c}) ? ALL : 0;

    for (int r = 0; r < GRID_LEN; r++) {
        for (int c = 0; c < GRID_LEN; c++) {
            int v = grid.get({r,c});
            if (v == 0) continue;
            uint16_t b = bit(v);
            for (int col=0;col<GRID_LEN;col++) cands[r][col]   &= ~b;
            for (int row=0;row<GRID_LEN;row++) cands[row][c]   &= ~b;
            int br=(r/3)*3, bc=(c/3)*3;
            for (int dr=0;dr<3;dr++)
                for (int dc=0;dc<3;dc++)
                    cands[br+dr][bc+dc] &= ~b;
            cands[r][c] = 0;
        }
    }
    return cands;
}

inline bool eliminate(CandidateBoard& cands, const Grid& grid,
                      int r, int c, int v)
{
    uint16_t b = bit(v);
    for (int col=0;col<GRID_LEN;col++) {
        if (col==c || !(cands[r][col]&b)) continue;
        cands[r][col] &= ~b;
        if (cands[r][col]==0 && grid.is_empty({r,col})) return false;
    }
    for (int row=0;row<GRID_LEN;row++) {
        if (row==r || !(cands[row][c]&b)) continue;
        cands[row][c] &= ~b;
        if (cands[row][c]==0 && grid.is_empty({row,c})) return false;
    }
    int br=(r/3)*3, bc=(c/3)*3;
    for (int dr=0;dr<3;dr++) {
        for (int dc=0;dc<3;dc++) {
            int pr=br+dr, pc=bc+dc;
            if ((pr==r&&pc==c)||!(cands[pr][pc]&b)) continue;
            cands[pr][pc] &= ~b;
            if (cands[pr][pc]==0 && grid.is_empty({pr,pc})) return false;
        }
    }
    return true;
}

inline bool place(Grid& grid, CandidateBoard& cands, int r, int c, int v)
{
    grid.update({r,c}, v);
    cands[r][c] = 0;
    return eliminate(cands, grid, r, c, v);
}

inline bool apply_naked_singles(Grid& grid, CandidateBoard& cands,
                                bool& contradiction)
{
    bool placed_any = false;
    for (int r=0;r<GRID_LEN;r++) {
        for (int c=0;c<GRID_LEN;c++) {
            if (!grid.is_empty({r,c})) continue;
            if (popcount(cands[r][c]) != 1) continue;
            int v = lowest_bit(cands[r][c]);
            if (!place(grid, cands, r, c, v)) { contradiction=true; return false; }
            placed_any = true;
        }
    }
    return placed_any;
}

inline bool apply_hidden_singles(Grid& grid, CandidateBoard& cands,
                                 bool& contradiction)
{
    bool placed_any = false;

    // Rows
    for (int r=0;r<GRID_LEN;r++) {
        for (int v=1;v<=9;v++) {
            int count=0, last_c=-1;
            for (int c=0;c<GRID_LEN;c++) {
                if (!grid.is_empty({r,c})) continue;
                if (cands[r][c]&bit(v)) { count++; last_c=c; }
            }
            if (count!=1) continue;
            if (!place(grid,cands,r,last_c,v)) { contradiction=true; return false; }
            placed_any = true;
        }
    }

    // Columns
    for (int c=0;c<GRID_LEN;c++) {
        for (int v=1;v<=9;v++) {
            int count=0, last_r=-1;
            for (int r=0;r<GRID_LEN;r++) {
                if (!grid.is_empty({r,c})) continue;
                if (cands[r][c]&bit(v)) { count++; last_r=r; }
            }
            if (count!=1) continue;
            if (!place(grid,cands,last_r,c,v)) { contradiction=true; return false; }
            placed_any = true;
        }
    }

    // Boxes
    for (int br=0;br<9;br+=3) {
        for (int bc=0;bc<9;bc+=3) {
            for (int v=1;v<=9;v++) {
                int count=0, last_r=-1, last_c=-1;
                for (int dr=0;dr<3;dr++) {
                    for (int dc=0;dc<3;dc++) {
                        int r=br+dr, c=bc+dc;
                        if (!grid.is_empty({r,c})) continue;
                        if (cands[r][c]&bit(v)) { count++; last_r=r; last_c=c; }
                    }
                }
                if (count!=1) continue;
                if (!place(grid,cands,last_r,last_c,v)) { contradiction=true; return false; }
                placed_any = true;
            }
        }
    }

    return placed_any;
}

inline bool propagate(Grid& grid, CandidateBoard& cands)
{
    while (true) {
        bool progress=false, contradiction=false;
        while (apply_naked_singles(grid,cands,contradiction)) {
            if (contradiction) return false;
            progress = true;
        }
        if (contradiction) return false;
        bool h = apply_hidden_singles(grid,cands,contradiction);
        if (contradiction) return false;
        if (h) { progress=true; continue; }
        if (!progress) break;
    }
    return true;
}

inline bool ac3(Grid& grid, CandidateBoard& cands)
{
    
    std::queue<Arc> q;

    for (int r = 0; r < GRID_LEN; r++) {
        for (int c = 0; c < GRID_LEN; c++) {
            if (!grid.is_empty({r,c})) continue;

            // Add arc from (r,c) to each of its 20 peers
            auto peers = get_peers(r, c);
            for (const Coord& peer : peers) {
                if (!grid.is_empty(peer))
                    continue; // no point checking filled cells
                q.push({{r,c}, peer});
            }
        }
    }

    while (!q.empty()) {
        auto [A, B] = q.front();
        q.pop();

        int ar = A.first,  ac_ = A.second;
        int br = B.first,  bc_ = B.second;

        if (!grid.is_empty(A)) continue;
        if (!grid.is_empty(B)) {
            int bval = grid.get(B);
            if (!(cands[ar][ac_] & bit(bval))) continue; // already eliminated

            cands[ar][ac_] &= ~bit(bval);

            if (cands[ar][ac_] == 0) return false;

            auto peers = get_peers(ar, ac_);
            for (const Coord& peer : peers)
                q.push({peer, A});

            if (popcount(cands[ar][ac_]) == 1) {
                int v = lowest_bit(cands[ar][ac_]);
                if (!place(grid, cands, ar, ac_, v)) return false;
                for (const Coord& peer : peers)
                    if (grid.is_empty(peer))
                        q.push({peer, A});
            }
            continue;
        }

        
        if (popcount(cands[br][bc_]) != 1) continue; 

        int bval = lowest_bit(cands[br][bc_]);
        if (!(cands[ar][ac_] & bit(bval))) continue; 
        // Remove bval from A
        cands[ar][ac_] &= ~bit(bval);

        if (cands[ar][ac_] == 0) return false; 

        auto a_peers = get_peers(ar, ac_);
        for (const Coord& peer : a_peers)
            if (grid.is_empty(peer))
                q.push({peer, A});

        if (popcount(cands[ar][ac_]) == 1) {
            int v = lowest_bit(cands[ar][ac_]);
            if (!place(grid, cands, ar, ac_, v)) return false;
        }
    }

    return true;
}

inline bool solve_recursive(Grid& grid, CandidateBoard& cands,
                             int& backtrack_count,
                             bool use_ac3)
{
    // MRV: pick empty cell with fewest candidates
    int   best_count = 10;
    Coord best_cell  = {-1, -1};

    for (int r=0;r<GRID_LEN;r++) {
        for (int c=0;c<GRID_LEN;c++) {
            if (!grid.is_empty({r,c})) continue;
            int cnt = popcount(cands[r][c]);
            if (cnt == 0) return false;
            if (cnt < best_count) { best_count=cnt; best_cell={r,c}; }
        }
    }

    if (best_cell.first == -1) return true; // solved

    int r = best_cell.first;
    int c = best_cell.second;
    uint16_t mask = cands[r][c];

    for (int v=1;v<=9;v++) {
        if (!(mask & bit(v))) continue;

        // Snapshot before attempting v
        Grid           grid_snap  = grid;
        CandidateBoard cands_snap = cands;

        if (place(grid, cands, r, c, v)) {
            // Run propagation after each guess
            bool ok = use_ac3 ? ac3(grid, cands)
                               : propagate(grid, cands);
            if (ok) {
                if (solve_recursive(grid, cands, backtrack_count, use_ac3))
                    return true;
            }
        }

        ++backtrack_count;
        grid  = grid_snap;
        cands = cands_snap;
    }
    return false;
}

inline bool is_fully_filled(const Grid& grid)
{
    for (int r=0;r<GRID_LEN;r++)
        for (int c=0;c<GRID_LEN;c++)
            if (grid.is_empty({r,c})) return false;
    return true;
}
inline void solve(Grid& grid)
{
    CandidateBoard cands = build_candidates(grid);

    if (!propagate(grid, cands))
        throw std::logic_error("Puzzle has no solution.");

    if (is_fully_filled(grid)) return;

    int dummy = 0;
    if (!solve_recursive(grid, cands, dummy, false))
        throw std::logic_error("Puzzle has no solution.");
}

inline SolveStats solve_with_stats(Grid& grid)
{
    SolveStats stats;
    auto t0 = std::chrono::high_resolution_clock::now();

    CandidateBoard cands = build_candidates(grid);

    if (!propagate(grid, cands)) {
        stats.solved_by = "unsolvable";
        stats.time_ms = std::chrono::duration<double,std::milli>(
            std::chrono::high_resolution_clock::now()-t0).count();
        throw std::logic_error("Puzzle has no solution.");
    }

    if (is_fully_filled(grid)) {
        stats.solved_by = "propagation";
    } else {
        if (!solve_recursive(grid, cands, stats.backtracks, false))
            throw std::logic_error("Puzzle has no solution.");
        stats.solved_by = stats.backtracks==0 ? "propagation" : "backtracking";
    }

    stats.time_ms = std::chrono::duration<double,std::milli>(
        std::chrono::high_resolution_clock::now()-t0).count();
    return stats;
}

inline SolveStats solve_ac3_with_stats(Grid& grid)
{
    SolveStats stats;
    auto t0 = std::chrono::high_resolution_clock::now();

    CandidateBoard cands = build_candidates(grid);

    if (!ac3(grid, cands)) {
        stats.solved_by = "unsolvable";
        stats.time_ms = std::chrono::duration<double,std::milli>(
            std::chrono::high_resolution_clock::now()-t0).count();
        throw std::logic_error("Puzzle has no solution.");
    }

    if (is_fully_filled(grid)) {
        stats.solved_by = "ac3-pure";
    } else {
        if (!solve_recursive(grid, cands, stats.backtracks, true))
            throw std::logic_error("Puzzle has no solution.");
        stats.solved_by = stats.backtracks==0 ? "ac3-pure" : "ac3+backtracking";
    }

    stats.time_ms = std::chrono::duration<double,std::milli>(
        std::chrono::high_resolution_clock::now()-t0).count();
    return stats;
}

inline bool baseline_recursive(Grid& grid, int& backtrack_count)
{
    int   best_count = 10;
    Coord best_cell  = {-1,-1};

    for (int r=0;r<GRID_LEN;r++) {
        for (int c=0;c<GRID_LEN;c++) {
            if (!grid.is_empty({r,c})) continue;
            auto cv = grid.get_possible_values({r,c});
            if (cv.empty()) return false;
            if ((int)cv.size() < best_count) {
                best_count=(int)cv.size(); best_cell={r,c};
            }
        }
    }

    if (best_cell.first == -1) return true;

    for (int v : grid.get_possible_values(best_cell)) {
        grid.update(best_cell, v);
        if (baseline_recursive(grid, backtrack_count)) return true;
        ++backtrack_count;
        grid.update(best_cell, 0);
    }
    return false;
}

inline SolveStats solve_baseline(Grid& grid)
{
    SolveStats stats;
    stats.solved_by = "backtracking-only";
    auto t0 = std::chrono::high_resolution_clock::now();
    if (!baseline_recursive(grid, stats.backtracks))
        throw std::logic_error("Puzzle has no solution.");
    stats.time_ms = std::chrono::duration<double,std::milli>(
        std::chrono::high_resolution_clock::now()-t0).count();
    return stats;
}

}

#endif
