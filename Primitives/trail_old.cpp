// Routine to identify good linear trails for a feistel cipher

// Include header
#include "trail_old.h"

// Helpers

// Function to interpret an LAT entry into a mask
std::tuple<bitstr, bitstr, bitstr> interpret_lats(feistel cipher, size_type sbox_num, size_type ip_mask, size_type op_mask)
{
  // Iterate and check
  size_type ip_start = 0;
  size_type op_start = 0;
  for (size_type i = 0; i < sbox_num; i++) {
    ip_start += cipher.sboxes[i].input_size;
    op_start += cipher.sboxes[i].output_size;
  }

  // Get key-mask
  bitstr key_ret(cipher.rf_before.op_size);
  key_ret(ip_start, ip_start+cipher.sboxes[sbox_num].input_size) = block_type(ip_mask);

  // Get input-mask
  bitstr ip_ret = key_ret.sinv_permute(cipher.rf_before);

  // Get output-mask
  bitstr temp(cipher.rf_after.ip_size);
  temp(op_start, op_start+cipher.sboxes[sbox_num].output_size) = block_type(op_mask);
  bitstr op_ret = temp.permute(cipher.rf_after);

  // Return
  return std::make_tuple(ip_ret, op_ret, key_ret);
}

// Print round_info
void print_round_info(const round_info& info)
{
  std::cout << "Total Rounds: " << info.tot_rounds << "\n";
  std::cout << "Current Round: " << info.current_round << "\n";
  std::cout << "Bias: " << info.bias << "\n";
  std::cout << "Input Masks: ";
  for (const auto& mask : info.ip_masks) {
    std::cout << mask.get_bits() << " : ";
  }
  std::cout << "\nOutput Masks: ";
  for (const auto& mask : info.op_masks) {
    std::cout << mask.get_bits() << " : ";
  }
  std::cout << "\nKey Masks: ";
  for (const auto& mask : info.key_masks) {
    std::cout << mask.get_bits() << " : ";
  }
  std::cout << "\nBiases: ";
  for (const auto& bias : info.biases) {
    std::cout << bias << " ";
  }
  std::cout << "\n" << std::endl;
}

// Evaluate overall bias
float eval_bias(std::vector<float> biases, size_type end)
{
  if (end == 0) return 0.5f;
  else if (end == 1) return biases[0];
  float res = biases[0];
  for (size_type i = 1; i < end; ++i) {
    res *= 2*biases[i];
  }
  return res;
}

// Find the first three trails
std::vector<std::vector<round_info>> first_three(feistel cipher)
{
  // ROUND 1: Iterate through all S-Box entries and pick the best MEMO_RATE ones
  std::vector<round_info> first(MEMO_RATE);
  std::vector<float> first_scores(MEMO_RATE, 0.0f);
  for (auto it = cipher.sboxes.begin(); it != cipher.sboxes.end(); ++it) {
    size_type sbox_num = it - cipher.sboxes.begin();
    for (auto lat_it = it->lat.begin() + 1; lat_it != it->lat.end(); ++lat_it) {
      // Get score
      float score = lat_it->second / float(1 << (it->input_size + 1));
      // Determine if and where to add this entry
      size_type index = 0;
      for (; index < MEMO_RATE; ++index) {
        if (std::abs(first_scores[index]) < std::abs(score)) {
          break;
        }
      }
      // If we found a place, insert it
      if (index < MEMO_RATE) {
        // Interpret the LAT entry
        auto [ip_mask, op_mask, key_mask] = interpret_lats(cipher, sbox_num, lat_it->first >> it->output_size, lat_it->first & ((1 << it->output_size) - 1));
        // Make vectors
        std::vector<bitstr> ip = {ip_mask}; 
        std::vector<bitstr> op = {op_mask}; 
        std::vector<bitstr> key = {key_mask};
        std::vector<float> bss = {score};
        // Create round_info
        round_info info(1, score, ip, op, key, bss);
        // Insert
        first.insert(first.begin() + index, info);
        first_scores.insert(first_scores.begin() + index, score);
        // If we exceeded the size, remove the last one
        if (first.size() > MEMO_RATE) {
          first.pop_back();
          first_scores.pop_back();
        }
      }
    }
  }

  // ROUND 2: Add all zeroes to the next round
  std::vector<round_info> second = first;
  for (auto& info : second) {
    // Add zeroes
    info.ip_masks.push_back(bitstr(cipher.rf_before.ip_size, 0));
    info.op_masks.push_back(bitstr(cipher.rf_after.op_size, 0));
    info.key_masks.push_back(bitstr(cipher.rf_before.op_size, 0));
    info.biases.push_back(0.5f);
    info.tot_rounds++;
    // Score remains the same
  }

  // ROUND 3: Add the first entry of the vector
  std::vector<round_info> third = second;
  for (auto& info : third) {
    // Add the first entry of the vector
    info.ip_masks.push_back(info.ip_masks[0]);
    info.op_masks.push_back(info.op_masks[0]);
    info.key_masks.push_back(info.key_masks[0]);
    info.biases.push_back(info.biases[0]);
    info.tot_rounds++;
    info.bias = 2 * info.bias * info.bias;
  }

  // Return the first three rounds
  std::vector<std::vector<round_info>> ret = {first, second, third};
  return ret;
}

