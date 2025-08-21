// Implementation of the Attack class

#include "attack.h"

// Helper function
bitstr rand_bitstr(size_type size) {
  bitstr result(size);
  for (size_type i = 0; i < size; ++i) {
      result[i] = rand() % 2; // Randomly set bits to 0 or 1
  }
  return result;
}

std::string get_tern_bits(std::vector<short_type> arr) {
  std::string result;
  for (size_type i = 0; i < arr.size(); ++i) {
    switch (arr[i]) {
      case 0:
        result += "0";
        break;
      case 1:
        result += "1";
        break;
      default:
        result += "x";
    }
  }
  return result;
}

// Recursive Walsh transform function
std::vector<float> walsh_transform(const std::vector<float> &input) {
  size_type n = input.size();
  if (n == 1) {
    return input; // Base case
  }

  // Split the input into even and odd parts
  std::vector<float> even(n / 2);
  std::vector<float> odd(n / 2);
  for (size_type i = 0; i < n / 2; ++i) {
    even[i] = input[2 * i];
    odd[i] = input[2 * i + 1];
  }

  // Recursive calls
  std::vector<float> even_transform = walsh_transform(even);
  std::vector<float> odd_transform = walsh_transform(odd);

  // Combine results
  std::vector<float> result(n);
  for (size_type i = 0; i < n / 2; ++i) {
    result[i] = even_transform[i] + odd_transform[i];
    result[i + n / 2] = even_transform[i] - odd_transform[i];
  }
  
  return result;
}

std::vector<float> walsh_transform(int* input, size_type size) {
  // Convert input to a vector
  std::vector<float> input_vector(size);
  for (size_type i = 0; i < size; ++i) {
    input_vector[i] = static_cast<float>(input[i]);
  }

  // Call the recursive Walsh transform function
  return walsh_transform(input_vector);
}

std::vector<float> walsh_transform(bool* input, size_type size) {
  // Convert input to a vector
  std::vector<float> input_vector(size);
  for (size_type i = 0; i < size; ++i) {
    input_vector[i] = input[i] ? 1.0f : -1.0f; // Convert bool to float
  }

  // Call the recursive Walsh transform function
  return walsh_transform(input_vector);
}

// Constructor
attack::attack(const bitstr &pt_mask, const bitstr &ct_mask, const bitstr &key_mask, float bias, size_type rounds, feistel cipher) {
  // Few assertions to be checked for
  if (pt_mask.bit_size != cipher.block_size) throw std::invalid_argument("Invalid plaintext mask");
  if (ct_mask.bit_size != cipher.block_size) throw std::invalid_argument("Invalid ciphertext mask");
  if (key_mask.bit_size != cipher.key_size) throw std::invalid_argument("Invalid key mask");

  // Assign
  this->pt_mask = pt_mask;
  this->ct_mask = ct_mask;
  this->key_mask = key_mask;
  this->bias = bias;
  this->rounds = rounds;
  this->cipher = cipher;
}

// Matsui's 1
bool attack::matsui1(size_type trials) {
  size_type cnt = 0;
  for (size_type i = 0; i < trials; ++i) {
    // Get random plaintext
    bitstr pt = rand_bitstr(cipher.block_size);
    // Get encryption
    bitstr ct = cipher.encrypt(pt, rounds);

    // Modify pt and ct as per initial and final permutations
    bitstr pt_mod = pt.permute(cipher.ip);
    bitstr ct_mod = ct.inv_permute(cipher.fp);

    // Get rhs value
    bool rhs = (pt_mod * pt_mask) ^ (ct_mod * ct_mask);
    if (rhs) cnt++;
  }

  if ( (cnt-trials/2) * bias >= 0) return true;
  else return false;
}

