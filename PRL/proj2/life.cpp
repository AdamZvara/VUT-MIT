/**
 * @file life.cpp
 * @author Adam Zvara (xzvara01@stud.fit.vutbr.cz)
 * @brief Parallel game of life implemented using MPI
 * @date 04/2024
 *
 * @details
 * This version implements warp-around grid with square shaped and non-square shaped
 * grid with variable number of rows/columns (supporting odd number of rows/columns).
 * It is possible to run with number of processors <= number of rows, but it will
 * crash if the number of processors > the number of rows (therefore, only run with
 * the test.sh script, which selects min(processors, lines)). Each processor manages
 * from 1 to N rows, where N is the number of rows in the grid divided by the number
 * of processors (plus remaining rows, if the divison is not whole number).
 */

#include <iostream>
#include <fstream>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <cassert>
#include <mpi.h>

// Define the MPI types
#define COMM        MPI_COMM_WORLD
#define NO_TAG      0
#define PRINT_TAG   1


/** @brief Calculate `a` % `b`, also supports cases where -b < a < 0 */
inline int mod(int a, int b) {
    if (a < 0)
        return b + a;
    return a % b;
}

// Define types for single row of the grid and the whole grid
typedef std::vector<uint8_t> grid_row_t;
typedef std::vector<grid_row_t> grid_t;

/** @brief Class representing a single processor */
class Proc {
private:
    int rank;            // Rank of the processor
    int world_count;     // Number of processors
    uint32_t row_length; // Number of columns in each grid row
    uint32_t row_count;  // Number of rows in the grid
    grid_t grid;         // The grid configuration, each item represents a row managed by the processor
public:
    /** @brief Default constructor */
    Proc(int rank, int world_count) : rank(rank), world_count(world_count) {}

    /**
     * @brief Read the input grid configuration from given file
     * @param[in] filename File containing the grid configuration
    */
    void from_file(const char* filename) {
        // Open the file
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Cannot open the file with the grid configuration" << std::endl;
            MPI_Abort(COMM, 1);
        }

        // Get the grid configuration from the file
        uint lines_read = 0;
        uint32_t line_length = 0;
        for (std::string line; std::getline(file, line);) {
            // Remove whitespaces
            line.erase(remove_if(line.begin(), line.end(), isspace), line.end());
            if (line.size() == 0) {
                break;
            }

            // Store the length of the row from the first line, check if all lines have the same length
            if (lines_read == 0) {
                line_length = line.size();
            } else if (line.size() != line_length) {
                std::cerr << "The grid contains rows of different lengths" << std::endl;
                MPI_Abort(COMM, 1);
            }

            // Parse the line and store it in the grid
            grid.push_back(grid_row_t());
            for (char c : line) {
                if (!isspace(c)) {
                    grid[lines_read].push_back(c == '1' ? 1 : 0);
                }
            }
            lines_read++;
        }

