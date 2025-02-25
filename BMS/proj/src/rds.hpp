/**
 * @file rds.hpp
 * @brief Definitions for types and structures used in RDS encoding and decoding (header file)
 * @author Adam Zvara (xzvara01@vutbr.cz)
 * @date 11/2024
 */

#pragma once

#include <cstdint>
#include <bitset>
#include <unordered_map>

// Type definitions for RDS messages
typedef std::bitset<104> RDS_chunk; // 104-bit RDS message chunk consisting of 4 blocks
typedef std::bitset<26>  RDS_block; // 26-bit RDS block consisting of 16-bit data and 10-bit CRC
typedef std::bitset<16>  RDS_data;  // 16-bit RDS data
typedef std::bitset<10>  RDS_CRC;   // 10-bit RDS CRC

/** @brief Shared RDS fields */
struct RDS_shared {
    uint16_t prog_identification;
    uint8_t prog_type;
    bool traffic_program;
};

/** @brief RDS groups */
enum RDS_Group { GROUP_0A, GROUP_2A };

/** @brief RDS block IDs */
enum RDS_BlockID { BLOCK_INVALID = -1, BLOCK_A, BLOCK_B, BLOCK_C, BLOCK_D };

/**
 * @brief Calculate CRC for given RDS data
 * 
 * @param data RDS data to calculate CRC for
 * 
 * @return Calculated CRC
 */
RDS_CRC RDS_calculate_CRC(RDS_data data);

/**
 * @brief Encode RDS block with given type
 * 
 * Encoding means calculating CRC, which is XORed with block ID and the result
 * is appended to the `data` resulting in 26-bit block. 
 * 
 * @param data Data to be encoded
 * @param id Blok ID to be used during encoding
 * 
 * @return Encoded RDS block
 */
RDS_block RDS_encode(RDS_data data, RDS_BlockID id);

/**
 * @brief Deduce type of the RDS block (26-bit)
 * 
 * This function calculates CRC for the block and tries to XOR it with block IDs
 * to find out which block it is. If the CRC does not match any block ID, the block
 * is considered invalid.
 * 
 * @param block RDS block to be decoded
 * 
 * @return Type of the decoded block
 */
RDS_BlockID RDS_get_block_type(RDS_block block);

/**
 * @brief Deduce type of the RDS message from a chunk (104-bit)
 * 
 * The difference between this function and `RDS_get_block_type` is that this function
 * processes all blocks in the chunk and finds the `BLOCK_B` block of the message 
 * (which could be out of order). If the chunk contains invalid blocks, the message is considered invalid.
 * 
 * @param chunk RDS chunk to be decoded
 * 
 * @return Type of the decoded message
 */
RDS_Group RDS_get_message_type(RDS_chunk chunk);

/**
 * @brief Encode (create CRC and append) 16-bit value into RDS block with given block id
 * 
 * @param val 16-bit value to be encoded
 * @param id Block ID
 * 
 * @return Encoded RDS block
 */
RDS_block RDS_uint16_encode(uint16_t val, RDS_BlockID id);

/**
 * @brief Encode two 8-bit values into RDS block with given block id
 * 
 * @param val1 First 8-bit value
 * @param val2 Second 8-bit value
 * @param id Block ID
 * 
 * @return Encoded RDS block
 */
RDS_block RDS_2uint8_encode(uint8_t val1, uint8_t val2, RDS_BlockID id);
 
/**
 * @brief Decode 16-bit value from RDS block
 * 
 * @param block RDS block to be decoded
 * 
 * @return Decoded 16-bit value
 */
uint16_t RDS_uint16_decode(RDS_data data);

/**
 * @brief Decode two 8-bit values from RDS block
 * 
 * @param data RDS block to be decoded
 * 
 * @return Pair of decoded 8-bit values
 */
std::pair<uint8_t, uint8_t> RDS_2uint8_decode(RDS_data data);

/**
 * @brief Process RDS chunk (104-bit) and decode it to separate blocks (26-bit)
 * 
 * The blocks are stored in a map with block ID as a key. When each block is decoded,
 * the CRC is also checked to ensure the block is valid. If the CRC does not match,
 * the block is considered invalid and an exception is thrown. The same applies to 
 * when the same block type is used multiple times in a chunk.
 * 
 * @param chunk RDS chunk to be processed
 * 
 * @return Map of decoded blocks
 */
std::unordered_map<RDS_BlockID, RDS_data> RDS_process_chunk(RDS_chunk chunk);
