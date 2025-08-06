// Implementations of the Trail class methods

#include "trail.h"

// Macro for std::abs
#define FABS(x) (std::abs(x))
#define RATE 0.01f
#define PILING(x,y) (std::abs(2*x*y))
#define PILING2(x,y,z) (std::abs(4*x*y*z))
#define PILING3(x,y,z,w) (std::abs(8*x*y*z*w))

// Global variable for debugging
bool best = false;
size_type comp1[] = {0, 0, 0, 0, 15, 0, 0, 0};
size_type comp2[] = {4, 0, 0, 0, 0, 0, 0, 0};

// Main Constructor
trail::trail(const feistel& cipher)
{
    // Stage values
    stage_0 = cipher.rf_before.ip_size;
    stage_1 = cipher.rf_before.op_size;
    stage_2 = cipher.rf_after.ip_size;
    stage_3 = cipher.rf_after.op_size;

    // SBox Vector
    sboxes = cipher.sboxes;

    // SBox Infos
    sbox_ip_start = std::vector<size_type>(sboxes.size());
    sbox_op_start = std::vector<size_type>(sboxes.size());
    sbox_ip_sizes = std::vector<size_type>(sboxes.size());
    sbox_op_sizes = std::vector<size_type>(sboxes.size());
    size_type istart = 0;
    size_type ostart = 0;
    for (size_type i = 0; i < sboxes.size(); ++i) {
      sbox_ip_start[i] = istart;
      sbox_op_start[i] = ostart;
      sbox_ip_sizes[i] = sboxes[i].input_size;
      sbox_op_sizes[i] = sboxes[i].output_size;
      istart += sboxes[i].input_size;
      ostart += sboxes[i].output_size;
    }

   // Permutations
   rf_before = cipher.rf_before;
   rf_after = cipher.rf_after;

   // Max Rounds
   max_rounds = cipher.max_rounds;

   // Key Schedule
   round_sch = cipher.round_sch;
   key_size = cipher.key_size;

   // States
   curr_round_info = round_info();
   curr_sbox_info = sbox_info();
}

// Evaluate bias for a vector
float trail::eval_bias(std::vector<float> biases, size_type end)
{
  float res = 0.5f;
  for (size_type i = 0; i < end; ++i) res *= 2*biases[i];
  return res;
}

// Print round-info
void trail::print_round_info(const round_info& rinfo)
{
  // Current Round
  std::cout << "Round: " << rinfo.curr_round << std::endl;
  // Current Bias
  std::cout << "Current Bias: " << rinfo.curr_bias << std::endl;
  // Print vectors
  for (size_type i = 0; i < rinfo.biases.size(); ++i) {
    std::cout << "Round " << i << std::endl;
    std::cout << "Input Mask " << rinfo.ip_masks[i].get_bits() << std::endl;
    std::cout << "Output Mask " << rinfo.op_masks[i].get_bits() << std::endl;
    std::cout << "Key Mask " << rinfo.key_masks[i].get_bits() << std::endl;
    std::cout << "Bias: " << rinfo.biases[i] << std::endl;
  }
  return;
}

// Print sbox-info
void trail::print_sbox_info(const sbox_info& sinfo)
{
  // Current SBox
  std::cout << "SBox: " << sinfo.curr_box << std::endl;
  // Current Bias
  std::cout << "Current Bias: " << sinfo.curr_bias << std::endl;
  // Print vectors
  for (size_type i = 0; i < sinfo.biases.size(); ++i) {
    std::cout << "SBox " << i << std::endl;
    std::cout << "Input Mask " << sinfo.ip_masks[i] << std::endl;
    std::cout << "Output Mask " << sinfo.op_masks[i] << std::endl;
    std::cout << "Bias: " << sinfo.biases[i] << std::endl;
  }
  return;
}

