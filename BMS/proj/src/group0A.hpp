/**
 * @file group0A.hpp
 * @brief RDS block type 0A (header file)
 * @author Adam Zvara (xzvara01@vutbr.cz)
 * @date 11/2024
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <cassert>
#include "rds.hpp"

/** @brief Group 0A specific fields. */
struct RDS_group0A {
    struct RDS_shared shared;
    bool music_speech;
    bool traffic_announce;
    std::vector<uint16_t> alternative_freq;
    std::string program_service;
    uint8_t di = 0;

    friend std::ostream& operator<<(std::ostream& os, const RDS_group0A& g);
};

/**
 * @brief Initialize block B for 0A group containing radio parameters.
 * The block is initialized with shared fields and flags but does not contain segment number
 * nor CRC, which can be added using `RDS_g0a_groupB_encode` function.
 * 
 * @param g0a Group 0A data to be encoded
 */
RDS_data RDS_g0a_groupB_init(const struct RDS_group0A &g0a);

/**
 * @brief Append segment to initialized group 0A block B and calculate CRC for it.
 * 
 * @param[out] data Block to be encoded
 * @param segment Segment number describing position of this block in the message
 * 
 * @return Encoded RDS block B for group 0A
 */
RDS_block RDS_g0a_groupB_encode(RDS_data &data, const uint8_t segment);

/**
 * @brief Decode group 0A block B.
 * 
 * @param data Block to be decoded
 * @param[out] g0a Group 0A data to be filled
 * 
 * @return Segment number  found in the block
 */
uint8_t RDS_g0a_groupB_decode(const RDS_data &data, struct RDS_group0A &g0a);