// Find more trails
round_info find_trail_one(feistel cipher, size_type rounds, std::vector<std::vector<round_info>> memo)
{
  // Ensure memo has length = rounds-1
  if (memo.size() != rounds-1) throw std::runtime_error("Invalid memo size.");

  // Instatiate a state
  round_info state;
  state.tot_rounds = rounds;
  state.current_round = 0;
  state.bias = 0.02;
  state.ip_masks = std::vector<bitstr>(rounds);
  state.op_masks = std::vector<bitstr>(rounds);
  state.key_masks = std::vector<bitstr>(rounds);
  state.biases = std::vector<float>(rounds);
  state.baseline = 0.0f;

  // For every candidate output-mask (SBox wise)
  for (auto it = cipher.sboxes.begin(); it != cipher.sboxes.end(); ++it)
  {
    // Get S-Box
    size_type sbox_num = it - cipher.sboxes.begin();
    size_type lim = 1 << it->output_size;
    // Iterate through output-masks
    for (size_type op = 0; op < lim; ++op)
    {
      // Get the best entry from sieve-LAT
      auto lat_entry = it->sieve_lat(op)[0];
      // Get scores
      float score = lat_entry.second / float(1 << (it->input_size + 1));
      // Check if better
      if (std::abs(2*score*memo[rounds-2][0].bias) > std::abs(state.bias)) {
        // Interpret the LAT entry
        auto [ip_mask, op_mask, key_mask] = interpret_lats(cipher, sbox_num, lat_entry.first >> it->output_size, lat_entry.first & ((1 << it->output_size) - 1));
        // Update state
        state.ip_masks[state.current_round] = ip_mask;
        state.op_masks[state.current_round] = op_mask;
        state.key_masks[state.current_round] = key_mask;
        state.biases[state.current_round] = score;
        state.current_round++;
        print_round_info(state);
        // Call next
        find_trail_two(cipher, memo, state);
        state.current_round--;
      } 
    }
  }

  // If we reach here, we have no more candidates
  return state;
}

void find_trail_two(feistel cipher, std::vector<std::vector<round_info>> memo, round_info &state)
{
  // Iterate through each LAT entry of each S-Box
  for (auto it = cipher.sboxes.begin(); it != cipher.sboxes.end(); ++it)
  {
    // Get S-Box number
    size_type sbox_num = it - cipher.sboxes.begin();
    // Iterate through all LAT entries
    for (auto lat_it = it->lat.begin() + 1; lat_it != it->lat.end(); ++lat_it)
    {
      // Get score
      float score = (lat_it->second / float(1 << (it->input_size + 1)));
      // Check if better than current state
      if (std::abs((2*score*eval_bias(state.biases, 1)) * memo[state.tot_rounds-3][0].bias) > std::abs(state.bias)) {
        // Interpret the LAT entry
        auto [ip_mask, op_mask, key_mask] = interpret_lats(cipher, sbox_num, lat_it->first >> it->output_size, lat_it->first & ((1 << it->output_size) - 1));
        // Update state
        state.ip_masks[state.current_round] = ip_mask;
        state.op_masks[state.current_round] = op_mask;
        state.key_masks[state.current_round] = key_mask;
        state.biases[state.current_round] = score;
        state.current_round++;
        print_round_info(state);
        // Call next
        if(state.current_round == state.tot_rounds-1) {
          find_trail_fin(cipher, memo, state);
          state.current_round--;
        }
        else {
          find_trail_inter(cipher, memo, state);
          state.current_round--;
        }
      }
    }
  }
  return;
}