// Expand a lat-entry to a round trail
std::tuple<bitstr, bitstr, bitstr> trail::expand_lats(size_type sbox_num, size_type ip_mask, size_type op_mask)
{
  // Create bitstr for key mask
  bitstr key_mask_bits = bitstr(stage_1);
  key_mask_bits(sbox_ip_start[sbox_num], sbox_ip_start[sbox_num] + sbox_ip_sizes[sbox_num]) = block_type(ip_mask);

  // Create bitstr for input mask
  bitstr input_mask_bits = key_mask_bits.sinv_permute(rf_before);

  // Create bitstr for output mask
  bitstr inter_mask_bits = bitstr(stage_2);
  inter_mask_bits(sbox_op_start[sbox_num], sbox_op_start[sbox_num] + sbox_op_sizes[sbox_num]) = block_type(op_mask);
  bitstr output_mask_bits = inter_mask_bits.permute(rf_after);

  // Create a tuple and return it
  return std::make_tuple(input_mask_bits, output_mask_bits, key_mask_bits);
}

// First Three
void trail::first_three()
{
  // Empty fin_trails and fin_biases
  fin_trails.clear();
  fin_biases.clear();

  // Round 1: Find the best sbox-lat-entry amongst all sboxes
  // Find best entry
  std::pair<size_type, short_type> lat_entry = {0, 0};
  size_type best_sbox = 0;
  for (size_type i = 0; i < sboxes.size(); ++i) {
    auto temp = sboxes[i].lat[1];
    if (FABS(temp.second) > FABS(lat_entry.second)) { 
      lat_entry = temp;
      best_sbox = i;
    }
  }
  // Interpret
  auto [ip, op, key] = expand_lats(best_sbox, lat_entry.first >> sboxes[best_sbox].output_size, 
                                  lat_entry.first & ((1 << sboxes[best_sbox].output_size) - 1));
  // Create a new round_info
  auto save_1 = round_info();
  save_1.curr_round = 0;
  save_1.curr_bias = float(lat_entry.second) / (1 << (sboxes[best_sbox].input_size + 1));
  save_1.ip_masks.push_back(ip);
  save_1.op_masks.push_back(op);
  save_1.key_masks.push_back(key);
  save_1.biases.push_back(save_1.curr_bias);
  fin_trails.push_back(save_1);

  // Round 2: Append all zeroes to the first round
  auto save_2 = save_1;
  save_2.curr_round = 1;
  save_2.ip_masks.push_back(bitstr(stage_0));
  save_2.op_masks.push_back(bitstr(stage_3));
  save_2.key_masks.push_back(bitstr(stage_1));
  save_2.biases.push_back(0.5f);
  fin_trails.push_back(save_2);

  // Round 3: Append the first round again
  auto save_3 = save_2;
  save_3.curr_round = 2;
  save_3.curr_bias = 2 * save_2.curr_bias * save_2.curr_bias;
  save_3.ip_masks.push_back(save_2.ip_masks[0]);
  save_3.op_masks.push_back(save_2.op_masks[0]);
  save_3.key_masks.push_back(save_2.key_masks[0]);
  save_3.biases.push_back(save_2.biases[0]);
  fin_trails.push_back(save_3);

  // Done
  return;
}

// Find a trail for more than three rounds
void trail::more_than_three(size_type rounds)
{
  std::cout << fin_trails.size() << " trails found for " << rounds - 1 << " rounds." << std::endl;
  // Check if memo has size <rounds-1>
  if (fin_trails.size() != rounds-1) throw std::runtime_error("Memo vector size does not match necessary size.");

  // Create an empty round_info and update its bias
  round_info new_round = round_info();
  new_round.curr_bias = RATE * fin_trails[rounds - 2].curr_bias;

  // Push back to memo
  fin_trails.push_back(new_round);

  // Initialize curr_round_info
  curr_round_info = round_info();

  // Create vectors for input, output and key masks and biases of size "rounds"
  curr_round_info.ip_masks = std::vector<bitstr>(rounds);
  curr_round_info.op_masks = std::vector<bitstr>(rounds);
  curr_round_info.key_masks = std::vector<bitstr>(rounds);
  curr_round_info.biases = std::vector<float>(rounds);

  // Call recursive function
  more_than_three_one(rounds);
}