// Matsui's 2
std::tuple<std::vector<short_type>, bool> attack::matsui2(size_type trials) {
  // Get left and right halves of the ct_mask
  bitstr ct_mask_l = ct_mask.extract(0, cipher.block_size / 2);
  bitstr ct_mask_r = ct_mask.extract(cipher.block_size / 2, cipher.block_size);

  // Peel back right half of the ciphertext mask
  bitstr ct_mask_peel = ct_mask_r.inv_permute(cipher.rf_after);

  // DEBUG
  std::cout << "ct_mask_peel: " << ct_mask_peel.get_bits() << std::endl;

  // Identify active s-boxes
  std::vector<std::pair<size_type, size_type>> actives;
  size_type start_ip = 0;
  size_type start_op = 0;
  size_type net = 0;

  for (auto it = cipher.sboxes.begin(); it != cipher.sboxes.end(); ++it) {
    if (ct_mask_peel.value(start_op, start_op + it->output_size) != 0) {
      std::cout << "active sbox at: " << start_ip << " with input size: " << it->input_size << std::endl;
      actives.emplace_back(start_ip, it->input_size);
      net += it->input_size;
    }
    start_ip += it->input_size;
    start_op += it->output_size;
  }

  // Create baskets for candidates
  std::vector<std::pair<size_type, int>> baskets;
  for (size_type i = 0; i < (size_type(1)<<net); ++i) baskets.emplace_back(i, 0);

  // Iterate through trials
  for (size_type i = 0; i < trials; ++i) {
    // Get random plaintext
    bitstr pt = rand_bitstr(cipher.block_size);
    // Get encryption
    bitstr ct = cipher.encrypt(pt, rounds+1);
    // Modify pt and ct as per initial and final permutations
    bitstr pt_mod = pt.permute(cipher.ip);
    bitstr ct_mod = ct.inv_permute(cipher.fp);
    // Get left and right halves of ct_mod
    bitstr ct_mod_l = ct_mod.extract(0, cipher.block_size / 2);
    bitstr ct_mod_r = ct_mod.extract(cipher.block_size / 2, cipher.block_size);
    // Get temporary rhs value
    bool rhs = (pt_mod * pt_mask) ^ (ct_mod_l * ct_mask_r) ^ (ct_mod_r * ct_mask_l);

    // Iterate through all candidates
    for (size_type j = 0; j < baskets.size(); ++j) {
      // Create empty bistring of round-key size
      bitstr cand(cipher.rf_before.op_size);
      // Set bits in appropriate positions
      size_type shift = net;
      for (auto it = actives.begin(); it != actives.end(); ++it) {
        shift -= it->second;
        cand(it->first, it->first + it->second) = (j >> shift) & ((1 << it->second) - 1);
      }
      // Compute round function on right-half of ciphertext
      bitstr rf_out = cipher.rfunc(ct_mod_r, cand);

      // DEBUG
      if (i == 0) {
        std::cout << rf_out.get_bits() << std::endl;
        std::cout << ct_mask_r.get_bits() << std::endl;
        std::cout << std::endl;
      }

      // Get new rhs value
      bool new_rhs = rhs ^ (rf_out * ct_mask_r);
      // Update basket appropriately
      if (new_rhs) {
        baskets[j].second++;
      } else {
        baskets[j].second--;
      }
    }
  }

  // Sort baskets based on absolute counts
  std::sort(baskets.begin(), baskets.end(), [](const std::pair<size_type, int> &a, const std::pair<size_type, int> &b) {
    return std::abs(a.second) > std::abs(b.second);
  });

  // Print baskets for debugging
  std::cout << "Trials: \t" << trials << std::endl;
  for (const auto &basket : baskets) {
    std::cout << basket.first << ": " << basket.second << "\t\t";
  }
  std::cout << "\n" << std::endl;

  // Get best basket
  size_type best_basket = baskets[0].first;

  // Get round+1 key-schedule
  auto key_sch = cipher.round_sch[rounds];

  // Compute and make the candidate key
  std::vector<short_type> final_cand(cipher.key_size, -1);
  size_type shift = net;
  for (auto it = actives.begin(); it != actives.end(); ++it) {
    for (size_type j = 0; j < it->second; ++j) {
      bool bit = (best_basket >> (shift - j - 1)) & 1;
      final_cand[key_sch[it->first + j]] = bit ? 1 : 0;
    }
    shift -= it->second;
  }

  // DEBUG: get the top 10 candidate keys and print
  std::cout << "Top 10 candidate keys:" << std::endl;
  for (size_type i = 0; i < std::min(size_type(10), baskets.size()); ++i) {
    size_type cand = baskets[i].first;
    std::vector<short_type> cand_key(cipher.key_size, -1);
    size_type shift = net;
    for (auto it = actives.begin(); it != actives.end(); ++it) {
      for (size_type j = 0; j < it->second; ++j) {
        bool bit = (cand >> (shift - j - 1)) & 1;
        cand_key[key_sch[it->first + j]] = bit ? 1 : 0;
      }
      shift -= it->second;
    }
    std::cout << "Candidate " << i + 1 << "\t: " << get_tern_bits(cand_key) << " with count: " << baskets[i].second << std::endl;
  }

  // Get final rhs bool
  bool final_rhs = (baskets[0].second * bias > 0);
  
  // Return the candidate key and the final rhs bool
  return std::make_tuple(final_cand, final_rhs);
}

