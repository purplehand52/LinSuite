// Method Immplementations for the sbox class

// Include header
#include "sbox.h"

// Helper function to compute dot products (bitwise)
bool bit_dot(short_type a, short_type b)
{
  bool result = 0;
  while (a && b) {
    if (a & 1) result ^= (b & 1);
    a >>= 1;
    b >>= 1;
  }
  return result;
}

// Constructor for sbox
sbox::sbox(size_type input_size, size_type output_size, size_type* table)
{
  this->input_size = input_size;
  this->output_size = output_size;
  this->table = new size_type[1 << input_size];
  for (size_type i = 0; i < (size_type(1) << input_size); ++i) {
    if (table[i] >= (size_type(1) << output_size)) {
      throw std::out_of_range("S-Box output exceeds defined output size");
    }    
    this->table[i] = table[i];
  }
  generate_lat();
}

// Copy constructor for sbox
sbox::sbox(const sbox &other)
{
  this->input_size = other.input_size;
  this->output_size = other.output_size;
  this->table = new size_type[1 << input_size];
  for (size_type i = 0; i < (size_type(1) << input_size); ++i) {
    this->table[i] = other.table[i];
  }
  this->lat = other.lat;
}

// Get entry from S-Box
size_type sbox::operator[](size_type input) const
{
  if (input >= (size_type(1) << input_size)) {
    throw std::out_of_range("Input exceeds defined input size");
  }
  return table[input];
}

// Generate Linear Approximation Table (LAT) [indexed by <input, output>]
void sbox::generate_lat()
{
  // Create empty entries
  for (size_type i = 0; i < (size_type(1) << (input_size+output_size)); ++i) {
    lat.push_back(std::make_pair(i, short_type(0)));
  }

  // Compute the LAT
  for (size_type input = 0; input < (size_type(1) << input_size); ++input) {
    size_type output = table[input];
    // Iterate thru lat
    for (auto it = lat.begin(); it != lat.end(); ++it) {
      size_type input_mask = it->first >> output_size;
      size_type output_mask = it->first & ((1 << output_size) - 1);
      it->second = (bit_dot(input, input_mask) == bit_dot(output, output_mask)) ? it->second + 1 : it->second - 1;
    }
  }

  // Sort the LAT by decreasing second values
  std::sort(lat.begin(), lat.end(), [](const std::pair<size_type, short_type>& a, const std::pair<size_type, short_type>& b) {
    return std::abs(a.second) > std::abs(b.second);
  });

}

// Sieve LAT based on output_mask
std::vector<std::pair<size_type, short_type>> sbox::sieve_lat(size_type output_mask)
{
  std::vector<std::pair<size_type, short_type>> result;
  for (const auto& entry : lat) {
    if ((entry.first & ((1 << output_size) - 1)) == output_mask) {
      result.push_back(entry);
    }
  }

  // Sort the result by decreasing second values
  std::sort(result.begin(), result.end(), [](const std::pair<size_type, short_type>& a, const std::pair<size_type, short_type>& b) {
    return std::abs(a.second) > std::abs(b.second);
  });

  return result;
}

// Print LAT
void sbox::print_lat()
{
  std::cout << std::endl;
  for (auto it = lat.begin(); it != lat.end(); ++it) {
    size_type input_mask = it->first >> output_size;
    size_type output_mask = it->first & ((1 << output_size) - 1);
    std::cout << "<" << input_mask << ", " << output_mask << ", " << it->second << ">" << std::endl;
  }
  std::cout << std::endl;
}
