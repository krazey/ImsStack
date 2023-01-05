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
#include "RcObject.h"

PUBLIC
RcObject::RcObject() :
        m_nRefCount(0),
        m_bShareable(IMS_TRUE)
{
}

PUBLIC
RcObject::RcObject(IN const RcObject& /*other*/) :
        m_nRefCount(0),
        m_bShareable(IMS_TRUE)
{
}

PUBLIC VIRTUAL RcObject::~RcObject() {}

PUBLIC
// The member variables will be controlled by the RcPtr object.
// cppcheck-suppress operatorEqVarError
RcObject& RcObject::operator=(IN const RcObject& /*other*/)
{
    return (*this);
}

PUBLIC
void RcObject::AddReference()
{
    ++m_nRefCount;
}

PUBLIC
void RcObject::RemoveReference()
{
    if (--m_nRefCount == 0)
    {
        delete this;
    }
}

PUBLIC
void RcObject::MarkUnshareable()
{
    m_bShareable = IMS_FALSE;
}

PUBLIC
IMS_BOOL RcObject::IsShareable() const
{
    return m_bShareable;
}

PUBLIC
IMS_BOOL RcObject::IsShared() const
{
    return m_nRefCount > 1;
}
