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
// Needs fixing: Needs to a sub-key-mask for the final round function!
//
std::tuple<std::vector<short_type>, bool> attack::matsui2(size_type trials) {
  // Identify active s-boxes
  std::vector<std::pair<size_type, size_type>> actives;
  size_type start = 0;
  size_type net = 0;

  for (auto it = cipher.sboxes.begin(); it != cipher.sboxes.end(); ++it) {
    if (fin_key_mask.value(start, start + it->input_size) != 0) {
      actives.emplace_back(start, it->input_size);
      net += it->input_size;
    }
    start += it->input_size;
  }

  // Create baskets for candidates
  std::vector<std::pair<size_type, int>> baskets;
  for (size_type i = 0; i < (size_type(1)<<net); ++i) baskets.emplace_back(i, -trials/2);

  // Iterate over trials
  for (size_type i = 0; i < trials; ++i) {
    std::cout << "Trial: " << i+1 << std::endl;
    // Get random plaintext
    bitstr pt = rand_bitstr(cipher.block_size);
    // Get encryption
    bitstr ct = cipher.encrypt(pt, rounds+1);

    // Modify pt and ct as per initial and final permutations
    bitstr pt_mod = pt.permute(cipher.ip);
    bitstr ct_mod = ct.inv_permute(cipher.fp);

    // Get left and right halves of ciphertext
    bitstr ct_lt = ct_mod.extract(0, cipher.block_size / 2);
    bitstr ct_rt = ct_mod.extract(cipher.block_size / 2, cipher.block_size);

    // Temp rhs value
    bool t_rhs = (pt_mod * pt_mask);

    // Iterate over candidates
    bitstr candidate = bitstr(cipher.rf_before.op_size);
    for (size_type j = 0; j < (size_type(1)<<net); ++j) {
      // Set round key
      size_type shift = net;
      for (auto it = actives.begin(); it != actives.end(); ++it) {
        candidate(it->first, it->first + it->second) = j >> (shift - it->second);
        shift -= it->second;
      }
      // Apply round function
      bitstr dec = cipher.rfunc(ct_rt, candidate);
      // Reconstruct new ciphertext
      bitstr temp = ct_lt ^ dec;
      temp += ct_rt;
      // Get rhs value
      bool rhs = t_rhs ^ (temp * ct_mask);
      if (rhs) baskets[j].second += 1;
    }
  }

  // Sort baskets
  std::sort(baskets.begin(), baskets.end(), [](const std::pair<size_type, int>& a, const std::pair<size_type, int>& b) {
    return std::abs(a.second) > std::abs(b.second);
  });

  // Extract first entry and create partial-key
  size_type best_cand = baskets[0].first;
  std::cout << "Best candidate: " << best_cand << " with score: " << baskets[0].second << std::endl;
  std::vector<short_type> partial(cipher.key_size, -1);
  size_type shift = net;
  for (auto it = actives.begin(); it != actives.end(); ++it) {
    for (size_type a = 0; a < it->second; ++a) {
      partial[cipher.round_sch[rounds][it->first + a]] = ((best_cand >> (shift - a - 1)) & 1) ? 1 : 0; 
    }
    shift -= it->second;
  }

  // Find rhs
  bool fin_rhs = (baskets[0].second * bias > 0) ? true : false;

  // Return
  return std::make_tuple(partial, fin_rhs);
}
