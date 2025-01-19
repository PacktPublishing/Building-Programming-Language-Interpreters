#include <networkprotocoldsl/print_optreenode.hpp>

#include <networkprotocoldsl/operation.hpp>
#include <networkprotocoldsl/optree.hpp>

#include <iostream>
#include <sstream>

namespace networkprotocoldsl {

void print_optreenode(const OpTreeNode &node, std::ostream &os,
                      const std::string &prefix_firstline,
                      const std::string &prefix_others) {

  std::visit(
      [&](const auto &op) {
        if constexpr (StringifiableOperationConcept<decltype(op)>) {
          std::string opStr = op.stringify();
          std::istringstream stream(opStr);
          std::string line;
          bool first = true;
          while (std::getline(stream, line)) {
            os << (first ? prefix_firstline : prefix_others) << line
               << std::endl;
            first = false;
          }
        } else {
          os << prefix_firstline << "<Non-printable Operation>" << std::endl;
        }
      },
      node.operation);

  for (size_t i = 0; i < node.children.size(); ++i) {
    const auto &child = node.children[i];
    std::string new_prefix_firstline =
        prefix_others + (i == node.children.size() - 1 ? " └── " : " ├── ");
    std::string new_prefix_others =
        prefix_others + (i == node.children.size() - 1 ? "     " : " │   ");
    print_optreenode(child, os, new_prefix_firstline, new_prefix_others);
  }
}

} // namespace networkprotocoldsl
