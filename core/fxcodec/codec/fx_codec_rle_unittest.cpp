// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>

#include <limits>
#include <memory>

#include "core/fpdfapi/parser/fpdf_parser_decode.h"
#include "core/fxcodec/codec/ccodec_basicmodule.h"
#include "core/fxcodec/fx_codec.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(fxcodec, RLETestBadInputs) {
  const uint8_t src_buf[] = {1};
  uint8_t* dest_buf = nullptr;
  uint32_t dest_size = 0;

  CCodec_BasicModule* pEncoders = CCodec_ModuleMgr().GetBasicModule();
  EXPECT_TRUE(pEncoders);

  // Error codes, not segvs, should callers pass us a nullptr pointer.
  EXPECT_FALSE(pEncoders->RunLengthEncode(src_buf, &dest_buf, nullptr));
  EXPECT_FALSE(pEncoders->RunLengthEncode(src_buf, nullptr, &dest_size));
  EXPECT_FALSE(pEncoders->RunLengthEncode({}, &dest_buf, &dest_size));
}

// Check length 1 input works. Check terminating character is applied.
TEST(fxcodec, RLETestShortInput) {
  const uint8_t src_buf[] = {1};
  uint8_t* dest_buf = nullptr;
  uint32_t dest_size = 0;

  CCodec_BasicModule* pEncoders = CCodec_ModuleMgr().GetBasicModule();
  EXPECT_TRUE(pEncoders);

  EXPECT_TRUE(pEncoders->RunLengthEncode(src_buf, &dest_buf, &dest_size));
  ASSERT_EQ(3u, dest_size);
  EXPECT_EQ(0, dest_buf[0]);
  EXPECT_EQ(1, dest_buf[1]);
  EXPECT_EQ(128, dest_buf[2]);

  FX_Free(dest_buf);
}

// Check a few basic cases (2 matching runs in a row, matching run followed
// by a non-matching run, and non-matching run followed by a matching run).
TEST(fxcodec, RLETestNormalInputs) {
  // Match, match
  const uint8_t src_buf_1[] = {2, 2, 2, 2, 4, 4, 4, 4, 4, 4};

  // Match, non-match
  const uint8_t src_buf_2[] = {2, 2, 2, 2, 1, 2, 3, 4, 5, 6};

  // Non-match, match
  const uint8_t src_buf_3[] = {1, 2, 3, 4, 5, 3, 3, 3, 3, 3};

  uint32_t dest_size = 0;
  uint8_t* dest_buf = nullptr;

  CCodec_BasicModule* pEncoders = CCodec_ModuleMgr().GetBasicModule();
  EXPECT_TRUE(pEncoders);

  // Case 1:
  EXPECT_TRUE(pEncoders->RunLengthEncode(src_buf_1, &dest_buf, &dest_size));
  std::unique_ptr<uint8_t, FxFreeDeleter> decoded_buf;
  uint32_t decoded_size = 0;
  RunLengthDecode({dest_buf, dest_size}, &decoded_buf, &decoded_size);
  ASSERT_EQ(sizeof(src_buf_1), decoded_size);
  for (uint32_t i = 0; i < decoded_size; i++)
    EXPECT_EQ(src_buf_1[i], decoded_buf.get()[i]) << " at " << i;
  FX_Free(dest_buf);

  // Case 2:
  dest_buf = nullptr;
  dest_size = 0;
  EXPECT_TRUE(pEncoders->RunLengthEncode(src_buf_2, &dest_buf, &dest_size));
  decoded_buf.reset();
  decoded_size = 0;
  RunLengthDecode({dest_buf, dest_size}, &decoded_buf, &decoded_size);
  ASSERT_EQ(sizeof(src_buf_2), decoded_size);
  for (uint32_t i = 0; i < decoded_size; i++)
    EXPECT_EQ(src_buf_2[i], decoded_buf.get()[i]) << " at " << i;
  FX_Free(dest_buf);

  // Case 3:
  dest_buf = nullptr;
  dest_size = 0;
  EXPECT_TRUE(pEncoders->RunLengthEncode(src_buf_3, &dest_buf, &dest_size));
  decoded_buf.reset();
  decoded_size = 0;
  RunLengthDecode({dest_buf, dest_size}, &decoded_buf, &decoded_size);
  ASSERT_EQ(sizeof(src_buf_3), decoded_size);
  for (uint32_t i = 0; i < decoded_size; i++)
    EXPECT_EQ(src_buf_3[i], decoded_buf.get()[i]) << " at " << i;
  FX_Free(dest_buf);
}