// Recursion steps
// At round 1
void trail::more_than_three_one(size_type rounds)
{
  // Check for every possible output-mask
  for (auto it = sboxes.begin(); it != sboxes.end(); ++it) {
    // Current SBox
    size_type sbox_num = it - sboxes.begin();
    size_type lim = (1 << it->output_size);
    // Iterate over all possible output-masks
    for (size_type op = 1; op < lim; ++op) {
      if (op == 15 && sbox_num == 4) {
        best = true;
        std::cout << "SBox 4, Output Mask 15" << std::endl;
      }
      else best = false;
      // Best Sieve LAT-entry
      auto lat_entry = it->sieve_lat(op)[0];
      float score = lat_entry.second / float(1 << (it->input_size + 1));
      // Check if better
      if (PILING(score, fin_trails[rounds - 2].curr_bias) > FABS(fin_trails[rounds - 1].curr_bias)) {
        // If best, print debug info
        if (best) std::cout << "Going from 1 to 2" << std::endl;
        // Expand the lat-entry
        auto [ip, op_mask, key] = expand_lats(sbox_num, lat_entry.first >> it->output_size, 
                                              lat_entry.first & ((1 << it->output_size) - 1));
        // Temporarily store curr_round_info (to be replaced after recursion)
        auto temp = curr_round_info;
        // Update curr_round_info
        curr_round_info.curr_round = 1;
        curr_round_info.curr_bias = score;
        curr_round_info.ip_masks[0] = ip;
        curr_round_info.op_masks[0] = op_mask;
        curr_round_info.key_masks[0] = key;
        curr_round_info.biases[0] = score;
        // Call recursion
        more_than_three_two(rounds);
        // Reset curr_round_info
        curr_round_info = temp;
      }
    }
  }

  // Reach here (no more candidates)
  return;
}

// At round 2
void trail::more_than_three_two(size_type rounds)
{
  // Iterate through every S-Box
  for(auto it = sboxes.begin(); it != sboxes.end(); ++it) {
    // Current SBox
    size_type sbox_num = it - sboxes.begin();
    // Iterate through all LAT entries
    for (auto lat_it = it->lat.begin(); lat_it != it->lat.end(); ++lat_it) {
      // Get score
      float score = lat_it->second / float(1 << (it->input_size + 1));
      // Debugging
      if (best && lat_it->first == 0) {
        best = true;
      }
      else {
        best = false;
      }
      // Check if better
      if (PILING2(curr_round_info.curr_bias, score, fin_trails[rounds - 3].curr_bias) > FABS(fin_trails[rounds - 1].curr_bias)) {
        if (best) std::cout << "Going from 2 to 3" << std::endl;
        // Expand the lat-entry
        auto [ip, op_mask, key] = expand_lats(sbox_num, lat_it->first >> it->output_size, 
                                              lat_it->first & ((1 << it->output_size) - 1));
        // Temporarily store curr_round_info (to be replaced after recursion)
        auto temp = curr_round_info;
        // Update curr_round_info
        curr_round_info.curr_round = 2;
        curr_round_info.curr_bias = PILING(score, curr_round_info.curr_bias);
        curr_round_info.ip_masks[1] = ip;
        curr_round_info.op_masks[1] = op_mask;
        curr_round_info.key_masks[1] = key;
        curr_round_info.biases[1] = score;
        // Call recursion
        more_than_three_intermediate(rounds);
        // Reset curr_round_info
        curr_round_info = temp;
      }
    }
  }
  // Reach here (no more candidates)
  return;
}

