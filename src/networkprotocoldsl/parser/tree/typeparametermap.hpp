#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_TYPEPARAMETERMAP_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_TYPEPARAMETERMAP_HPP

#include <map>
#include <memory>
#include <string>

#include <networkprotocoldsl/parser/tree/typeparametervalue.hpp>

namespace networkprotocoldsl::parser::tree {

using TypeParameterMap =
    std::map<std::string, std::shared_ptr<const TypeParameterValue>>;

}

#endif // INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_TYPEPARAMETERMAP_HPP