void find_trail_inter(feistel cipher, std::vector<std::vector<round_info>> memo, round_info &state)
{
  // Fix output mask
  bitstr fixed_op_mask = state.op_masks[state.current_round - 2] ^ state.ip_masks[state.current_round - 1];
  bitstr inv_op_mask = fixed_op_mask.inv_permute(cipher.rf_after);

  // Instantiate a new sbox state
  sbox_info sbox_state;
  sbox_state.tot_boxes = cipher.sboxes.size();
  sbox_state.bias = 0.5f;
  sbox_state.ip_masks = std::vector<size_type>(sbox_state.tot_boxes, 0);
  sbox_state.op_masks = std::vector<size_type>(sbox_state.tot_boxes, 0);
  sbox_state.biases = std::vector<float>(sbox_state.tot_boxes, 0.0f);
  sbox_state.current_box = 0;

  // Call
  find_trail_inter_sbox(cipher, memo, state, sbox_state, inv_op_mask);
  return;
}

void find_trail_inter_sbox(feistel cipher, std::vector<std::vector<round_info>> memo, round_info &state, sbox_info &sbox_state, bitstr op_mask)
{
  // Start index of slice
  size_type start_index = 0;
  for (size_type i = 0; i < sbox_state.current_box; i++) {
    start_index += cipher.sboxes[i].output_size;
  }
  // Get slice
  size_type op = op_mask.value(start_index, start_index + cipher.sboxes[sbox_state.current_box].output_size);
  // Fix this as op-mask and iterate through all sieve-LAT entries
  auto lat_entries = cipher.sboxes[sbox_state.current_box].sieve_lat(op);
  for (auto it = lat_entries.begin(); it != lat_entries.end(); ++it) {
    // Get score
    float score = it->second / float(1 << (cipher.sboxes[sbox_state.current_box].input_size + 1));
    // Check if better than current state
    if (std::abs((2*eval_bias(state.biases, state.current_round-1)*(2*score*sbox_state.bias)) * memo[state.tot_rounds-state.current_round-1][0].bias) > std::abs(state.bias)) {
      // Update sbox state
      sbox_state.ip_masks[sbox_state.current_box] = it->first >> cipher.sboxes[sbox_state.current_box].output_size;
      sbox_state.op_masks[sbox_state.current_box] = it->first & ((1 << cipher.sboxes[sbox_state.current_box].output_size) - 1);
      sbox_state.biases[sbox_state.current_box] = score;
      sbox_state.bias = eval_bias(sbox_state.biases, sbox_state.current_box);

      // Call next sbox or next round
      if (sbox_state.current_box == sbox_state.tot_boxes - 1) {
        // Determine key-mask
        bitstr key_mask(cipher.rf_before.op_size);
        size_type ip_start = 0;
        for (size_type i = 0; i < sbox_state.tot_boxes; i++) {
          key_mask(ip_start, ip_start + cipher.sboxes[i].input_size) |= block_type(sbox_state.ip_masks[i]);
          ip_start += cipher.sboxes[i].input_size;
        }

        // Determine input-mask
        bitstr ip_mask = key_mask.sinv_permute(cipher.rf_before);

        // Update state
        state.ip_masks[state.current_round] = ip_mask;
        state.op_masks[state.current_round] = op_mask.permute(cipher.rf_after);
        state.key_masks[state.current_round] = key_mask;
        state.biases[state.current_round] = sbox_state.bias;
        state.current_round++;

        // Check if we have enough rounds
        if (state.current_round == state.tot_rounds-1) {
          // Call final
          print_round_info(state);
          find_trail_fin(cipher, memo, state);
          state.current_round--;
        }
        else {
          // Call next
          print_round_info(state);
          find_trail_inter(cipher, memo, state);
          state.current_round--;
        }
      }
      else {
        sbox_state.current_box++;
        // Call next
        find_trail_inter_sbox(cipher, memo, state, sbox_state, op_mask);
      }
    }
  }
  return;
}

