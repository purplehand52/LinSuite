// Code to represent arbitrary Boolean functions
// With 1 output bit

#include "sbox.h"
#include "bitstr.h"

#include <iostream>
#include <vector>
#include <cmath>
#include <stdexcept>
#include <format>

#ifndef BOOL_FN_H
#define BOOL_FN_H

using uint = unsigned int;

// Main class
class bool_fn
{
  public:
    uint in;                                  // number of input bits
    uint out;                                 // number of output bits (always 1 for this class)
    std::vector<uint> values;                 // values of the function, indexed by input bit patterns
    std::vector<uint> poly;                   // polynomial representation of the function

    bool is_bij;                              // true if the function is bijective, false otherwise
    std::vector<std::vector<uint>> cycles;    // assumes the function is bijective, stores cycles of the function
    std::vector<bitstr> balanced_invariants;  // stores balanced invariants of the function

    // Constructors
    bool_fn(uint in, uint out, std::vector<uint> values);
    bool_fn(const sbox& mybox);

    // Destructor
    ~bool_fn();

    // Print polynomial representation
    void print_polynomial() const;

    // Copy constructor
    bool_fn(const bool_fn& other);

    // Getting value for a specific input pattern
    uint operator[](uint input) const;

    // Getting the polynomial representation
    void get_polynomial();

    // Check for bijectivity
    void is_bijective();

    // Getting all cycles
    void get_cycles();

    // Get balanced invariants
    void get_balanced_invariants();
    
    // Get inv functions
    std::vector<bool_fn> get_inv_functions() const;
};

#endif

