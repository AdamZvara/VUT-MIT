/**
 * @file pms.cpp
 * @author Adam Zvara (xzvara01@stud.fit.vutbr.cz)
 * @brief Parallel merge sort using MPI
 * @date 2024-02-25
 */

#include <iostream>
#include <fstream>
#include <cmath>
#include <queue>
#include <cstdint>
#include <mpi.h>

#define FILENAME    "numbers"
#define COMM        MPI_COMM_WORLD
#define MASTER      0
#define NOTAG       0
#define MPI_EOF     1   // Tag for the end of file signal

/** 
 * @brief Open the file with the numbers
 * 
 * @param[in] filename Name of the file to open
*/
inline std::ifstream open_file(const char* filename) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        MPI_Abort(COMM, 1);
    }

    return file;
}

/** 
 * @brief Read the next number from the file
 * 
 * @param[in] file File stream to read from
 * @param[in] max_cnt Maximum number of numbers to read
 * 
 * @warning If the input size (number of input integers) exceeds the the maximum theorietical value of 
 * 2^(world_count-1), the program will truncate the input to the maximum possible value
*/
inline int read_number(std::ifstream &file, uint32_t max_cnt) {
    static uint32_t nums_read = 0;

    // Check if the maximum number of numbers has been read
    if (++nums_read > max_cnt) {
        return EOF;
    }

    return file.get();
}

/**
 * @brief Send number from processor with given rank to the next (rank+1) processor
 * 
 * @param[in] rank Rank of the source processor
 * @param[in] num Number to send
 */
inline void send_msg(int rank, int num) {
    MPI_Send(&num, 1, MPI_INT, rank + 1, NOTAG, COMM);
}

/**
 * @brief Send the end of file signal to the next processor
 * 
 * @param[in] rank Rank of the source processor
 */
inline void send_eof(int rank) {
    MPI_Send(&rank, 1, MPI_INT, rank + 1, MPI_EOF, COMM);
}

/**
 * @brief Read the message from the previous processor
 * 
 * @param[in] rank Rank of the current processor
 * @return The number received or EOF if the end of file signal was received
 */
inline int read_msg(int rank) {
    MPI_Status s;
    int number;
    int prev_proc = rank - 1;

    // Grab the message, extract tag and number
    MPI_Recv(&number, 1, MPI_INT, prev_proc, MPI_ANY_TAG, COMM, &s);

    // Check if the message is the end of file signal
    if (s.MPI_TAG == MPI_EOF) {
        return EOF;
    }

    return number;
}

/**
 * @brief Master processor reads the numbers from the file and sends them to the next processor
 * 
 * @param[in] world_count Number of processors
 */
void master_process_input(int world_count) {
    // Calculate maximum possible count of numbers based on the number of processors
    uint32_t max_num_cnt = std::pow(2, world_count - 1); 

    int num;
    std::ifstream file = open_file(FILENAME);

    while ((num = read_number(file, max_num_cnt)) != EOF) {
        std::cout << num << " ";
        send_msg(MASTER, num); // Send the number to the next process
    }


    std::cout << std::endl;

    // Send the signal to stop reading
    send_eof(MASTER);

    file.close();
}

/**
 * @brief Processor class that reads the numbers from the previous processor, sorts them 
 * and sends them to the next processor
 */
struct processor {
public:
    int rank;
    bool eof = false;   // Signal to stop reading from the previous processor

    processor(int rank, void (*outfun)(int, int)) : rank(rank), outfun(outfun) {}

    /** * @brief Read the message from the previous processor and put it in correct queue */
    void parse_msg();

    /** * @brief Check if the processor has enough numbers to start sorting */
    bool is_mergeable() { return q1.size() == batch_size / 2 && q2.size() > 1; }

    /** * @brief Merge the numbers from the two queues and send them to the next processor */
    void merge();

    /** * @brief Finish the sorting and send the signal to the next processor to stop reading */
    void end(int world_count);

private:
    uint32_t nums_received = 0;
    
    std::queue<int> q1;
    std::queue<int> q2;
    uint32_t queue_switch = std::pow(2, rank - 1); // # of numbers to read before switching queues

    uint32_t batch_size = std::pow(2, rank); // Size of the batch for processor
    uint32_t q1_used = 0;    // # of numbers used from the first queue in current batch
    uint32_t q2_used = 0;    // # of numbers used from the second queue in current batch
    void (*outfun)(int, int); // Function applied after merging the numbers

    void send_from_queue(std::queue<int> &q, uint32_t &used) {
        outfun(rank, q.front());
        q.pop();
        used++;
    }
};

void processor::parse_msg() {
    int number;
    if ((number = read_msg(rank)) != EOF) {
        nums_received++;

        // Push the number in the correct queue
        if (nums_received <= queue_switch) {
            q1.push(number);
        } else {
            q2.push(number);
        }

        if (nums_received == 2 * queue_switch) {
            nums_received = 0;
        }
    } else {
        // EOF reached
        eof = true;
    }
}

void processor::merge() {
    // If we have taken all the numbers available from the first batch
    if (q1_used == batch_size / 2) {
        // Only take number from the second batch
        send_from_queue(q2, q2_used);
    } else if (q2_used == batch_size / 2) {
        // Only take number from the first batch
        send_from_queue(q1, q1_used);
    } else {
        // If only one of the queues is empty, take the number from the other
        if (q1.empty()) {
            send_from_queue(q2, q2_used);
        } else if (q2.empty()) {
            send_from_queue(q1, q1_used);
        } else {
            // Take the smallest number from the two batches
            if (q1.front() < q2.front()) {
                send_from_queue(q1, q1_used);
            } else {
                send_from_queue(q2, q2_used);
            }
        }
    }

    // All numbers from current batch have been used, reset the counters
    if (q1_used + q2_used == batch_size) {
        q1_used = q2_used = 0;
    }
}

inline void processor::end(int world_count) {
    // If no more numbers are coming, merge the remaining numbers
    while (!q1.empty() || !q2.empty()) {
        merge();
    }

    // Send the signal to the next processor to stop reading
    if (rank != world_count - 1) {
        send_eof(rank);
    }
}

/** @brief This function is used by the last processor to print the numbers instead of passing them 
 * to the next processor */
inline void print_number(int _, int num) {
    std::cout << num << std::endl;
}

int main(int argc, char **argv) {
    // Initialize MPI
    int world_rank;
    int world_count;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_count);

    if (world_rank == MASTER) {
        master_process_input(world_count);
    } else {
        int number; 

        // The last processor will print the numbers instead of passing them to the next processor
        struct processor proc(world_rank, (world_rank != world_count - 1) ? send_msg : print_number);

        while (!proc.eof) {
            // Read number from prev process, put it in the correct queue
            proc.parse_msg();

            // Sort if the processor has enough numbers
            if (proc.is_mergeable()) {
                proc.merge();
            }
        }

        // Finish the sorting and send the signal to the next processor to stop reading
        proc.end(world_count);
    }

    // Finalize MPI
    MPI_Finalize();
    return 0;
}