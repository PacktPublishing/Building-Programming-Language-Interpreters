#ifndef NETWORKPROTOCOLDSL_OPTREE_HPP
#define NETWORKPROTOCOLDSL_OPTREE_HPP

#include <networkprotocoldsl/operation.hpp>

#include <optional>
#include <vector>

namespace networkprotocoldsl {

/**
 * A node in the operation tree.
 */
struct OpTreeNode {
  const Operation operation;
  const std::vector<OpTreeNode> children;
};

/**
 * The immutable representation
 */
struct OpTree {
  const OpTreeNode root;
  OpTree(const OpTreeNode r) : root(r){};
};

} // namespace networkprotocoldsl

#endif // NETWORKPROTOCOLDSL_OPTREE_HPP
