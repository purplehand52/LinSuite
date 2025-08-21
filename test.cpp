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
#include "Primitives/bool_fn.h"

// Main
int main() {
  /*  // Test Feistel Class - DES
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
  des.assign_key(key);

  // Check trail class init
  trail t(des);

  // Get first three
  t.upto(4);
  // Print
  std::cout << "Trails found: " << t.fin_trails.size() << std::endl;
  for (int i = 0; i < 4; i++) {
    { t.print_round_info(t.fin_trails[i]);
      std::cout << std::endl;
      std::cout << "--------------------------------------" << std::endl;
    }
  }

  // Get trail mask
  auto [a, b, c] = t.trail_masks(3);
  std::cout << a.get_bits() << std::endl;
  std::cout << b.get_bits() << std::endl;
  std::cout << c.get_bits() << std::endl;

  // Launch Attack
  attack a1(a, b, c, t.fin_trails[2].curr_bias, 3, des);

  // Matsui-2
  auto [x,y] = a1.matsui2_dist(350000);
  std::cout << get_tern_bits(x) << std::endl;
  key.print_bits(); 

  // Test Bool-fn
  std::vector<uint> inputs = {0, 1, 4, 5, 2, 3, 6, 7};
  bool_fn temp = bool_fn(3, 3, inputs);
  temp.get_balanced_invariants();
  std::vector<bitstr> inv = temp.balanced_invariants;

  // Print cycles
  for (uint i = 0; i < temp.cycles.size(); ++i) {
    std::cout << "Cycle " << i << ": ";
    for (uint j = 0; j < temp.cycles[i].size(); ++j) {
      std::cout << temp.cycles[i][j] << " ";
    }
    std::cout << std::endl;
  }

  // Print balanced invariants
  for (uint i = 0; i < inv.size(); ++i) {
    std::cout << "Balanced invariant: ";
    inv[i].print_bits();
    std::cout << std::endl;
  }

  // Get invariant functions
  auto funcs = temp.get_inv_functions();
  for (const auto& func : funcs) {
    std::cout << "Invariant function: ";
    func.print_polynomial();
  } */

  // Test AES SBox
  /* size_type aes_entries[256] = { 
                        0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
                        0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 
                        0xc0, 0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 
                        0x31, 0x15, 0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 
                        0x27, 0xb2, 0x75, 0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 
                        0x29, 0xe3, 0x2f, 0x84, 0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 
                        0x39, 0x4a, 0x4c, 0x58, 0xcf, 0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 
                        0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8, 0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 
                        0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2, 0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 
                        0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73, 0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 
                        0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb, 0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 
                        0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79, 0xe7, 0xc8, 0x37, 0x6d, 0x8d, 
                        0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08, 0xba, 0x78, 0x25, 0x2e, 
                        0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a, 0x70, 0x3e, 0xb5, 
                        0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e, 0xe1, 0xf8, 
                        0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf, 0x8c, 
                        0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16};
  sbox aes_sbox(8, 8, aes_entries);

  // Create bool_fn from AES SBox
  bool_fn test = bool_fn(aes_sbox);

  // Print cycles
  test.get_balanced_invariants();
  std::vector<bitstr> inv = test.balanced_invariants;
  for (uint i = 0; i < test.cycles.size(); ++i) {
    std::cout << "Cycle " << i << ": ";
    for (uint j = 0; j < test.cycles[i].size(); ++j) {
      std::cout << test.cycles[i][j] << " ";
    }
    std::cout << std::endl;
  }

  // Print cycle sizes
  std::cout << "Cycle sizes: ";
  for (const auto& cycle : test.cycles) {
    std::cout << cycle.size() << " ";
  }

  // Print balanced invariants
  for (uint i = 0; i < inv.size(); ++i) {
    std::cout << "Balanced invariant: ";
    inv[i].print_bits();
    std::cout << std::endl;
  }

  // Get invariant functions
  auto funcs = test.get_inv_functions();
  for (const auto& func : funcs) {
    std::cout << "Invariant function: ";
    func.print_polynomial();
  } */

  // Test SM4 SBox
  size_type sm4_entries[256] = {
    0xd6, 0x90, 0xe9, 0xfe, 0xcc, 0xe1, 0x3d, 0xb7, 0x16, 0xb6, 0x14, 0xc2, 0x28, 0xfb, 0x2c, 0x05,
    0x2b, 0x67, 0x9a, 0x76, 0x2a, 0xbe, 0x04, 0xc3, 0xaa, 0x44, 0x13, 0x26, 0x49, 0x86, 0x06, 0x99,
    0x9c, 0x42, 0x50, 0xf4, 0x91, 0xef, 0x98, 0x7a, 0x33, 0x54, 0x0b, 0x43, 0xed, 0xcf, 0xac, 0x62,
    0xe4, 0xb3, 0x1c, 0xa9, 0xc9, 0x08, 0xe8, 0x95, 0x80, 0xdf, 0x94, 0xfa, 0x75, 0x8f, 0x3f, 0xa6,
    0x47, 0x07, 0xa7, 0xfc, 0xf3, 0x73, 0x17, 0xba, 0x83, 0x59, 0x3c, 0x19, 0xe6, 0x85, 0x4f, 0xa8,
    0x68, 0x6b, 0x81, 0xb2, 0x71, 0x64, 0xda, 0x8b, 0xf8, 0xeb, 0x0f, 0x4b, 0x70, 0x56, 0x9d, 0x35,
    0x1e, 0x24, 0x0e, 0x5e, 0x63, 0x58, 0xd1, 0xa2, 0x25, 0x22, 0x7c, 0x3b, 0x01, 0x21, 0x78, 0x87,
    0xd4, 0x00, 0x46, 0x57, 0x9f, 0xd3, 0x27, 0x52, 0x4c, 0x36, 0x02, 0xe7, 0xa0, 0xc4, 0xc8, 0x9e,
    0xea, 0xbf, 0x8a, 0xd2, 0x40, 0xc7, 0x38, 0xb5, 0xa3, 0xf7, 0xf2, 0xce, 0xf9, 0x61, 0x15, 0xa1,
    0xe0, 0xae, 0x5d, 0xa4, 0x9b, 0x34, 0x1a, 0x55, 0xad, 0x93, 0x32, 0x30, 0xf5, 0x8c, 0xb1, 0xe3,
    0x1d, 0xf6, 0xe2, 0x2e, 0x82, 0x66, 0xca, 0x60, 0xc0, 0x29, 0x23, 0xab, 0x0d, 0x53, 0x4e, 0x6f,
    0xd5, 0xdb, 0x37, 0x45, 0xde, 0xfd, 0x8e, 0x2f, 0x03, 0xff, 0x6a, 0x72, 0x6d, 0x6c, 0x5b, 0x51,
    0x8d, 0x1b, 0xaf, 0x92, 0xbb, 0xdd, 0xbc, 0x7f, 0x11, 0xd9, 0x5c, 0x41, 0x1f, 0x10, 0x5a, 0xd8,
    0x0a, 0xc1, 0x31, 0x88, 0xa5, 0xcd, 0x7b, 0xbd, 0x2d, 0x74, 0xd0, 0x12, 0xb8, 0xe5, 0xb4, 0xb0,
    0x89, 0x69, 0x97, 0x4a, 0x0c, 0x96, 0x77, 0x7e, 0x65, 0xb9, 0xf1, 0x09, 0xc5, 0x6e, 0xc6, 0x84,
    0x18, 0xf0, 0x7d, 0xec, 0x3a, 0xdc, 0x4d, 0x20, 0x79, 0xee, 0x5f, 0x3e, 0xd7, 0xcb, 0x39, 0x48
  };
  sbox sm4_sbox(8, 8, sm4_entries);

  // Create bool fn from SM4 Sbox
  bool_fn test = bool_fn(sm4_sbox);

  // Print cycles
  test.get_balanced_invariants();
  std::vector<bitstr> inv = test.balanced_invariants;
  for (uint i = 0; i < test.cycles.size(); ++i) {
    std::cout << "Cycle " << i << ": ";
    for (uint j = 0; j < test.cycles[i].size(); ++j) {
      std::cout << test.cycles[i][j] << " ";
    }
    std::cout << std::endl;
  }

  // Print cycle sizes
  std::cout << "Cycle sizes: ";
  for (const auto& cycle : test.cycles) {
    std::cout << cycle.size() << " ";
  }

  // Print balanced invariants
  for (uint i = 0; i < inv.size(); ++i) {
    std::cout << "Balanced invariant: ";
    inv[i].print_bits();
    std::cout << std::endl;
  }

  // Get invariant functions
  auto funcs = test.get_inv_functions();
  for (const auto& func : funcs) {
    std::cout << "Invariant function: ";
    func.print_polynomial();
  }

  return 0;
}
