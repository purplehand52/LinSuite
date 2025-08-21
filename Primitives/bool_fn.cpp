// Code for boolean function class & helpers

#include "bool_fn.h"

// Mobius matrix entries
uint get_mobius(uint r, uint c, uint n)
{
    // Base matrix:
    // | 1 1 |
    // | 0 1 |

    // Higher matrices are determined by tensoring with self.
    if ((r < 0) || (c < 0) || (r > (1 << n)) || (c > (1 << n)))
    {
        throw std::out_of_range("r or c out of range");
    }

    uint mobius = 1;
    for (uint i = 0; i < n; i++)
    {
        if ((r & (uint(1) << i)) && !(c & (uint(1) << i)))
        {
            mobius = 0;
            break;
        }
    }
    return mobius;
}



// Constructors
bool_fn::bool_fn(uint in, uint out, std::vector<uint> values) {
  // Assert that the number of values matches the output size
  if (values.size() != (1 << in)) {
    throw std::invalid_argument("Number of values does not match input size");
  }
  this->in = in;
  this->out = out;
  for (uint i = 0; i < (1 << in); i++) {
    if (values[i] >= (1 << out)) {
      throw std::invalid_argument("Output value exceeds output size");
    }
  }
  this->values = values;
  get_cycles(); // Automatically compute cycles on construction
  get_polynomial(); // Automatically compute polynomial representation on construction
}

// Constructor from SBox
bool_fn::bool_fn(const sbox& mybox) {
  this->in = mybox.input_size;
  this->out = mybox.output_size;
  this->values.resize(1 << in);
  for (uint i = 0; i < (1 << in); i++) {
    this->values[i] = mybox[i];
  }
  get_cycles(); // Automatically compute cycles on construction
  get_polynomial(); // Automatically compute polynomial representation on construction
}

// Copy constructor
bool_fn::bool_fn(const bool_fn& other) {
  this->in = other.in;
  this->out = other.out;
  this->values = other.values;
  this->is_bij = other.is_bij;
  if (is_bij) this->cycles = other.cycles;
  if (out == 1) this->poly = other.poly;
}

// Destructor (empty)
bool_fn::~bool_fn() {
  // No dynamic memory to free, so nothing to do here
}

// Getter
uint bool_fn::operator[](uint input) const {
  if (input >= (1 << in)) {
    throw std::out_of_range("Input value exceeds input size");
  }
  return values[input];
}

// Get polynomial representation
void bool_fn::get_polynomial() {
  // Do only if out == 1
  if (out != 1) {
    std::cerr << "Polynomial representation is only available for single-output functions." << std::endl;
    return;
  }

  // Initialize polynomial coefficients
  uint size = 1 << in;
  poly.resize(1 << in, 0);
  for (uint i = 0; i < size; i++) {
    poly[i] = 0;
    for (uint j = 0; j < size; j++) {
      poly[i] ^= (values[j] & get_mobius(j, i, in));
    }
  }
}

// Print polynomial representation
void bool_fn::print_polynomial() const {
  // Do only if out == 1
  if (out != 1) {
    std::cerr << "Polynomial representation is only available for single-output functions." << std::endl;
    return;
  }

  for (uint i = 0; i < (uint(1)<<in); i++)
    {
        if (poly[i] == 0)
            continue;
        for(uint j = 0; j < in; j++)
        {
            if (i & (uint(1) << j))
            {
                std::cout << "x" << j;
            }
        }
        std::cout << " + ";
    }
    std::cout << std::endl;

}

// Check if bijective
void bool_fn::is_bijective() {
  // Check if in == out
  if (in != out) {
    is_bij = false;
    return;
  }

  // Check if all output values are unique (use bitstrings)
  bitstr output_seen(1 << out);
  for (uint i = 0; i < (1 << in); i++) {
    uint output_value = values[i];
    if (output_seen[output_value]) {
      // If we have seen this output value before, it is not bijective
      is_bij = false;
      return;
    }
    else {
      // Mark this output value as seen
      output_seen[output_value] = 1;
    }
  }

  // Otherwise, it is bijective
  is_bij = true;
}

// Getting all cycles
void bool_fn::get_cycles() {
  // Check if bijective first
  is_bijective();
  if (!is_bij) {
    std::cerr << "Function is not bijective, cannot get cycles." << std::endl;
    return;
  }

  // Clear all existing cycles
  cycles.clear();

  // Create a bitstr to track visited inputs
  bitstr visited(1 << in);
  for (uint i = 0; i < (1 << in); i++) {
    if (visited[i]) continue; // Skip already visited inputs

    // Start a new cycle
    std::vector<uint> cycle;
    uint current = i;

    do {
      cycle.push_back(current);
      visited[current] = 1;
      current = values[current]; // Move to the next input in the cycle
    } while (current != i); // Stop when we return to the start

    // Add the completed cycle to the list
    cycles.push_back(cycle);
  }

  // End
  return;
}

// Get balanced invariants
void bool_fn::get_balanced_invariants() {
  // Ensure the function is bijective
  if (!is_bij) {
    std::cerr << "Function is not bijective, cannot get balanced invariants." << std::endl;
    return; 
  }

  // Create a vector to hold cycle lengths
  std::vector<uint> cycle_lengths;
  for (const auto& cycle : cycles) {
    cycle_lengths.push_back(cycle.size());
  }

  // Need to get all possible subsets of cycle lengths which add up to (1<<out) / 2
  balanced_invariants.clear();
  uint target_sum = (1 << out) / 2;
  
  // Brute force (?)
  for (uint i = 0; i < (1 << cycles.size()); i++) {
    bitstr invariant(cycles.size());
    uint sum = 0;
    for (uint j = 0; j < cycles.size(); j++) {
      if (i & (1 << j)) {
        sum += cycle_lengths[j];
      }
    }
    if (sum == target_sum) {
      invariant(0, cycles.size()) = i;
      balanced_invariants.push_back(invariant);
    }
  } 
  return;
}

std::vector<bool_fn> bool_fn::get_inv_functions() const {
  // Ensure the function is bijective
  if (!is_bij) {
    std::cerr << "Function is not bijective, cannot get inverse functions." << std::endl;
    return {};
  }

  // Create a vector to hold invariant functions
  std::vector<bool_fn> inv_functions;

  // For each balanced invariant, create a new bool_fn
  for (const auto& invariant : balanced_invariants) {
    std::vector<uint> inv_values(1 << in, 0);
    for (uint i = 0; i < (cycles.size()); i++) {
      uint val = invariant[i];
      for (uint j = 0; j < cycles[i].size(); j++) {
        inv_values[cycles[i][j]] = val;
      }
    }
    inv_functions.emplace_back(in, 1, inv_values);
  }

  return inv_functions;
}

