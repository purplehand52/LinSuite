// Class to represent and analyse S-Boxes for Feistel Networks

#ifndef SBOX_H
#define SBOX_H

// Standard C++ libraries
#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <stdexcept>

// Aliases for types
using size_type = size_t;
using short_type = short int;

class sbox {
  public:
    // Member Variables
    size_type input_size;
    size_type output_size;
    size_type* table;
    std::vector<std::pair<size_type, short_type>> lat;

    // Constructors
    sbox(size_type input_size, size_type output_size, size_type* table);
    sbox(const sbox &other);

    // Get entry
    size_type operator[](size_type input) const;

    // Generate Linear Approximation Table (LAT)
    void generate_lat();

    // Sieve LAT based on output_mask
    std::vector<std::pair<size_type, short_type>> sieve_lat(size_type output_mask);

    // Print LAT
    void print_lat();
};

#endif
