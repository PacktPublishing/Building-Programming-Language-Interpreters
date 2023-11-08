#include <networkprotocoldsl/interpretedprogram.hpp>
#include <networkprotocoldsl/optree.hpp>

#include <cstring>
#include <gtest/gtest.h>

TEST(operator_eq, is_equal) {
  using namespace networkprotocoldsl;

  operation::Int32Literal il1(10);
  operation::Int32Literal il2(20);
  operation::Eq eq;

  auto optree = std::make_shared<OpTree>(OpTree({eq, {{il1, {}}, {il2, {}}}}));

  InterpretedProgram p(optree);
  Interpreter i1 = p.get_instance();
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(10, std::get<int32_t>(std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(20, std::get<int32_t>(std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Exited, i1.step());
  ASSERT_EQ(false, std::get<bool>(std::get<Value>(i1.get_result())));
}

TEST(operator_eq, is_different) {
  using namespace networkprotocoldsl;

  operation::Int32Literal il1(10);
  operation::Int32Literal il2(10);
  operation::Eq eq;

  auto optree = std::make_shared<OpTree>(OpTree({eq, {{il1, {}}, {il2, {}}}}));

  InterpretedProgram p(optree);
  Interpreter i1 = p.get_instance();
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(10, std::get<int32_t>(std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(10, std::get<int32_t>(std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Exited, i1.step());
  ASSERT_EQ(true, std::get<bool>(std::get<Value>(i1.get_result())));
}
