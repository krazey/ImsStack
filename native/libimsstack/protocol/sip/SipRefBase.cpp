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
#include "SipRefBase.h"

/******************************************************************************
 * Function name      : SipRefBase::SipRefBase
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipRefBase::SipRefBase()
{
    m_nRefCount = SIP_ONE;
}

/******************************************************************************
 * Function name      : SipRefBase::~SipRefBase
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipRefBase::~SipRefBase()
{}

/******************************************************************************
 * Function name      : SipRefBase::increment
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
/**
  Increment the total references to the object
 */
SIP_VOID SipRefBase::increment()
{
    m_nRefCount++;
}

/******************************************************************************
 * Function name      : SipRefBase::decrement
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
/**
  Decrement the total references to the object
 */
SIP_VOID SipRefBase::decrement()
{
    m_nRefCount--;
}

/******************************************************************************
 * Function name      : SipRefBase::SipDelete
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
/**
  Delete the object only if there are no other references to it
 */
SIP_VOID SipRefBase::SipDelete()
{
    m_nRefCount--;
    if (m_nRefCount == SIP_ZERO)
    {
        delete this;
    }
}
