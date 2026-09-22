//===-- StackIDTest.cpp
//----------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "lldb/Target/StackID.h"
#include "gtest/gtest.h"

using namespace lldb_private;

TEST(StackIDTest, NativeFramesRetainAddressOrdering) {
  StackID callee(0x100, 0x2000, nullptr, nullptr);
  StackID caller(0x200, 0x3000, nullptr, nullptr);
  EXPECT_LT(callee, caller);
  EXPECT_NE(callee, caller);
}

TEST(StackIDTest, DepthOrdersFramesRegardlessOfStackDirection) {
  for (lldb::addr_t callee_cfa : {0x2000, 0x3000, 0x4000}) {
    StackID callee(0x100, callee_cfa, nullptr, nullptr, 2);
    StackID caller(0x100, 0x3000, nullptr, nullptr, 1);
    EXPECT_LT(callee, caller);
    EXPECT_FALSE(caller < callee);
    EXPECT_NE(callee, caller);
    EXPECT_EQ(callee.GetCallFrameAddressWithoutMetadata(), callee_cfa);
  }
}

TEST(StackIDTest, RecursiveFramesWithoutCFARemainDistinct) {
  StackID callee(0x100, LLDB_INVALID_ADDRESS, nullptr, nullptr, 2);
  StackID caller(0x100, LLDB_INVALID_ADDRESS, nullptr, nullptr, 1);
  EXPECT_NE(callee, caller);
  EXPECT_LT(callee, caller);
  EXPECT_EQ(callee.GetCallFrameAddressWithMetadata(), LLDB_INVALID_ADDRESS);
  EXPECT_EQ(callee.GetCallFrameAddressWithoutMetadata(), LLDB_INVALID_ADDRESS);
}

TEST(StackIDTest, IdentitySurvivesGuestStackAndPCChanges) {
  StackID before(0x100, 0x3000, nullptr, nullptr, 1);
  StackID after(0x108, 0x2fc0, nullptr, nullptr, 1);
  EXPECT_EQ(before, after);
  EXPECT_FALSE(before < after);
  EXPECT_FALSE(after < before);
}

TEST(StackIDTest, DepthDoesNotMatchFallbackAddressIdentity) {
  StackID remote(0x100, LLDB_INVALID_ADDRESS, nullptr, nullptr, 0);
  StackID fallback(0x100, LLDB_INVALID_ADDRESS, nullptr, nullptr);
  EXPECT_NE(remote, fallback);
}

TEST(StackIDTest, ClearRemovesDepthAndAddressMetadata) {
  StackID id(0x100, 0x3000, nullptr, nullptr, 0);
  id.Clear();
  EXPECT_FALSE(id.IsValid());
  EXPECT_FALSE(id.GetCallFrameDepth());
  EXPECT_EQ(id.GetCallFrameAddressWithMetadata(), LLDB_INVALID_ADDRESS);
}