// At round i > 2 (but not the last round)
void trail::more_than_three_intermediate(size_type rounds) {
  // Fix output masks
  bitstr fixed_op_mask = curr_round_info.op_masks[curr_round_info.curr_round - 2] ^
                          curr_round_info.ip_masks[curr_round_info.curr_round - 1];
  bitstr inv_op_mask = fixed_op_mask.inv_permute(rf_after);

  // Make a s-box state
  curr_sbox_info = sbox_info();

  // Create vectors for input, output and biases of size "sboxes.size()"
  curr_sbox_info.ip_masks = std::vector<size_type>(sboxes.size());
  curr_sbox_info.op_masks = std::vector<size_type>(sboxes.size());
  curr_sbox_info.biases = std::vector<float>(sboxes.size());

  // Call recursive function
  more_than_three_intermediate_sbox(rounds, inv_op_mask);
  return;
}

// At round i > 2 (but not the last round) for each sbox
void trail::more_than_three_intermediate_sbox(size_type rounds, bitstr op_mask) {
  // Get slice
  size_type op = op_mask.value(sbox_op_start[curr_sbox_info.curr_box], 
                               sbox_op_start[curr_sbox_info.curr_box] + sbox_op_sizes[curr_sbox_info.curr_box]); 
  // Get sieved entries
  auto entries = sboxes[curr_sbox_info.curr_box].sieve_lat(op);
  for (auto it = entries.begin(); it != entries.end(); ++it) {
    // Get score
    float score = it->second / float(1 << (sboxes[curr_sbox_info.curr_box].input_size + 1));
    // Debugging
    if (best && comp1[curr_sbox_info.curr_box] == (it->first & ((1 << sboxes[curr_sbox_info.curr_box].output_size) - 1))) {
      best = true;
    }
    else {
      best = false;
    }
    // Check if better
    if (PILING3(curr_round_info.curr_bias, score, curr_sbox_info.curr_bias, 
                fin_trails[rounds - 2 - curr_round_info.curr_round].curr_bias) > 
        FABS(fin_trails[rounds - 1].curr_bias)) {
      // If best, print debug info
      if (best) std::cout << "In recursion 3 for sbox " << curr_sbox_info.curr_box << std::endl;
      // Store current sbox info
      auto temp_s = curr_sbox_info;
      // Update current sbox info
      curr_sbox_info.ip_masks[curr_sbox_info.curr_box] = it->first >> sboxes[curr_sbox_info.curr_box].output_size;
      curr_sbox_info.op_masks[curr_sbox_info.curr_box] = it->first & ((1 << sboxes[curr_sbox_info.curr_box].output_size) - 1);
      curr_sbox_info.biases[curr_sbox_info.curr_box] = score;
      curr_sbox_info.curr_bias = eval_bias(curr_sbox_info.biases, curr_sbox_info.curr_box + 1);
      // Check if we reached the last sbox
      if (curr_sbox_info.curr_box + 1 < sboxes.size()) {
        // Increment current sbox
        curr_sbox_info.curr_box++;
        // Call recursion
        more_than_three_intermediate_sbox(rounds, op_mask);
        // Reset current sbox info
        curr_sbox_info = temp_s;
      } else {
        // Determine key-mask
        bitstr key_mask_bits = bitstr(stage_1);
        for (size_type i = 0; i < sboxes.size(); ++i) {
          key_mask_bits(sbox_ip_start[i], sbox_ip_start[i] + sbox_ip_sizes[i]) |= 
            block_type(curr_sbox_info.ip_masks[i]);
        }
        // Determine input-mask
        bitstr input_mask_bits = key_mask_bits.sinv_permute(rf_before);
        // Store current round info
        auto temp_r = curr_round_info;
        // Update current round info
        curr_round_info.ip_masks[curr_round_info.curr_round] = input_mask_bits;
        curr_round_info.op_masks[curr_round_info.curr_round] = op_mask;
        curr_round_info.key_masks[curr_round_info.curr_round] = key_mask_bits;
        curr_round_info.biases[curr_round_info.curr_round] = curr_sbox_info.curr_bias;
        curr_round_info.curr_bias = PILING(curr_round_info.curr_bias, curr_sbox_info.curr_bias);
        curr_round_info.curr_round = curr_round_info.curr_round + 1;
        // Check if we are at the penultimate round
        if (curr_round_info.curr_round < rounds - 1) {
          more_than_three_intermediate(rounds);
          curr_round_info = temp_r;
          curr_sbox_info = temp_s;
        }
        else {
          more_than_three_final(rounds);
          curr_round_info = temp_r;
          curr_sbox_info = temp_s;
        }
      }
    }
  }
  // Reach here (no more candidates)
  return;
}