// Matsui's 2 (Distilled Version)
std::tuple<std::vector<short_type>, bool> attack::matsui2_dist(size_type trials) {
  // Get left and right halves of the ct_mask
  bitstr ct_mask_l = ct_mask.extract(0, cipher.block_size / 2);
  bitstr ct_mask_r = ct_mask.extract(cipher.block_size / 2, cipher.block_size);

  // Peel back right half of the ciphertext mask
  bitstr ct_mask_peel = ct_mask_r.inv_permute(cipher.rf_after);

  // Find active sboxes on the bottom-side
  std::vector<std::pair<size_type, size_type>> active;
  size_type start_ip = 0;
  size_type start_op = 0;
  size_type net = 0;
  bitstr key_bits(cipher.rf_before.op_size);
  for (auto it = cipher.sboxes.begin(); it != cipher.sboxes.end(); ++it) {
    if (ct_mask_peel.value(start_op, start_op + it->output_size) != 0) {
      active.emplace_back(start_ip, it->input_size);
      net += it->input_size;
      key_bits(start_ip, start_ip + it->input_size) = (1 << it->input_size) - 1;
    }
    start_ip += it->input_size;
    start_op += it->output_size;
  }
  std::cout << key_bits.get_bits() << std::endl;
  bitstr ct_bits = key_bits.sinv_permute(cipher.rf_before);

  // Get indices for effective ct bits
  auto eff_ct_bits = ct_bits.one_indices();

  // DISTILLATION PHASE
  // Create table for effective ct bits (net)
  int* table = new int[1 << (net)]();
  for (size_type i = 0; i < (size_type(1) << (net)); ++i) table[i] = 0;

  // Iterate through trials and update table
  for (size_type i = 0; i < trials; ++i) {
    // Generate random plaintext
    bitstr pt = rand_bitstr(cipher.block_size);
    // Encrypt plaintext (rounds+2)
    bitstr ct = cipher.encrypt(pt, rounds + 1);
    // Modify pt and ct as per initial and final permutations
    bitstr pt_mod = pt.permute(cipher.ip);
    bitstr ct_mod = ct.inv_permute(cipher.fp);
    // Get left and right halves of ct_mod
    bitstr ct_mod_l = ct_mod.extract(0, cipher.block_size / 2);
    bitstr ct_mod_r = ct_mod.extract(cipher.block_size / 2, cipher.block_size);

    // Get temporary rhs value 
    bool rhs = (pt_mod * pt_mask) ^ (ct_mod_l * ct_mask_r) ^ (ct_mod_r * ct_mask_l);

    // Extract effective bits from pt_mod and ct_mod
    size_type eff_ct_mod = ct_mod_r.get_bits(eff_ct_bits);

    // Increment or decrement table entry based on rhs
    if (rhs) {
      table[eff_ct_mod]++;
    } else {
      table[eff_ct_mod]--;
    }
  }

  // Print table for debugging
  std::cout << "Trials: \t" << trials << std::endl;
  for (size_type i = 0; i < (size_type(1) << net); ++i) {
    std::cout << i << ":\t" << table[i] << std::endl;
  }

  // ANALYSIS PHASE
  // Find the best candidate key (net_top + net_bot bits) via baskets
  std::vector<std::pair<size_type, int>> baskets;
  for (size_type i = 0; i < (size_type(1) << net); ++i) {
    baskets.emplace_back(i, 0);
  }

  // Iterate through each candidate key
  for (size_type i = 0; i < (size_type(1) << net); ++i) {
    // Candidate key (bottom)
    bitstr cand(cipher.rf_before.op_size);
    size_type shift = net;
    for (auto it = active.begin(); it != active.end(); ++it) {
      shift -= it->second;
      cand(it->first, it->first + it->second) = (i >> shift) & ((1 << it->second) - 1);
    }

    // Iterate over each table entry
    for (size_type j = 0; j < (size_type(1) << net); ++j) {
      // Create bitstrings for effective bits and set them
      bitstr eff(cipher.rf_before.ip_size);
      eff.set_bits(eff_ct_bits, j);

      // Compute round function on right-half of ciphertext
      bitstr rf_out = cipher.rfunc(eff, cand);

      // Get temp rhs value
      if (rf_out * ct_mask_r) {
        baskets[i].second += table[j];
      } else {
        baskets[i].second -= table[j];
      }
    }
  }

  // Sort baskets based on absolute counts
  std::sort(baskets.begin(), baskets.end(), [](const std::pair<size_type, int> &a, const std::pair<size_type, int> &b) {
    return std::abs(a.second) > std::abs(b.second);
  });

  // Print baskets for debugging
  std::cout << "Trials: \t" << trials << std::endl;
  for (const auto &basket : baskets) {
    std::cout << basket.first << ": " << basket.second << "\t\t";
  }
  std::cout << "\n" << std::endl;

  // Get best basket
  size_type best_basket = baskets[0].first;

  // Get (round+1)th key-schedule
  auto key_sch = cipher.round_sch[rounds];

  // Compute and make the candidate key
  std::vector<short_type> final_cand(cipher.key_size, -1);
  size_type shift = net;
  for (auto it = active.begin(); it != active.end(); ++it) {
    for (size_type j = 0; j < it->second; ++j) {
      bool bit = (best_basket >> (shift - j - 1)) & 1;
      final_cand[key_sch[it->first + j]] = bit ? 1 : 0;
    }
    shift -= it->second;
  }

  // Get final rhs bool
  bool final_rhs = (baskets[0].second * bias > 0);

  // Clean up table memory
  delete[] table;

  // Return the candidate key and the final rhs bool
  return std::make_tuple(final_cand, final_rhs);
}

