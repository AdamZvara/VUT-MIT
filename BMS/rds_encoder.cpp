/**
 * @file rds_encoder.cpp
 * @brief RDS encoder (source file)
 * @author Adam Zvara (xzvara01@vutbr.cz)
 * @date 11/2024
 */

#include <iostream>
#include "rds_encoder.hpp"
#include "src/rds.hpp"

void parse_g0A(int argc, char *argv[], ArgParser &p, struct RDS_group0A &g0a) {
    // Group argument is processed separately, we just declare it so we can skip it
    p.add_argument("-g",   [&](std::string val) { return val; });
    // Declare other arguments
    p.add_argument("-pi",  [&](uint16_t val) { g0a.shared.prog_identification = val; }, UINT16_MAX);
    p.add_argument("-pty", [&](uint8_t val) { g0a.shared.prog_type = val; }, 0b11111);
    p.add_argument("-tp",  [&](bool val) { g0a.shared.traffic_program = val; });
    p.add_argument("-ms",  [&](bool val) { g0a.music_speech = val; });
    p.add_argument("-ta",  [&](bool val) { g0a.traffic_announce = val; });
    p.add_argument("-af",  [&](std::vector<uint16_t> val) { g0a.alternative_freq = val; });
    p.add_argument("-ps",  [&](std::string val) { g0a.program_service = val; });

    // Parse arguments
    p.parse_args(argc, argv);

    // Check if ps is in range
    if (g0a.program_service.size() > 8) {
        throw std::invalid_argument("PS string is too long");
    }

    // Check if af is in range
    if (g0a.alternative_freq.size() > 8) {
        throw std::invalid_argument("AF vector is too long");
    }
}

void parse_g2A(int argc, char *argv[], ArgParser &p, struct RDS_group2A &g2a) {
    // Group argument is processed separately, we just declare it so we can skip it
    p.add_argument("-g",   [&](std::string val) { return val; });
    // Declare other arguments
    p.add_argument("-pi",  [&](uint16_t val) { g2a.shared.prog_identification = val; }, UINT16_MAX);
    p.add_argument("-pty", [&](uint8_t val) { g2a.shared.prog_type = val; }, 0b11111);
    p.add_argument("-tp",  [&](bool val) { g2a.shared.traffic_program = val; });
    p.add_argument("-rt",  [&](std::string val) { g2a.radio_text = val; });
    p.add_argument("-ab",  [&](bool val) { g2a.radio_flag = val; });

    // Parse arguments
    p.parse_args(argc, argv);

    // Check if rt is in range
    if (g2a.radio_text.size() > 64) {
        throw std::invalid_argument("Radio text string is too long");
    }
}

std::string RDS_g0A_encode(struct RDS_group0A &g0a) {
    // Encode block A before iterating since its the same for all segments
    RDS_block A = RDS_uint16_encode(g0a.shared.prog_identification, RDS_BlockID::BLOCK_A);

    // Initialize block B with shared fields among all segments
    RDS_data B_data = RDS_g0a_groupB_init(g0a);

    // Expand variable length fields (AF and PS) so its easier to work with them
    if (g0a.program_service.size() < 8) {
        g0a.program_service.append(8 - g0a.program_service.size(), ' ');
    }

    if (g0a.alternative_freq.size() < 8) {
        // Set new elements to 875 (default value) - will become 0 after conversion
        g0a.alternative_freq.resize(8, 0);
    }

    // Segment number and result string
    uint8_t segment = 0;
    std::string result;

    // Iterate through two PS characters at a time
    for (size_t i = 0; i < g0a.program_service.size(); i += 2) {
        // Append encoded block A
        result.append(A.to_string());

        // Add segment number to block B and append it
        RDS_block B = RDS_g0a_groupB_encode(B_data, segment);
        result.append(B.to_string());

        // Encode block C - convert alternative frequencies to integers
        uint16_t af1 = g0a.alternative_freq[i];
        uint16_t af2 = g0a.alternative_freq[i + 1];
        RDS_block C = RDS_2uint8_encode(af1, af2, RDS_BlockID::BLOCK_C);
        result.append(C.to_string());

        // Encode block D
        RDS_block D = RDS_2uint8_encode(g0a.program_service[i], g0a.program_service[i + 1], RDS_BlockID::BLOCK_D);
        result.append(D.to_string());

        segment++;
    }

    return result;
}

std::string RDS_g2A_encode(struct RDS_group2A &g2a) {
    // Encode block A before iterating since its the same for all segments
    RDS_block A = RDS_uint16_encode(g2a.shared.prog_identification, BLOCK_A);
    
    // Initialize block B with shared fields among all segments
    RDS_data B_data = RDS_g2a_groupB_init(g2a);
    
    // Expand variable length fields (RT) so its easier to work with them
    if (g2a.radio_text.size() < 64) {
        g2a.radio_text.append(64 - g2a.radio_text.size(), ' ');
    }

    // Segment number and result string
    uint8_t segment = 0;
    std::string result;

    // Iterate over radio text in 4 character blocks
    for (size_t i = 0; i < g2a.radio_text.size(); i += 4) {
        // Append encoded block A
        result.append(A.to_string());

        // Add segment number to block B and append it
        RDS_block B = RDS_g2a_groupB_encode(B_data, segment);
        result.append(B.to_string());

        // Encode block C and append it
        RDS_block C = RDS_2uint8_encode(g2a.radio_text[i], g2a.radio_text[i + 1], BLOCK_C);
        result.append(C.to_string());

        // Encode block D a
        RDS_block D = RDS_2uint8_encode(g2a.radio_text[i + 2], g2a.radio_text[i + 3], BLOCK_D);
        result.append(D.to_string());

        segment++;
    }

    return result;
}

int main(int argc, char *argv[]) {
    ArgParser p;

    // Help string is processed separately before other arguments
    p.find_help_flag(argc, argv, help_string);

    std::string group_type;

    // Determine group type
    try {
        group_type = p.find_arg_value("-g", argc, argv);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        exit(ERR_INVALID_ARG);
    }

    // Parse group arguments and create messages
    try {
        if (group_type == "0A") {
            struct RDS_group0A g0a;
            parse_g0A(argc, argv, p, g0a);
            std::cout << RDS_g0A_encode(g0a) << "\n";
        } else if (group_type == "2A") {
            struct RDS_group2A g2a;
            parse_g2A(argc, argv, p, g2a);
            std::cout << RDS_g2A_encode(g2a) << "\n";
        } else if (group_type == "") {
            throw std::invalid_argument("Group type not specified");
        } else {
            throw std::invalid_argument("Invalid group type " + group_type);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        exit(ERR_INVALID_ARG);
    }
}