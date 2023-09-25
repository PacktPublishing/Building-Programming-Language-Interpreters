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
  ASSERT_FALSE(i1.get_result().has_value());
  ASSERT_FALSE(i2.get_result().has_value());
  ASSERT_EQ(ExecutionStackFrameState::MissingArguments, i1.step());
  ASSERT_EQ(ExecutionStackFrameState::MissingArguments, i2.step());
  ASSERT_EQ(Value(10), *(i1.get_result()));
  ASSERT_EQ(Value(10), *(i2.get_result()));
  ASSERT_EQ(ExecutionStackFrameState::Ready, i1.step());
  ASSERT_EQ(ExecutionStackFrameState::Ready, i2.step());
  ASSERT_EQ(Value(20), *(i1.get_result()));
  ASSERT_EQ(Value(20), *(i2.get_result()));
  ASSERT_EQ(ExecutionStackFrameState::Exited, i1.step());
  ASSERT_EQ(ExecutionStackFrameState::Exited, i2.step());
  ASSERT_EQ(Value(30), *(i1.get_result()));
  ASSERT_EQ(Value(30), *(i2.get_result()));
}

TEST(interpreter_global_state, callback) {
  using namespace networkprotocoldsl;

  operation::Int32Literal il1(10);
  operation::UnaryCallback uc1("multiply_by_2");

  auto optree = std::make_shared<OpTree>(OpTree({uc1, {{il1, {}}}}));

  InterpretedProgram p(optree);
  Interpreter i = p.get_instance();
  ASSERT_FALSE(i.get_result().has_value());
  ASSERT_EQ(ExecutionStackFrameState::WaitingForCallback, i.step());
  ASSERT_EQ(Value(10), *(i.get_result()));
  ASSERT_EQ(ExecutionStackFrameState::WaitingForCallback, i.step());
  ASSERT_EQ("multiply_by_2", *(i.get_callback_key()));
  i.set_callback_called();
  ASSERT_EQ(ExecutionStackFrameState::WaitingCallbackData, i.step());
  i.set_callback_return(std::get<int32_t>(*(i.get_result())) * 2);
  ASSERT_EQ(ExecutionStackFrameState::Exited, i.step());
  ASSERT_EQ(Value(20), *(i.get_result()));
}
