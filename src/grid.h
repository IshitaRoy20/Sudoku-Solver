
#ifndef SRC_GRID_H_
#define SRC_GRID_H_

#include <array>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "../src/coord.h"

namespace sudoku
{

    class Grid
    {
    private:
        std::array<std::array<int, 9>, 9> grid;
        std::set<Coord> coords_that_were_pre_filled;

    public:
        Grid()
        {
            std::array<int, 9> filled_array = {0, 0, 0, 0, 0, 0, 0, 0, 0};
            grid.fill(filled_array);
        }

        explicit Grid(std::array<std::array<int, 9>, 9> grid)
        {
            set_initial_state(grid);
        }



        void set_initial_state(std::array<std::array<int, 9>, 9> grid)
        {
            coords_that_were_pre_filled.clear();

            for (int row = 0; row < GRID_LEN; row++)
            {
                for (int col = 0; col < GRID_LEN; col++)
                {
                    int cell_value = grid.at(row).at(col);

                    if (cell_value < 0 || cell_value > 9)
                    {
                        throw std::invalid_argument(
                            "Cell value should be in range 0 <= x <= 9");
                    }

                    this->grid.at(row).at(col) = cell_value;

                    if (cell_value != 0)
                    {
                        coords_that_were_pre_filled.insert({row, col});
                    }
                }
            }
        }

        void set_initial_state_from_file(const std::string &filename)
        {
            std::ifstream file(filename);
            if (file.fail())
            {
                throw std::invalid_argument("The file couldn't be opened.");
            }

            std::array<std::array<int, 9>, 9> temp_grid{};

            for (int i = 0; i < GRID_LEN; i++)
            {
                for (int j = 0; j < GRID_LEN; j++)
                {
                    if (!(file >> temp_grid[i][j]))
                    {
                        throw std::invalid_argument("Invalid Sudoku file.");
                    }
                }
            }

            set_initial_state(temp_grid);
        }

        void update(Coord coord, int value)
        {
            if (value < 0 || value > 9)
            {
                throw std::invalid_argument("Cell value should be in range 0–9");
            }
            grid.at(coord.first).at(coord.second) = value;
        }

        int get(Coord coord) const
        {
            return grid.at(coord.first).at(coord.second);
        }

        void clear_values_starting_from_coord(Coord coord)
        {
            for (int row = coord.first; row < GRID_LEN; row++)
            {
                for (int col = coord.second; col < GRID_LEN; col++)
                {
                    Coord current = {row, col};
                    if (coords_that_were_pre_filled.find(current) == coords_that_were_pre_filled.end())
                    {
                        grid[row][col] = 0;
                    }
                }
            }
        }



        bool value_exists_elsewhere_in_column(Coord coord, int value) const
        {
            return std::any_of(
                grid.begin(), grid.end(),
                [&](const std::array<int, 9> &row)
                {
                    return row[coord.second] == value;
                });
        }

        bool value_exists_elsewhere_in_row(Coord coord, int value) const
        {
            return std::any_of(
                grid[coord.first].begin(),
                grid[coord.first].end(),
                [&](int cell)
                { return cell == value; });
        }

        bool value_exists_elsewhere_in_3x3_grid(Coord coord, int value) const
        {
            int rs = (coord.first / 3) * 3;
            int cs = (coord.second / 3) * 3;

            for (int r = rs; r < rs + 3; r++)
            {
                for (int c = cs; c < cs + 3; c++)
                {
                    if ((r != coord.first || c != coord.second) &&
                        grid[r][c] == value)
                    {
                        return true;
                    }
                }
            }
            return false;
        }

        bool coord_was_pre_filled(Coord coord) const
        {
            return coords_that_were_pre_filled.find(coord) != coords_that_were_pre_filled.end();
        }

        std::vector<int> get_possible_values_for_cell_at_coord(Coord coord)
        {
            std::vector<int> result;

            for (int v = 1; v <= 9; v++)
            {
                if (!value_exists_elsewhere_in_column(coord, v) &&
                    !value_exists_elsewhere_in_row(coord, v) &&
                    !value_exists_elsewhere_in_3x3_grid(coord, v))
                {
                    result.push_back(v);
                }
            }
            return result;
        }

        bool operator==(const Grid &other) const
        {
            return grid == other.grid;
        }

        friend std::ostream &operator<<(std::ostream &out, const Grid &grid);
    };

    std::ostream &operator<<(std::ostream &out, const Grid &grid)
    {
        out << "+-------+-------+-------+\n";
        for (int r = 0; r < GRID_LEN; r++)
        {
            out << "|";
            for (int c = 0; c < GRID_LEN; c++)
            {
                out << " " << grid.grid[r][c];
                if (c % 3 == 2)
                    out << " |";
            }
            out << "\n";
            if (r % 3 == 2)
                out << "+-------+-------+-------+\n";
        }
        return out;
    }

} 
#endif 