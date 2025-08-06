// Method Implementations for the _bitstr_ class and
// its associated proxy classes.

// Header Inclusion
#include "bitstr.h"

// Helpers
// Function to reverse the bits for an integer of a specific size
block_type reverse_bits(block_type value, size_type size) {
  block_type result = 0;
  for (size_type i = 0; i < size; ++i) {
    result <<= 1;
    result |= (value & 1);
    value >>= 1;
  }
  return result;
}

// Constructors for bitstr
bitstr::bitstr() : bit_size(0), blocks(nullptr) {
  // Initialize with zero size
  blocks = new block_type[1](); // Allocate one block for zero size
}

bitstr::bitstr(size_type size) : bit_size(size) {
  size_type block_count = (size + BITS_PER_BLOCK - 1) / BITS_PER_BLOCK;
  blocks = new block_type[block_count]();
}

bitstr::bitstr(block_type *data, size_type size) : bit_size(size) {
  size_type block_count = (size + BITS_PER_BLOCK - 1) / BITS_PER_BLOCK;
  blocks = new block_type[block_count];
  for (size_type i = 0; i < block_count; ++i) {
    blocks[i] = data[i];
  }
}

bitstr::bitstr(const bitstr &other) : bit_size(other.bit_size) {
  size_type block_count = (bit_size + BITS_PER_BLOCK - 1) / BITS_PER_BLOCK;
  blocks = new block_type[block_count];
  for (size_type i = 0; i < block_count; ++i) {
      blocks[i] = other.blocks[i];
  }
}

bitstr::bitstr(block_type value, size_type size) {
  // Reverse
  block_type temp = reverse_bits(value, size);
  bit_size = size;
  blocks = new block_type[1];
  blocks[0] = temp;
}

// Bit-Access Proxy Implementations
bitstr::bit_proxy bitstr::operator[](unsigned index){
  // Index within bounds
  if (index >= bit_size) throw std::out_of_range("Index out of range");

  // Calculate block index and bit index
  size_type block_index = index / BITS_PER_BLOCK;
  size_type bit_index = index % BITS_PER_BLOCK;
  return bit_proxy(blocks[block_index], bit_index);
}

bool bitstr::operator[](unsigned index) const {
  // Index within bounds
  if (index >= bit_size) throw std::out_of_range("Index out of range");

  // Calculate block index and bit index
  size_type block_index = index / BITS_PER_BLOCK;
  size_type bit_index = index % BITS_PER_BLOCK;
  return (blocks[block_index] & (block_type(1) << bit_index)) != 0;
}

// Inner Implementations
bitstr::bit_proxy& bitstr::bit_proxy::operator=(bool value) {
  if (value) {
    block |= (block_type(1) << index);
  } else {
    block &= ~(block_type(1) << index);
  }
  return *this;
}

bitstr::bit_proxy& bitstr::bit_proxy::operator=(const bit_proxy &other) {
  return *this = static_cast<bool>(other);
}

bitstr::bit_proxy& bitstr::bit_proxy::operator|=(bool value) {
  if (value) block |= (block_type(1) << index);
  return *this;
}

bitstr::bit_proxy& bitstr::bit_proxy::operator&=(bool value) {
  if (!value) block &= ~(block_type(1) << index);
  return *this;
}

bitstr::bit_proxy& bitstr::bit_proxy::operator^=(bool value) {
  if (value) block ^= (block_type(1) << index);
  return *this;
}

bitstr::bit_proxy& bitstr::bit_proxy::operator~() {
  block ^= (block_type(1) << index);
  return *this;
}

bitstr::bit_proxy::operator bool() const {
  return (block & (block_type(1) << index)) != 0;
}

// Slice-Access Proxy Implementations
bitstr::slice_proxy bitstr::operator()(unsigned start, unsigned end) {
  if (start >= end || end > bit_size) throw std::out_of_range("Invalid slice range");
  
  block_type *beg_block = blocks + (start / BITS_PER_BLOCK);
  block_type *end_block = blocks + ((end-1) / BITS_PER_BLOCK);
  unsigned beg_offset = start % BITS_PER_BLOCK;
  unsigned end_offset = ((end-1) % BITS_PER_BLOCK) + 1; // end_offset is inclusive

  return slice_proxy(beg_block, end_block, beg_offset, end_offset, end - start);
}

bitstr bitstr::extract(unsigned start, unsigned end) const {
  if (start >= end || end > bit_size) throw std::out_of_range("Invalid slice range");
  size_type new_size = end - start;
  bitstr result(new_size);
  for (size_type i = start; i < end; ++i) {
    result[i - start] = (*this)[i];
  }
  return result;
}

