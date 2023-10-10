#include <networkprotocoldsl/interpretedprogram.hpp>
#include <networkprotocoldsl/optree.hpp>

#include <cstring>
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

TEST(interpreter_global_state, input_output) {
  using namespace networkprotocoldsl;

  operation::ReadInt32Native r;
  operation::Add add;
  operation::WriteInt32Native w;

  auto optree =
      std::make_shared<OpTree>(OpTree({w, {{add, {{r, {}}, {r, {}}}}}}));

  InterpretedProgram p(optree);
  Interpreter i = p.get_instance();

  ASSERT_EQ(ContinuationState::Blocked, i.step());
  ASSERT_EQ(ReasonForBlockedOperation::WaitingForRead,
            std::get<ReasonForBlockedOperation>(i.get_result()));

  int input1 = 42;
  int input2 = 900;
  char buffer[8] = {};
  std::memcpy(&(buffer[0]), &input1, 4);
  std::memcpy(&(buffer[4]), &input2, 4);

  size_t amount_read;

  amount_read = i.handle_read({buffer, 2});
  ASSERT_EQ(2, amount_read);

  ASSERT_EQ(ContinuationState::Blocked, i.step());
  ASSERT_EQ(ReasonForBlockedOperation::WaitingForRead,
            std::get<ReasonForBlockedOperation>(i.get_result()));

  amount_read = i.handle_read({&(buffer[2]), 3});
  ASSERT_EQ(2, amount_read);

  ASSERT_EQ(ContinuationState::Ready, i.step());
  ASSERT_EQ(Value(42), std::get<Value>(i.get_result()));

  ASSERT_EQ(ContinuationState::Blocked, i.step());
  ASSERT_EQ(ReasonForBlockedOperation::WaitingForRead,
            std::get<ReasonForBlockedOperation>(i.get_result()));

  amount_read = i.handle_read({&(buffer[4]), 3});
  ASSERT_EQ(3, amount_read);

  ASSERT_EQ(ContinuationState::Blocked, i.step());
  ASSERT_EQ(ReasonForBlockedOperation::WaitingForRead,
            std::get<ReasonForBlockedOperation>(i.get_result()));

  amount_read = i.handle_read({&(buffer[7]), 1});
  ASSERT_EQ(1, amount_read);

  ASSERT_EQ(ContinuationState::Ready, i.step());
  ASSERT_EQ(Value(900), std::get<Value>(i.get_result()));

  ASSERT_EQ(ContinuationState::Ready, i.step());
  ASSERT_EQ(Value(942), std::get<Value>(i.get_result()));

  ASSERT_EQ(ContinuationState::Blocked, i.step());
  ASSERT_EQ(ReasonForBlockedOperation::WaitingForWrite,
            std::get<ReasonForBlockedOperation>(i.get_result()));

  std::string_view write_buf = i.get_write_buffer();
  ASSERT_EQ(4, write_buf.length());

  ASSERT_EQ(4, i.handle_write(4));

  int output;
  memcpy(&output, write_buf.begin(), 4);
  ASSERT_EQ(942, output);

  ASSERT_EQ(ContinuationState::Exited, i.step());
  ASSERT_EQ(Value(0), std::get<Value>(i.get_result()));
}
