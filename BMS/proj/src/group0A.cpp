/**
 * @file group0A.cpp
 * @brief RDS block type 0A
 * @author Adam Zvara (xzvara01@vutbr.cz)
 * @date 11/2024
 */

#include <bitset>
#include <iostream>
#include "group0A.hpp"

std::ostream& operator<<(std::ostream& os, const RDS_group0A& g) {
    // General fiels
    os << "PI: " << g.shared.prog_identification << "\n";
    os << "GT: " << "0A" << "\n";
    os << "TP: " << (int) g.shared.traffic_program << "\n";
    os << "PTY: " << (int) g.shared.prog_type << "\n";
    // Specific fields
    const char* tatext = g.traffic_announce ? "Active" : "Inactive";
    os << "TA: " << tatext << "\n";
    const char* mstext = g.music_speech ? "Music" : "Speech";
    os << "MS: " << mstext << "\n";
    os << "DI: " << (int) g.di << "\n";
    os << "AF: ";
    int freq_written = 0;
    for (auto af : g.alternative_freq) {
        // AF can be 0 if it is padded during encoding
        if (af == 0) {
            continue;
        }
        if (freq_written != 0) {
            os << ", ";
        }
        af += 875;
        // Put dot before last digit
        std::string afv = std::to_string(af);
        afv.insert(afv.size() - 1, ".");
        os << afv;
        freq_written++;
    }
    os << "\n";
    // Truncate trailing whitespaces
    std::string ps_cpy = g.program_service;
    ps_cpy.erase(ps_cpy.find_last_not_of(" ") + 1);
    os << "PS: " << "\"" << ps_cpy << "\"" << "\n";
    return os;
}

RDS_data RDS_g0a_groupB_init(const struct RDS_group0A &g0a) {
    return g0a.shared.traffic_program << 10 | g0a.shared.prog_type << 5 | g0a.traffic_announce << 4 | g0a.music_speech << 3 | g0a.di << 2;
}

RDS_block RDS_g0a_groupB_encode(RDS_data &data, const uint8_t segment) {
    assert (segment < 4 && "Segment must be in range <0, 3>");
    data &= 0b1111111111111100; // Clear segment bits before setting them
    data |= segment;
    return RDS_encode(data, BLOCK_B);
}

uint8_t RDS_g0a_groupB_decode(const RDS_data &data, struct RDS_group0A &g0a) {
    g0a.shared.traffic_program = data[10];
    g0a.shared.prog_type = (data >> 5).to_ulong() & 0b11111;
    g0a.traffic_announce = data[4];
    g0a.music_speech = data[3];
    g0a.di = data[2];

    return data.to_ulong() & 0b11;
}