// At last round
void trail::more_than_three_final(size_type rounds) {
  // Fix output masks
  bitstr fixed_op_mask = curr_round_info.op_masks[curr_round_info.curr_round - 2] ^
                          curr_round_info.ip_masks[curr_round_info.curr_round - 1];
  bitstr inv_op_mask = fixed_op_mask.inv_permute(rf_after);

  // Make a s-box state
  curr_sbox_info = sbox_info();

  // Create vectors for input, output and biases of size "sboxes.size()"
  curr_sbox_info.ip_masks = std::vector<size_type>(sboxes.size());
  curr_sbox_info.op_masks = std::vector<size_type>(sboxes.size());
  curr_sbox_info.biases = std::vector<float>(sboxes.size());

  // Call recursive function
  more_than_three_final_sbox(rounds, inv_op_mask);
  return;
}

// At last round for each sbox
void trail::more_than_three_final_sbox(size_type rounds, bitstr op_mask) {
  // Get slice
  size_type op = op_mask.value(sbox_op_start[curr_sbox_info.curr_box], 
                               sbox_op_start[curr_sbox_info.curr_box] + sbox_op_sizes[curr_sbox_info.curr_box]); 
  // Get top sieve-entry
  auto lat_entry = sboxes[curr_sbox_info.curr_box].sieve_lat(op)[0];
  // Get score
  float score = lat_entry.second / float(1 << (sboxes[curr_sbox_info.curr_box].input_size + 1));
  // Debugging
  if (best && comp2[curr_sbox_info.curr_box] == (lat_entry.first & ((1 << sboxes[curr_sbox_info.curr_box].output_size) - 1))) {
    best = true;
  }
  else {
    best = false;
  }
  // Check if better
  if (PILING2(curr_round_info.curr_bias, score, curr_sbox_info.curr_bias) > 
        FABS(fin_trails[rounds - 1].curr_bias)) {
    // If best, print debug info
    if (best) std::cout << "In recursion 4 for sbox " << curr_sbox_info.curr_box << std::endl;
    // Store current sbox info
    auto temp_s = curr_sbox_info;
    // Update current sbox info
    curr_sbox_info.ip_masks[curr_sbox_info.curr_box] = lat_entry.first >> sboxes[curr_sbox_info.curr_box].output_size;
    curr_sbox_info.op_masks[curr_sbox_info.curr_box] = lat_entry.first & ((1 << sboxes[curr_sbox_info.curr_box].output_size) - 1);
    curr_sbox_info.biases[curr_sbox_info.curr_box] = score;
    curr_sbox_info.curr_bias = eval_bias(curr_sbox_info.biases, curr_sbox_info.curr_box + 1);
    // Check if we reached the last sbox
    if (curr_sbox_info.curr_box + 1 < sboxes.size()) {
      // Increment current sbox
      curr_sbox_info.curr_box++;
      // Call recursion
      more_than_three_final_sbox(rounds, op_mask);
      // Reset current sbox info
      curr_sbox_info = temp_s;
    } else {
      // Determine key-mask
      bitstr key_mask_bits = bitstr(stage_1);
      for (size_type i = 0; i < sboxes.size(); ++i) {
        key_mask_bits(sbox_ip_start[i], sbox_ip_start[i] + sbox_ip_sizes[i]) |= 
          block_type(curr_sbox_info.ip_masks[i]);
      }
      // Determine input-mask
      bitstr input_mask_bits = key_mask_bits.sinv_permute(rf_before);
      // Store current round info
      auto temp_r = curr_round_info;
      // Update current round info
      curr_round_info.ip_masks[curr_round_info.curr_round] = input_mask_bits;
      curr_round_info.op_masks[curr_round_info.curr_round] = op_mask;
      curr_round_info.key_masks[curr_round_info.curr_round] = key_mask_bits;
      curr_round_info.biases[curr_round_info.curr_round] = curr_sbox_info.curr_bias;
      curr_round_info.curr_bias = PILING(curr_round_info.curr_bias, curr_sbox_info.curr_bias);
      std::cout << "BIAS UPDATE: " << curr_round_info.curr_bias << std::endl;
      // Add to final trails and biases
      fin_trails[rounds - 1] = curr_round_info;
      // Reset current info
      curr_round_info = temp_r;
      curr_sbox_info = temp_s;
    }
  }
  // Reach here (no more candidates)
  return;
}

