// Class to store states and possible trails for a given
// Feistel cipher

// Right now, we are only finding the best trails, we 
// will modify algorithms to find N trails later

// Later, convert set of rounds to usable trails (with the 
// help of a key-schedule)

#ifndef TRAIL_H
#define TRAIL_H

// Customs
#include "feistel.h"
#include "sbox.h"
#include "bitstr.h"
#include "perm.h"

// Mains
#include <iostream>
#include <algorithm>
#include <vector>
#include <stdexcept>
#include <cstdlib>
#include <cmath>

// Main Class
class trail
{
  public:
  // General Information (retrieved from cipher)
  // Stage_0 ----PRE_SBOX----> Stage_1 ----SBOX----> Stage_2 ----POST_SBOX----> Stage_3
  size_type stage_0;
  size_type stage_1;
  size_type stage_2;
  size_type stage_3;

  // SBox Infos
  std::vector<size_type> sbox_ip_start;
  std::vector<size_type> sbox_op_start;
  std::vector<size_type> sbox_ip_sizes;
  std::vector<size_type> sbox_op_sizes;

  // Sbox Vector
  std::vector<sbox> sboxes;

  // Permutations
  perm rf_before;
  perm rf_after;

  // Max Rounds
  size_type max_rounds;

  // State Structures for each round
  struct round_info
  {
    // Masks
    std::vector<bitstr> ip_masks;
    std::vector<bitstr> op_masks;
    std::vector<bitstr> key_masks;

    // Biases for each round
    std::vector<float> biases;

    // Current Stuff
    size_type curr_round;
    float curr_bias;

    // Default Constructor
    round_info() {
      // Empty vectors for masks and biases
      ip_masks = std::vector<bitstr>();
      op_masks = std::vector<bitstr>();
      key_masks = std::vector<bitstr>();
      biases = std::vector<float>();

      // Initialize current round and bias
      curr_round = 0;
      curr_bias = 0.5f;
    }
  };
  round_info curr_round_info;

  // State Structures for each SBox
  struct sbox_info
  {
    // Masks
    std::vector<size_type> ip_masks;
    std::vector<size_type> op_masks;
    
    // Biases for each SBox
    std::vector<float> biases;

    // Current Stuff
    size_type curr_box;
    float curr_bias;

    // Default Constructor
    sbox_info() {
      // Empty vectors for masks and biases
      ip_masks = std::vector<size_type>();
      op_masks = std::vector<size_type>();
      biases = std::vector<float>();

      // Initialize current box and bias
      curr_box = 0;
      curr_bias = 0.5f;
    }
  };
  sbox_info curr_sbox_info;

  // Overall Information
  std::vector<round_info> fin_trails;
  std::vector<float> fin_biases;

  // Import key-schedule from cipher
  std::vector<std::vector<size_type>> round_sch;
  size_type key_size;

  // Constructor
  trail(const feistel& cipher);

  // Helpers
  float eval_bias(std::vector<float> biases, size_type end);
  void print_round_info(const round_info& rinfo);
  void print_sbox_info(const sbox_info& sinfo);
  std::tuple<bitstr, bitstr, bitstr> expand_lats(size_type sbox_num, size_type ip_mask, size_type op_mask);

  // Routines
  void first_three();
  void more_than_three(size_type rounds);
  void upto(size_type rounds);

  // Sub-routines
  void more_than_three_one(size_type rounds);
  void more_than_three_two(size_type rounds);
  void more_than_three_intermediate(size_type rounds);
  void more_than_three_intermediate_sbox(size_type rounds, bitstr op_mask);
  void more_than_three_final(size_type rounds);
  void more_than_three_final_sbox(size_type rounds, bitstr op_mask);

  // Trail Masks
  std::tuple<bitstr, bitstr, bitstr> trail_masks(size_type rounds);
};

#endif
