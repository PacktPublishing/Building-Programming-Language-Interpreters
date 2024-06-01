#include <networkprotocoldsl/interpretedprogram.hpp>
#include <networkprotocoldsl/operation/dynamiclist.hpp>
#include <networkprotocoldsl/operation/functioncall.hpp>
#include <networkprotocoldsl/operation/lexicalpadget.hpp>
#include <networkprotocoldsl/operation/lexicalpadinitializeglobal.hpp>
#include <networkprotocoldsl/operation/opsequence.hpp>
#include <networkprotocoldsl/optree.hpp>
#include <networkprotocoldsl/value.hpp>

#include <cstring>
#include <gtest/gtest.h>
#include <variant>

TEST(recursive_factorial, complete) {
  using namespace networkprotocoldsl;

  operation::Int32Literal int_1(1);
  operation::Int32Literal int_5(5);
  operation::LesserEqual le;
  operation::Subtract minus;
  operation::Multiply mult;
  operation::LexicalPadGet get_factorial("factorial");
  operation::LexicalPadGet get_i("i");
  operation::LexicalPadInitializeGlobal init_factorial("factorial");
  operation::FunctionCall func;
  operation::DynamicList dynlist;
  operation::If conditional;
  operation::OpSequence opsequence;

  auto then_optree = std::make_shared<OpTree>(OpTree({{get_i, {}}}));
  operation::StaticCallable then_block(then_optree);

  auto else_optree = std::make_shared<OpTree>(
      OpTree({{mult,
               {{get_i, {}},
                {func,
                 {{get_factorial, {}},
                  {dynlist, {{minus, {{get_i, {}}, {int_1, {}}}}}}}}}}}));
  operation::StaticCallable else_block(else_optree);

  auto factorial_optree =
      std::make_shared<OpTree>(OpTree({{conditional,
                                        {{le, {{get_i, {}}, {int_1, {}}}},
                                         {then_block, {}},
                                         {else_block, {}}}}}));
  operation::StaticCallable factorial_callable(factorial_optree, {"i"}, false);

  auto main_optree = std::make_shared<OpTree>(
      OpTree({{opsequence,
               {{init_factorial, {{factorial_callable, {}}}},
                {func, {{get_factorial, {}}, {dynlist, {{int_5, {}}}}}}}}}));

  InterpretedProgram p(main_optree);
  Interpreter i1 = p.get_instance();

  // static callable
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(true, std::holds_alternative<value::Callable>(
                      std::get<Value>(i1.get_result())));
  // initialize global, returns the callable
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(true, std::holds_alternative<value::Callable>(
                      std::get<Value>(i1.get_result())));
  // functioncall -> lexicalpadget "factorial", returns the callable
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(true, std::holds_alternative<value::Callable>(
                      std::get<Value>(i1.get_result())));
  // functioncall -> dynamiclist -> int 5, returns 5
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(5, std::get<int32_t>(std::get<Value>(i1.get_result())));
  // functioncall -> dynamiclist, returns the list
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(true, std::holds_alternative<value::DynamicList>(
                      std::get<Value>(i1.get_result())));
  // functioncall -> prepare callable, no result yet
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(false, std::get<bool>(std::get<Value>(i1.get_result())));
  // functioncall -> if -> le -> lexicalpadget "i", returns 5
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(5, std::get<int32_t>(std::get<Value>(i1.get_result())));
  // functioncall -> if -> le -> int 1, returns 1
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(1, std::get<int32_t>(std::get<Value>(i1.get_result())));
  // functioncall -> if -> le, returns false
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(false, std::get<bool>(std::get<Value>(i1.get_result())));
  // functionall -> if, return callable
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(true, std::holds_alternative<value::Callable>(
                      std::get<Value>(i1.get_result())));
  // functioncall -> if -> continuing to prepare callable
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(true, std::holds_alternative<value::Callable>(
                      std::get<Value>(i1.get_result())));
  // functioncall -> if -> continuing to prepare callable
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(false, std::get<bool>(std::get<Value>(i1.get_result())));
  // functioncall -> if -> else -> mult -> lexicalget "i", returns 5
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(5, std::get<int32_t>(std::get<Value>(i1.get_result())));
  // functioncall -> if -> else -> mult -> functioncall -> lexicalget
  // "factorial", returns callable
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(true, std::holds_alternative<value::Callable>(
                      std::get<Value>(i1.get_result())));
  // functioncall -> if -> else -> mult -> functioncall -> dynlist -> minus ->
  // lexicalget "i", returns 5
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(5, std::get<int32_t>(std::get<Value>(i1.get_result())));
  // functioncall -> if -> else -> mult -> functioncall -> dynlist -> minus ->
  // int 1, returns 1
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(1, std::get<int32_t>(std::get<Value>(i1.get_result())));
  // functioncall -> if -> else -> mult -> functioncall -> dynlist -> minus,
  // returns 4
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(4, std::get<int32_t>(std::get<Value>(i1.get_result())));
  // functioncall -> if -> else -> mult -> functioncall -> dynlist, returns
  // dynlist
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(true, std::holds_alternative<value::DynamicList>(
                      std::get<Value>(i1.get_result())));
  // functioncall -> if -> else -> mult -> functioncall -> prepare callable, no
  // result yet
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(false, std::get<bool>(std::get<Value>(i1.get_result())));

  // first recursion

  // functioncall -> if -> le -> lexicalpadget "i", returns 4
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(4, std::get<int32_t>(std::get<Value>(i1.get_result())));
  // functioncall -> if -> le -> int 1, returns 1
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(1, std::get<int32_t>(std::get<Value>(i1.get_result())));
  // functioncall -> if -> le, returns false
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(false, std::get<bool>(std::get<Value>(i1.get_result())));
  // functionall -> if, return callable
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(true, std::holds_alternative<value::Callable>(
                      std::get<Value>(i1.get_result())));
  // functioncall -> if -> continuing to prepare callable
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(true, std::holds_alternative<value::Callable>(
                      std::get<Value>(i1.get_result())));
  // functioncall -> if -> continuing to prepare callable
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(false, std::get<bool>(std::get<Value>(i1.get_result())));
  // functioncall -> if -> else -> mult -> lexicalget "i", returns 4
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(4, std::get<int32_t>(std::get<Value>(i1.get_result())));
  // functioncall -> if -> else -> mult -> functioncall -> lexicalget
  // "factorial", returns callable
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(true, std::holds_alternative<value::Callable>(
                      std::get<Value>(i1.get_result())));
  // functioncall -> if -> else -> mult -> functioncall -> dynlist -> minus ->
  // lexicalget "i", returns 4
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(4, std::get<int32_t>(std::get<Value>(i1.get_result())));
  // functioncall -> if -> else -> mult -> functioncall -> dynlist -> minus ->
  // int 1, returns 1
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(1, std::get<int32_t>(std::get<Value>(i1.get_result())));
  // functioncall -> if -> else -> mult -> functioncall -> dynlist -> minus,
  // returns 3
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(3, std::get<int32_t>(std::get<Value>(i1.get_result())));
  // functioncall -> if -> else -> mult -> functioncall -> dynlist, returns
  // dynlist
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(true, std::holds_alternative<value::DynamicList>(
                      std::get<Value>(i1.get_result())));
  // functioncall -> if -> else -> mult -> functioncall -> prepare callable, no
  // result yet
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(false, std::get<bool>(std::get<Value>(i1.get_result())));

  // second recursion

  // functioncall -> if -> le -> lexicalpadget "i", returns 3
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(3, std::get<int32_t>(std::get<Value>(i1.get_result())));
  // functioncall -> if -> le -> int 1, returns 1
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(1, std::get<int32_t>(std::get<Value>(i1.get_result())));
  // functioncall -> if -> le, returns false
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(false, std::get<bool>(std::get<Value>(i1.get_result())));
  // functionall -> if, return callable
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(true, std::holds_alternative<value::Callable>(
                      std::get<Value>(i1.get_result())));
  // functioncall -> if -> continuing to prepare callable
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(true, std::holds_alternative<value::Callable>(
                      std::get<Value>(i1.get_result())));
  // functioncall -> if -> continuing to prepare callable
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(false, std::get<bool>(std::get<Value>(i1.get_result())));
  // functioncall -> if -> else -> mult -> lexicalget "i", returns 3
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(3, std::get<int32_t>(std::get<Value>(i1.get_result())));
  // functioncall -> if -> else -> mult -> functioncall -> lexicalget
  // "factorial", returns callable
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(true, std::holds_alternative<value::Callable>(
                      std::get<Value>(i1.get_result())));
  // functioncall -> if -> else -> mult -> functioncall -> dynlist -> minus ->
  // lexicalget "i", returns 3
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(3, std::get<int32_t>(std::get<Value>(i1.get_result())));
  // functioncall -> if -> else -> mult -> functioncall -> dynlist -> minus ->
  // int 1, returns 1
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(1, std::get<int32_t>(std::get<Value>(i1.get_result())));
  // functioncall -> if -> else -> mult -> functioncall -> dynlist -> minus,
  // returns 2
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(2, std::get<int32_t>(std::get<Value>(i1.get_result())));
  // functioncall -> if -> else -> mult -> functioncall -> dynlist, returns
  // dynlist
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(true, std::holds_alternative<value::DynamicList>(
                      std::get<Value>(i1.get_result())));
  // functioncall -> if -> else -> mult -> functioncall -> prepare callable, no
  // result yet
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(false, std::get<bool>(std::get<Value>(i1.get_result())));

  // third recursion

  // functioncall -> if -> le -> lexicalpadget "i", returns 2
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(2, std::get<int32_t>(std::get<Value>(i1.get_result())));
  // functioncall -> if -> le -> int 1, returns 1
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(1, std::get<int32_t>(std::get<Value>(i1.get_result())));
  // functioncall -> if -> le, returns false
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(false, std::get<bool>(std::get<Value>(i1.get_result())));
  // functionall -> if, return callable
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(true, std::holds_alternative<value::Callable>(
                      std::get<Value>(i1.get_result())));
  // functioncall -> if -> continuing to prepare callable
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(true, std::holds_alternative<value::Callable>(
                      std::get<Value>(i1.get_result())));
  // functioncall -> if -> continuing to prepare callable
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(false, std::get<bool>(std::get<Value>(i1.get_result())));
  // functioncall -> if -> else -> mult -> lexicalget "i", returns 2
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(2, std::get<int32_t>(std::get<Value>(i1.get_result())));
  // functioncall -> if -> else -> mult -> functioncall -> lexicalget
  // "factorial", returns callable
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(true, std::holds_alternative<value::Callable>(
                      std::get<Value>(i1.get_result())));
  // functioncall -> if -> else -> mult -> functioncall -> dynlist -> minus ->
  // lexicalget "i", returns 2
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(2, std::get<int32_t>(std::get<Value>(i1.get_result())));
  // functioncall -> if -> else -> mult -> functioncall -> dynlist -> minus ->
  // int 1, returns 1
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(1, std::get<int32_t>(std::get<Value>(i1.get_result())));
  // functioncall -> if -> else -> mult -> functioncall -> dynlist -> minus,
  // returns 1
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(1, std::get<int32_t>(std::get<Value>(i1.get_result())));
  // functioncall -> if -> else -> mult -> functioncall -> dynlist, returns
  // dynlist
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(true, std::holds_alternative<value::DynamicList>(
                      std::get<Value>(i1.get_result())));
  // functioncall -> if -> else -> mult -> functioncall -> prepare callable, no
  // result yet
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(false, std::get<bool>(std::get<Value>(i1.get_result())));

  // fourth recursion

  // functioncall -> if -> le -> lexicalpadget "i", returns 1
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(1, std::get<int32_t>(std::get<Value>(i1.get_result())));
  // functioncall -> if -> le -> int 1, returns 1
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(1, std::get<int32_t>(std::get<Value>(i1.get_result())));
  // functioncall -> if -> le, returns true
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(true, std::get<bool>(std::get<Value>(i1.get_result())));
  // functionall -> if, return callable
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(true, std::holds_alternative<value::Callable>(
                      std::get<Value>(i1.get_result())));
  // functioncall -> if -> continuing to prepare callable
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(true, std::holds_alternative<value::Callable>(
                      std::get<Value>(i1.get_result())));
  // functioncall -> if -> continuing to prepare callable
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(false, std::get<bool>(std::get<Value>(i1.get_result())));
  // functioncall -> if -> then -> lexicalget "i", returns 1
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(1, std::get<int32_t>(std::get<Value>(i1.get_result())));

  // unwinding recursion now
  // returns 1
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(1, std::get<int32_t>(std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(1, std::get<int32_t>(std::get<Value>(i1.get_result())));
  // multiply by 2
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(2, std::get<int32_t>(std::get<Value>(i1.get_result())));
  // returns 2
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(2, std::get<int32_t>(std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(2, std::get<int32_t>(std::get<Value>(i1.get_result())));
  // multiply by 3
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(6, std::get<int32_t>(std::get<Value>(i1.get_result())));
  // returns 6
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(6, std::get<int32_t>(std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(6, std::get<int32_t>(std::get<Value>(i1.get_result())));
  // multiply by 4
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(24, std::get<int32_t>(std::get<Value>(i1.get_result())));
  // returns 24
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(24, std::get<int32_t>(std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(24, std::get<int32_t>(std::get<Value>(i1.get_result())));
  // multiply by 5
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(120, std::get<int32_t>(std::get<Value>(i1.get_result())));
  // returns 120
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(120, std::get<int32_t>(std::get<Value>(i1.get_result())));
  ASSERT_EQ(ContinuationState::Ready, i1.step());
  ASSERT_EQ(120, std::get<int32_t>(std::get<Value>(i1.get_result())));

  // final
  ASSERT_EQ(ContinuationState::Exited, i1.step());
  ASSERT_EQ(120, std::get<int32_t>(std::get<Value>(i1.get_result())));
}