// Only works when short
size_type bitstr::value(unsigned start, unsigned end) const {
  bitstr slice = extract(start, end);
  if (slice.bit_size > BITS_PER_BLOCK) {
    throw std::overflow_error("Slice size exceeds short type size");
  }
  return reverse_bits(slice.blocks[0], slice.bit_size);
}

// Inner Implememtations
bitstr::slice_proxy& bitstr::slice_proxy::operator=(block_type value) {
  // Reverse bits in value
  value = reverse_bits(value, len);

  // Single block case
  if (beg_block == end_block) {
    *beg_block &= (~block_type(0) << end_offset) | ((1UL << beg_offset) - 1);
    *beg_block |= (block_type(value) << beg_offset);
  }

  // Multiple block case
  else {
    size_type offset = 0;
    // Begin block
    *beg_block &= ((block_type(1) << beg_offset) - 1);
    *beg_block |= (block_type(value) << beg_offset);
    offset = BITS_PER_BLOCK - beg_offset;

    // Other blocks
    for (block_type *block = beg_block + 1; block < end_block; ++block) {
      *block = block_type(value) >> offset;
      offset += BITS_PER_BLOCK;
    }

    // End block
    *end_block &= (~0) << end_offset;
    *end_block |= (block_type(value) >> offset);
  }
  return *this;
}

bitstr::slice_proxy& bitstr::slice_proxy::operator|=(block_type value) {
  // Reverse bits in value
  value = reverse_bits(value, len);

  // Single block case
  if (beg_block == end_block) {
    *beg_block |= (block_type(value) << beg_offset);
  }

  // Multiple block case
  else {
    size_type offset = 0;
    // Begin block
    *beg_block |= (block_type(value) << beg_offset);
    offset = BITS_PER_BLOCK - beg_offset;

    // Other blocks
    for (block_type *block = beg_block + 1; block < end_block; ++block) {
      *block |= block_type(value) >> offset;
      offset += BITS_PER_BLOCK;
    }

    // End block
    *end_block |= (block_type(value) >> offset);
  }
  return *this;
}

bitstr::slice_proxy& bitstr::slice_proxy::operator&=(block_type value) {
  // Reverse bits in value
  value = reverse_bits(value, len);

  // Single block case
  if (beg_block == end_block) {
    *beg_block &= (~block_type(0) << end_offset) | ((block_type(1) << beg_offset) - 1) | (block_type(value) << beg_offset);
  }

  // Multiple block case
  else {
    size_type offset = 0;
    // Begin block
    *beg_block &= (block_type(value) << beg_offset) | ((block_type(1) << beg_offset) - 1);
    offset = BITS_PER_BLOCK - beg_offset;

    // Other blocks
    for (block_type *block = beg_block + 1; block < end_block; ++block) {
      *block &= block_type(value) >> offset;
      offset += BITS_PER_BLOCK;
    }

    // End block
    *end_block &= (block_type(value) >> offset) | (~block_type(0) << end_offset);
  }
  return *this;
}

bitstr::slice_proxy& bitstr::slice_proxy::operator^=(block_type value) {
  // Reverse bits in value
  value = reverse_bits(value, len);

  // Single block case
  if (beg_block == end_block) {
    *beg_block ^= (block_type(value) << beg_offset);
  }

  // Multiple block case
  else {
    size_type offset = 0;
    // Begin block
    *beg_block ^= (block_type(value) << beg_offset);
    offset = BITS_PER_BLOCK - beg_offset;

    // Other blocks
    for (block_type *block = beg_block + 1; block < end_block; ++block) {
      *block ^= block_type(value) >> offset;
      offset += BITS_PER_BLOCK;
    }

    // End block
    *end_block ^= (block_type(value) >> offset);
  }
  return *this;
}

bitstr::slice_proxy& bitstr::slice_proxy::operator~() {
  // Single block case
  if (beg_block == end_block) {
    *beg_block ^= ~(((block_type(1) << beg_offset) - 1) | (~block_type(0) << end_offset));
  }

  // Multiple block case
  else {
    // Begin block
    *beg_block ^= ~((block_type(1) << beg_offset) - 1);
    
    // Other blocks
    for (block_type *block = beg_block + 1; block < end_block; ++block) {
      *block ^= ~block_type(0);
    }

    // End block
    *end_block ^= ~(~block_type(0) << end_offset);
  }
  return *this;
}

