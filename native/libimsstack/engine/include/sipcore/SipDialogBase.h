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
#ifndef SIP_DIALOG_BASE_H_
#define SIP_DIALOG_BASE_H_

#include "SipDialogState.h"

/**
 * @brief This class is a base class for SIP dialog.
 */
class SipDialogBase : public RCObject
{
public:
    SipDialogBase() = delete;
    inline explicit SipDialogBase(IN SipDialogState* pDState) :
            RCObject(),
            m_pDState(pDState)
    {
    }
    inline SipDialogBase(IN const SipDialogBase& other) :
            RCObject(other),
            m_pDState(other.m_pDState)
    {
    }
    inline virtual ~SipDialogBase() {}

public:
    inline SipDialogBase& operator=(IN const SipDialogBase& other)
    {
        RCObject::operator=(other);
        return (*this);
    }

public:
    virtual IMS_BOOL OnInit() = 0;
    virtual void OnTerminated() = 0;
    virtual IMS_SINT32 OnUpdateDialogDetails(IN const SipMessageInfo& objMsgInfo,
            IN IMS_SINT32 nUsage, IN IMS_SINT32 nAction, IN IMS_SINT32 nTrigger) = 0;

    inline SipDialogState* GetDialogState() const { return m_pDState.Get(); }
    inline IMS_SINT32 GetState() const { return m_pDState->GetState(); }

    static IMS_BOOL IsDialogCreatable(IN const SipMethod& objMethod);

private:
    // Shared dialog information among all dialog usages
    RCPtr<SipDialogState> m_pDState;
};

#endif
