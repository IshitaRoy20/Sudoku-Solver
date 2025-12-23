
#ifndef TESTS_TEST_COORD_UTILS_H_
#define TESTS_TEST_COORD_UTILS_H_

#include <utility>
#include <cassert>
#include "../src/coord_utils.h"

void test_successive_cell_coords_are_generated_correctly()
{
    
    assert(
        sudoku::get_next_cell_coord(std::make_pair(0, 0)) == std::make_pair(0, 1));

    assert(
        sudoku::get_next_cell_coord(std::make_pair(0, 8)) == std::make_pair(1, 0));

    assert(
        sudoku::get_next_cell_coord(std::make_pair(8, 8)) == std::make_pair(8, 8));
}

#endif 