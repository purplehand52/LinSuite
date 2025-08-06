// Class to start a Matsui attack given a linear approximation

#ifndef ATTACK_H
#define ATTACK_H

// Customs
#include "trail.h"
#include "perm.h"
#include "feistel.h"
#include "sbox.h"
#include "bitstr.h"

// Mains
#include <iostream>
#include <vector>
#include <stdexcept>
#include <cstdlib>
#include <cmath>
#include <string>

// Helpers
bitstr rand_bitstr(size_type size);
std::string get_tern_bits(std::vector<short_type> arr);

// Main Class
class attack
{
  public:
    // Member Variables
    bitstr pt_mask;
    bitstr ct_mask;
    bitstr key_mask;
    bitstr fin_key_mask;
    float bias;
    size_type rounds;
    feistel cipher;

    // Constructor
    attack(const bitstr &pt_mask, const bitstr &ct_mask, const bitstr &key_mask, const bitstr &fin_key_mask, float bias, size_type rounds, feistel cipher)
        : pt_mask(pt_mask), ct_mask(ct_mask), key_mask(key_mask), fin_key_mask(fin_key_mask), bias(bias), rounds(rounds), cipher(cipher) {}

    // Standard Matsui's
    bool matsui1(size_type trials);
    std::tuple<std::vector<short_type>, bool> matsui2(size_type trials);
};

#endif