// Check that runs longer than 128 are broken up properly, both matched and
// non-matched.
TEST(fxcodec, RLETestFullLengthInputs) {
  // Match, match
  const uint8_t src_buf_1[260] = {1};

  // Match, non-match
  uint8_t src_buf_2[260] = {2};
  for (uint16_t i = 128; i < 260; i++)
    src_buf_2[i] = (uint8_t)(i - 125);

  // Non-match, match
  uint8_t src_buf_3[260] = {3};
  for (uint8_t i = 0; i < 128; i++)
    src_buf_3[i] = i;

  // Non-match, non-match
  uint8_t src_buf_4[260];
  for (uint16_t i = 0; i < 260; i++)
    src_buf_4[i] = (uint8_t)(i);

  uint32_t dest_size = 0;
  uint8_t* dest_buf = nullptr;

  CCodec_BasicModule* pEncoders = CCodec_ModuleMgr().GetBasicModule();
  EXPECT_TRUE(pEncoders);

  // Case 1:
  EXPECT_TRUE(pEncoders->RunLengthEncode(src_buf_1, &dest_buf, &dest_size));
  std::unique_ptr<uint8_t, FxFreeDeleter> decoded_buf;
  uint32_t decoded_size = 0;
  RunLengthDecode({dest_buf, dest_size}, &decoded_buf, &decoded_size);
  ASSERT_EQ(sizeof(src_buf_1), decoded_size);
  for (uint32_t i = 0; i < decoded_size; i++)
    EXPECT_EQ(src_buf_1[i], decoded_buf.get()[i]) << " at " << i;
  FX_Free(dest_buf);

  // Case 2:
  dest_buf = nullptr;
  dest_size = 0;
  EXPECT_TRUE(pEncoders->RunLengthEncode(src_buf_2, &dest_buf, &dest_size));
  decoded_buf.reset();
  decoded_size = 0;
  RunLengthDecode({dest_buf, dest_size}, &decoded_buf, &decoded_size);
  ASSERT_EQ(sizeof(src_buf_2), decoded_size);
  for (uint32_t i = 0; i < decoded_size; i++)
    EXPECT_EQ(src_buf_2[i], decoded_buf.get()[i]) << " at " << i;
  FX_Free(dest_buf);

  // Case 3:
  dest_buf = nullptr;
  dest_size = 0;
  EXPECT_TRUE(pEncoders->RunLengthEncode(src_buf_3, &dest_buf, &dest_size));
  decoded_buf.reset();
  decoded_size = 0;
  RunLengthDecode({dest_buf, dest_size}, &decoded_buf, &decoded_size);
  ASSERT_EQ(sizeof(src_buf_3), decoded_size);
  for (uint32_t i = 0; i < decoded_size; i++)
    EXPECT_EQ(src_buf_3[i], decoded_buf.get()[i]) << " at " << i;
  FX_Free(dest_buf);

  // Case 4:
  dest_buf = nullptr;
  dest_size = 0;
  EXPECT_TRUE(pEncoders->RunLengthEncode(src_buf_4, &dest_buf, &dest_size));
  decoded_buf.reset();
  decoded_size = 0;
  RunLengthDecode({dest_buf, dest_size}, &decoded_buf, &decoded_size);
  ASSERT_EQ(sizeof(src_buf_4), decoded_size);
  for (uint32_t i = 0; i < decoded_size; i++)
    EXPECT_EQ(src_buf_4[i], decoded_buf.get()[i]) << " at " << i;
  FX_Free(dest_buf);
}