// Concatenation
void bitstr::operator+=(const bitstr other)
{
  // Update Size
  bit_size += other.bit_size;

  // Calculate new block count
  size_type new_block_count = (bit_size + BITS_PER_BLOCK - 1) / BITS_PER_BLOCK;
  block_type *new_blocks = new block_type[new_block_count]();

  // Copy existing blocks
  std::copy(blocks, blocks + (bit_size - other.bit_size + BITS_PER_BLOCK - 1) / BITS_PER_BLOCK, new_blocks);

  // Copy new blocks
  size_type offset = (bit_size - other.bit_size) % BITS_PER_BLOCK;
  block_type *start_block = new_blocks + (bit_size - other.bit_size + BITS_PER_BLOCK - 1) / BITS_PER_BLOCK - 1;
  if (offset == 0) start_block += 1; // Ensure we start at the next block if no offset

  for (size_type i = 0; i < (other.bit_size + BITS_PER_BLOCK - 1) / BITS_PER_BLOCK; ++i) {
    // Left shift the bits of the new block    
    *start_block |= (other.blocks[i] << offset);
    // If offset != 0, assign remaining bits to the next block
    if (offset > 0) {
      start_block += 1;
      *start_block |= (other.blocks[i] >> (BITS_PER_BLOCK - offset));
    }
  }

  // Delete old blocks and assign new blocks
  delete[] blocks;
  blocks = new_blocks;
  return;
}

// Other Operators
void bitstr::operator|=(const bitstr other)
{
  if (bit_size != other.bit_size) {
    throw std::invalid_argument("Bitstr sizes do not match for OR operation");
  }
  for (size_type i = 0; i < (bit_size + BITS_PER_BLOCK - 1) / BITS_PER_BLOCK; ++i) {
    blocks[i] |= other.blocks[i];
  }
  return;
}

void bitstr::operator&=(const bitstr other)
{
  if (bit_size != other.bit_size) {
    throw std::invalid_argument("Bitstr sizes do not match for AND operation");
  }
  for (size_type i = 0; i < (bit_size + BITS_PER_BLOCK - 1) / BITS_PER_BLOCK; ++i) {
    blocks[i] &= other.blocks[i];
  }
  return;
}

void bitstr::operator^=(const bitstr other)
{
  if (bit_size != other.bit_size) {
    throw std::invalid_argument("Bitstr sizes do not match for XOR operation");
  }
  for (size_type i = 0; i < (bit_size + BITS_PER_BLOCK - 1) / BITS_PER_BLOCK; ++i) {
    blocks[i] ^= other.blocks[i];
  }
  return;
}

// Dot Product
bool bitstr::operator*(const bitstr other) const
{
  if (bit_size != other.bit_size) {
    throw std::invalid_argument("Bitstr sizes do not match for dot product");
  }
  bool result = false;
  for (size_type i = 0; i < (bit_size + BITS_PER_BLOCK - 1) / BITS_PER_BLOCK; ++i) {
    result ^= (blocks[i] & other.blocks[i]);
  }
  return result;
}

bool bitstr::operator==(const bitstr other) const
{
  if (bit_size != other.bit_size) {
    throw std::invalid_argument("Bitstr sizes do not match for equality");
  }
  for (size_type i = 0; i < (bit_size + BITS_PER_BLOCK - 1) / BITS_PER_BLOCK; ++i) {
    if (blocks[i] != other.blocks[i]) return false;
  }
  return true; 
}

bool bitstr::operator!=(const bitstr other) const
{
  if (bit_size != other.bit_size) {
    throw std::invalid_argument("Bitstr sizes do not match for inequality");
  }
  for (size_type i = 0; i < (bit_size + BITS_PER_BLOCK - 1) / BITS_PER_BLOCK; ++i) {
    if (blocks[i] != other.blocks[i]) return true;
  }
  return false; 
}

bitstr bitstr::operator^(const bitstr other) const
{
  if (bit_size != other.bit_size) {
    throw std::invalid_argument("Bitstr sizes do not match for OR operation");
  }
  bitstr result(*this);
  result ^= other;
  return result;
}

void bitstr::operator~()
{
  for (size_type i = 0; i < (bit_size + BITS_PER_BLOCK - 1) / BITS_PER_BLOCK; ++i) {
    blocks[i] = ~blocks[i];
  }
  return;
}

// Substitution
bitstr bitstr::substitute(std::vector<size_type> sub_arr, size_type sub_size) const {
  // Create new bitstr with size of sub_size
  bitstr result(sub_size);

  // Iterate and substitute bits
  for (size_type i = 0; i < sub_size; ++i) {
    if (sub_arr[i] >= bit_size) {
      throw std::out_of_range("Substitution index out of range");
    }
    result[i] = (*this)[sub_arr[i]];
  }
  return result;
}

