// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/cpdfsdk_fieldaction.h"

CPDFSDK_FieldAction::CPDFSDK_FieldAction()
    : bModifier(false),
      bShift(false),
      bKeyDown(false),
      nSelEnd(0),
      nSelStart(0),
      bWillCommit(false),
      bFieldFull(false),
      bRC(true) {}