        row_length = line_length;
        row_count = lines_read;
    }

    /** @brief Distribute the grid configuration from master to all processors */
    void distribute() {
        assert(rank == 0 && "Only the master processor can distribute the grid");

        // Broadcast the number of columns to all processors
        MPI_Bcast(&row_length, 1, MPI_UINT32_T, rank, COMM);

        // Calculate the number of rows each processor will get
        uint32_t rows_per_proc[world_count];
        std::fill(rows_per_proc, rows_per_proc + world_count, row_count / world_count);
        for (int i = 0; i < row_count % world_count; i++) {
            rows_per_proc[i]++;
        }

        // Send the number of rows to each processor
        MPI_Scatter(rows_per_proc, 1, MPI_UINT32_T, &row_count, 1, MPI_UINT32_T, rank, COMM);

        // Send the actual rows to each processor
        auto rows_iter = grid.begin();
        for (int i = 0; i < world_count; i++) {
            for (int j = 0; j < rows_per_proc[i]; j++) {
                MPI_Send(&(*rows_iter)[0], row_length, MPI_UINT8_T, i, NO_TAG, COMM);
                rows_iter++;
            }
        }

        // Keep only the rows that belong to the master processor
        grid.erase(grid.begin() + rows_per_proc[0], grid.end());
    }

    /** @brief Accept the grid configuration from the master processor */
    void accept() {
        assert(rank != 0 && "Only the slave processors can accept the grid");

        // Receive the number of columns
        MPI_Bcast(&row_length, 1, MPI_UINT32_T, 0, COMM);

        // Receive the number of rows managed by this processor
        MPI_Scatter(NULL, 0, MPI_UINT32_T, &row_count, 1, MPI_UINT32_T, 0, COMM);

        // Receive the actual rows managed by this processor
        grid_t accepted_rows;
        accepted_rows.resize(row_count);
        for (int i = 0; i < row_count; i++) {
            accepted_rows[i].resize(row_length);
            MPI_Recv(&accepted_rows[i][0], row_length, MPI_UINT8_T, 0, NO_TAG, COMM, MPI_STATUS_IGNORE);
        }
        grid = accepted_rows;
    }

    /** @brief Perform a single iteration of game of live with on current grid */
    void single_iteration() {
        // Send edges (first and last row) to neighbors
        MPI_Send(&grid.front()[0], row_length, MPI_UINT8_T, mod(rank - 1, world_count), NO_TAG, COMM);
        MPI_Send(&grid.back()[0], row_length, MPI_UINT8_T, mod(rank + 1, world_count), NO_TAG, COMM);

        // Receive edges (first line == up, last line == down) from neighbors
        grid_row_t up = grid_row_t();
        grid_row_t down = grid_row_t();
        up.resize(row_length);
        down.resize(row_length);
        MPI_Recv(&up[0], row_length, MPI_UINT8_T, mod(rank - 1, world_count), NO_TAG, COMM, MPI_STATUS_IGNORE);
        MPI_Recv(&down[0], row_length, MPI_UINT8_T, mod(rank + 1, world_count), NO_TAG, COMM, MPI_STATUS_IGNORE);

        // Expand the grid by adding the neighbors to top and bottom, so we can perform the iteration
        // The grid will look like this:
        //    grid[0] = UP NEIGHBOR
        //    grid[1] = ROWS OF THE MANAGED GRID
        //      ...
        //    grid[-1] = DOWN NEIGHBOR
        grid.insert(grid.begin(), up);
        grid.push_back(down);

        // Prepare new grid, which will be updated with the new row values
        grid_t new_grid;
        new_grid.resize(row_count);
        for (int i = 0; i < row_count; i++) {
            new_grid[i].resize(row_length);
        }

        // Perform the iteration for each row in original grid
        for (int i = 1; i <= row_count; i++) {
            // Create a temporary field with the currently processed row and its neighbors (3 lines in total)
            grid_t tmp_field(grid.begin() + (i - 1), grid.begin() + (i + 2));
            // Perform the iteration on the row and store the result
            single_row_iteration(tmp_field);
            // Only care about the middle row (index 1), which is the row we are updating
            new_grid[i-1] = tmp_field[1];
        }

        // Update the grid with the new rows
        grid = new_grid;
    }

    /**
     * @brief Perform a single iteration of game of live on a single row
     * @param[in] grid The grid with the row to be updated (must be of size 3)
    */
    void single_row_iteration(grid_t &grid) {
        // Create a new (result) row
        grid_row_t new_row = grid[1];

        // Iterate over the grid and update the new row
        for (uint i = 0; i < row_length; i++) {
            int neigh = cell_count_alive_neighbors(grid, i);
            if (grid[1][i] == 1) {
                if (neigh < 2 || neigh > 3) {
                    new_row[i] = 0;
                }
            } else {
                if (neigh == 3) {
                    new_row[i] = 1;
                }
            }
        }

        // Update the grid with the new row
        grid[1] = new_row;
    }

    /**
     * @brief Count the number of alive neighbors of a cell
     * @param[in] grid The grid with the cell and its neighbors (must be of size 3)
     * @param[in] col The column of the cell
     * @return The number of alive neighbors
    */
    int cell_count_alive_neighbors(const grid_t &grid, int col) {
        assert(grid.size() == 3 && "The grid must have 3 rows");

        int count = 0;
        // Handle edge cases of first and last col
        if (col == 0) {
            for (int i = 0; i < 3; i++) {
                count += grid[i][row_length-1];
            }
            for (int i = 0; i < 3; i++) {
                count += grid[i][col+1];
            }
        } else if (col == row_length - 1) {
            for (int i = 0; i < 3; i++) {
                count += grid[i][col-1];
            }
            for (int i = 0; i < 3; i++) {
                count += grid[i][0];
            }
        } else {
            for (int i = 0; i < 3; i++) {
                count += grid[i][col-1];
            }
            for (int i = 0; i < 3; i++) {
                count += grid[i][col+1];
            }
        }
        count += grid[0][col] + grid[2][col];
        return count;
    }

    /** @brief Print the grid configuration in order */
    void print_in_order() {
        // Starting from the first processor, print the managed grid and pass token to the next processor
        if (rank == 0) {
            bool print = true;
            std::cout << *this;
            MPI_Send(&print, 1, MPI_C_BOOL, 1, PRINT_TAG, COMM);
        } else {
            bool print = false;
            MPI_Recv(&print, 1, MPI_C_BOOL, rank - 1, PRINT_TAG, COMM, MPI_STATUS_IGNORE);
            if (print) {
                std::cout << *this;
                if (rank < world_count - 1) {
                    MPI_Send(&print, 1, MPI_C_BOOL, rank + 1, PRINT_TAG, COMM);
                }
            }
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const Proc& g) {
        for (int i = 0; i < g.grid.size(); i++) {
            os << g.rank << ": ";
            for (int j = 0; j < g.grid[i].size(); j++) {
                os << +g.grid[i][j]; // integer promotion to print the number as integer
            }
            os << std::endl;
        }
        return os;
    }
};

int main(int argc, char **argv) {
    // Check the number of arguments
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <input_file> <iterations>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    uint32_t all_iter = std::stoi(argv[2]);

    int world_rank;
    int world_count;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_count);

    Proc p = Proc(world_rank, world_count);

    if (world_rank == 0) {
        // Read the grid configuration and distribute it to other processors
        p.from_file(filename.c_str());
        p.distribute();
    } else {
        // Slaves accept the grid configuration from the master processor
        p.accept();
    }

    // Perform the iterations
    for (uint32_t current_iter = 0; current_iter < all_iter; current_iter++) {
        p.single_iteration();
    }

    // Print the final grid configuration
    p.print_in_order();

    MPI_Finalize();
    return 0;
}