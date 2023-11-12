#include <networkprotocoldsl/interpretedprogram.hpp>
#include <networkprotocoldsl/optree.hpp>

#include <cstring>
#include <gtest/gtest.h>

TEST(operator_if, run_else) {
  using namespace networkprotocoldsl;

  operation::Int32Literal il1(10);
  operation::Int32Literal il2(20);
  operation::Eq eq;

  auto then_optree = std::make_shared<OpTree>(OpTree({il1, {}}));
  auto else_optree = std::make_shared<OpTree>(OpTree({il2, {}}));
  operation::StaticCallable sc1(then_optree);
  operation::StaticCallable sc2(else_optree);
  operation::If if1;

  auto optree = std::make_shared<OpTree>(
      OpTree({{if1, {{eq, {{il1, {}}, {il2, {}}}}, {sc1, {}}, {sc2, {}}}}}));

  InterpretedProgram p(optree);
  Interpreter i1 = p.get_instance();
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(10, std::get<int32_t>(std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(20, std::get<int32_t>(std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(false, std::get<bool>(std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(true, std::holds_alternative<value::Callable>(
                      std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(true, std::holds_alternative<value::Callable>(
                      std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  // no result yet, since we're just inside the new continuation
  ASSERT_EQ(false, std::get<bool>(std::get<Value>(i1.get_result())));
  // now it will return to the main thing
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(20, std::get<int32_t>(std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Exited, i1.step());
  ASSERT_EQ(20, std::get<int32_t>(std::get<Value>(i1.get_result())));
}

TEST(operator_if, run_then) {
  using namespace networkprotocoldsl;

  operation::Int32Literal il1(10);
  operation::Int32Literal il2(20);
  operation::Eq eq;

  auto then_optree = std::make_shared<OpTree>(OpTree({il1, {}}));
  auto else_optree = std::make_shared<OpTree>(OpTree({il2, {}}));
  operation::StaticCallable sc1(then_optree);
  operation::StaticCallable sc2(else_optree);
  operation::If if1;

  auto optree = std::make_shared<OpTree>(
      OpTree({{if1, {{eq, {{il1, {}}, {il1, {}}}}, {sc1, {}}, {sc2, {}}}}}));

  InterpretedProgram p(optree);
  Interpreter i1 = p.get_instance();
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(10, std::get<int32_t>(std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(10, std::get<int32_t>(std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(true, std::get<bool>(std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(true, std::holds_alternative<value::Callable>(
                      std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(true, std::holds_alternative<value::Callable>(
                      std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  // no result yet, since we're just inside the new continuation
  ASSERT_EQ(false, std::get<bool>(std::get<Value>(i1.get_result())));
  // now it will return to the main thing
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(10, std::get<int32_t>(std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Exited, i1.step());
  ASSERT_EQ(10, std::get<int32_t>(std::get<Value>(i1.get_result())));
}
