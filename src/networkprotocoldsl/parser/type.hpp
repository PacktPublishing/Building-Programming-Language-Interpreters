#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_TYPE_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_TYPE_HPP

#include <memory>
#include <networkprotocoldsl/parser/identifierreference.hpp>
#include <networkprotocoldsl/parser/typeparametermap.hpp>

namespace networkprotocoldsl::parser {

struct Type {
  std::shared_ptr<const IdentifierReference> name;
  std::shared_ptr<const TypeParameterMap> parameters;
};

} // namespace networkprotocoldsl::parser

#endif