/**
 * @file rds_encoder.hpp
 * @brief RDS encoder (header file)
 * @author Adam Zvara (xzvara01@vutbr.cz)
 * @date 11/2024
 */

#pragma once

#include "src/argparse.hpp"
#include "src/group0A.hpp"
#include "src/group2A.hpp"

const char* help_string =
"Usage: rds_encoder -g <group_type> [-pi <value> -pty <value> -tp <value> -af <value> -ps <value> -rt <value> -ab <value>]\n \
Group types:\n \
\t0A - Basic tuning and switching information\n \
\t2A - Radio text\n \
\n \
Arguments required for both group types:\n \
\t-pi <value> - Program identification code (uint16) - both groups\n \
\t-pty <value> - Program type code (uint5) - both groups\n \
\t-tp <value> - Traffic program (bool) - both groups\n \
\n \
Arguments required for group 0A:\n \
\t-ms <value> - Music/speech (bool)\n \
\t-ta <value> - Traffic announcement (bool)\n \
\t-af <value> - Alternative frequencies (vector of uint16)\n \
\t-ps <value> - Program service name (string)\n \
\n \
Arguments required for group 2A:\n \
\t-rt <value> - Radio text (string)\n \
\t-ab <value> - Ab flag (bool)";

/**
 * @brief Parse arguments specific to group 0A
 *
 * @param argc Number of arguments
 * @param argv Array of arguments
 * @param p ArgParser object
 * @param[out] g0a Group 0A structure
 *
 * @throws std::invalid_argument if arguments cant be parsed (missing arguments, invalid values)
 */
void parse_g0A(int argc, char *argv[], ArgParser &p, struct RDS_group0A &g0a);

/**
 * @brief Parse arguments specific to group 2A
 *
 * @param argc Number of arguments
 * @param argv Array of arguments
 * @param p Parser object
 * @param[out] g2a Group 2A structure
 *
 * @throws std::invalid_argument if arguments cant be parsed (missing arguments, invalid values)
 */
void parse_g2A(int argc, char *argv[], ArgParser &p, struct RDS_group2A &g2a);

/**
 * @brief Encode group 0A structure into RDS message
 *
 * @param g0a Group 0A structure
 *
 * @return Encoded RDS message
 */
std::string RDS_g0A_encode(struct RDS_group0A &g0a);

/**
 * @brief Encode group 2A structure into RDS message
 *
 * @param g2a Group 2A structure
 *
 * @return Encoded RDS message
 */
std::string RDS_g2A_encode(struct RDS_group2A &g2a);