// Matsui's 2 (Walsh Version)
std::tuple<std::vector<short_type>, bool> attack::matsui2_walsh(size_type trials) {
  // Get left and right halves of the ct_mask
  bitstr ct_mask_l = ct_mask.extract(0, cipher.block_size / 2);
  bitstr ct_mask_r = ct_mask.extract(cipher.block_size / 2, cipher.block_size);

  // Peel back right half of the ciphertext mask
  bitstr ct_mask_peel = ct_mask_r.inv_permute(cipher.rf_after);

  // Find active sboxes on the bottom-side
  std::vector<std::pair<size_type, size_type>> active;
  size_type start_ip = 0;
  size_type start_op = 0;
  size_type net = 0;
  bitstr key_bits(cipher.rf_before.op_size);
  for (auto it = cipher.sboxes.begin(); it != cipher.sboxes.end(); ++it) {
    if (ct_mask_peel.value(start_op, start_op + it->output_size) != 0) {
      active.emplace_back(start_ip, it->input_size);
      net += it->input_size;
      key_bits(start_ip, start_ip + it->input_size) = (1 << it->input_size) - 1;
    }
    start_ip += it->input_size;
    start_op += it->output_size;
  }
  std::cout << key_bits.get_bits() << std::endl;
  bitstr ct_bits = key_bits.sinv_permute(cipher.rf_before);

  // Get indices for effective ct bits
  auto eff_ct_bits = ct_bits.one_indices();

  // DISTILLATION PHASE
  // Create table for effective ct bits (net)
  int* table = new int[1 << (net)]();
  for (size_type i = 0; i < (size_type(1) << (net)); ++i) table[i] = 0;

  // Iterate through trials and update table
  for (size_type i = 0; i < trials; ++i) {
    // Generate random plaintext
    bitstr pt = rand_bitstr(cipher.block_size);
    // Encrypt plaintext (rounds+2)
    bitstr ct = cipher.encrypt(pt, rounds + 1);
    // Modify pt and ct as per initial and final permutations
    bitstr pt_mod = pt.permute(cipher.ip);
    bitstr ct_mod = ct.inv_permute(cipher.fp);
    // Get left and right halves of ct_mod
    bitstr ct_mod_l = ct_mod.extract(0, cipher.block_size / 2);
    bitstr ct_mod_r = ct_mod.extract(cipher.block_size / 2, cipher.block_size);

    // Get temporary rhs value 
    bool rhs = (pt_mod * pt_mask) ^ (ct_mod_l * ct_mask_r) ^ (ct_mod_r * ct_mask_l);

    // Extract effective bits from pt_mod and ct_mod
    size_type eff_ct_mod = ct_mod_r.get_bits(eff_ct_bits);

    // Increment or decrement table entry based on rhs
    if (rhs) {
      table[eff_ct_mod]++;
    } else {
      table[eff_ct_mod]--;
    }
  }

  // Print table for debugging
  std::cout << "Trials: \t" << trials << std::endl;
  for (size_type i = 0; i < (size_type(1) << net); ++i) {
    std::cout << i << ":\t" << table[i] << std::endl;
  }

  // ANALYSIS PHASE
  // Iterate through all candidate keys
  bool* func_values = new bool[1 << net];
  for (size_type i = 0; i < (size_type(1) << net); ++i) {
    // Create empty bistring of round-key size
    bitstr cand(cipher.rf_before.op_size);
    // Set bits in appropriate positions
    size_type shift = net;
    for (auto it = active.begin(); it != active.end(); ++it) {
      shift -= it->second;
      cand(it->first, it->first + it->second) = (i >> shift) & ((1 << it->second) - 1);
    }

    // Create empty bitstring
    bitstr add_space = bitstr(cipher.rf_before.ip_size);

    // Compute round function on right-half of ciphertext
    bitstr rf_out = cipher.rfunc(add_space, cand);
    func_values[i] = (rf_out * ct_mask_r);
  }

  // Compute Walsh Transforms on table, func_values
  std::vector<float> table_walsh = walsh_transform(table, 1 << net);
  std::vector<float> func_values_walsh = walsh_transform(func_values, 1 << net);

  // Make Baskets
  std::vector<std::pair<size_type, float>> baskets;
  for (size_type i = 0; i < (size_type(1) << net); ++i) {
    baskets.emplace_back(i, 0.0f);
  }

  // Multiply the Walsh transforms (component-wise) and store results in baskets
  for (size_type i = 0; i < (size_type(1) << net); ++i) {
    baskets[i].second = table_walsh[i] * func_values_walsh[i];
  }

  // Sort baskets based on absolute counts
  std::sort(baskets.begin(), baskets.end(), [](const std::pair<size_type, int> &a, const std::pair<size_type, int> &b) {
    return std::abs(a.second) > std::abs(b.second);
  });

  // Print baskets for debugging
  std::cout << "Trials: \t" << trials << std::endl;
  for (const auto &basket : baskets) {
    std::cout << basket.first << ": " << basket.second << "\t\t";
  }
  std::cout << "\n" << std::endl;

  // Get best basket
  size_type best_basket = baskets[0].first;

  // Get (round+1)th key-schedule
  auto key_sch = cipher.round_sch[rounds];

  // Compute and make the candidate key
  std::vector<short_type> final_cand(cipher.key_size, -1);
  size_type shift = net;
  for (auto it = active.begin(); it != active.end(); ++it) {
    for (size_type j = 0; j < it->second; ++j) {
      bool bit = (best_basket >> (shift - j - 1)) & 1;
      final_cand[key_sch[it->first + j]] = bit ? 1 : 0;
    }
    shift -= it->second;
  }

  // Get final rhs bool
  bool final_rhs = (baskets[0].second * bias > 0);

  // Clean up table memory
  delete[] table;
  delete[] func_values;

  // Return the candidate key and the final rhs bool
  return std::make_tuple(final_cand, final_rhs);
}


