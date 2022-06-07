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
#ifndef SIP_DIALOG_EX_H_
#define SIP_DIALOG_EX_H_

#include "SipDialogBase.h"

class SipDialogUsage;

/**
 * @brief This class defines an extended SIP dialog.
 *
 * It has a dialog usage to support a multiple dialog usage in SIP.
 */
class SipDialogEx : public SipDialogBase
{
public:
    SipDialogEx() = delete;
    explicit SipDialogEx(IN SipDialogState* pDState);
    SipDialogEx(IN const SipDialogEx& other);
    virtual ~SipDialogEx();

public:
    SipDialogEx& operator=(IN const SipDialogEx& other);

public:
    // For an initial requests
    IMS_BOOL InitDialog(IN const SipMethod& objMethod);
    // For a dialog request or incoming requests
    IMS_BOOL InitDialog(IN const SipMessageInfo& objMsgInfo);
    IMS_BOOL InitDialogWithDelay(IN const SipMessageInfo& objMsgInfo);
    IMS_BOOL CompareTo(IN const SipMessageInfo& objMsgInfo) const;
    IMS_BOOL Equals(IN SipDialogEx* pDialogEx) const;
    IMS_BOOL IsInviteUsage() const;
    inline IMS_BOOL IsDialogTerminated() const { return m_bIsDialogTerminated; }
    void TerminateDialogUsage();
    IMS_SINT32 UpdateDialogDetails(IN const SipMessageInfo& objMsgInfo);

    static SipDialogEx* CreateDialog(IN const SipMethod& objMethod);
    static SipDialogEx* CreateDialog(IN SipDialogState* pDState, IN const SipMethod& objMethod);
    static SipDialogEx* CreateDialog(
            IN SipDialogState* pDState, IN const SipMessageInfo& objMsgInfo);

protected:
    // SipDialogBase class
    IMS_BOOL OnInit() override;
    void OnTerminated() override;
    IMS_SINT32 OnUpdateDialogDetails(IN const SipMessageInfo& objMsgInfo, IN IMS_SINT32 nUsage,
            IN IMS_SINT32 nAction, IN IMS_SINT32 nTrigger) override;

private:
    IMS_BOOL m_bIsPermanentDialog;
    IMS_BOOL m_bInitWithDelay;
    IMS_BOOL m_bIsDialogTerminated;
    SipDialogUsage* m_pDialogUsage;
};

#endif
