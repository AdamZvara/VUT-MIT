/**
 * @file rds_decoder.hpp
 * @brief RDS decoder (header file)
 * @author Adam Zvara (xzvara01@vutbr.cz)
 * @date 11/2024
 */

#pragma once

#include <vector>
#include <bitset>

#include "src/argparse.hpp"
#include "src/group0A.hpp"
#include "src/group2A.hpp"

#define ERR_INVALID_CRC 2

const char* help_string ="Usage: rds_decoder -b <binary string> \n\nDecodes RDS message from binary string";

/**
 * @brief Parse arguments specific to group 0A
 *
 * @param argc Number of arguments
 * @param argv Array of arguments
 * @param p ArgParser object
 *
 * @throws std::invalid_argument if arguments cant be parsed (missing arguments, invalid values)
 */
std::string parse_args(int argc, char *argv[], ArgParser &p);

/**
 * @brief Split binary string into 104-bit chunks
 *
 * @param blob Binary string to be split
 *
 * @return Vector of 104-bit chunks
 */
std::vector<RDS_chunk> split_blob(const std::string &blob);

/**
 * @brief Process group 0A RDS message
 *
 * @param chunks Vector of 104-bit RDS message chunks
 * @param[out] g0a Group 0A structure to be filled
 *
 * @throws std::invalid_argument if message is too long
 */
void process_g0a(const std::vector<RDS_chunk> &chunks, struct RDS_group0A &g0a);

/**
 * @brief Process group 2A RDS message
 *
 * @param chunks Vector of 104-bit RDS message chunks
 * @param[out] g2a Group 2A structure to be filled
 *
 * @throws std::invalid_argument if message is too long
 */
void process_g2a(const std::vector<RDS_chunk> &chunks, struct RDS_group2A &g2a);
