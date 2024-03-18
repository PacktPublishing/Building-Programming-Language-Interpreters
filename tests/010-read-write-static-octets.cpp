#include "networkprotocoldsl/operation/readstaticoctets.hpp"
#include "networkprotocoldsl/operation/writestaticoctets.hpp"
#include "networkprotocoldsl/value.hpp"
#include <networkprotocoldsl/interpretedprogram.hpp>
#include <networkprotocoldsl/optree.hpp>

#include <cstring>
#include <gtest/gtest.h>
#include <variant>

TEST(read_write_static_octets, good_write) {
  using namespace networkprotocoldsl;

  operation::WriteStaticOctets wso("GET");
  auto optree1 = std::make_shared<OpTree>(OpTree({wso, {}}));
  InterpretedProgram p1(optree1);
  Interpreter i1 = p1.get_instance();

  operation::ReadStaticOctets rso("GET");
  auto optree2 = std::make_shared<OpTree>(OpTree({rso, {}}));
  InterpretedProgram p2(optree2);
  Interpreter i2 = p2.get_instance();

  ASSERT_EQ(ContinuationState::Blocked, i1.step());
  ASSERT_EQ(ReasonForBlockedOperation::WaitingForWrite,
            std::get<ReasonForBlockedOperation>(i1.get_result()));

  ASSERT_EQ(ContinuationState::Blocked, i2.step());
  ASSERT_EQ(ReasonForBlockedOperation::WaitingForRead,
            std::get<ReasonForBlockedOperation>(i2.get_result()));

  auto buf = i1.get_write_buffer();
  ASSERT_EQ(3, buf.length());
  ASSERT_EQ("GET", buf);
  i1.handle_write(buf.length());

  ASSERT_EQ(3, i2.handle_read(buf));

  ASSERT_EQ(ContinuationState::Exited, i1.step());
  ASSERT_EQ(true, std::holds_alternative<Value>(i1.get_result()));
  ASSERT_EQ(true,
            std::holds_alternative<int32_t>(std::get<Value>(i1.get_result())));
  ASSERT_EQ(0, std::get<int32_t>(std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Exited, i2.step());
  ASSERT_EQ(true, std::holds_alternative<Value>(i2.get_result()));
  ASSERT_EQ(true,
            std::holds_alternative<bool>(std::get<Value>(i2.get_result())));
  ASSERT_EQ(true, std::get<bool>(std::get<Value>(i2.get_result())));
}

TEST(read_write_static_octets, bad_write) {
  using namespace networkprotocoldsl;

  operation::WriteStaticOctets wso("GOT");
  auto optree1 = std::make_shared<OpTree>(OpTree({wso, {}}));
  InterpretedProgram p1(optree1);
  Interpreter i1 = p1.get_instance();

  operation::ReadStaticOctets rso("GET");
  auto optree2 = std::make_shared<OpTree>(OpTree({rso, {}}));
  InterpretedProgram p2(optree2);
  Interpreter i2 = p2.get_instance();

  ASSERT_EQ(ContinuationState::Blocked, i1.step());
  ASSERT_EQ(ReasonForBlockedOperation::WaitingForWrite,
            std::get<ReasonForBlockedOperation>(i1.get_result()));

  ASSERT_EQ(ContinuationState::Blocked, i2.step());
  ASSERT_EQ(ReasonForBlockedOperation::WaitingForRead,
            std::get<ReasonForBlockedOperation>(i2.get_result()));

  auto buf = i1.get_write_buffer();
  ASSERT_EQ(3, buf.length());
  ASSERT_EQ("GOT", buf);
  i1.handle_write(buf.length());

  ASSERT_EQ(3, i2.handle_read(buf));

  ASSERT_EQ(ContinuationState::Exited, i1.step());
  ASSERT_EQ(true, std::holds_alternative<Value>(i1.get_result()));
  ASSERT_EQ(true,
            std::holds_alternative<int32_t>(std::get<Value>(i1.get_result())));
  ASSERT_EQ(0, std::get<int32_t>(std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Exited, i2.step());
  ASSERT_EQ(true, std::holds_alternative<Value>(i2.get_result()));
  ASSERT_EQ(true, std::holds_alternative<value::RuntimeError>(
                      std::get<Value>(i2.get_result())));
  ASSERT_EQ(value::RuntimeError::ProtocolMismatchError,
            std::get<value::RuntimeError>(std::get<Value>(i2.get_result())));
}
