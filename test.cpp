// Scratch-pad
// Std-libs
#include <iostream>
#include <vector>
#include <cstdlib>
#include <stdexcept>
#include <random>
#include <string>

// Custom-libs
#include "Primitives/bitstr.h"
#include "Primitives/sbox.h"
#include "Primitives/perm.h"
#include "Primitives/feistel.h"
#include "Primitives/trail.h"
#include "Primitives/trail_adv.h"
#include "Primitives/attack.h"

// Main
int main() {
  // Test Feistel Class - DES
  // Basic Parameters
  size_type block_size = 64;
  size_type max_rounds = 16;
  size_type key_size = 64;

  // Initial and Final Permutations
  std::vector<size_type> ip_table = {
    57, 49, 41, 33, 25, 17, 9, 1,
    59, 51, 43, 35, 27, 19, 11, 3,
    61, 53, 45, 37, 29, 21, 13, 5,
    63, 55, 47, 39, 31, 23, 15, 7,
    56, 48, 40, 32, 24, 16, 8, 0,
    58, 50, 42, 34, 26, 18, 10, 2,
    60, 52, 44, 36, 28, 20, 12, 4,
    62, 54, 46, 38, 30, 22, 14, 6
  };
  std::vector<size_type> fp_table = {
    39, 7, 47, 15, 55, 23, 63, 31,
    38, 6, 46, 14, 54, 22, 62, 30,
    37, 5, 45, 13, 53, 21, 61, 29,
    36, 4, 44, 12, 52, 20, 60, 28,
    35, 3, 43, 11, 51, 19, 59, 27,
    34, 2, 42, 10, 50, 18, 58, 26,
    33, 1, 41, 9 , 49, 17, 57, 25,
    32 ,0 ,40 ,8 , 48, 16, 56, 24
  };
  perm ip(block_size, block_size, ip_table);
  perm fp(block_size, block_size, fp_table);

  // SBoxes
  std::vector<sbox> sboxes;
  // Populate sboxes with appropriate input and output sizes
  size_type data1[] = {
    14,  0,  4, 15, 13,  7,  1,  4,  2, 14, 15,  2, 11, 13,  8,  1,
     3, 10, 10,  6,  6, 12, 12, 11,  5,  9,  9,  5,  0,  3,  7,  8,
     4, 15,  1, 12, 14,  8,  8,  2, 13,  4,  6,  9,  2,  1, 11,  7,
    15,  5, 12, 11,  9,  3,  7, 10,  3, 14, 10,  0,  5,  6,  0, 13
  };
  size_type data2[] = {
    15,  3,  0, 13, 14,  8, 13,  6, 10, 15,  7,  1,  3,  4,  9, 11,
     0, 14,  6, 10,  9,  2,  1,  8,  7, 13,  2, 15, 12, 12,  5,  4,
     8,  1, 14,  6,  4,  7, 11, 10,  1,  9, 10,  0,  5,  3,  0, 14,
     6, 11,  2,  8, 13,  5, 15,  2,  9, 12, 12,  3,  7,  4,  5, 11
  };
  size_type data3[] = {
    10, 13,  0,  7,  9,  0, 14,  9,  6,  3,  3,  4, 15,  6,  5, 10,
     1,  2, 13,  8, 12,  5,  7, 14, 11, 12,  4, 11,  2, 15,  8,  1,
    13,  1,  6, 10,  4, 13,  9,  0,  8,  6, 15,  9,  3,  8,  0,  7,
    11,  4,  1, 15,  2, 14, 12,  3,  5, 11, 10,  5, 14,  2,  7, 12
  };
  size_type data4[] = {
    7, 13, 13,  8, 14, 11,  3,  5,  0,  6,  6, 15,  9,  0, 10,  3,
     1,  4,  2,  7,  8,  2,  5, 12, 11,  1, 12, 10,  4, 14, 15,  9,
    10,  3,  6, 15,  9,  0,  0,  6, 12, 10, 11,  1,  7, 13, 13,  8,
    15,  9,  1,  4,  3,  5, 14, 11,  5, 12,  2,  7,  8,  2,  4, 14
  };
  size_type data5[] = {
    2, 14, 12, 11,  4,  2,  1, 12,  7,  4, 10,  7, 11, 13,  6,  1,
     8,  5,  5,  0,  3, 15, 15, 10, 13,  3,  0,  9, 14,  8,  9,  6,
     4, 11,  2,  8,  1, 12, 11,  7, 10,  1, 13, 14,  7,  2,  8, 13,
    15,  6,  9, 15, 12,  0,  5,  9,  6, 10,  3,  4,  0,  5, 14,  3
  };
  size_type data6[] = {
    12, 10,  1, 15, 10,  4, 15,  2,  9,  7,  2, 12,  6,  9,  8,  5,
     0,  6, 13,  1,  3, 13,  4, 14, 14,  0, 11,  3,  5, 11,  7,  8,
     1, 13,  6,  0,  4, 11, 11,  7, 13, 12,  0,  5, 10, 14,  3, 10,
     9,  3, 14,  9,  5,  6,  2,  8,  7,  2,  8,  1, 12,  4, 15, 15
  };
  size_type data7[] = {
     4, 13, 11,  0,  2, 11, 14,  7, 15,  4,  0,  9,  8,  1, 13, 10,
     3, 14, 12,  3,  9,  5,  7, 12,  5,  2, 10, 15,  6,  8,  1,  6,
     1,  6,  4, 11, 11, 13, 13,  8, 12,  1,  3,  4,  7, 10, 14,  7,
    10,  9, 15,  5,  6,  0,  8, 15,  0, 14,  5,  2,  9,  3,  2, 12
  };
  size_type data8[] = {
    13,  1,  2, 15,  8, 13,  4,  8,  6, 10, 15,  3, 11,  7,  1,  4,
    10, 12,  9,  5,  3,  6, 14, 11,  5,  0,  0, 14, 12,  9,  7,  2,
     7,  2, 11,  1,  4, 14,  1,  7,  9,  4, 12, 10, 14,  8,  2, 13,
     0, 15,  6, 12, 10,  9, 13,  0, 15,  3,  3,  5,  5,  6,  8, 11
  };

  sboxes.push_back(sbox(6, 4, data1));
  sboxes.push_back(sbox(6, 4, data2));
  sboxes.push_back(sbox(6, 4, data3));
  sboxes.push_back(sbox(6, 4, data4));
  sboxes.push_back(sbox(6, 4, data5));
  sboxes.push_back(sbox(6, 4, data6));
  sboxes.push_back(sbox(6, 4, data7));
  sboxes.push_back(sbox(6, 4, data8));

  // Round Function Before and After
  std::vector<size_type> expansion = {
    31, 0, 1, 2, 3, 4,
    3, 4, 5, 6, 7, 8,
    7, 8, 9, 10, 11, 12,
    11, 12, 13, 14, 15, 16,
    15, 16, 17, 18, 19, 20,
    19, 20, 21, 22, 23, 24,
    23, 24, 25, 26, 27, 28,
    27, 28, 29, 30, 31, 0
  };
  std::vector<size_type> p_table = {
    15, 6, 19, 20, 28, 11, 27, 16,
    0, 14, 22, 25, 4, 17, 30, 9,
    1, 7, 23, 13, 31, 26, 2, 8,
    18, 12, 29, 5, 21, 10, 3, 24
  };
  perm rf_before(32, 48, expansion);
  perm rf_after(32, 32, p_table);

  // Round Schedule
  std::vector<size_type> ks0 = {13, 16, 10, 23, 0, 4, 2, 27, 14, 5, 20, 9, 22, 18, 11, 3, 25, 7, 15, 6, 26, 19, 12, 1, 40, 51, 30, 36, 46, 54, 29, 39, 50, 44, 32, 47, 43, 48, 38, 55, 33, 52, 45, 35, 28, 31, 37, 34};
  std::vector<size_type> ks1 = {12, 15, 9, 22, 63, 3, 1, 26, 13, 4, 19, 8, 21, 17, 10, 2, 24, 6, 14, 5, 25, 18, 11, 0, 39, 50, 29, 35, 45, 53, 28, 38, 49, 43, 31, 46, 42, 47, 37, 54, 32, 51, 44, 34, 27, 30, 36, 33};
  std::vector<size_type> ks2 = {10, 13, 7, 20, 61, 1, 63, 24, 11, 2, 17, 6, 19, 15, 8, 0, 22, 4, 12, 3, 23, 16, 9, 62, 37, 48, 27, 33, 43, 51, 26, 36, 47, 41, 29, 44, 40, 45, 35, 52, 30, 49, 42, 32, 25, 28, 34, 31};
  std::vector<size_type> ks3 = {8, 11, 5, 18, 59, 63, 61, 22, 9, 0, 15, 4, 17, 13, 6, 62, 20, 2, 10, 1, 21, 14, 7, 60, 35, 46, 25, 31, 41, 49, 24, 34, 45, 39, 27, 42, 38, 43, 33, 50, 28, 47, 40, 30, 23, 26, 32, 29};
  std::vector<size_type> ks4 = {6, 9, 3, 16, 57, 61, 59, 20, 7, 62, 13, 2, 15, 11, 4, 60, 18, 0, 8, 63, 19, 12, 5, 58, 33, 44, 23, 29, 39, 47, 22, 32, 43, 37, 25, 40, 36, 41, 31, 48, 26, 45, 38, 28, 21, 24, 30, 27};
  std::vector<size_type> ks5 = {4, 7, 1, 14, 55, 59, 57, 18, 5, 60, 11, 0, 13, 9, 2, 58, 16, 62, 6, 61, 17, 10, 3, 56, 31, 42, 21, 27, 37, 45, 20, 30, 41, 35, 23, 38, 34, 39, 29, 46, 24, 43, 36, 26, 19, 22, 28, 25};
  std::vector<size_type> ks6 = {2, 5, 63, 12, 53, 57, 55, 16, 3, 58, 9, 62, 11, 7, 0, 56, 14, 60, 4, 59, 15, 8, 1, 54, 29, 40, 19, 25, 35, 43, 18, 28, 39, 33, 21, 36, 32, 37, 27, 44, 22, 41, 34, 24, 17, 20, 26, 23};
  std::vector<size_type> ks7 = {1, 4, 62, 11, 52, 56, 54, 15, 2, 57, 8, 61, 10, 6, 63, 55, 13, 59, 3, 58, 14, 7, 0, 53, 28, 39, 18, 24, 34, 42, 17, 27, 38, 32, 20, 35, 31, 36, 26, 43, 21, 40, 33, 23, 16, 19, 25, 22};
  std::vector<size_type> ks8 = {0, 3, 61, 10, 51, 55, 53, 14, 1, 56, 7, 60, 9, 5, 62, 54, 12, 58, 2, 57, 13, 6, 63, 52, 27, 38, 17, 23, 33, 41, 16, 26, 37, 31, 19, 34, 30, 35, 25, 42, 20, 39, 32, 22, 15, 18, 24, 21};
  std::vector<size_type> ks9 = {62, 2, 60, 9, 50, 54, 52, 13, 0, 55, 6, 59, 8, 4, 61, 53, 11, 57, 1, 56, 12, 5, 62, 51, 26, 37, 16, 22, 32, 40, 15, 25, 36, 30, 18, 33, 29, 34, 24, 41, 19, 38, 31, 21, 14, 17, 23, 20};
  std::vector<size_type> ksA = {60, 0, 58, 7, 48, 52, 50, 11, 62, 53, 4, 57, 6, 2, 59, 51, 9, 55, 63, 54, 10, 3, 60, 49, 24, 35, 14, 20, 30, 38, 13, 23, 34, 28, 16, 31, 27, 32, 22, 39, 17, 36, 29, 19, 12, 15, 21, 18};
  std::vector<size_type> ksB = {58, 62, 56, 5, 46, 50, 48, 9, 60, 51, 2, 55, 4, 0, 57, 49, 7, 53, 61, 52, 8, 1, 58, 47, 22, 33, 12, 18, 28, 36, 11, 21, 32, 26, 14, 29, 25, 30, 20, 37, 15, 34, 27, 17, 10, 13, 19, 16};
  std::vector<size_type> ksC = {56, 60, 54, 3, 44, 48, 46, 7, 58, 49, 0, 53, 2, 62, 55, 47, 5, 51, 59, 50, 6, 63, 56, 45, 20, 31, 10, 16, 26, 34, 9, 19, 30, 24, 12, 27, 23, 28, 18, 35, 13, 32, 25, 15, 8, 11, 17, 14};
  std::vector<size_type> ksD = {55, 59, 53, 2, 43, 47, 45, 6, 57, 48, 63, 52, 1, 61, 54, 46, 4, 50, 58, 49, 5, 62, 55, 44, 19, 30, 9, 15, 25, 33, 8, 18, 29, 23, 11, 26, 22, 27, 17, 34, 12, 31, 24, 14, 7, 10, 16, 13};
  std::vector<size_type> ksE = {54, 58, 52, 1, 42, 46, 44, 5, 56, 47, 62, 51, 0, 60, 53, 45, 3, 49, 57, 48, 4, 61, 54, 43, 18, 29, 8, 14, 24, 32, 7, 17, 28, 22, 10, 25, 21, 26, 16, 33, 11, 30, 23, 13, 6, 9, 15, 12};
  std::vector<size_type> ksF = {53, 57, 51, 0, 41, 45, 43, 4, 55, 46, 61, 50, 63, 59, 52, 44, 2, 48, 56, 47, 3, 60, 53, 42, 17, 28, 7, 13, 23, 31, 6, 16, 27, 21, 9, 24, 20, 25, 15, 32, 10, 29, 22, 12, 5, 8, 14, 11};
 
  std::vector<std::vector<size_type>> round_sch = {ks0, ks1, ks2, ks3, ks4, ks5, ks6, ks7, ks8, ks9, ksA, ksB, ksC, ksD, ksE, ksF};

  // Create Feistel instance
  feistel des(block_size, max_rounds, key_size, ip, fp, sboxes, rf_before, rf_after, round_sch);

  // Create a random key and assign
  bitstr key(64);
  for (size_type i = 0; i < 64; ++i) {
    key[i] = rand() % 2; // Randomly set bits
  }
  // std::cout << key.bit_size << std::endl;
  // key.print_bits();
  des.assign_key(key);

  // Test encryption and decryption
  bitstr plaintext(block_size);
  for (size_type i = 0; i < 64; ++i) {
    plaintext[i] = rand() % 2;
  }
  // plaintext.print_bits();
  bitstr ciphertext = des.encrypt(plaintext, max_rounds);
  // ciphertext.print_bits();
  bitstr decrypted = des.decrypt(ciphertext, max_rounds);
  // decrypted.print_bits();

  // Check trail class init
  trail_adv t(des);

  // Get first three
  t.upto(4);
  // Print
  std::cout << "Trails found: " << t.fin_trails.size() << std::endl;
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < MEMO_SIZE; ++j)
    { t.print_round_info(t.fin_trails[i][j]);
      std::cout << std::endl;
      std::cout << "--------------------------------------" << std::endl;
    }
  }

  // Get trail mask
  /* auto [a, b, c] = t.trail_masks(4);
  std::cout << a.get_bits() << std::endl;
  std::cout << b.get_bits() << std::endl;
  std::cout << c.get_bits() << std::endl;

  // Mount attack - 1
  attack m4(a, b, c, t.fin_trails[3].curr_bias, 3, des); 
  for (int i = 0; i < 50; ++i) {
    bool test = m4.matsui1(100);
    std::cout << test << " : " << (key * c) << std::endl;
  } */

  /* auto [arr, test2] = m4.matsui2(100);
  std::cout << get_tern_bits(arr) << std::endl;
  std::cout << key.get_bits() << std::endl; */

  return 0;
}
