/**
 * @file group2A.cpp
 * @brief RDS group type 2A specific functions
 * @author Adam Zvara (xzvara01@vutbr.cz)
 * @date 11/2024
 */

#include <iostream>
#include "group2A.hpp"

std::ostream& operator<<(std::ostream& os, const RDS_group2A& g) {
    // General fiels
    os << "PI: " << g.shared.prog_identification << "\n";
    os << "GT: " << "2A" << "\n";
    os << "TP: " << (int) g.shared.traffic_program << "\n";
    os << "PTY: " << (int) g.shared.prog_type << "\n";
    // Specific fields
    // Truncate trailing whitespaces
    std::string radio_text_cpy = g.radio_text;
    radio_text_cpy.erase(radio_text_cpy.find_last_not_of(" ") + 1);
    os << "A/B: " << (int) g.radio_flag << "\n";
    os << "RT: " << "\"" << radio_text_cpy << "\"" << "\n";
    return os;
}

RDS_data RDS_g2a_groupB_init(const struct RDS_group2A &g2a) {
    return 0b00100 << 11 | g2a.shared.traffic_program << 10 | g2a.shared.prog_type << 5 | g2a.radio_flag << 4;
}

RDS_block RDS_g2a_groupB_encode(RDS_data &data, const uint8_t segment) {
    assert (segment < 16 && "Segment must be in range <0, 15>");
    data &= 0b1111111111110000; // Clear segment bits before setting them
    data |= segment;
    return RDS_encode(data, BLOCK_B);
}

uint8_t RDS_g2a_groupB_decode(const RDS_data &data, struct RDS_group2A &g2a) {
    g2a.shared.traffic_program = data[10];
    g2a.shared.prog_type = (data >> 5).to_ulong() & 0b11111;
    g2a.radio_flag = data[4];
    
    return data.to_ulong() & 0b1111;
}