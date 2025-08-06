// Routines to identify good trails for a Matsui-linear
// attack

#ifndef TRAIL_OLD_H
#define TRAIL_OLD_H

// Libraries
#include "feistel.h"
#include "sbox.h"
#include "bitstr.h"

#include <iostream>
#include <algorithm>
#include <vector>
#include <stdexcept>
#include <cstdlib>

// Parameters
#define GUESS_RATE 0.1
#define MEMO_RATE 3

// Structures to assist
struct round_info
{
  // Overall stuff
  size_type tot_rounds;
  float bias;
  std::vector<bitstr> ip_masks;
  std::vector<bitstr> op_masks;
  std::vector<bitstr> key_masks;
  std::vector<float> biases;

  // Current State Information
  size_type current_round;
  float baseline;

  // Constructor (default)
  round_info()
    : tot_rounds(0), bias(0.0), current_round(0), baseline(0.0), 
      ip_masks(), op_masks(), key_masks(), biases() {}
  // Constructor (with parameters)
  round_info(size_type rounds, float b, 
             std::vector<bitstr> ip, std::vector<bitstr> op, 
             std::vector<bitstr> key, std::vector<float> bss)
    : tot_rounds(rounds), bias(b), ip_masks(ip), 
      op_masks(op), key_masks(key), biases(bss), 
      current_round(0), baseline(0.0) {}
};

struct sbox_info
{
  // Overall stuff
  size_type tot_boxes;
  float bias;
  std::vector<size_type> ip_masks;
  std::vector<size_type> op_masks;
  std::vector<float> biases;

  // Current State Information
  size_type current_box;

  // Constructor (default)
  sbox_info()
    : tot_boxes(0), bias(0.0), current_box(0), 
      ip_masks(), op_masks(), biases() {}
};

// Helpers
void print_round_info(const round_info& info);
float eval_bias(std::vector<float> biases, size_type end);

// Routines
std::tuple<bitstr, bitstr, bitstr> interpret_lats(feistel cipher, size_type sbox_num, size_type ip_mask, size_type op_mask);
std::vector<std::vector<round_info>> first_three(feistel cipher);
round_info find_trail_one(feistel cipher, size_type rounds, std::vector<std::vector<round_info>> memo);
void find_trail_two(feistel cipher, std::vector<std::vector<round_info>> memo, round_info &state);
void find_trail_inter(feistel cipher, std::vector<std::vector<round_info>> memo, round_info &state);
void find_trail_inter_sbox(feistel cipher, std::vector<std::vector<round_info>> memo, round_info &state, sbox_info &sbox_state, bitstr op_mask);
void find_trail_fin(feistel cipher, std::vector<std::vector<round_info>> memo, round_info &state);
void find_trail_fin_sbox(feistel cipher, std::vector<std::vector<round_info>> memo, round_info &state, sbox_info &sbox_state, bitstr op_mask);


#endif