bitstr bitstr::permute(perm stuff) const {
  // Check if permutation is valid
  if (stuff.ip_size != bit_size) {
    throw std::invalid_argument("Permutation input size does not match bitstr size");
  }

  // Create new bitstr with output_size
  bitstr result(stuff.op_size);

  // Iterate and substitute
  for (size_type i = 0; i < stuff.op_size; ++i) result[i] = (*this)[stuff.main_table[i]];
  return result;
}

bitstr bitstr::inv_permute(perm stuff) const {
  // Check if permutation is valid
  if (stuff.op_size != bit_size) {
    throw std::invalid_argument("Permutation output size does not match bitstr size");
  }

  // Case 1: Invertible
  bitstr result(stuff.ip_size);
  if (stuff.if_minv)
  {
    // Iterate and substitute
    for (size_type i = 0; i < stuff.ip_size; ++i) result[i] = (*this)[stuff.minv_table[i]];
    return result;
  }
  // Case 2: Pseudo-invertible
  else if (stuff.if_pinv)
  {
    // Iterate and substitute
    for (size_type i = 0; i < stuff.ip_size; ++i) result[i] = (*this)[stuff.pinv_table[i]];
    return result;
  }
  // Case 3: Neither
  else return result; // Return empty bitstr
}

bitstr bitstr::sinv_permute(perm stuff) const {
  // Check if permutation is valid
  if (stuff.op_size != bit_size) {
    throw std::invalid_argument("Permutation output size does not match bitstr size");
  }

  // Create new bitstr with input_size
  bitstr result(stuff.ip_size);

  // Iterate and substitute
  for (size_type i = 0; i < stuff.op_size; ++i) {
    result[stuff.main_table[i]] |= (*this)[i];
  }

  // Return
  return result;
}

// Printers
void bitstr::print_bits() const {
  size_type num_blocks = (bit_size + BITS_PER_BLOCK - 1) / BITS_PER_BLOCK;
  for (size_type i = 0; i < num_blocks; ++i) {
    size_type lim = (i == num_blocks - 1) ? (bit_size % BITS_PER_BLOCK) : BITS_PER_BLOCK;
    if (lim == 0) lim = BITS_PER_BLOCK;
    for (size_type j = 0; j < lim; ++j) {
      std::cout << ((blocks[i] >> j) % 2 ? '1' : '0');
    }
    std::cout << ' ';
  }
  std::cout << std::endl;
}

void bitstr::print_hex() const {
  size_type num_blocks = (bit_size + BITS_PER_BLOCK - 1) / BITS_PER_BLOCK;
  for (size_type i = 0; i < num_blocks; ++i) {
    size_type lim = (i == num_blocks - 1) ? (bit_size % BITS_PER_BLOCK) : BITS_PER_BLOCK;
    if (lim == 0) lim = BITS_PER_BLOCK;
    for (size_type j = 0; j < lim; j += 4) {
      std::cout << std::hex << (reverse_bits(blocks[i] >> j, 4) & 0xF);
    }
    std::cout << ' ';
  }
  std::cout << std::dec << std::endl;
}

std::string bitstr::get_bits() const {
  std::string result;
  size_type num_blocks = (bit_size + BITS_PER_BLOCK - 1) / BITS_PER_BLOCK;
  for (size_type i = 0; i < num_blocks; ++i) {
    size_type lim = (i == num_blocks - 1) ? (bit_size % BITS_PER_BLOCK) : BITS_PER_BLOCK;
    if (lim == 0) lim = BITS_PER_BLOCK;
    for (size_type j = 0; j < lim; ++j) {
      result += ((blocks[i] >> j) % 2 ? '1' : '0');
    }
  }
  return result;
}

std::string bitstr::get_hex() const {
  std::string result;
  size_type num_blocks = (bit_size + BITS_PER_BLOCK - 1) / BITS_PER_BLOCK;
  for (size_type i = 0; i < num_blocks; ++i) {
    size_type lim = (i == num_blocks - 1) ? (bit_size % BITS_PER_BLOCK) : BITS_PER_BLOCK;
    if (lim == 0) lim = BITS_PER_BLOCK;
    for (size_type j = 0; j < lim; j += 4) {
      result += "0123456789ABCDEF"[reverse_bits(blocks[i] >> j, 4) & 0xF];
    }
  }
  return result;
}

