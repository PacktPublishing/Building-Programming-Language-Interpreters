#include <networkprotocoldsl/interpretedprogram.hpp>
#include <networkprotocoldsl/operation/int32literal.hpp>
#include <networkprotocoldsl/operation/inttoascii.hpp>
#include <networkprotocoldsl/operation/opsequence.hpp>
#include <networkprotocoldsl/operation/readintfromascii.hpp>
#include <networkprotocoldsl/operation/readoctetsuntilterminator.hpp>
#include <networkprotocoldsl/operation/writeoctets.hpp>
#include <networkprotocoldsl/optree.hpp>
#include <networkprotocoldsl/value.hpp>

#include <cstring>
#include <gtest/gtest.h>
#include <string_view>
#include <variant>

TEST(read_int_and_terminator, good_read) {
  using namespace networkprotocoldsl;

  const std::string_view input = "a=42 ";

  operation::OpSequence ops;
  operation::ReadOctetsUntilTerminator read_until_eq("=");
  operation::ReadIntFromAscii read_i;

  auto optree1 = std::make_shared<OpTree>(
      OpTree({ops, {{read_until_eq, {}}, {read_i, {}}}}));
  InterpretedProgram p1(optree1);
  Interpreter i1 = p1.get_instance();

  ASSERT_EQ(ContinuationState::Blocked, i1.step());
  ASSERT_EQ(ReasonForBlockedOperation::WaitingForRead,
            std::get<ReasonForBlockedOperation>(i1.get_result()));

  size_t consumed = i1.handle_read(input);
  ASSERT_EQ(2, consumed);

  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ("a",
            *(std::get<value::Octets>(std::get<Value>(i1.get_result())).data));

  ASSERT_EQ(ContinuationState::Blocked, i1.step());
  ASSERT_EQ(ReasonForBlockedOperation::WaitingForRead,
            std::get<ReasonForBlockedOperation>(i1.get_result()));

  consumed = i1.handle_read(input.substr(consumed));
  ASSERT_EQ(2, consumed);

  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(42, std::get<int32_t>(std::get<Value>(i1.get_result())));
}
