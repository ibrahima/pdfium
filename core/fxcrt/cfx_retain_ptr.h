// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXCRT_CFX_RETAIN_PTR_H_
#define CORE_FXCRT_CFX_RETAIN_PTR_H_

#include <memory>
#include <utility>

#include "core/fxcrt/fx_memory.h"

template <class T>
class CFX_RetainPtr {
 public:
  explicit CFX_RetainPtr(T* pObj) : m_pObj(pObj) {
    if (m_pObj)
      m_pObj->Retain();
  }

  CFX_RetainPtr() {}
  CFX_RetainPtr(const CFX_RetainPtr& that) : CFX_RetainPtr(that.Get()) {}
  CFX_RetainPtr(CFX_RetainPtr&& that) { Swap(that); }

  // Deliberately implicit to allow returning nullptrs.
  CFX_RetainPtr(std::nullptr_t ptr) {}

  template <class U>
  CFX_RetainPtr(const CFX_RetainPtr<U>& that) : CFX_RetainPtr(that.Get()) {}

  void Reset(T* obj = nullptr) {
    if (obj)
      obj->Retain();
    m_pObj.reset(obj);
  }

  T* Get() const { return m_pObj.get(); }
  void Swap(CFX_RetainPtr& that) { m_pObj.swap(that.m_pObj); }

  CFX_RetainPtr& operator=(const CFX_RetainPtr& that) {
    if (*this != that)
      Reset(that.Get());
    return *this;
  }

  bool operator==(const CFX_RetainPtr& that) const {
    return Get() == that.Get();
  }

  bool operator!=(const CFX_RetainPtr& that) const { return !(*this == that); }

  explicit operator bool() const { return !!m_pObj; }
  T& operator*() const { return *m_pObj.get(); }
  T* operator->() const { return m_pObj.get(); }

 private:
  std::unique_ptr<T, ReleaseDeleter<T>> m_pObj;
};

// Trivial implementation - internal ref count with virtual destructor.
class CFX_Retainable {
 public:
  bool HasOneRef() const { return m_nRefCount == 1; }

 protected:
  virtual ~CFX_Retainable() {}

 private:
  template <typename U>
  friend struct ReleaseDeleter;

  template <typename U>
  friend class CFX_RetainPtr;

  void Retain() { ++m_nRefCount; }
  void Release() {
    if (--m_nRefCount == 0)
      delete this;
  }

  intptr_t m_nRefCount = 0;
};

namespace pdfium {

// Helper to make a CFX_RetainPtr along the lines of std::make_unique<>(),
// or pdfium::MakeUnique<>(). Arguments are forwarded to T's constructor.
// Classes managed by CFX_RetainPtr should have protected (or private)
// constructors, and should friend this function.
template <typename T, typename... Args>
CFX_RetainPtr<T> MakeRetain(Args&&... args) {
  return CFX_RetainPtr<T>(new T(std::forward<Args>(args)...));
}

}  // namespace pdfium

#endif  // CORE_FXCRT_CFX_RETAIN_PTR_H_
