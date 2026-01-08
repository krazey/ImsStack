/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef __SIP_REF_BASE_H__
#define __SIP_REF_BASE_H__

#include "SipDatatypes.h"

/**
  This is a base class for all classes that intend to implement reference counting.
  Increment() function should be used when either reference is getting stored
  and Decrement() function should be used when either reference is permanently deleted.
  SipDelete() should always be used to delete the derived class objects. This ensure that
  objects which have reference in other modules shall not be deleted.
 */

class SipRefBase
{
protected:
    SIP_INT16 m_nRefCount;

public:
    SipRefBase();
    SIP_VOID Increment();
    SIP_VOID Decrement();
    virtual SIP_VOID SipDelete();

protected:
    virtual ~SipRefBase();

private:
    SipRefBase& operator=(IN const SipRefBase& objRHS);
    SipRefBase(IN const SipRefBase& objRHS);
};

#endif  //__SIP_REF_BASE_H__
