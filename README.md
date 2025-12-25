🧩 Sudoku Solver (C++)

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

▶️ How to Run
Compile
g++ --std=c++17 main.cpp /g++ --std=c++17 sudokugenerator.cpp/ g++ --std=c++17 sudokusolver.cpp

Execute
./sudoku

🧠 How It Works

1. main.cpp--

      Accepts user input puzzle and solves it using backtracking
   
3. SudokuSolver.cpp--

      Generates and solves the sudoku puzzle

      Uses Backtracking to try valid numbers in empty cells.

      Recursively explores possibilities and backtracks on conflicts.

📌 Sample Output
Unsolved Sudoku Puzzle:
+-------+-------+-------+
| 0 0 0 | 0 0 0 | 6 8 0 |
| 0 0 0 | 0 7 3 | 0 0 9 |
| 3 0 9 | 0 0 0 | 0 4 5 |
+-------+-------+-------+
| 4 9 0 | 0 0 0 | 0 0 0 |
| 8 0 3 | 0 5 0 | 9 0 2 |
| 0 0 0 | 0 0 0 | 0 3 6 |
+-------+-------+-------+
| 9 6 0 | 0 0 0 | 3 0 8 |
| 7 0 0 | 6 8 0 | 0 0 0 |
| 0 2 8 | 0 0 0 | 0 0 0 |
+-------+-------+-------+
Solved Sudoku:
+-------+-------+-------+
| 1 4 2 | 9 3 5 | 6 7 8 |
| 5 8 7 | 1 2 6 | 3 4 9 |
| 6 3 9 | 4 7 8 | 1 2 5 |
+-------+-------+-------+
| 2 1 3 | 5 4 7 | 8 9 6 |
| 4 5 6 | 2 8 9 | 7 3 1 |
| 7 9 8 | 3 6 1 | 2 5 4 |
+-------+-------+-------+
| 3 2 1 | 6 5 4 | 9 8 7 |
| 8 6 4 | 7 9 2 | 5 1 3 |
| 9 7 5 | 8 1 3 | 4 6 2 |
+-------+-------+-------+

⏱️ Performance optimization for larger grids

🤝 Contributing

Contributions are welcome!
Feel free to fork this repository and submit a pull request.



