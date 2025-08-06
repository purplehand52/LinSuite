// Implementation of the Feistel cipher algorithm

// Header Inclusion
#include "feistel.h"

// Constructor
feistel::feistel(size_type block_size, size_type max_rounds, size_type key_size,
            const perm& ip, const perm& fp,
            const std::vector<sbox>& sboxes,
            const perm& rf_before, const perm& rf_after,
            const std::vector<std::vector<size_type>>& round_sch)
{
  // Basics
  if (block_size % 2 != 0) {
    throw std::invalid_argument("Block size must be even.");
  }
  if (max_rounds < 1) {
    throw std::invalid_argument("Max rounds must be at least 1.");
  }
  this->block_size = block_size;
  this->max_rounds = max_rounds;
  this->key_size = key_size;

  // Initial/Final Permutations
  if (ip.ip_size != block_size || fp.ip_size != block_size) {
    throw std::invalid_argument("Initial and final permutations must match block size.");
  }
  this->ip = ip;
  this->fp = fp;

  // SBoxes
  if (sboxes.empty()) {
    throw std::invalid_argument("At least one SBox must be provided.");
  }
  size_type bf_sub = 0;
  size_type af_sub = 0;
  for (auto it = sboxes.begin(); it != sboxes.end(); ++it) {
    bf_sub += it->input_size;
    af_sub += it->output_size;
  }

  // Round Function Checks
  if (rf_before.ip_size != block_size / 2 || rf_after.op_size != block_size / 2) {
    throw std::invalid_argument("Round function before and after must match half block size.");
  }
  if (bf_sub != rf_before.op_size || af_sub != rf_after.ip_size) {
    throw std::invalid_argument("Round function sizes do not match SBox sizes.");
  }
  this->rf_before = rf_before;
  this->rf_after = rf_after;
  this->sboxes = sboxes;

  // Key Scheduling
  if (round_sch.size() != max_rounds) {
    throw std::invalid_argument("Round schedule must match the number of rounds.");
  }
  for (auto it = round_sch.begin(); it != round_sch.end(); ++it) {
    for (auto key_it = it->begin(); key_it != it->end(); ++key_it) {
      if (*key_it >= key_size) {
        throw std::invalid_argument("Key indices in round schedule must be less than key size.");
      }
    }
  }
  this->round_sch = round_sch;
}

// Assign a key to the Feistel cipher
void feistel::assign_key(const bitstr& key)
{
  if (key.bit_size != key_size) {
    throw std::invalid_argument("Key size does not match the cipher's key size.");
  }
  this->key = key;
  return;
}

// Round Function
bitstr feistel::rfunc(const bitstr& input, const bitstr& round) const
{
  if (input.bit_size != block_size / 2) {
    throw std::invalid_argument("Input size must match half of the block size.");
  }
  
  // Apply the initial permutation
  bitstr permuted_input = input.permute(rf_before);

  // Add round key
  if (permuted_input.bit_size != round.bit_size) {
    throw std::invalid_argument("Round key must match size of permuted input.");
  }
  permuted_input ^= round;
  
  // Apply SBoxes
  bitstr sbox_output(0);
  size_type start = 0;
  for (auto it = sboxes.begin(); it != sboxes.end(); ++it) {
    // Get s-box input size and extract
    size_type ip_sz = it->input_size;
    size_type input = permuted_input.value(start, start + ip_sz);

    // Feed
    size_type output = (*it)[input];
    
    // Append to output
    sbox_output += bitstr(block_type(output), it->output_size);
    start += ip_sz;
  }

  // Apply the final permutation
  bitstr final_output = sbox_output.permute(rf_after);
  return final_output;
}

// Encrypt a block of data
bitstr feistel::encrypt(const bitstr& input, size_type rounds) const{
  if (input.bit_size != block_size) {
    throw std::invalid_argument("Input size must match the block size.");
  }
  if (rounds > max_rounds) {
    throw std::invalid_argument("Number of rounds exceeds maximum allowed.");
  }

  // Initial permutation
  bitstr permuted_input = input.permute(ip);

  // Split into left and right halves
  bitstr left = permuted_input.extract(0, block_size / 2);
  bitstr right = permuted_input.extract(block_size / 2, block_size);

  // Perform rounds
  for (size_type i = 0; i < rounds; ++i) {
    // Get the round key
    bitstr round = key.substitute(round_sch[i], round_sch[i].size());

    // Apply round function to right half
    bitstr rf_output = rfunc(right, round);

    // XOR with left half
    left ^= rf_output;

    // Swap halves for next round
    if (i < rounds - 1) { // No swap on the last round
      std::swap(left, right);
    }
  }

  // Final permutation
  left += right;
  bitstr final_output = left.permute(fp);
  
  return final_output;
}

// Decrypt a block of data
bitstr feistel::decrypt(const bitstr& input, size_type rounds) const {
  if (input.bit_size != block_size) {
    throw std::invalid_argument("Input size must match the block size.");
  }
  if (rounds > max_rounds) {
    throw std::invalid_argument("Number of rounds exceeds maximum allowed.");
  }

  // Initial permutation
  bitstr permuted_input = input.inv_permute(fp);

  // Split into left and right halves
  bitstr left = permuted_input.extract(0, block_size / 2);
  bitstr right = permuted_input.extract(block_size / 2, block_size);

  // Perform rounds in reverse order
  for (size_type i = rounds; i > 0; i--) {

    // Get the round key
    bitstr round = key.substitute(round_sch[i-1], round_sch[i-1].size());

    // Apply round function to right half
    bitstr rf_output = rfunc(right, round);

    // XOR with left half
    left ^= rf_output;

    // Swap halves for next round
    if (i > 1) { // No swap on the first round
      std::swap(left, right);
    }
  }

  // Final permutation
  left += right;
  bitstr final_output = left.inv_permute(ip);
  
  return final_output;
}

