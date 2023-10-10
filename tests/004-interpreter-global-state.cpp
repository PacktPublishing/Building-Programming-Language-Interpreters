#include <networkprotocoldsl/interpretedprogram.hpp>
#include <networkprotocoldsl/optree.hpp>

#include <gtest/gtest.h>

TEST(interpreter_global_state, iterate) {
  using namespace networkprotocoldsl;

  operation::Int32Literal il1(10);
  operation::Int32Literal il2(20);
  operation::Add a1;

  auto optree = std::make_shared<OpTree>(OpTree({a1, {{il1, {}}, {il2, {}}}}));

  InterpretedProgram p(optree);
  Interpreter i1 = p.get_instance();
  Interpreter i2 = p.get_instance();
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(ContinuationState::Ready, i2.step());
  ASSERT_EQ(Value(10), std::get<Value>(i1.get_result()));
  ASSERT_EQ(Value(10), std::get<Value>(i2.get_result()));
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(ContinuationState::Ready, i2.step());
  ASSERT_EQ(Value(20), std::get<Value>(i1.get_result()));
  ASSERT_EQ(Value(20), std::get<Value>(i2.get_result()));
  ASSERT_EQ(ContinuationState::Exited, i1.step());
  ASSERT_EQ(ContinuationState::Exited, i2.step());
  ASSERT_EQ(Value(30), std::get<Value>(i1.get_result()));
  ASSERT_EQ(Value(30), std::get<Value>(i2.get_result()));
}

TEST(interpreter_global_state, callback) {
  using namespace networkprotocoldsl;

  operation::Int32Literal il1(10);
  operation::UnaryCallback uc1("multiply_by_2");

  auto optree = std::make_shared<OpTree>(OpTree({uc1, {{il1, {}}}}));

  InterpretedProgram p(optree);
  Interpreter i = p.get_instance();
  ASSERT_EQ(ContinuationState::Ready, i.step());
  ASSERT_EQ(Value(10), std::get<Value>(i.get_result()));

  ASSERT_EQ(ContinuationState::Blocked, i.step());
  ASSERT_EQ(ReasonForBlockedOperation::WaitingForCallback,
            std::get<ReasonForBlockedOperation>(i.get_result()));

  ASSERT_EQ(ContinuationState::Blocked, i.step());
  ASSERT_EQ(ReasonForBlockedOperation::WaitingForCallback,
            std::get<ReasonForBlockedOperation>(i.get_result()));

  ASSERT_EQ("multiply_by_2", i.get_callback_key());
  i.set_callback_called();

  ASSERT_EQ(ContinuationState::Blocked, i.step());
  ASSERT_EQ(ReasonForBlockedOperation::WaitingCallbackData,
            std::get<ReasonForBlockedOperation>(i.get_result()));

  ASSERT_EQ(ContinuationState::Blocked, i.step());
  ASSERT_EQ(ReasonForBlockedOperation::WaitingCallbackData,
            std::get<ReasonForBlockedOperation>(i.get_result()));

  Value arg = i.get_callback_arguments()[0];
  i.set_callback_return(std::get<int32_t>(arg) * 2);

  ASSERT_EQ(ContinuationState::Exited, i.step());
  ASSERT_EQ(Value(20), std::get<Value>(i.get_result()));
}
