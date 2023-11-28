#include <networkprotocoldsl/interpretedprogram.hpp>
#include <networkprotocoldsl/optree.hpp>

#include <cstring>
#include <gtest/gtest.h>

TEST(operator_lexicalpad, get_nameerror) {
  using namespace networkprotocoldsl;

  operation::LexicalPadGet g1("unknown");

  auto optree = std::make_shared<OpTree>(OpTree({g1, {}}));

  InterpretedProgram p(optree);
  Interpreter i1 = p.get_instance();
  ASSERT_EQ(ContinuationState::Exited, i1.step());
  ASSERT_EQ(true, std::holds_alternative<value::RuntimeError>(
                      std::get<Value>(i1.get_result())));
  ASSERT_EQ(value::RuntimeError::NameError,
            std::get<value::RuntimeError>(std::get<Value>(i1.get_result())));
}

TEST(operator_lexicalpad, set_nameerror) {
  using namespace networkprotocoldsl;

  operation::LexicalPadSet s1("unknown");
  operation::Int32Literal il1(10);

  auto optree = std::make_shared<OpTree>(OpTree({s1, {{il1, {}}}}));

  InterpretedProgram p(optree);
  Interpreter i1 = p.get_instance();
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(10, std::get<int32_t>(std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Exited, i1.step());
  ASSERT_EQ(true, std::holds_alternative<value::RuntimeError>(
                      std::get<Value>(i1.get_result())));
  ASSERT_EQ(value::RuntimeError::NameError,
            std::get<value::RuntimeError>(std::get<Value>(i1.get_result())));
}

TEST(operator_lexicalpad, initialize_get_and_set) {
  using namespace networkprotocoldsl;

  operation::LexicalPadInitialize init("a");
  operation::LexicalPadGet g1("a");
  operation::LexicalPadSet s1("a");
  operation::Int32Literal il1(10);
  operation::Int32Literal il2(20);
  operation::OpSequence ops;

  auto optree = std::make_shared<OpTree>(OpTree({ops,
                                                 {
                                                     {init, {{il1, {}}}},
                                                     {g1, {}},
                                                     {s1, {{il2, {}}}},
                                                     {g1, {}},
                                                 }}));

  InterpretedProgram p(optree);
  Interpreter i1 = p.get_instance();
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(10, std::get<int32_t>(std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(10, std::get<int32_t>(std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(10, std::get<int32_t>(std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(20, std::get<int32_t>(std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(10, std::get<int32_t>(std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(20, std::get<int32_t>(std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Exited, i1.step());
  ASSERT_EQ(20, std::get<int32_t>(std::get<Value>(i1.get_result())));
}

TEST(operator_lexicalpad, initialize_global) {
  using namespace networkprotocoldsl;

  operation::LexicalPadInitializeGlobal init("a");
  operation::LexicalPadGet g1("a");
  operation::LexicalPadSet s1("a");
  operation::Int32Literal il1(10);
  operation::Int32Literal il2(20);
  operation::OpSequence ops;

  auto optree = std::make_shared<OpTree>(OpTree({ops,
                                                 {
                                                     {init, {{il1, {}}}},
                                                     {g1, {}},
                                                     {s1, {{il2, {}}}},
                                                     {g1, {}},
                                                 }}));

  InterpretedProgram p(optree);
  Interpreter i1 = p.get_instance();
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(10, std::get<int32_t>(std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(10, std::get<int32_t>(std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(10, std::get<int32_t>(std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(20, std::get<int32_t>(std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(10, std::get<int32_t>(std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(20, std::get<int32_t>(std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Exited, i1.step());
  ASSERT_EQ(20, std::get<int32_t>(std::get<Value>(i1.get_result())));
}
