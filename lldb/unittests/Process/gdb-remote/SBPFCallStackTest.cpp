//===-- SBPFCallStackTest.cpp ---------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "Plugins/Process/gdb-remote/SBPFCallStack.h"
#include "gtest/gtest.h"

using namespace lldb_private::process_gdb_remote;

TEST(SBPFCallStackTest, ParsesCurrentAndCallerFrames) {
  auto frames = ParseSBPFCallStack("20010000000000000020000002000000"
                                   "68010000000000000010000002000000"
                                   "80010000000000000000000002000000");
  ASSERT_TRUE(static_cast<bool>(frames));
  ASSERT_EQ(frames->size(), 3u);
  EXPECT_EQ((*frames)[0].pc, 0x120u);
  EXPECT_EQ((*frames)[0].frame_pointer, 0x200002000u);
  EXPECT_EQ((*frames)[1].pc, 0x168u);
  EXPECT_EQ((*frames)[1].frame_pointer, 0x200001000u);
  EXPECT_EQ((*frames)[2].pc, 0x180u);
  EXPECT_EQ((*frames)[2].frame_pointer, 0x200000000u);
}

TEST(SBPFCallStackTest, RejectsInvalidHex) {
  auto frames = ParseSBPFCallStack("not-hex");
  ASSERT_FALSE(static_cast<bool>(frames));
  llvm::consumeError(frames.takeError());
}

TEST(SBPFCallStackTest, RejectsIncompleteFrame) {
  auto frames = ParseSBPFCallStack("2001000000000000");
  ASSERT_FALSE(static_cast<bool>(frames));
  llvm::consumeError(frames.takeError());
}

TEST(SBPFCallStackTest, RejectsOddHexLength) {
  auto frames = ParseSBPFCallStack("00112233445566778899aabbccddeef");
  ASSERT_FALSE(static_cast<bool>(frames));
  llvm::consumeError(frames.takeError());
}

TEST(SBPFCallStackTest, RejectsEmptyStack) {
  auto frames = ParseSBPFCallStack("");
  ASSERT_FALSE(static_cast<bool>(frames));
  llvm::consumeError(frames.takeError());
}
