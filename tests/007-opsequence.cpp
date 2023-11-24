#include <networkprotocoldsl/interpretedprogram.hpp>
#include <networkprotocoldsl/optree.hpp>

#include <cstring>
#include <gtest/gtest.h>

TEST(operator_opsequence, interrupt) {
  using namespace networkprotocoldsl;

  operation::Int32Literal il1(10);
  auto unused = std::make_shared<OpTree>(OpTree({il1, {}}));
  operation::StaticCallable sc1(unused);
  operation::Eq eq;
  operation::OpSequence ops;
  operation::Int32Literal il2(20);

  auto optree =
      std::make_shared<OpTree>(OpTree({ops,
                                       {
                                           {il1, {}},
                                           {eq, {{il2, {}}, {sc1, {}}}},
                                           {il2, {}},
                                       }}));

  InterpretedProgram p(optree);
  Interpreter i1 = p.get_instance();
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(10, std::get<int32_t>(std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(20, std::get<int32_t>(std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(true, std::holds_alternative<value::Callable>(
                      std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(true, std::holds_alternative<value::RuntimeError>(
                      std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Exited, i1.step());
}

TEST(operator_opsequence, fallthrough) {
  using namespace networkprotocoldsl;

  operation::Int32Literal il1(10);
  operation::Int32Literal il2(20);
  operation::Eq eq;
  operation::OpSequence ops;

  auto optree =
      std::make_shared<OpTree>(OpTree({ops,
                                       {
                                           {il1, {}},
                                           {eq, {{il2, {}}, {il1, {}}}},
                                           {il2, {}},
                                       }}));

  InterpretedProgram p(optree);
  Interpreter i1 = p.get_instance();
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(10, std::get<int32_t>(std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(20, std::get<int32_t>(std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(10, std::get<int32_t>(std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(false, std::get<bool>(std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(20, std::get<int32_t>(std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Exited, i1.step());
  ASSERT_EQ(20, std::get<int32_t>(std::get<Value>(i1.get_result())));
}
