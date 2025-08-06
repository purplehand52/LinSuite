// Class to handle arbitrarily long bit-strings with limited 
// functionalities for cryptanalytic methods and analysis.

#ifndef BITSTR_H
#define BITSTR_H

// Custom C++ libraries
#include "perm.h"

// Standard C++ libraries
#include <iostream>
#include <cstdlib>
#include <vector>
#include <string>
#include <algorithm>

// Aliases for types
// Using unsigned long for blocks to store bits
using block_type = unsigned long;
// Using size_t for sizes
using size_type = size_t;

// Class Definition
class bitstr {
  public:
    // Member Variables
    static const int BITS_PER_BLOCK = sizeof(block_type) * 8;
    size_type bit_size;
    block_type *blocks;

    // Constructors
    bitstr();
    bitstr(size_type size);
    bitstr(block_type *data, size_type size);
    bitstr(const bitstr &other);
    bitstr(block_type value, size_type size);

    // Proxy Object for bit access
    class bit_proxy {
      private:
        block_type &block;
        size_type index;

      public:
        // Constructor
        bit_proxy(block_type &block, size_type index)
          : block(block), index(index) {}

        // Void reference (not to be defined)
        void operator&();

        // Bit access
        bit_proxy& operator=(bool value);               // a[i] = val
        bit_proxy& operator=(const bit_proxy &other);   // a[i] = a[j]

        // And, Or, Xor Operators
        bit_proxy& operator|=(bool value);          // a[i] |= val
        bit_proxy& operator&=(bool value);          // a[i] &= val
        bit_proxy& operator^=(bool value);          // a[i] ^= val

        // Not Operator
        bit_proxy& operator~();                     // ~a[i]

        // Conversion to bool
        operator bool() const;                      // a[i] implicitly used as booleans
    };

    // Bit Setters & Getters
    bit_proxy operator[](unsigned index);
    bool operator[](unsigned index) const;

    // Slice-access Proxy
    class slice_proxy {
      private:
        block_type *beg_block;
        block_type *end_block;
        unsigned beg_offset;
        unsigned end_offset;
        size_type len;

      public:
        // Constructor
        slice_proxy(block_type *beg_block, block_type *end_block, unsigned beg, unsigned end, size_type len)
          : beg_block(beg_block), end_block(end_block), beg_offset(beg), end_offset(end), len(len) {}

        // Void Reference (not to be defined)
        void operator&();

        // Slice access
        slice_proxy& operator=(block_type value);    // a[i:j] = value

        // And, Or, Xor Operators
        slice_proxy& operator|=(block_type value);   // a[i:j] |= value
        slice_proxy& operator&=(block_type value);   // a[i:j] &= value
        slice_proxy& operator^=(block_type value);   // a[i:j] ^= value

        // Not Operator
        slice_proxy& operator~();                   // ~a[i:j]
    };


    // Slice-access
    slice_proxy operator()(unsigned start, unsigned end);
    bitstr extract(unsigned start, unsigned end) const;
    size_type value(unsigned start, unsigned end) const;

    // Concatenation
    void operator+=(const bitstr other);

    // Operator Overloads
    void operator|=(const bitstr other);
    void operator^=(const bitstr other);
    void operator&=(const bitstr other);
    void operator~();
    bool operator*(const bitstr other) const;   // Dot product (Hamming weight)
    bool operator==(const bitstr other) const;
    bool operator!=(const bitstr other) const;
    bitstr operator^(const bitstr other) const;

    // Substitution
    bitstr substitute(std::vector<size_type> sub_arr, size_type sub_size) const;
    bitstr permute(perm stuff) const;
    bitstr inv_permute(perm stuff) const;
    bitstr sinv_permute(perm stuff) const;

    // Printers
    void print_bits() const;
    void print_hex() const;
    std::string get_bits() const;
    std::string get_hex() const;
};

#endif