void find_trail_fin(feistel cipher, std::vector<std::vector<round_info>> memo, round_info &state)
{
  // Fix output mask
  bitstr fixed_op_mask = state.op_masks[state.current_round - 2] ^ state.ip_masks[state.current_round - 1];
  bitstr inv_op_mask = fixed_op_mask.inv_permute(cipher.rf_after);

  // Instantiate a new sbox state
  sbox_info sbox_state;
  sbox_state.tot_boxes = cipher.sboxes.size();
  sbox_state.bias = 0.5f;
  sbox_state.ip_masks = std::vector<size_type>(sbox_state.tot_boxes, 0);
  sbox_state.op_masks = std::vector<size_type>(sbox_state.tot_boxes, 0);
  sbox_state.biases = std::vector<float>(sbox_state.tot_boxes, 0.0f);
  sbox_state.current_box = 0;

  // Call
  find_trail_fin_sbox(cipher, memo, state, sbox_state, inv_op_mask);
  return; 
}

void find_trail_fin_sbox(feistel cipher, std::vector<std::vector<round_info>> memo, round_info &state, sbox_info &sbox_state, bitstr op_mask)
{
  // Start index of slice
  size_type start_index = 0;
  for (size_type i = 0; i < sbox_state.current_box; i++) {
    start_index += cipher.sboxes[i].output_size;
  }
  // Get slice
  size_type op = op_mask.value(start_index, start_index + cipher.sboxes[sbox_state.current_box].output_size);
  // Get best possible sieve-LAT entry
  auto lat_entry = cipher.sboxes[sbox_state.current_box].sieve_lat(op)[0];
  // Get score
  float score = lat_entry.second / float(1 << (cipher.sboxes[sbox_state.current_box].input_size + 1));


  // Check if better than current state
  if (std::abs(eval_bias(state.biases, state.current_round-1)*(2*score*sbox_state.bias)) > std::abs(state.bias)) {
    // Update sbox state
    sbox_state.ip_masks[sbox_state.current_box] = lat_entry.first >> cipher.sboxes[sbox_state.current_box].output_size;
    sbox_state.op_masks[sbox_state.current_box] = lat_entry.first & ((1 << cipher.sboxes[sbox_state.current_box].output_size) - 1);
    sbox_state.biases[sbox_state.current_box] = score;
    sbox_state.bias = eval_bias(sbox_state.biases, sbox_state.current_box);
    std::cout << sbox_state.bias << std::endl;
    
    // Call next sbox?
    if (sbox_state.current_box != sbox_state.tot_boxes - 1) {
      sbox_state.current_box++;
      // Call next
      find_trail_fin_sbox(cipher, memo, state, sbox_state, op_mask);
    }
    else {
      // Determine key-mask
      bitstr key_mask(cipher.rf_before.op_size);
      size_type ip_start = 0;
      for (size_type i = 0; i < sbox_state.tot_boxes; i++) {
        key_mask(ip_start, ip_start + cipher.sboxes[i].input_size) |= block_type(sbox_state.ip_masks[i]);
        ip_start += cipher.sboxes[i].input_size;
      }

      // Determine input-mask
      bitstr ip_mask = key_mask.sinv_permute(cipher.rf_before);

      // Update state
      state.ip_masks[state.current_round] = ip_mask;
      state.op_masks[state.current_round] = op_mask.permute(cipher.rf_after);
      state.key_masks[state.current_round] = key_mask;
      state.biases[state.current_round] = sbox_state.bias;
      state.bias = eval_bias(state.biases, state.tot_rounds);
      std::cout << "GGGGGGGGGGGGGGGGGGGGGGGGGGG" << std::endl;
      print_round_info(state);
    }
  }
 
}
