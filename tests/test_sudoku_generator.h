#ifndef TESTS_TEST_SUDOKU_GENERATOR_H_
#define TESTS_TEST_SUDOKU_GENERATOR_H_

#include"../src/sudoku_generator.h"

void test_puzzle_generated_is_solvable() {

    auto generated_puzzle = sudoku::generate_puzzle();
    solve(&generated_puzzle);

    assert(is_valid_solution(generated_puzzle) == true);
}

void test_no_two_successive_puzzles_generated_are_same() {
   
    auto first_generated_puzzle = sudoku::generate_puzzle();
    auto second_generated_puzzle = sudoku::generate_puzzle();

    assert((first_generated_puzzle == second_generated_puzzle) == false);
}

#endif