// Upto n trails
void trail::upto(size_type rounds) {
  // Check if rounds is n0.0190735ot more than max_rounds
  if (rounds > max_rounds) {
    throw std::runtime_error("Number of rounds exceeds maximum rounds.");
  }
  // Check if rounds is less than/ equal to 3
  first_three();
  if (rounds <= 3) {
    return;
  }
  else {
    for (size_type i = 4; i <= rounds; ++i) more_than_three(i);
    return;
  }
  return;
}

// Get trail masks
std::tuple<bitstr, bitstr, bitstr> trail::trail_masks(size_type rounds) {
  // Checks
  if (rounds > max_rounds) {
    throw std::runtime_error("Number of rounds exceeds maximum rounds.");
  }
  if (rounds > fin_trails.size()) {
    throw std::runtime_error("Number of rounds exceeds number of trails found.");
  }

  // Get plaintext mask
  bitstr pt_mask = fin_trails[rounds-1].op_masks[0];
  pt_mask += (fin_trails[rounds-1].ip_masks[0] ^ fin_trails[rounds-1].op_masks[1]);

  // Get ciphertext mask
  bitstr ct_mask = fin_trails[rounds-1].op_masks[rounds-1];
  ct_mask += (fin_trails[rounds-1].ip_masks[rounds-1] ^ fin_trails[rounds-1].op_masks[rounds-2]);

  // Get key mask
  bitstr key_mask = bitstr(key_size);
  for (size_type i = 0; i < rounds; ++i) {
    auto curr_mask = fin_trails[rounds-1].key_masks[i];
    for (size_type j = 0; j < curr_mask.bit_size; ++j) {
      if (curr_mask[j]) ~key_mask[round_sch[i][j]];
    }
  }

  // Return the masks
  return std::make_tuple(pt_mask, ct_mask, key_mask);
}

bitstr trail::sub_trail_masks(size_type rounds) {
  // Checks
  if (rounds > max_rounds) {
    throw std::runtime_error("Number of rounds exceeds maximum rounds.");
  }
  if (rounds > fin_trails.size()) {
    throw std::runtime_error("Number of rounds exceeds number of trails found.");
  }

  // Get sub-key mask (just for last round)
  bitstr sub_key_mask = bitstr(key_size);
  auto curr_mask = fin_trails[rounds-1].key_masks[rounds-1];
  for (size_type j = 0; j < curr_mask.bit_size; ++j) {
    if (curr_mask[j]) ~sub_key_mask[round_sch[rounds-1][j]];
  }

  // Return the sub-key mask
  return sub_key_mask;
}
