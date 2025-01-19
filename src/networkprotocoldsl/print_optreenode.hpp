#ifndef NETWORKPROTOCOLDSL_PRINT_OPTREENODE_HPP
#define NETWORKPROTOCOLDSL_PRINT_OPTREENODE_HPP

#include <iostream>
#include <networkprotocoldsl/optree.hpp>
#include <sstream>
#include <string>

namespace networkprotocoldsl {

void print_optreenode(const OpTreeNode &node, std::ostream &os,
                      const std::string &prefix_firstline,
                      const std::string &prefix_others);

} // namespace networkprotocoldsl

#endif // NETWORKPROTOCOLDSL_PRINT_OPTREENODE_HPP
