// Method Implementation for the permutation class

// Include header
#include "perm.h"

// Constructor
perm::perm()
{
  // Set default sizes
  ip_size = 0;
  op_size = 0;

  // Initialize main table
  main_table.clear();

  // Initialize inverse tables
  minv_table.clear();
  pinv_table.clear();

  // Set flags
  if_minv = false;
  if_pinv = false;
}

perm::perm(size_type ip_size, size_type op_size, std::vector<size_type> main_table)
{
  // Check if sizes match
  if (main_table.size() != op_size) throw std::runtime_error("Mismatch in input and output sizes of table");

  // Iterate and check
  for (auto it = main_table.begin(); it != main_table.end(); ++it) {
    if (*it >= ip_size) throw std::out_of_range("Out of range");
  }

  // Set
  this->ip_size = ip_size;
  this->op_size = op_size;
  this->main_table = main_table;

  // Compute other stuff
  compute_minv();
  if (!if_minv) compute_pinv();
}

// Compute inverse
void perm::compute_minv()
{
  // Input-size == Output-size
  if_minv = false;
  if (ip_size != op_size) return;

  // Compute step by step
  minv_table.resize(ip_size, op_size);
  if_minv = true;
  for (size_type i = 0; i < op_size; ++i) {
    // Check
    if (minv_table[main_table[i]] != op_size)
    {
      minv_table.clear();
      if_minv = false;
      break;
    }
    // Set
    minv_table[main_table[i]] = i;
  }
  return;
}

// Compute pseudo-inverse
void perm::compute_pinv()
{
  // Compute step by step
  pinv_table.resize(ip_size, op_size);
  if_pinv = true;
  for (size_type i = 0; i < op_size; ++i) {
    pinv_table[main_table[i]] = i;
  }
  // Check for warnings
  for (size_type i = 0; i < ip_size; ++i) {
    if (pinv_table[i] == op_size)
    {
      pinv_table.clear();
      if_pinv = false;
      break;
    }
  }
  return;
}
