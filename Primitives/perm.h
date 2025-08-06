// Class to enable permutations and compute inverses
// if possible

#ifndef PERM_H
#define PERM_H

// Standard Imports
#include <iostream>
#include <vector>
#include <stdexcept>

// Data types
using size_type = size_t;

class perm
{
  public:
    // Member Variables
    size_type ip_size;
    size_type op_size;
    std::vector<size_type> main_table;
    bool if_minv;
    std::vector<size_type> minv_table;
    bool if_pinv;
    std::vector<size_type> pinv_table;

    // Constructors
    perm();
    perm(size_type ip_size, size_type op_size, std::vector<size_type> main_table);

    // Compute inverses
    void compute_minv();
    void compute_pinv();
};

#endif
