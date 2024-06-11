#ifndef INCLUDED_TESTLIBS_HTTP_MESSAGE_OPTREES_HPP
#define INCLUDED_TESTLIBS_HTTP_MESSAGE_OPTREES_HPP

#include <networkprotocoldsl/operation.hpp>

namespace testlibs {
networkprotocoldsl::Operation get_read_request_callable();
networkprotocoldsl::Operation get_read_response_callable();
networkprotocoldsl::Operation get_write_request_callable();
networkprotocoldsl::Operation get_write_response_callable();
} // namespace testlibs

#endif