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
#ifndef SIP_DIALOG_SHARED_STATE_H_
#define SIP_DIALOG_SHARED_STATE_H_

#include "ImsList.h"

#include "SipMessageInfo.h"

class SipDialogEx;

/**
 * @brief This class defines a shared dialog state.
 *
 * SipDialogState class MUST have this class as its member.
 */
class SipDialogSharedState
{
public:
    inline SipDialogSharedState() :
            m_nSharedState(SHARED_STATE_INIT)
    {
    }
    ~SipDialogSharedState();

    SipDialogSharedState(IN const SipDialogSharedState&) = delete;
    SipDialogSharedState& operator=(IN const SipDialogSharedState&) = delete;

private:
    IMS_BOOL AddDialog(IN SipDialogEx* pDialogEx);
    void RemoveDialog(IN SipDialogEx* pDialogEx);
    SipDialogEx* GetDialog(IN const SipMessageInfo& objMsgInfo);
    IMS_BOOL HasMultipleDialogUsages() const;
    inline IMS_BOOL IsShared() const { return (m_nSharedState == SHARED_STATE_ACTIVE); }

private:
    friend class SipDialogState;

    enum
    {
        SHARED_STATE_INIT = 0,
        SHARED_STATE_ACTIVE,
        SHARED_STATE_TERMINATED
    };

    IMS_SINT32 m_nSharedState;
    // References of dialog state
    IMSList<SipDialogEx*> m_objDialogExs;
};

#endif
