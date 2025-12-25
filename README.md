#🧩 Sudoku Solver (C++)

A C++ based Sudoku Solver and Generator that can automatically generate valid Sudoku puzzles and solve them using backtracking algorithms. The project demonstrates efficient problem-solving, recursion, and clean modular design in C++.

🚀 Features

✔️ Generates valid 9×9 Sudoku puzzles

✔️ Solves Sudoku puzzles using Backtracking

✔️ Ensures all Sudoku constraints (row, column, subgrid)

✔️ Modular and extensible C++ codebase

✔️ Fast and memory-efficient solution

🛠️ Tech Stack

Language: C++ (C++17 standard)

Concepts Used:

Backtracking

Recursion

Object-Oriented Programming

STL (vector, array, set)

Compiler: g++

📂 Project Structure
Sudoku-Solver/
│
├── src/
│   ├── sudoku_solver.cpp      # Solving logic
│   ├── sudoku_generator.cpp   # Puzzle generation
│   ├── coord.cpp              # Coordinate utilities
│
├── include/
│   ├── sudoku_solver.h
│   ├── sudoku_generator.h
│   ├── coord.h
│
├── main.cpp                   # Entry point
└── README.md

▶️ How to Run
Compile
g++ --std=c++17 main.cpp /g++ --std=c++17 sudokugenerator.cpp/ g++ --std=c++17 sudokusolver.cpp

Execute
./sudoku

🧠 How It Works
1.main.cpp--

Accepts user input puzzle and solves it using backtracking

2.Sudokugenerator.cpp--

Puzzle Generation

Creates a complete valid Sudoku grid.

Removes numbers while maintaining Sudoku validity.

3.SudokuSolver.cpp--

Generates and solves the sudoku puzzle

Uses Backtracking to try valid numbers in empty cells.

Recursively explores possibilities and backtracks on conflicts.

📌 Sample Output
Generated Sudoku Puzzle:
5 3 . . 7 . . . .
6 . . 1 9 5 . . .
. 9 8 . . . . 6 .

Solved Sudoku:
5 3 4 6 7 8 9 1 2
6 7 2 1 9 5 3 4 8
1 9 8 3 4 2 5 6 7
...
⏱️ Performance optimization for larger grids

🤝 Contributing

Contributions are welcome!
Feel free to fork this repository and submit a pull request.