/* std::tuple<std::vector<short_type>, bool> attack::matsui2_dist(size_type trials) {
  // Get left and right halves of the pt_mask
  bitstr pt_mask_l = pt_mask.extract(0, cipher.block_size / 2);
  bitstr pt_mask_r = pt_mask.extract(cipher.block_size / 2, cipher.block_size);
  // Get left and right halves of the ct_mask
  bitstr ct_mask_l = ct_mask.extract(0, cipher.block_size / 2);
  bitstr ct_mask_r = ct_mask.extract(cipher.block_size / 2, cipher.block_size);

  // Peel back left half of the plaintext mask
  bitstr pt_mask_peel = pt_mask_l.inv_permute(cipher.rf_after);

  // Find active sboxes on the top-side
  std::vector<std::pair<size_type, size_type>> active_top;
  size_type start_ip_top = 0;
  size_type start_op_top = 0;
  size_type net_top = 0;
  bitstr top_key_bits(cipher.rf_before.op_size);
  for (auto it = cipher.sboxes.begin(); it != cipher.sboxes.end(); ++it) {
    if (pt_mask_peel.value(start_op_top, start_op_top + it->output_size) != 0) {
      active_top.emplace_back(start_ip_top, it->input_size);
      net_top += it->input_size;
      top_key_bits(start_ip_top, start_ip_top + it->input_size) = (1 << it->input_size) - 1;
    }
    start_ip_top += it->input_size;
    start_op_top += it->output_size;
  }
  std::cout << top_key_bits.get_bits() << std::endl;
  bitstr top_pt_bits = top_key_bits.sinv_permute(cipher.rf_before);

  // Peel back right half of the ciphertext mask
  bitstr ct_mask_peel = ct_mask_r.inv_permute(cipher.rf_after);

  // Find active sboxes on the bottom-side
  std::vector<std::pair<size_type, size_type>> active_bot;
  size_type start_ip_bot = 0;
  size_type start_op_bot = 0;
  size_type net_bot = 0;
  bitstr bot_key_bits(cipher.rf_before.op_size);
  for (auto it = cipher.sboxes.begin(); it != cipher.sboxes.end(); ++it) {
    if (ct_mask_peel.value(start_op_bot, start_op_bot + it->output_size) != 0) {
      active_bot.emplace_back(start_ip_bot, it->input_size);
      net_bot += it->input_size;
      bot_key_bits(start_ip_bot, start_ip_bot + it->input_size) = (1 << it->input_size) - 1;
    }
    start_ip_bot += it->input_size;
    start_op_bot += it->output_size;
  }
  std::cout << bot_key_bits.get_bits() << std::endl;
  bitstr bot_ct_bits = bot_key_bits.sinv_permute(cipher.rf_before);

  // Get indices for effective pt bits
  auto eff_pt_bits = top_pt_bits.one_indices();
  // Get indices for effective ct bits
  auto eff_ct_bits = bot_ct_bits.one_indices();

  // DISTILLATION PHASE
  // Create table for effective pt and ct bits (net_top + net_bot)
  int* table = new int[1 << (net_top + net_bot)]();
  for (size_type i = 0; i < (size_type(1) << (net_top + net_bot)); ++i) table[i] = 0;

  // Iterate through trials and update table
  for (size_type i = 0; i < trials; ++i) {
    // Generate random plaintext
    bitstr pt = rand_bitstr(cipher.block_size);
    // Encrypt plaintext (rounds+2)
    bitstr ct = cipher.encrypt(pt, rounds + 2);
    // Modify pt and ct as per initial and final permutations
    bitstr pt_mod = pt.permute(cipher.ip);
    bitstr ct_mod = ct.inv_permute(cipher.fp);
    // Get left and right halves of pt_mod
    bitstr pt_mod_l = pt_mod.extract(0, cipher.block_size / 2);
    bitstr pt_mod_r = pt_mod.extract(cipher.block_size / 2, cipher.block_size);
    // Get left and right halves of ct_mod
    bitstr ct_mod_l = ct_mod.extract(0, cipher.block_size / 2);
    bitstr ct_mod_r = ct_mod.extract(cipher.block_size / 2, cipher.block_size);

    // Get temporary rhs value 
    bool rhs = (pt_mod_l * pt_mask_r) ^ (pt_mod_r * pt_mask_l) ^ (ct_mod_l * ct_mask_r) ^ (ct_mod_r * ct_mask_l);

    // Extract effective bits from pt_mod and ct_mod
    size_type eff_pt_mod = pt_mod_r.get_bits(eff_pt_bits);
    size_type eff_ct_mod = ct_mod_r.get_bits(eff_ct_bits);

    // Determine table index
    size_type table_index = (eff_pt_mod << net_bot) | eff_ct_mod;

    // Increment or decrement table entry based on rhs
    if (rhs) {
      table[table_index]++;
    } else {
      table[table_index]--;
    }
  }

  // Print table for debugging
  std::cout << "Trials: \t" << trials << std::endl;
  for (size_type i = 0; i < (size_type(1) << (net_top + net_bot)); ++i) {
    std::cout << i << ":\t" << table[i] << std::endl;
  }

  // ANALYSIS PHASE
  // Find the best candidate key (net_top + net_bot bits) via baskets
  std::vector<std::pair<size_type, int>> baskets;
  for (size_type i = 0; i < (size_type(1) << (net_top + net_bot)); ++i) {
    baskets.emplace_back(i, 0);
  }

  // Iterate through each candidate key
  for (size_type i = 0; i < (size_type(1) << (net_top + net_bot)); ++i) {
    // Candidate key (top)
    size_type top_value = i >> net_bot;
    bitstr cand_top(cipher.rf_before.op_size);
    size_type shift_top = net_top;
    for (auto it = active_top.begin(); it != active_top.end(); ++it) {
      shift_top -= it->second;
      cand_top(it->first, it->first + it->second) = (top_value >> shift_top) & ((1 << it->second) - 1);
    }
   
    // Candidate key (bottom)
    size_type bot_value = i & ((1 << net_bot) - 1);
    bitstr cand_bot(cipher.rf_before.op_size);
    size_type shift_bot = net_bot;
    for (auto it = active_bot.begin(); it != active_bot.end(); ++it) {
      shift_bot -= it->second;
      cand_bot(it->first, it->first + it->second) = (bot_value >> shift_bot) & ((1 << it->second) - 1);
    }

    // Iterate over each table entry
    for (size_type j = 0; j < (size_type(1) << (net_top + net_bot)); ++j) {
      // Extract effective bits from the table index
      size_type eff_pt_mod = j >> net_bot;
      size_type eff_ct_mod = j & ((1 << net_bot) - 1);

      // Create bitstrings for effective bits and set them
      bitstr eff_top(cipher.rf_before.ip_size);
      bitstr eff_bot(cipher.rf_before.ip_size);
      eff_top.set_bits(eff_pt_bits, eff_pt_mod);
      eff_bot.set_bits(eff_ct_bits, eff_ct_mod);

      // Compute round function on right-half of plaintext
      bitstr rf_out_top = cipher.rfunc(eff_top, cand_top);

      // Compute round function on right-half of ciphertext
      bitstr rf_out_bot = cipher.rfunc(eff_bot, cand_bot);

      // Get temp rhs value
      if ((rf_out_top * pt_mask_r) ^ (rf_out_bot * ct_mask_r)) {
        baskets[i].second += table[j];
      } else {
        baskets[i].second -= table[j];
      }
    }
  }

  // Sort baskets based on absolute counts
  std::sort(baskets.begin(), baskets.end(), [](const std::pair<size_type, int> &a, const std::pair<size_type, int> &b) {
    return std::abs(a.second) > std::abs(b.second);
  });

  // Print baskets for debugging
  std::cout << "Trials: \t" << trials << std::endl;
  for (const auto &basket : baskets) {
    std::cout << basket.first << ": " << basket.second << "\t";
  }
  std::cout << "\n" << std::endl;

  // Get best basket
  size_type best_basket = baskets[0].first;
  size_type best_basket_top = best_basket >> net_bot;
  size_type best_basket_bot = best_basket & ((1 << net_bot) - 1);

  // Get 1st and (rounds+2)th key-schedule
  auto key_sch_top = cipher.round_sch[0];
  auto key_sch_bot = cipher.round_sch[rounds + 1];

  // Compute and make the candidate key
  std::vector<short_type> final_cand(cipher.key_size, -1);
  // Top-part
  size_type shift_top = net_top;
  for (auto it = active_top.begin(); it != active_top.end(); ++it) {
    for (size_type j = 0; j < it->second; ++j) {
      bool bit = (best_basket_top >> (shift_top - j - 1)) & 1;
      final_cand[key_sch_top[it->first + j]] = bit ? 1 : 0;
    }
    shift_top -= it->second;
  }
  // Bottom-part
  size_type shift_bot = net_bot;
  for (auto it = active_bot.begin(); it != active_bot.end(); ++it) {
    for (size_type j = 0; j < it->second; ++j) {
      bool bit = (best_basket_bot >> (shift_bot - j - 1)) & 1;
      final_cand[key_sch_bot[it->first + j]] = bit ? 1 : 0;
    }
    shift_bot -= it->second;
  }

  // Get final rhs bool
  bool final_rhs = (baskets[0].second * bias > 0);

  // Clean up table memory
  delete[] table;

  // Return the candidate key and the final rhs bool
  return std::make_tuple(final_cand, final_rhs);
}
*/
