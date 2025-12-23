#include <iostream>
#include "src/grid.h"
#include "src/sudoku_solver.h"

using namespace std;

int main()
{
    try
    {
        sudoku::Grid grid;

        cout << "Enter your Sudoku puzzle row by row (use 0 for empty cells):\n";

        for (int row = 0; row < 9; ++row)
        {

            for (int col = 0; col < 9; ++col)
            {
                int value;
                cin >> value;
                if (value < 0 || value > 9)
                    throw invalid_argument("Numbers must be between 0 and 9.");
                grid.update({row, col}, value);
            }
        }

        cout << "\nUnsolved Sudoku Puzzle:\n";
        cout << grid << endl;

        sudoku::solve(&grid);

        cout << "Solved Sudoku Puzzle:\n";
        cout << grid << endl;
    }
    catch (const invalid_argument &e)
    {
        cerr << "Invalid input: " << e.what() << endl;
    }
    catch (const logic_error &e)
    {
        cerr << "Sudoku cannot be solved: " << e.what() << endl;
    }
    catch (...)
    {
        cerr << "An unexpected error occurred." << endl;
    }

    return 0;
}
