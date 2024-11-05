#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_TYPEPARAMETERMAP_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_TYPEPARAMETERMAP_HPP

#include <map>
#include <memory>
#include <string>

#include <networkprotocoldsl/parser/typeparametervalue.hpp>

namespace networkprotocoldsl::parser {

using TypeParameterMap =
    std::map<std::string, std::shared_ptr<const TypeParameterValue>>;

}

#endif