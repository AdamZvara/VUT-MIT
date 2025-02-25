/**
 * @file rds_decoder.cpp
 * @brief RDS decoder (source file)
 * @author Adam Zvara (xzvara01@vutbr.cz)
 * @date 11/2024
 */

#include <iostream>
#include <unordered_map>
#include <stdexcept>
#include "src/rds.hpp"
#include "rds_decoder.hpp"

std::string parse_args(int argc, char *argv[], ArgParser &p) {
    std::string blob;
    // Declare arguments
    p.add_argument("-b", [&](std::string val) { blob = val; });

    // Parse arguments
    p.parse_args(argc, argv);

    // Check if blob is is multiple of 104
    if (blob.size() % 104 != 0) {
        throw std::invalid_argument("Binary string is not multiple of 104");
    }

    // Check if blob only contains 0s and 1s
    if (blob.find_first_not_of("01") != std::string::npos) {
        throw std::invalid_argument("Binary string contains invalid characters");
    }

    return blob;
}

std::vector<RDS_chunk> split_blob(const std::string &blob) {
    // Split blob into 104-bit blocks
    std::vector<RDS_chunk> chunks;
    for (size_t i = 0; i < blob.size(); i += 104) {
        chunks.push_back(RDS_chunk(blob.substr(i, 104)));
    }
    return chunks;
}

void process_g0a(const std::vector<RDS_chunk> &chunks, struct RDS_group0A &g0a) {
    // Check if chunks do not exceed allowed length
    if (chunks.size() > 4) {
        throw std::invalid_argument("Too many chunks for group 2A");
    }

    // Process first block to get PI
    const RDS_data A = RDS_process_chunk(chunks[0]).at(BLOCK_A);
    g0a.shared.prog_identification = RDS_uint16_decode(A);

    // Fill variable length parameters with default values
    g0a.program_service.insert(g0a.program_service.begin(), 8, ' ');
    g0a.alternative_freq.insert(g0a.alternative_freq.begin(), 8, 0);

    uint8_t segment;

    // Process sequenced data
    for (const auto &chunk : chunks) {
        // Process chunk
        const auto &blocks = RDS_process_chunk(chunk);

        // Get offset and other group 0A infromation from block B
        segment = RDS_g0a_groupB_decode(blocks.at(BLOCK_B), g0a);

        // Process block C
        const auto &afs = RDS_2uint8_decode(blocks.at(BLOCK_C));
        g0a.alternative_freq[segment * 2] = afs.first;
        g0a.alternative_freq[segment * 2 + 1] = afs.second;

        // Process block D
        const auto &ps = RDS_2uint8_decode(blocks.at(BLOCK_D));
        g0a.program_service[segment * 2] = ps.first;
        g0a.program_service[segment * 2 + 1] = ps.second;
    }
}

void process_g2a(const std::vector<RDS_chunk> &chunks, struct RDS_group2A &g2a) {
    // Check if chunks do not exceed allowed length
    if (chunks.size() > 16) {
        throw std::invalid_argument("Too many chunks for group 2A\n");
    }

    // Process first block to get common information
    const RDS_data A = RDS_process_chunk(chunks[0]).at(BLOCK_A);
    g2a.shared.prog_identification = RDS_uint16_decode(A);

    // Initialize radio text with spaces
    g2a.radio_text.insert(g2a.radio_text.begin(), 64, ' ');

    uint8_t segment;

    // Process sequenced data
    for (const auto &chunk : chunks) {
        const auto &blocks = RDS_process_chunk(chunk);

        // Get offset from block B
        segment = RDS_g2a_groupB_decode(blocks.at(BLOCK_B), g2a);

        // Process block C and D and store results
        const auto &rt1 = RDS_2uint8_decode(blocks.at(BLOCK_C));
        const auto &rt2 = RDS_2uint8_decode(blocks.at(BLOCK_D));

        // Store results
        g2a.radio_text[segment * 4] = rt1.first;
        g2a.radio_text[segment * 4 + 1] = rt1.second;
        g2a.radio_text[segment * 4 + 2] = rt2.first;
        g2a.radio_text[segment * 4 + 3] = rt2.second;
    }
}

int main(int argc, char *argv[]) {
    ArgParser p;

    // Help string is processed separately before other arguments
    p.find_help_flag(argc, argv, help_string);

    // Parse arguments
    std::string blob;
    try {
        blob = parse_args(argc, argv, p);
    } catch (std::exception &e) {
        std::cerr << e.what() << "\n";
        return ERR_INVALID_ARG;
    }

    // Split blob into chunks
    std::vector<RDS_chunk> chunks = split_blob(blob);

    RDS_Group group_type;
    struct RDS_group0A g0a;
    struct RDS_group2A g2a;

    // Determine group type, call aproppriate processing function and print results
    try {
        group_type = RDS_get_message_type(chunks[0]);

        if (group_type == GROUP_0A) {
            process_g0a(chunks, g0a);
            std::cout << g0a;
        } else if (group_type == GROUP_2A) {
            process_g2a(chunks, g2a);
            std::cout << g2a;
        } else {
            throw std::invalid_argument("Group type could not be determined");
        }
    } catch (std::exception &e) {
        std::cerr << e.what() << "\n";
        return ERR_INVALID_CRC;
    }

    return 0;
}