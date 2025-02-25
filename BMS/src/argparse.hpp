/**
 * @file argparse.hpp
 * @brief Parsing of command line arguments (header file)
 * @author Adam Zvara (xzvara01@vutbr.cz)
 * @date 11/2024
 */

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <cstdint>

#define ERR_INVALID_ARG 1

/** @brief A generic command-line argument parser. */
class ArgParser {
public:
    /**
     * @brief Adds a new argument definition to the parser (uint16_t and lower).
     *
     * @param flag The command-line flag (e.g., "-g").
     * @param store A function that processes and stores the parsed value.
     * @param max_value The maximum value that the argument can take.
     */
    void add_argument(const std::string& flag, std::function<void(uint16_t)> store, size_t max_value);

    /**
     * @brief Adds a new argument definition to the parser (boolean).
     *
     * @param flag The command-line flag (e.g., "-g").
     * @param store A function that processes and stores the parsed value.
     */
    void add_argument(const std::string& flag, std::function<void(bool)> store);

    /**
     * @brief Adds a new argument definition to the parser (vector of uint16_t and lower).
     *
     * @param flag The command-line flag (e.g., "-g").
     * @param store A function that processes and stores the parsed value.
     */
    void add_argument(const std::string& flag, std::function<void(std::vector<uint16_t>)> store);

    /**
     * @brief Adds a new argument definition to the parser (string).
     *
     * @param flag The command-line flag (e.g., "-g").
     * @param store A function that processes and stores the parsed value.
     */
    void add_argument(const std::string& flag, std::function<void(std::string)> store);

    /**
     * @brief Parses the command-line arguments.
     *
     * @param argc The number of command-line arguments.
     * @param argv The array of command-line arguments.
     *
     * @throws std::runtime_error if required arguments are missing or invalid or argument is specified multiple times.
     */
    void parse_args(int argc, char* argv[]);

    /**
     * @brief Find specific argument in command line arguments.
     *
     * @param flag The flag to search for.
     * @param argc The number of command-line arguments.
     * @param argv The array of command-line arguments.
     *
     * @return The value of the argument or empty if not present.
     */
    std::string find_arg_value(const std::string& flag, int argc, char* argv[]);

    /**
     * @brief Find help flag in command line arguments.
     *
     * @param argc The number of command-line arguments.
     * @param argv The array of command-line arguments.
     * @param help_string The help string to print.
     */
    void find_help_flag(const int argc, char* argv[], const std::string& help_string);

private:
    /** @brief Stores the type and processing function for a command-line argument. */
    struct Argument {
        std::function<void(const std::string&)> store;
    };

    std::unordered_map<std::string, Argument> arguments_;
    std::string help_string_;

    /** Functions for converting command line arguments into different types -
      * each takes appropriate flag and value and returns the parsed value. */
    static uint16_t parse_uint16(const std::string& flag, const std::string& value, size_t upper_range);
    static bool parse_bool(const std::string& flag, const std::string& value);
    static std::vector<uint16_t> parse_float_arr(const std::string& flag, const std::string& value);
};