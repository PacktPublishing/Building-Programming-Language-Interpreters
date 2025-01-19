#include <gtest/gtest.h>
#include <networkprotocoldsl/operation/transitionlookahead.hpp>
#include <networkprotocoldsl/operationconcepts.hpp>
#include <networkprotocoldsl/value.hpp>

using namespace networkprotocoldsl;
using namespace networkprotocoldsl::operation;

TEST(TransitionLookaheadTest, MatchEOFCondition) {
  TransitionLookahead lookahead{
      {{TransitionLookahead::EOFCondition{}, "EOFState"}}};

  InputOutputOperationContext ctx;
  ctx.eof = true;

  auto result = lookahead(ctx, {});
  ASSERT_TRUE(std::holds_alternative<Value>(result));
  auto v = std::get<Value>(result);
  ASSERT_TRUE(std::holds_alternative<value::Octets>(v));
  auto octets = std::get<value::Octets>(v);
  EXPECT_EQ(*(octets.data), "EOFState");
}

TEST(TransitionLookaheadTest, MatchUntilTerminatorCondition) {
  TransitionLookahead lookahead{
      {{TransitionLookahead::MatchUntilTerminator{"\r\n"}, "TerminatorState"}}};

  InputOutputOperationContext ctx;
  ctx.buffer = "Hello\r\nWorld";

  auto result = lookahead(ctx, {});
  ASSERT_TRUE(std::holds_alternative<Value>(result));
  auto &v = std::get<Value>(result);
  ASSERT_TRUE(std::holds_alternative<value::Octets>(v));
  EXPECT_EQ(*std::get<value::Octets>(v).data, "TerminatorState");
}

TEST(TransitionLookaheadTest, MatchStaticStringCondition) {
  TransitionLookahead lookahead{
      {std::make_pair<TransitionLookahead::TransitionCondition, std::string>(
          "Hello", "StaticStringState")}};

  InputOutputOperationContext ctx;
  ctx.buffer = "HelloWorld";
  auto result = lookahead(ctx, {});
  ASSERT_TRUE(std::holds_alternative<Value>(result));
  auto v = std::get<Value>(result);
  ASSERT_TRUE(std::holds_alternative<value::Octets>(v));
  auto octets = std::get<value::Octets>(v);
  EXPECT_EQ(*(octets.data), "StaticStringState");
}

TEST(TransitionLookaheadTest, NoMatchCondition) {
  TransitionLookahead lookahead{
      {{TransitionLookahead::MatchUntilTerminator{"\r\n"}, "TerminatorState"},
       std::make_pair<TransitionLookahead::TransitionCondition, std::string>(
           "Hello", "StaticStringState")}};

  InputOutputOperationContext ctx;
  ctx.eof = false;
  ctx.buffer = "Goodbye";

  auto result = lookahead(ctx, {});
  ASSERT_TRUE(std::holds_alternative<ReasonForBlockedOperation>(result));
  EXPECT_EQ(std::get<ReasonForBlockedOperation>(result),
            ReasonForBlockedOperation::WaitingForRead);
}

TEST(TransitionLookaheadTest, ProtocolMismatchError) {
  TransitionLookahead lookahead{
      {std::make_pair<TransitionLookahead::TransitionCondition, std::string>(
          "Hello", "StaticStringState")}};

  InputOutputOperationContext ctx;
  ctx.buffer = "Goodbye";

  auto result = lookahead(ctx, {});
  ASSERT_TRUE(std::holds_alternative<Value>(result));
  EXPECT_EQ(std::get<value::RuntimeError>(std::get<Value>(result)),
            value::RuntimeError::ProtocolMismatchError);
}
