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
#ifndef SIP_MANAGER_H_
#define SIP_MANAGER_H_

#include "SipConnectionNotifier.h"
#include "SipDialogState.h"

class SipManager
{
private:
    SipManager();

public:
    ~SipManager();

    SipManager(IN const SipManager&) = delete;
    SipManager& operator=(IN const SipManager&) = delete;

public:
    IMS_BOOL AttachDialogState(IN SipDialogState* pDState);
    void DetachDialogState(IN SipDialogState* pDState);
    RcPtr<SipDialogState> LookupDialogState(IN SipDialogState* pDState, IN ::SipMessage* pSipMsg,
            IN IMS_BOOL bCheckForked = IMS_FALSE, OUT IMS_BOOL* pbIsForked = IMS_NULL);

    IMS_BOOL AttachConnectionNotifier(IN SipConnectionNotifier* pScn);
    void DetachConnectionNotifier(IN SipConnectionNotifier* pScn);
    SipConnectionNotifier* LookupConnectionNotifier(IN const SipTransportAddress& objTAddr,
            IN const AString& strFilter = AString::ConstNull());

    static SipManager* GetInstance();

private:
    IMS_BOOL StartUp();
    void CleanUp();

private:
    friend class StaticSip;

    enum
    {
        STATE_INACTIVE,
        STATE_ACTIVE,
        STATE_PENDING
    };

    IMS_SINT32 m_nState;
    IMSList<SipDialogState*> m_objDialogStates;
    IMSList<SipConnectionNotifier*> m_objScns;
};

#endif
