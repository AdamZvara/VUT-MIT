/**
 * @file argparse.cpp
 * @brief Parsing of command line arguments (source file)
 * @author Adam Zvara (xzvara01@vutbr.cz)
 * @date 11/2024
 */

#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <sstream>
#include "argparse.hpp"

/** @brief Convert `value` stored as string to integer and check if it is in range. */
uint16_t str2int_range(const std::string &value, const std::string& flag, const int upper_range, const int lower_range = 0) {
    int parsed;

    try {
        parsed = std::stoi(value);
    } catch (const std::invalid_argument& e) {
        throw std::invalid_argument("Invalid value for " + flag + ": " + value);
    }

    if (parsed < lower_range || parsed > upper_range) {
        throw std::out_of_range("Value out of range for " + flag + ": " + value);
    }

    return static_cast<uint16_t>(parsed);
}

uint16_t ArgParser::parse_uint16(const std::string& flag, const std::string& value, size_t max_value) {
    int parsed = str2int_range(value, flag, max_value);
    return static_cast<uint16_t>(parsed);
}

void ArgParser::add_argument(const std::string& flag, std::function<void(uint16_t)> store, size_t max_value) {
    arguments_[flag] = {[store, flag, max_value](const std::string& value) {
        store(parse_uint16(flag, value, max_value));
    }};
}

bool ArgParser::parse_bool(const std::string& flag, const std::string& value) {
    if (value == "1") {
        return true;
    } else if (value == "0") {
        return false;
    } else {
        throw std::invalid_argument("Invalid value for " + flag + ": " + value);
    }
}

void ArgParser::add_argument(const std::string& flag, std::function<void(bool)> store) {
    arguments_[flag] = {[store, flag](const std::string& value) {
        store(parse_bool(flag, value));
    }};
}

std::vector<uint16_t> ArgParser::parse_float_arr(const std::string& flag, const std::string& value) {
    std::vector<uint16_t> values;
    std::stringstream ss(value);
    std::string item;

    u_char parsed_cnt = 0;

    while (std::getline(ss, item, ',')) {
        // If value contains dot, remove it
        size_t dot_position;
        if ((dot_position = item.find('.')) != std::string::npos) {
            // Truncate to include only 1 digit after the dot
            item = item.substr(0, dot_position + 2);
            item.erase(std::remove(item.begin(), item.end(), '.'), item.end());
        } else { // No dot, append 0 to the end
            item.append(1, '0');
        }

        // The minimum frequency can be 87.6 MHz and maximum can be 107.9 MHz - 0 value means not used
        int parsed;
        try {
            parsed = str2int_range(item, flag, 1079, 876);
        } catch (const std::out_of_range& e) {
            throw std::out_of_range("AF values must be in range 87.6 - 107.9");
        }

        parsed -= 875; // Convert to 0-based index
        values.push_back(parsed);

        parsed_cnt++;
    }

    return values;
}

void ArgParser::add_argument(const std::string& flag, std::function<void(std::vector<uint16_t>)> store) {
    arguments_[flag] = {[store, flag](const std::string& value) {
        store(parse_float_arr(flag, value));
    }};
}

void ArgParser::add_argument(const std::string& flag, std::function<void(std::string)> store) {
    arguments_[flag] = {[store](const std::string& value) {
        store(value);
    }};
}

void ArgParser::parse_args(int argc, char* argv[]) {
    std::unordered_map<std::string, std::string> provided_args;

    for (int i = 1; i < argc; i += 2) {
        std::string flag = argv[i];

        // Check if flag is valid
        if (!arguments_.count(flag)) {
            throw std::runtime_error("Invalid argument: " + flag);
        }

        // Check if value is provided
        if (i + 1 >= argc) {
            throw std::runtime_error("Missing value for argument: " + flag);
        }
        std::string value = argv[i + 1];

        // If value starts with "-", it is a flag
        if (value.rfind("-", 0) == 0) {
            throw std::runtime_error("Missing value for argument: " + std::string(argv[i]));
        }

        provided_args[flag] = value;
    }

    for (const auto& arg : arguments_) {
        const std::string& flag = arg.first;
        const Argument& definition = arg.second;

        if (!provided_args.count(flag)) {
            throw std::runtime_error("Missing required argument: " + flag);
        }

        const std::string& value = provided_args[flag];
        definition.store(value);
    }
}

std::string ArgParser::find_arg_value(const std::string& flag, int argc, char* argv[]) {
    for (int i = 1; i < argc; i += 1) {
        if (std::string(argv[i]) == flag) {
            if (i + 1 >= argc) {
                throw std::runtime_error("Missing value for " + flag);
            }
            return std::string(argv[i + 1]);
        }
    }

    return "";
}

void ArgParser::find_help_flag(const int argc, char* argv[], const std::string& help_string) {
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "-h" || std::string(argv[i]) == "--help") {
            std::cout << help_string << std::endl;
            exit(0);
        }
    }
}