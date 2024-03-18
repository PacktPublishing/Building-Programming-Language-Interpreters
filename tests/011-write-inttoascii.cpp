#include <networkprotocoldsl/interpretedprogram.hpp>
#include <networkprotocoldsl/operation/int32literal.hpp>
#include <networkprotocoldsl/operation/inttoascii.hpp>
#include <networkprotocoldsl/operation/writeoctets.hpp>
#include <networkprotocoldsl/optree.hpp>
#include <networkprotocoldsl/value.hpp>

#include <cstring>
#include <gtest/gtest.h>
#include <variant>

TEST(write_inttoascii, good_write) {
  using namespace networkprotocoldsl;

  operation::Int32Literal lit(42);
  operation::IntToAscii itoa;
  operation::WriteOctets wo;

  auto optree1 = std::make_shared<OpTree>(OpTree({wo, {{itoa, {{lit, {}}}}}}));
  InterpretedProgram p1(optree1);
  Interpreter i1 = p1.get_instance();

  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(42, std::get<int32_t>(std::get<Value>(i1.get_result())));

  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ("42",
            *(std::get<value::Octets>(std::get<Value>(i1.get_result())).data));

  ASSERT_EQ(ContinuationState::Blocked, i1.step());
  ASSERT_EQ(ReasonForBlockedOperation::WaitingForWrite,
            std::get<ReasonForBlockedOperation>(i1.get_result()));

  auto buf = i1.get_write_buffer();
  ASSERT_EQ(2, buf.length());
  ASSERT_EQ("42", buf);
  i1.handle_write(buf.length());

  ASSERT_EQ(ContinuationState::Exited, i1.step());
  ASSERT_EQ(true, std::holds_alternative<Value>(i1.get_result()));
  ASSERT_EQ(true,
            std::holds_alternative<int32_t>(std::get<Value>(i1.get_result())));
}
