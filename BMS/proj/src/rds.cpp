/**
 * @file rds.cpp
 * @brief Implementation of RDS block encoding and decoding
 * @author Adam Zvara (xzvara01@vutbr.cz)
 * @date 11/2024
 */

#include <stdexcept>
#include <vector>
#include <iostream>
#include "rds.hpp"

/** @brief Polynomial used for CRC calculation */
static constexpr uint32_t RDS_POLYNOMIAL = 0b10110111001;

/** @brief Block IDs used for CRC calculation (XORin with reminder) */
static const std::unordered_map<RDS_BlockID, RDS_CRC> BLOCK_IDS = {
    {BLOCK_A, 0b0011111100},
    {BLOCK_B, 0b0110011000},
    {BLOCK_C, 0b0101101000},
    {BLOCK_D, 0b0110110100}
};

RDS_CRC RDS_calculate_CRC(RDS_data data) {
    // Expand block to 26 bits, appending zeros
    RDS_block block_with_zeros = data.to_ulong() << 10;
    // Create a sliding window of 11 bits starting at the most significant bit
    std::bitset<11> window(block_with_zeros.to_ulong() >> 15);
    for (int i = 14; i >= -1; i--) {
        // Check if most significant bit is 1 (multiplication)
        if (window[10] == 1) {
            // XOR all bits with polynomial (division)
            window ^= RDS_POLYNOMIAL;
        }

        if (i >= 0) {
            // Shift window
            window <<= 1;
            // Add next bit
            window[0] = block_with_zeros[i];
        }
    }

    // Return only last 10 bits
    return RDS_CRC(window.to_ulong());
}

RDS_block RDS_encode(RDS_data data, RDS_BlockID id) {
    // Calculate CRC and XOR with block ID for given id
    RDS_CRC crc = RDS_calculate_CRC(data) ^ BLOCK_IDS.at(id);
    return RDS_block(data.to_ulong() << 10 | crc.to_ulong());
}

RDS_BlockID RDS_get_block_type(RDS_block block) {
    // Get data and CRC from block
    RDS_data received_data = (block >> 10).to_ulong();
    RDS_CRC received_crc = block.to_ulong();

    // Calculate CRC based on received data
    RDS_CRC crc = RDS_calculate_CRC(received_data);

    // Try XORing received CRC with block IDs to find out which block it is
    for (const auto& pair : BLOCK_IDS) {
        const auto& type = pair.first;
        const auto& id = pair.second;
        if ((received_crc ^ id) == crc) {
            return type;
        }
    }

    // No block found
    return BLOCK_INVALID;
}

RDS_block RDS_uint16_encode(uint16_t val, RDS_BlockID id) {
    return RDS_encode(val, id);
}

uint16_t RDS_uint16_decode(RDS_data block) {
    return block.to_ulong();
}

RDS_block RDS_2uint8_encode(uint8_t val1, uint8_t val2, RDS_BlockID id) {
    // Combine two 8-bit values into one 16-bit value
    uint16_t val = (val1 << 8) | val2;
    return RDS_encode(val, id);
}

std::pair<uint8_t, uint8_t> RDS_2uint8_decode(RDS_data data) {
    // Get values from 16-bit data
    uint16_t val = data.to_ulong();
    return std::make_pair((val >> 8) & 0xFF, val & 0xFF);
}

std::unordered_map<RDS_BlockID, RDS_data> RDS_process_chunk(RDS_chunk chunk) {
    std::vector<RDS_block> blocks;
    std::unordered_map<RDS_BlockID, RDS_data> decoded_blocks;
    
    // Convert 104-bit chunk to 26-bit blocks
    for (size_t i = 0; i < 104; i += 26) {
        blocks.push_back(RDS_block(chunk.to_string().substr(i, 26)));
    }

    // Decode blocks and store them in a map
    for (const auto &block : blocks) {
        RDS_block b;

        // Find out which block type it is
        RDS_BlockID type = RDS_get_block_type(block);
        if (type == BLOCK_INVALID) {
            throw std::runtime_error("Invalid CRC");
        }

        // Check if it was not already used
        if (decoded_blocks.find(type) != decoded_blocks.end()) {
            throw std::runtime_error("Same block type used multiple times in a message");
        }

        // Get only data from block
        RDS_data data = (block >> 10).to_ulong();

        // Store block
        decoded_blocks[type] = data;
    }

    return decoded_blocks;
}

RDS_Group RDS_get_message_type(RDS_chunk chunk) {
    // Process chunk
    const auto &blocks = RDS_process_chunk(chunk);

    // Check if block B is present
    if (blocks.find(BLOCK_B) == blocks.end()) {
        throw std::invalid_argument("Block B not found");
    }

    // Return block type based on information in block B
    const RDS_data B = blocks.at(BLOCK_B);

    // Check if it is group 0A
    if ((B >> 12).to_ulong() == 0b0000) {
        return GROUP_0A;
    } else if ((B >> 12).to_ulong() == 0b0010) {
        return GROUP_2A;
    } else {
        throw std::invalid_argument("Invalid group type");
    }
}