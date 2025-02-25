/**
 * @file group2A.hpp
 * @brief RDS group type 2A specific functions (header file)
 * @author Adam Zvara (xzvara01@vutbr.cz)
 * @date 11/2024
 */

#pragma once

#include <string>
#include <cstdint>
#include <cassert>
#include "rds.hpp"

/** @brief Group 2A specific fields */
struct RDS_group2A {
    struct RDS_shared shared;
    std::string radio_text;
    bool radio_flag;

    friend std::ostream& operator<<(std::ostream& os, const RDS_group2A& g);
};

/**
 * @brief Initialize block B for 2A group containing radio parameters.
 * The block is initialized with shared fields and flags but does not contain segment number
 * nor CRC, which can be added using `RDS_g2a_groupB_encode` function.
 * 
 * @param g2a Group 2A data to be encoded
 * 
 * @return Initialized data block B for group 2A
 */
RDS_data RDS_g2a_groupB_init(const struct RDS_group2A &g2a);

/**
 * @brief Append segment to initialized group 2A block B and calculate CRC for it.
 * 
 * @param[out] data Block to be encoded
 * @param segment Segment number describing position of this block in the message
 * 
 * @return Encoded RDS block B for group 2A
 */
RDS_block RDS_g2a_groupB_encode(RDS_data &data, const uint8_t segment);

/**
 * @brief Decode group 2A block B into group2A structure.
 * 
 * @param data Block to be decoded
 * @param[out] g2a Group 2A data to be filled
 * 
 * @return Segment number found in the block
 */
uint8_t RDS_g2a_groupB_decode(const RDS_data &data, struct RDS_group2A &g2a);