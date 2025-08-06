// Class to manage and utilise a generalised symmetric Feistel 
// network for less than a specified number of rounds

// Following are the assumptions and framework that this code 
// operates in:

/* 
 * SYMMETRICITY:
 * We assume that the total block size is even and is evenly 
 * split into left and right halves for swapping and XORing.
 */

/*
 * ROUND FUNCTION:
 * The round function is assumed to consist of a pre S-Box
 * substitution, S-Box substitution and a post S-Box substitution. 
 * Each of the S-Boxes can be seperately specified.
 */

/*
 * ROUND KEYS:
 * The round key bits are assumed to be direct bits of the master 
 * key which are specified until the maximum number of rounds 
 * specified.
 */

/*
 * BETWEEN ROUNDS:
 * We assume that the right half is fed as input to the round 
 * function and the output of the round function is XORed with the
 * left half. The halves are then swapped.
 */

#ifndef FEISTEL_H
#define FEISTEL_H

// Custom Libraries
#include "bitstr.h"
#include "sbox.h"
#include "perm.h"

// Standard C++ Libraries
#include <iostream>
#include <cstdlib>
#include <vector>
#include <string>
#include <stdexcept>

// Class
class feistel {
  private:
    // Secret key
    bitstr key;

  public:
    // Member Variables - Basics
    size_type block_size;
    size_type max_rounds;
    size_type key_size;

    // Member Variables - Initial/Final Permutations
    perm ip;
    perm fp;

    // Member Variables - S-Boxes
    std::vector<sbox> sboxes;

    // Member Variables - Round Function
    perm rf_before;
    perm rf_after;

    // Member Variables - Round Keys
    std::vector<std::vector<size_type>> round_sch;

    // Constructor
    feistel(size_type block_size, size_type max_rounds, size_type key_size,
            const perm& ip, const perm& op,
            const std::vector<sbox>& sboxes,
            const perm& rf_before, const perm& rf_after,
            const std::vector<std::vector<size_type>>& round_sch);

    // Assign Key
    void assign_key(const bitstr& key);

    // Round Function
    bitstr rfunc(const bitstr& input, const bitstr& round_key) const;

    // Encryption/Decryption
    bitstr encrypt(const bitstr& input, size_type rounds) const;
    bitstr decrypt(const bitstr& input, size_type rounds) const;
};

#endif
